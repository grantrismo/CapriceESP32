#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"

#include "sdcard.h"

#define SDCARD_IO_MISO GPIO_NUM_19
#define SDCARD_IO_MOSI GPIO_NUM_23
#define SDCARD_IO_CLK GPIO_NUM_18
#define SDCARD_IO_CS GPIO_NUM_22

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN

static sdmmc_card_t *sdcard = NULL;
#define IDF3

int sdcard_init(const char *mount_path)
{
#ifdef IDF3
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.slot = VSPI_HOST;
	host.max_freq_khz = SDMMC_FREQ_DEFAULT;

	sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
	slot_config.gpio_miso = SDCARD_IO_MISO;
	slot_config.gpio_mosi = SDCARD_IO_MOSI;
	slot_config.gpio_sck = SDCARD_IO_CLK;
	slot_config.gpio_cs = SDCARD_IO_CS;

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {0};
	mount_config.format_if_mount_failed = false;
	mount_config.max_files = 5;

	return esp_vfs_fat_sdmmc_mount(mount_path, &host, &slot_config, &mount_config, &sdcard);

#else

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.slot = VSPI_HOST;
	host.max_freq_khz = SDMMC_FREQ_DEFAULT;

	spi_bus_config_t bus_cfg = {
			.mosi_io_num = SDCARD_IO_MOSI,
			.miso_io_num = SDCARD_IO_MISO,
			.sclk_io_num = SDCARD_IO_CLK,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			//.max_transfer_sz = 4000,
	};

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = SDCARD_IO_CS;
	slot_config.host_id = host.slot;

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {0};
	mount_config.format_if_mount_failed = false;
	mount_config.max_files = 5;

	return esp_vfs_fat_sdspi_mount(mount_path, &host, &slot_config, &mount_config, &sdcard);
#endif
}

int sdcard_deinit()
{
	if (!sdcard) {
		return ESP_FAIL;
	}

	return esp_vfs_fat_sdmmc_unmount();
}

bool sdcard_present(void) { return sdcard != NULL; }
