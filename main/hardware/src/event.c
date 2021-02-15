#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#include <event.h>
#include <keypad.h>

#define REPEAT_HOLDOFF (250 / portTICK_PERIOD_MS)
#define REPEAT_RATE (80 / portTICK_PERIOD_MS)

static QueueHandle_t event_queue;
static TimerHandle_t osd_timer = NULL;

static void keypad_task(void *arg)
{
	event_t event;
	uint16_t changes;

	TickType_t down_ticks[4];
	uint16_t repeat_count[4];

	while (true) {
		vTaskDelay(10 / portTICK_PERIOD_MS);
		event.type = EVENT_TYPE_KEYPAD;
		event.keypad.state = keypad_debounce(keypad_sample(), &changes);
		event.keypad.pressed = event.keypad.state & changes;
		event.keypad.released = ~event.keypad.state & changes;

		TickType_t now = xTaskGetTickCount();
		for (int i = 0; i < 4; i++) {
			if ((event.keypad.pressed >> i) & 1) {
				down_ticks[i] = now;
				repeat_count[i] = UINT16_MAX;
			} else if ((event.keypad.state >> i) & 1) {
				if (now - down_ticks[i] >= REPEAT_HOLDOFF) {
					uint16_t n = (now - down_ticks[i] - REPEAT_HOLDOFF) / REPEAT_RATE;
					if (repeat_count[i] != n) {
						repeat_count[i] = n;
						event.keypad.pressed |= (1 << i);
					}
				}
			}
		}

		if (event.keypad.pressed || event.keypad.released) {
			push_event(&event);
		}
	}
}

uint32_t KeyCurrentState(void)
{
	return(keypad_sample());
}

void event_init(void)
{
	event_queue = xQueueCreate(10, sizeof(event_t));
	xTaskCreate(keypad_task, "keypad", 4096, NULL, 5, NULL);
}

int wait_event(event_t *event)
{
	int got_event = xQueueReceive(event_queue, event, portMAX_DELAY);
	if (got_event != pdTRUE) {
		event->type = EVENT_TYPE_UNUSED;
		event->keypad.pressed = 0;
		event->keypad.released = 0;
		return -1;
	}
	return 1;
}

int poll_event(event_t *event)
{
	int got_event = xQueueReceive(event_queue, event, 0);
	if (got_event != pdTRUE) {
		event->type = EVENT_TYPE_UNUSED;
		event->keypad.pressed = 0;
		event->keypad.released= 0;
		return -1;
	}
	return 1;
}

int push_event(event_t *event) { return xQueueSend(event_queue, event, 10 / portTICK_PERIOD_MS); }


static void timer_callback(TimerHandle_t xTimer)
{
	event_t ev = {.caprice.head.type = EVENT_TYPE_CAPRICE, .caprice.event = CapriceEventTimerEvent};
	push_event(&ev);
}

void timer_event_start(uint32_t tickms)
{
	osd_timer = xTimerCreate("osdtimer", tickms / portTICK_PERIOD_MS, pdTRUE, ( void * )0, timer_callback);
	xTimerStart(osd_timer,0);
}

void timer_event_stop()
{
	if (osd_timer != NULL)
	{
		xTimerStop(osd_timer,0);
		xTimerDelete(osd_timer,0);
		osd_timer = NULL;
	}
}

void timer_event_reset(uint32_t tickms)
{
	xTimerReset(osd_timer,0);
}

void kick_osd_event()
{
	event_t ev = {.caprice.head.type = EVENT_TYPE_CAPRICE, .caprice.event = CapriceEventTimerEvent};
	push_event(&ev);
}
