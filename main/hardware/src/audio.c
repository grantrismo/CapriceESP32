#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/rtc_io.h>
#include <driver/dac.h>
#include <ringbuf.h>

#include <stdio.h>
#include <string.h>

#include "Native_CPC.h"

#include "Sound.h"
#include "audio.h"

#define AUDIO_IO_NEGATIVE 					GPIO_NUM_25
#define AUDIO_IO_POSITIVE 					GPIO_NUM_26
#define I2S_NUM 										I2S_NUM_0

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

//Num_bytes = (bits_per_sample / 8) * num_chan * dma_buf_count * dma_buf_len
#define I2S_BUFFER_LEN 							(256)
#define I2S_BUFFER_COUNT 						(4)
#define I2S_BYTES_PER_FRAME					(SND_STREAM_BITS/8 * SND_STREAM_STEREO * I2S_BUFFER_LEN)

typedef enum {
	STATE_DRIVER_DISABLED = 0,
	STATE_DRIVER_IDLE,
	STATE_DRIVER_ENABLED,
	STATE_SILENCE_FEEDING,
	STATE_ACTIVE_FEEDING,
} DriverState;

typedef enum {
  EMU_AV_MEDIA_STATE_IDLE,
  EMU_AV_MEDIA_STATE_RUNNING,
} EmuState;

const i2s_pin_config_t dac_pin_config = {
		.bck_io_num = 4,
		.ws_io_num = 12,
		.data_out_num = 15,
		.data_in_num = -1 // Not used
};

static int audio_volume = 50;
static float audio_volume_f = 0.5f;

static AudioOutput chosen_output = AudioOutputSpeaker;
static ringbuf_handle_t audio_rb = NULL;
static DriverState driver_state = STATE_DRIVER_DISABLED;
static EmuState s_emu_state = EMU_AV_MEDIA_STATE_IDLE;

static uint16_t AudioTaskCommand = 1;
static QueueHandle_t audioQueue = NULL;
static xTaskHandle vid_task_handle = NULL;

// some prototypes
static void audio_submit_i2s(int16_t *buf, int n_frames);
static void audioTask(void* arg);

void audio_set_rb_read(ringbuf_handle_t rb_handle)
{
	audio_rb = rb_handle;
}

static int drive_speaker()
{
	esp_err_t error;
	if ((error = i2s_set_pin(I2S_NUM, NULL)) != ESP_OK) {
		fprintf(stderr, "Could not set i2s pin: %s\n", esp_err_to_name(error));
		return -1;
	}

	if ((error = i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN)) != ESP_OK){
		fprintf(stderr, "Issue on enabling DAC: %s\n", esp_err_to_name(error));
	}
	return -1;
}

static int shutdown_speaker()
{
	esp_err_t error;
	if (driver_state == STATE_ACTIVE_FEEDING)
	{
		fprintf(stderr, "Driver still active feeding, cannot shutdown driver\n");
		return -1;
	}
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
	driver_state = STATE_DRIVER_IDLE;
	return 0;
}

static int16_t silence[64] = {0};

int audio_init()
{
	if (driver_state != STATE_DRIVER_DISABLED) {
		fprintf(stderr, "Audio already initialized!\n");
		return -1;
	}
	AudioOutput output = chosen_output;

	const i2s_mode_t mode_dac = I2S_MODE_MASTER | I2S_MODE_TX;
	const i2s_mode_t mode_speaker = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN;
	const i2s_comm_format_t commfmt_dac = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB;
	const i2s_comm_format_t commfmt_speaker = I2S_COMM_FORMAT_I2S_MSB;

	i2s_config_t i2s_config = {.mode = output == AudioOutputSpeaker ? mode_speaker : mode_dac,
				   .sample_rate = SND_FREQ,
				   .bits_per_sample = SND_STREAM_BITS ,
				   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
				   .communication_format = output == AudioOutputSpeaker ? commfmt_speaker : commfmt_dac,
				   .dma_buf_count = I2S_BUFFER_COUNT,
				   .dma_buf_len = I2S_BUFFER_LEN,
				   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
				   .use_apll = output == true ? true : false};

	esp_err_t error;
	if ((error = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL)) != ESP_OK) {
		// TODO: Need some other system wide error handling
		fprintf(stderr, "Could not install i2s driver: %s\n", esp_err_to_name(error));
		return -1;
	}

	driver_state = STATE_DRIVER_ENABLED;
	chosen_output = output;

	if (output == AudioOutputDAC) {
		// Enable the Pins
		i2s_set_pin(I2S_NUM, &dac_pin_config);
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
	if (audioQueue == NULL)
		audioQueue = xQueueCreate(1, sizeof(uint16_t*));

	if (vid_task_handle == NULL)
		xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024*2, NULL, 5, &vid_task_handle, 0);

	return 0;
}

