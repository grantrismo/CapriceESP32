#include <battery.h>
#include <display.h>
#include <backlight.h>
#include <system.h>
#include <event.h>

#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <esp_sleep.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define DEFAULT_VREF 1100
#define BATTERY_ADC_CHANNEL ADC1_CHANNEL_0
#define SAMPLES 4
#define BLINK_PERCENTAGE 5
#define SLEEP_PERCENTAGE 3
#define STATUS_UPDATE_SEC 120


static bool battery_initialized = false;
static esp_adc_cal_characteristics_t adc_characteristics;
static BatteryInfo latest_battery_info;
static BatteryInfo old_battery_info;
static SemaphoreHandle_t info_mutex = NULL;
static uint16_t per_threshold_old;

static void battery_task(void *arg);

static BatteryInfo battery_measure(void)
{
	assert(battery_initialized);

	uint32_t adc_vol_mv = 0;
	for (int i = 0; i < SAMPLES; i++) {
		const uint32_t adc_raw = (uint32_t)adc1_get_raw(BATTERY_ADC_CHANNEL);
		// printf("read_raw[%d] = %d\n", i, adc_raw);
		const uint32_t adc_read_mv = esp_adc_cal_raw_to_voltage(adc_raw, &adc_characteristics);
		// printf("read_vol[%d] = %d, %f\n", i, adc_read_mv, adc_read_mv * 0.001f);
		adc_vol_mv += adc_read_mv;
	}
	adc_vol_mv /= SAMPLES;
	// printf("adc_vol: %d\n", adc_vol_mv);

	// Voltage divider equation simplifies to this because R1 and R2 are both 1K
	const uint32_t battery_vol_mv = 2 * adc_vol_mv;
	const float voltage_full = 4.2f, voltage_empty = 3.5f;

	// TODO: Come up with a better nonlinear SOC estimation function
	uint8_t percentage = (uint8_t)((battery_vol_mv * 0.001f - voltage_empty) / (voltage_full - voltage_empty) * 100.0f);
	if (percentage > 100) {
		percentage = 100;
	}

	assert(percentage < UINT8_MAX);
	assert(battery_vol_mv < UINT16_MAX);
	return (BatteryInfo){.voltage_mv = (uint16_t)battery_vol_mv, .percentage = percentage};
}

int battery_init(void)
{
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(BATTERY_ADC_CHANNEL, ADC_ATTEN_11db);
	esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_characteristics);

	info_mutex = xSemaphoreCreateMutex();
	if (info_mutex == NULL) {
		return -1;
	}

	battery_initialized = true;
	xSemaphoreTake(info_mutex, portMAX_DELAY);
	latest_battery_info = battery_measure();
	xSemaphoreGive(info_mutex);
	memcpy (&old_battery_info,&latest_battery_info, sizeof(BatteryInfo));
	per_threshold_old = 100;

	xTaskCreate(battery_task, "battery_task", 4096, NULL, 5, NULL);

	return 0;
}

int battery_deinit(void) { return 0; }


int battery_read(BatteryInfo *info)
{
	assert(battery_initialized);
	if (xSemaphoreTake(info_mutex, 10 / portTICK_PERIOD_MS) == pdFALSE) {
		return -1;
	}
	*info = latest_battery_info;
	xSemaphoreGive(info_mutex);
	return 0;
}

static void battery_task(void *arg)
{
	(void)arg;
	int period_seconds = STATUS_UPDATE_SEC;
	static bool blink = false;
	for (;;) {
		xSemaphoreTake(info_mutex, portMAX_DELAY);
		latest_battery_info = battery_measure();
		xSemaphoreGive(info_mutex);
		printf("voltage: %d, percentage: %d\n", latest_battery_info.voltage_mv, latest_battery_info.percentage);

		if (latest_battery_info.percentage > BLINK_PERCENTAGE) {
			period_seconds = STATUS_UPDATE_SEC;
			system_led_set(0);
		} else if (latest_battery_info.percentage <= BLINK_PERCENTAGE) {
			period_seconds = 1;
			system_led_set(blink);
			blink = !blink;
			if (latest_battery_info.percentage <= SLEEP_PERCENTAGE) {
				backlight_percentage_set(0);
				display_poweroff();
				esp_deep_sleep_start();
			}
		}

		if (latest_battery_info.voltage_mv <= old_battery_info.voltage_mv)
		{
			// battery is drained
			uint16_t per_threshold = ((uint16_t)(latest_battery_info.percentage + 1)/10)*10;
			if ((old_battery_info.percentage >= per_threshold) && (latest_battery_info.percentage < per_threshold) &&
					(per_threshold < per_threshold_old))
			{
				// we hit the threshold per 10% boundary
				per_threshold_old = per_threshold;
				event_t ev = {.a2dp.head.type = EVENT_TYPE_CAPRICE, .a2dp.event = CapriceEventBattery10Event};
				push_event(&ev);
				printf("Battery Event fired!\n");
			}
		}

		// update values
		memcpy (&old_battery_info,&latest_battery_info, sizeof(BatteryInfo));

		vTaskDelay(period_seconds * 1000 / portTICK_PERIOD_MS);
	}
}
