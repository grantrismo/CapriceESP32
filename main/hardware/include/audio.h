#pragma once

typedef enum AudioOutput {
	AudioOutputSpeaker,
	AudioOutputDAC,
} AudioOutput;

/// Initialize audio driver/i2s with given sample rate and output mode.
int audio_init();

/// Shutdown audio driver.
int audio_shutdown(void);

/// Submit signed 16bit audio for playing.
/// This blocks until all samples are played/copied to dma buffer.
void audio_submit(void);

/// Change audio volume.
/// vol has to be between 0 and 100.
int audio_volume_set(int volume_percent);

/// Get the audio volume.
int audio_volume_get(void);

/// Set the audio output.
void audio_output_set(AudioOutput output);

/// Set the currently set audio output.
AudioOutput audio_output_get(void);

// get Initialized states
bool audio_device_active(void);

void audio_play(void);

void audio_pause (void);