static void audioTask(void* arg)
{
  // sound
  const uint16_t* param;
	static uint16_t streamAudioBuffer[I2S_BYTES_PER_FRAME];
	int bytes_read;

	printf("audioTask: starting\n");

  while(driver_state != STATE_DRIVER_DISABLED)
  {
			xQueuePeek(audioQueue, &param, portMAX_DELAY);
			if (driver_state == STATE_ACTIVE_FEEDING)
			{
				while (rb_bytes_filled(audio_rb) >= I2S_BYTES_PER_FRAME)
				{
					bytes_read = rb_read(audio_rb, (char *)&streamAudioBuffer, I2S_BYTES_PER_FRAME, 0);
					if (bytes_read != I2S_BYTES_PER_FRAME)
						memset(&streamAudioBuffer[0], 0, I2S_BYTES_PER_FRAME);

					audio_submit_i2s(streamAudioBuffer, I2S_BUFFER_LEN);
				}
				if (s_emu_state == EMU_AV_MEDIA_STATE_IDLE)
				{
						driver_state = STATE_SILENCE_FEEDING;
						rb_reset(audio_rb);
				}
			}
			else
			{
				if ((rb_bytes_filled(audio_rb) >= I2S_BYTES_PER_FRAME) && (s_emu_state == EMU_AV_MEDIA_STATE_RUNNING))
						driver_state = STATE_ACTIVE_FEEDING;
			}

			xQueueReceive(audioQueue, &param, portMAX_DELAY);
  }

	vQueueDelete(audioQueue);
 	vTaskDelete(NULL);

	while (1) {};

}


void audio_submit(void)
{
	if (s_emu_state == EMU_AV_MEDIA_STATE_RUNNING)
		xQueueSend(audioQueue, &AudioTaskCommand, portMAX_DELAY);
}

int audio_shutdown(void)
{
	if (driver_state == STATE_DRIVER_DISABLED) {
		fprintf(stderr, "Audio was not initialized!\n");
		return -1;
	}

	esp_err_t error;
	if ((error = i2s_driver_uninstall(I2S_NUM)) != ESP_OK) {
		fprintf(stderr, "Could uninstall i2s driver: %s\n", esp_err_to_name(error));
		return -1;
	}

	// IO Control
	if (chosen_output == AudioOutputDAC)
		i2s_set_pin(I2S_NUM, NULL);
	else
		shutdown_speaker();

	driver_state = STATE_DRIVER_DISABLED;

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
	if (driver_state != STATE_DRIVER_DISABLED)
	{
		//i2s_start(I2S_NUM);

		if (chosen_output == AudioOutputDAC)
			i2s_set_pin(I2S_NUM, &dac_pin_config);
		else
			drive_speaker();

		driver_state = STATE_ACTIVE_FEEDING;
	}
}

// device pause
void audio_pause(void)
{
	if (driver_state != STATE_DRIVER_DISABLED)
	{
		// IO Control
		if (chosen_output == AudioOutputDAC)
			i2s_set_pin(I2S_NUM, NULL);
		else
			shutdown_speaker();

		//i2s_stop(I2S_NUM);
		driver_state = STATE_SILENCE_FEEDING;
	}
}

// device open status
bool audio_device_active(void)
{
	return (driver_state != STATE_DRIVER_DISABLED);
}

void audio_submit_i2s(short *buf, int n_frames)
{
	if (driver_state == STATE_DRIVER_DISABLED) {
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

void i2s_set_emu_state_idle()
{
  s_emu_state = EMU_AV_MEDIA_STATE_IDLE;
}

void i2s_set_emu_state_running()
{
  s_emu_state = EMU_AV_MEDIA_STATE_RUNNING;
}

int i2s_get_rb_size()
{
	return (I2S_BUFFER_COUNT * I2S_BYTES_PER_FRAME);
}

int audio_volume_get(void) { return audio_volume; }

void audio_output_set(AudioOutput output) { chosen_output = output; }
AudioOutput audio_output_get(void) { return chosen_output; }
