#pragma once

#include <stdint.h>

typedef enum {
	EVENT_TYPE_UNUSED,
	/// Event to react to keypad presses
	EVENT_TYPE_KEYPAD,
	/// Caprice events
	EVENT_TYPE_CAPRICE,
	// A2DP Events
	EVENT_TYPE_A2DP,
	/// Update screen, mainly used in simulation
	EVENT_TYPE_UPDATE,
	/// Close program, mainily used in simulation
	EVENT_TYPE_QUIT,
} event_type_t;

typedef struct {
	event_type_t type;
} event_head_t;

typedef struct {
	event_head_t head;
	uint16_t state;
	uint16_t pressed;
	uint16_t released;
} event_keypad_t;

typedef enum CapriceEvent {
	CapriceEventKeyboardRedraw,
	CapriceEventMenuRedraw,
	CapriceEventRestartAudioPipe,
	CapriceEventTimerEvent,
	CapriceEventVolumeOkEvent,
	CapriceEventVolumeFailEvent,
	CapriceEventBattery10Event,
	CapriceEventBrightnessOkEvent,
} CapriceEvent;

typedef struct {
	event_head_t head;
	CapriceEvent event;
} event_caprice_t;

typedef enum A2dpEvent {
	A2dpEventDeviceConnected,
	A2dpEventDeviceUnconnected,
	A2dpEventMediaStarted,
	A2dpEventMediaSuspend,
	A2dpEventMediaStopped,
} A2dpEvent;

typedef struct {
	event_head_t head;
	A2dpEvent event;
} event_a2dp_t;

typedef union {
	event_type_t type;
	event_keypad_t keypad;
	event_caprice_t caprice;
	event_a2dp_t a2dp;
} event_t;

extern void event_init(void);
extern uint32_t KeyCurrentState();
extern int wait_event(event_t *event);
extern int poll_event(event_t *event);
extern int push_event(event_t *event);
extern void timer_event_start(uint32_t tickms);
extern void timer_event_stop();
extern void timer_event_reset(uint32_t tickms);
extern void kick_osd_event();
