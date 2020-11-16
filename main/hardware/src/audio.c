#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/rtc_io.h>
#include <driver/dac.h>

#include <stdio.h>

#include "Native_CPC.h"

#include "Sound.h"
#include "audio.h"

#define AUDIO_IO_NEGATIVE GPIO_NUM_25
#define AUDIO_IO_POSITIVE GPIO_NUM_26
#define I2S_NUM I2S_NUM_0

#if SND_16BITS == 0
#define SND_STREAM_BITS  8
#else
#define SND_STREAM_BITS  16
#endif

#if SND_STEREO == 0
#define SND_STREAM_STEREO  1
#else /* SND_STEREO */
#define SND_STREAM_STEREO  2
#endif /* SND_STEREO */


static int audio_volume = 50;
static float audio_volume_f = 0.5f;
static uint8_t audioPause = 1;
static bool initialized = false;
static AudioOutput chosen_output = AudioOutputSpeaker;

static uint16_t AudioTaskCommand = 1;
static QueueHandle_t audioQueue;

// some prototypes
static void audio_submit_i2s(int16_t *buf, int n_frames);
static void audioTask(void* arg);

static int shutdown_speaker()
{
	esp_err_t error;
#define error_message "Could not shutdown dac or amplifier amp: %s\n"
	if ((error = i2s_set_dac_mode(I2S_DAC_CHANNEL_DISABLE)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	if ((error = gpio_set_direction(AUDIO_IO_NEGATIVE, GPIO_MODE_OUTPUT)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	if ((error = gpio_set_direction(AUDIO_IO_POSITIVE, GPIO_MODE_DISABLE)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	if ((error = gpio_set_level(AUDIO_IO_NEGATIVE, 0)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
#undef error_message
	return 0;
}

static int16_t silence[64] = {0};

int audio_init()
{
	if (initialized) {
		fprintf(stderr, "Audio already initialized!\n");
		return -1;
	}
	static AudioOutput output = AudioOutputSpeaker;

	const i2s_mode_t mode_dac = I2S_MODE_MASTER | I2S_MODE_TX;
	const i2s_mode_t mode_speaker = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN;
	const i2s_comm_format_t commfmt_dac = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB;
	const i2s_comm_format_t commfmt_speaker = I2S_COMM_FORMAT_I2S_MSB;

	i2s_config_t i2s_config = {.mode = output == AudioOutputSpeaker ? mode_speaker : mode_dac,
				   .sample_rate = SND_FREQ,
				   .bits_per_sample = SND_STREAM_BITS ,
				   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
				   .communication_format = output == AudioOutputSpeaker ? commfmt_speaker : commfmt_dac,
				   .dma_buf_count = 6,
				   .dma_buf_len = 512,
				   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
				   .use_apll = output == true ? true : false};

	const i2s_pin_config_t dac_pin_config = {
	    .bck_io_num = 4,
	    .ws_io_num = 12,
	    .data_out_num = 15,
	    .data_in_num = -1 // Not used
	};

	esp_err_t error;
	if ((error = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL)) != ESP_OK) {
		// TODO: Need some other system wide error handling
		fprintf(stderr, "Could not install i2s driver: %s\n", esp_err_to_name(error));
		return -1;
	}
	if ((error = i2s_set_pin(I2S_NUM, output == AudioOutputDAC ? &dac_pin_config : NULL)) != ESP_OK) {
		fprintf(stderr, "Could not set i2s pin: %s\n", esp_err_to_name(error));
		return -1;
	}

	initialized = true;
	chosen_output = output;

	if (output == AudioOutputDAC) {
		// Disable interal amplifier when using DAC
		shutdown_speaker();
	} else {
		// Hack: Send some silence to activate internal dac
		// Without this the initial samples sent by audio player wont be heard
		float old_volume = audio_volume_f;
		audio_volume_f = 0.0f;
		for (int i = 0; i < 128; i++) {
			audio_submit_i2s((int16_t *)silence, 32);
		}
		audio_volume_f = old_volume;
	}

	// Start the audio task LOOP
	audioQueue = xQueueCreate(1, sizeof(uint16_t*));
	xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024*2, NULL, 5, NULL, 0);

	return 0;
}

static void audioTask(void* arg)
{
  // sound
  const uint16_t* param;
	int16_t* streamAudioBuffer;

	printf("audioTask: starting\n");

  while(1)
  {
			// New object ready
      xQueuePeek(audioQueue, &param, portMAX_DELAY);

			// Currently 16bits audio is configured in Caprice, NOTE on changing it to 8bits, modification on buffer is needed
			// Buffer is used to transfer R+L to Mono, extend to 32bits DAC1+2 format HIGH,LOW byte holds equal value
			if (!audioPause)
			{
				streamAudioBuffer = (SoundBufferCurrent == 0) ? (int16_t*)&SoundBufferP[1][0] : (int16_t*)&SoundBufferP[0][0];
				audio_submit_i2s(streamAudioBuffer, SoundBytesToWrite>>2);
				SoundBytesToWrite = 0;
			}

		// delete object from the queue
    xQueueReceive(audioQueue, &param, portMAX_DELAY);
  }

  vTaskDelete(NULL);
	while (1) {};

}


void audio_submit(void)
{
	xQueueSend(audioQueue, &AudioTaskCommand, portMAX_DELAY);
}

int audio_shutdown(void)
{
	if (!initialized) {
		fprintf(stderr, "Audio was not initialized!\n");
		return -1;
	}

	esp_err_t error;
	if ((error = i2s_driver_uninstall(I2S_NUM)) != ESP_OK) {
		fprintf(stderr, "Could uninstall i2s driver: %s\n", esp_err_to_name(error));
		return -1;
	}

	shutdown_speaker();
	initialized = false;

	return 0;
}

/** Convert the given buffer to the proper format for internal dac. */
static void convert_internal_dac(short *buf, const int n_frames)
{
	for (int i = 0; i < n_frames * 2; i += 2) {
		int dac0, dac1;

		/* Down mix stero to mono in sample */
		int sample = ((int)buf[i] + (int)buf[i + 1]) >> 1;

		/* Normalize */
		const float normalized = (float)sample / 0x8000;


#if (1)

		const int magnitude = 127 + 127;
		// TODO: When volume is too high for some sounds this causes problems/stutters/clipping
		const float range = magnitude * normalized * audio_volume_f;

		/* Convert to differential output. */
		if (range > 127) {
			dac1 = (range - 127);
			dac0 = 127;
		} else if (range < -127) {
			dac1 = (range + 127);
			dac0 = -127;
		} else {
			dac1 = 0;
			dac0 = range;
		}

		dac0 += 0x80;
		dac1 = 0x80 - dac1;

#else

		/* Scale */
		const int magnitude = 127;
		// TODO: When volume is too high for some sounds this causes problems/stutters/clipping
		float range = magnitude * normalized * audio_volume_f;

		/* Convert to differential output. */
		if (range > 127.0) {(range = 126.9);}
		else if (range < -127.0){(range = -126.9);}

		dac0 = 0x80 + range;
		dac1 = 0x80 - range;

#endif

		dac0 <<= 8;
		dac1 <<= 8;

		buf[i] = (short)dac1;
		buf[i + 1] = (short)dac0;
	}
}

/** Apply volume for external dac buffer */
static void apply_volume(short *buf, const int n_frames)
{
	for (int i = 0; i < n_frames * 2; ++i) {
		// Apply volume
		int sample = (int)((float)buf[i] * audio_volume_f);

		// clamp sample to short values
		// TODO: Should not be needed if audio_volume_f <= 1.0f
		if (sample > 32767)
			sample = 32767;
		else if (sample < -32768)
			sample = -32767;

		buf[i] = (short)sample;
	}
}

// device play
void audio_play(void)
{
	if (initialized)
	{
		i2s_start(I2S_NUM);
		audioPause = 0;
	}
}

// device pause
void audio_pause(void)
{
	if (initialized)
	{
		i2s_stop(I2S_NUM);
		audioPause = 1;
	}
}

// device open status
bool audio_device_active(void)
{
	return(initialized);
}

void audio_submit_i2s(short *buf, int n_frames)
{
	if (!initialized) {
		fprintf(stderr, "audio not yet initialized");
		return;
	}
	if (audio_volume_f == 0.0f) {
		for (int i = 0; i < n_frames; i += 2) {
			buf[i] = 0;
			buf[i + 1] = 0;
		}
	}

	if (chosen_output == AudioOutputSpeaker) {
		convert_internal_dac(buf, n_frames);
	} else {
		apply_volume(buf, n_frames);
	}

	const size_t to_write = 2 * n_frames * sizeof(short);
	size_t written;

	i2s_write(I2S_NUM, buf, to_write, &written, portMAX_DELAY);
	if (written != to_write) {
		fprintf(stderr, "error submitting data to i2s");
	}
}

int audio_volume_set(int volume_percent)
{
	if (volume_percent > 100)
		volume_percent = 100;
	else if (volume_percent < 0)
		volume_percent = 0;
	audio_volume = volume_percent;
	audio_volume_f = (float)volume_percent / 100.0f;
	printf("Volume Set %d\n",audio_volume);
	return audio_volume;
}

int audio_volume_get(void) { return audio_volume; }

void audio_output_set(AudioOutput output) { chosen_output = output; }
AudioOutput audio_output_get(void) { return chosen_output; }
