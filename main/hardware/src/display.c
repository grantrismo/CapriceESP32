#pragma GCC optimize ("O3")

#include "display.h"
//#include "image_sd_card_alert.h"
//#include "image_sd_card_unknown.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "display.h"

#include "CPC.h"
#include "Keyboard.h"
#include "Routines.h"

static const gpio_num_t SPI_PIN_NUM_MISO = GPIO_NUM_19;
static const gpio_num_t SPI_PIN_NUM_MOSI = GPIO_NUM_23;
static const gpio_num_t SPI_PIN_NUM_CLK = GPIO_NUM_18;

static const gpio_num_t LCD_PIN_NUM_CS = GPIO_NUM_5;
static const gpio_num_t LCD_PIN_NUM_DC = GPIO_NUM_21;
static const int LCD_SPI_CLOCK_RATE = SPI_MASTER_FREQ_40M;


#define SPI_TRANSACTION_COUNT (4)
static spi_transaction_t trans[SPI_TRANSACTION_COUNT];
static spi_device_handle_t spi;


#define LINE_BUFFERS (2)
#define LINE_COUNT (5)
#define LINE_BUFFER_SIZE (SCREEN_WIDTH*LINE_COUNT)
uint16_t* line[LINE_BUFFERS];
QueueHandle_t spi_queue;
QueueHandle_t line_buffer_queue;
SemaphoreHandle_t spi_count_semaphore;
SemaphoreHandle_t display_mutex = NULL;
spi_transaction_t global_transaction;
bool use_polling = false;
bool isBackLightIntialized = false;

uint8_t stopDisplay = 0;
int32_t timeDelta = 0;
static uint16_t VideoTaskCommand = 1;
static QueueHandle_t videoQueue;
void videoTask(void* argP);
/*
 The ILI9341 needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[128];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} ili_init_cmd_t;

#define TFT_CMD_SWRESET	0x01
#define TFT_CMD_SLEEP 0x10
#define TFT_CMD_DISPLAY_OFF 0x28

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_MH 0x04
#define TFT_RGB_BGR 0x08

DRAM_ATTR static const ili_init_cmd_t ili_sleep_cmds[] = {
    {TFT_CMD_SWRESET, {0}, 0x80},
    {TFT_CMD_DISPLAY_OFF, {0}, 0x80},
    {TFT_CMD_SLEEP, {0}, 0x80},
    {0, {0}, 0xff}
};


/*
 CONFIG_LCD_DRIVER_CHIP_ODROID_GO
*/

DRAM_ATTR static const ili_init_cmd_t ili_init_cmds[] = {
    // VCI=2.8V
    //************* Start Initial Sequence **********//
    {TFT_CMD_SWRESET, {0}, 0x80},
    {0xCF, {0x00, 0xc3, 0x30}, 3},
    {0xED, {0x64, 0x03, 0x12, 0x81}, 4},
    {0xE8, {0x85, 0x00, 0x78}, 3},
    {0xCB, {0x39, 0x2c, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {0xC0, {0x1B}, 1},    //Power control   //VRH[5:0]
    {0xC1, {0x12}, 1},    //Power control   //SAP[2:0];BT[3:0]
    {0xC5, {0x32, 0x3C}, 2},    //VCM control
    {0xC7, {0x91}, 1},    //VCM control2
    {0x36, {(MADCTL_MV | MADCTL_MY | TFT_RGB_BGR)}, 1},    // Memory Access Control
    {0x3A, {0x55}, 1},
    {0xB1, {0x00, 0x1F}, 2},  // Frame Rate Control (1B=70, 1F=61, 10=119)
    {0xB6, {0x0A, 0xA2}, 2},    // Display Function Control
    {0xF6, {0x01, 0x30}, 2},
    {0xF2, {0x00}, 1},    // 3Gamma Function Disable
    {0x26, {0x01}, 1},     //Gamma curve selected

    //Set Gamma
    {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15},
    {0XE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15},

    {0x11, {0}, 0x80},    //Exit Sleep
    {0x29, {0}, 0x80},    //Display on

    {0, {0}, 0xff}
};


static inline uint16_t* line_buffer_get()
{
    uint16_t* buffer;
    if (use_polling) {
        return line[0];
    }

    if (xQueueReceive(line_buffer_queue, &buffer, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }

    return buffer;
}

static inline void line_buffer_put(uint16_t* buffer)
{
    if (xQueueSend(line_buffer_queue, &buffer, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }
}

static void spi_task(void *arg)
{
    printf("%s: Entered.\n", __func__);

    uint16_t* param;
    while(1)
    {
        // Ensure only LCD transactions are pulled
        if(xSemaphoreTake(spi_count_semaphore, portMAX_DELAY) == pdTRUE )
        {
            spi_transaction_t* t;

            esp_err_t ret = spi_device_get_trans_result(spi, &t, portMAX_DELAY);
            assert(ret==ESP_OK);

            int dc = (int)t->user & 0x80;
            if(dc)
            {
                line_buffer_put((uint16_t*)t->tx_buffer);
            }

            if(xQueueSend(spi_queue, &t, portMAX_DELAY) != pdPASS)
            {
                abort();
            }
        }
        else
        {
            printf("%s: xSemaphoreTake failed.\n", __func__);
        }
    }

    printf("%s: Exiting.\n", __func__);
    vTaskDelete(NULL);

    while (1) {}
}

static void spi_initialize()
{
    spi_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(void*));
    if(!spi_queue) abort();


    line_buffer_queue = xQueueCreate(LINE_BUFFERS, sizeof(void*));
    if(!line_buffer_queue) abort();

    spi_count_semaphore = xSemaphoreCreateCounting(SPI_TRANSACTION_COUNT, 0);
    if (!spi_count_semaphore) abort();

    xTaskCreatePinnedToCore(&spi_task, "spi_task", 1024 + 768, NULL, 5, NULL, 0);
}



static inline spi_transaction_t* spi_get_transaction()
{
    spi_transaction_t* t;

    if (use_polling) {
        t = &global_transaction;
    } else {
        xQueueReceive(spi_queue, &t, portMAX_DELAY);
    }

    memset(t, 0, sizeof(*t));

    return t;
}

static inline void spi_put_transaction(spi_transaction_t* t)
{
    t->rx_buffer = NULL;
    t->rxlength = t->length;

    if (t->flags & SPI_TRANS_USE_TXDATA)
    {
        t->flags |= SPI_TRANS_USE_RXDATA;
    }

    if (use_polling) {
        spi_device_polling_transmit(spi, t);
    } else {
        esp_err_t ret = spi_device_queue_trans(spi, t, portMAX_DELAY);
        assert(ret==ESP_OK);

        xSemaphoreGive(spi_count_semaphore);
    }
}


//Send a command to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
static void ili_cmd(const uint8_t cmd)
{
    spi_transaction_t* t = spi_get_transaction();

    t->length = 8;                     //Command is 8 bits
    t->tx_data[0] = cmd;               //The data is the cmd itself
    t->user = (void*)0;                //D/C needs to be set to 0
    t->flags = SPI_TRANS_USE_TXDATA;

    spi_put_transaction(t);
}

//Send data to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
static void ili_data(const uint8_t *data, int len)
{
    if (len)
    {
        spi_transaction_t* t = spi_get_transaction();

        if (len < 5)
        {
            for (int i = 0; i < len; ++i)
            {
                t->tx_data[i] = data[i];
            }
            t->length = len * 8;               //Len is in bytes, transaction length is in bits.
            t->user = (void*)1;                //D/C needs to be set to 1
            t->flags = SPI_TRANS_USE_TXDATA;
        }
        else
        {
            t->length = len * 8;               //Len is in bytes, transaction length is in bits.
            t->tx_buffer = data;               //Data
            t->user = (void*)1;                //D/C needs to be set to 1
            t->flags = 0;
        }

        spi_put_transaction(t);
    }
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
static void ili_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user & 0x01;
    gpio_set_level(LCD_PIN_NUM_DC, dc);
}

//Initialize the display
static void ili_init()
{
    int cmd = 0;

    //Initialize non-SPI GPIOs
    gpio_set_direction(LCD_PIN_NUM_DC, GPIO_MODE_OUTPUT);

    //Send all the commands
    while (ili_init_cmds[cmd].databytes != 0xff)
    {
        ili_cmd(ili_init_cmds[cmd].cmd);

        int len = ili_init_cmds[cmd].databytes & 0x7f;
        if (len) ili_data(ili_init_cmds[cmd].data, len);

        if (ili_init_cmds[cmd].databytes & 0x80)
        {
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        cmd++;
    }
}

static inline void send_reset_column(int left, int right, int len)
{
    ili_cmd(0x2A);
    const uint8_t data[] = { (left) >> 8, (left) & 0xff, right >> 8, right & 0xff };
    ili_data(data, len);
}

static inline void send_reset_page(int top, int bottom, int len)
{
    ili_cmd(0x2B);
    const uint8_t data[] = { top >> 8, top & 0xff, bottom >> 8, bottom & 0xff };
    ili_data(data, len);
}

void send_reset_drawing(int left, int top, int width, int height)
{
    static int last_left = -1;
    static int last_right = -1;
    static int last_top = -1;
    static int last_bottom = -1;

    int right = left + width - 1;
    if (height == 1) {
        if (last_right > right) right = last_right;
        else right = DISPLAY_WIDTH - 1;
    }
    if (left != last_left || right != last_right) {
        send_reset_column(left, right, (right != last_right) ?  4 : 2);
        last_left = left;
        last_right = right;
    }

    int bottom = (top + height - 1);
    if (top != last_top || bottom != last_bottom) {
        send_reset_page(top, bottom, (bottom != last_bottom) ? 4 : 2);
        last_top = top;
        last_bottom = bottom;
    }

    ili_cmd(0x2C);           //memory write
    if (height > 1) {
        ili_cmd(0x3C);           //memory write continue
    }
}

void send_continue_line(uint16_t *line, int width, int lineCount)
{
    spi_transaction_t* t = spi_get_transaction();
    t->length = width * 2 * lineCount * 8;
    t->tx_buffer = line;
    t->user = (void*)0x81;
    t->flags = 0;

    spi_put_transaction(t);
}

//
//   DISPLAY INIT
//
void display_init()
{
    // Init
    spi_initialize();

    // Line buffers
    const size_t lineSize = DISPLAY_WIDTH * LINE_COUNT * sizeof(uint16_t);
    for (int x = 0; x < LINE_BUFFERS; x++)
    {
        line[x] = heap_caps_malloc(lineSize, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (!line[x]) abort();

        printf("%s: line_buffer_put(%p)\n", __func__, line[x]);
        line_buffer_put(line[x]);
    }

    // Initialize transactions
    for (int x = 0; x < SPI_TRANSACTION_COUNT; x++)
    {
        void* param = &trans[x];
        xQueueSend(spi_queue, &param, portMAX_DELAY);
    }

    // Initialize SPI
    esp_err_t ret;
    //spi_device_handle_t spi;
    spi_bus_config_t buscfg;
		memset(&buscfg, 0, sizeof(buscfg));

    buscfg.miso_io_num = -1;
    buscfg.mosi_io_num = SPI_PIN_NUM_MOSI;
    buscfg.sclk_io_num = SPI_PIN_NUM_CLK;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    buscfg.flags = SPICOMMON_BUSFLAG_NATIVE_PINS;

    spi_device_interface_config_t devcfg;
		memset(&devcfg, 0, sizeof(devcfg));

    devcfg.clock_speed_hz = LCD_SPI_CLOCK_RATE;
    devcfg.mode = 0;                                //SPI mode 0
    devcfg.spics_io_num = LCD_PIN_NUM_CS;               //CS pin
    devcfg.queue_size = 7;                          //We want to be able to queue 7 transactions at a time
    devcfg.pre_cb = ili_spi_pre_transfer_callback;  //Specify pre-transfer callback to handle D/C line
    //devcfg.flags = SPI_DEVICE_NO_DUMMY; //SPI_DEVICE_HALFDUPLEX;

    //Initialize the SPI bus
    ret=spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    assert(ret==ESP_OK);

    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    assert(ret==ESP_OK);

    //Initialize the LCD
	  printf("LCD: calling ili_init.\n");
    ili_init();
    videoQueue = xQueueCreate(1, sizeof(uint16_t*));
    xTaskCreatePinnedToCore(&videoTask, "videoTask", 2048, NULL,  2 , NULL, 0);
    printf("LCD Initialized (%d Hz).\n", LCD_SPI_CLOCK_RATE);
}

void display_poweroff()
{
    // // Drain SPI queue
     esp_err_t err = ESP_OK;

    // Disable LCD panel
    xSemaphoreTake(display_mutex, portMAX_DELAY);
    int cmd = 0;
    while (ili_sleep_cmds[cmd].databytes != 0xff)
    {
        //printf("ili9341_poweroff: cmd=%d, ili_sleep_cmds[cmd].cmd=0x%x, ili_sleep_cmds[cmd].databytes=0x%x\n",
        //    cmd, ili_sleep_cmds[cmd].cmd, ili_sleep_cmds[cmd].databytes);

        ili_cmd(ili_sleep_cmds[cmd].cmd);
        ili_data(ili_sleep_cmds[cmd].data, ili_sleep_cmds[cmd].databytes & 0x7f);
        if (ili_sleep_cmds[cmd].databytes & 0x80)
        {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }
    xSemaphoreGive(display_mutex);

}

void videoTask(void* argP)
{

	uint16_t* param;
	uint8_t ybase = 0;
	int32_t timeTick;
  int32_t deviceId;

	while(1)
	{
		xQueuePeek(videoQueue, &param, portMAX_DELAY);
		timeTick = TimGetTicks();
		VideoTaskCommand = 0;

    display_lock();

    // clear the screen
    send_reset_drawing(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    for (short dy = 0; dy < DISPLAY_HEIGHT; dy += LINE_COUNT)
    {
        uint16_t* line_buffer = line_buffer_get();

        if (SystemHalt == 1)
        {
          // blit full screen menus
            blit_ind_565(OffScreenBuffer->data[0] + OffScreenBuffer->width * dy, line_buffer, OffScreenBuffer->width, LINE_COUNT);
        }
        else
        {
          // Keyboard-blit
          if ((NewRockerAttach == RockerAsVirtualKeyboard) && (dy >= (DISPLAY_HEIGHT - DISPLAY_KEYB_HEIGHT)))
          {
          ybase = GetCurrentKeyBaseCoordY();
          blit_keyboard_565(KeyBoardOffScreenBuffer->data[0] + KeyBoardOffScreenBuffer->width * (ybase + dy - (DISPLAY_HEIGHT - DISPLAY_KEYB_HEIGHT)),
            line_buffer,
            KeyBoardOffScreenBuffer->width, LINE_COUNT);
          }
          else
          {
            // EMU Blit
            blit_ind_565(OffScreenBuffer->data[0] + OffScreenBuffer->width * dy , line_buffer, OffScreenBuffer->width, LINE_COUNT);
          }
        }
        send_continue_line(line_buffer, DISPLAY_WIDTH, LINE_COUNT);
    }

    display_unlock();

    xQueueReceive(videoQueue, &param, portMAX_DELAY);
    timeTick = TimGetTicks() - timeTick;
    timeDelta += timeTick;
  }
}


//
//  Core drawing routine
//
void display_update(void)
{
  xQueueSend(videoQueue, (void*)&VideoTaskCommand, portMAX_DELAY);
  if (SystemHalt==1)
    vTaskDelay(100);
}


void write_frame_rectangle(short left, short top, short width, short height, uint16_t* buffer)
{
    short x, y;

    if (left < 0 || top < 0) abort();
    if (width < 1 || height < 1) abort();

    display_lock();
    //xTaskToNotify = xTaskGetCurrentTaskHandle();
    send_reset_drawing(left, top, width, height);

    if (buffer == NULL)
    {
        // clear the buffer
        for (int i = 0; i < LINE_BUFFERS; ++i)
        {
            memset(line[i], 0, DISPLAY_WIDTH * sizeof(uint16_t) * LINE_COUNT);
        }

        // clear the screen
        send_reset_drawing(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

        for (y = 0; y < DISPLAY_HEIGHT; y += LINE_COUNT)
        {
            uint16_t* line_buffer = line_buffer_get();
            send_continue_line(line_buffer, DISPLAY_WIDTH, LINE_COUNT);
        }
    }
    else
    {
      for (y = 0; y < height; y++)
        {
            uint16_t* line_buffer = line_buffer_get();
            memcpy(line_buffer, buffer + y * width, width * sizeof(uint16_t));
            send_continue_line(line_buffer, width, 1);
        }
    }

    display_unlock();
}

void display_clear(uint16_t color)
{
    display_lock();
    //xTaskToNotify = xTaskGetCurrentTaskHandle();

    send_reset_drawing(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);


    // clear the buffer
    for (int i = 0; i < LINE_BUFFERS; ++i)
    {
        for (int j = 0; j < DISPLAY_WIDTH * LINE_COUNT; ++j)
        {
            line[i][j] = color;
        }
    }

    // clear the screen
    send_reset_drawing(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    for (int y = 0; y < DISPLAY_HEIGHT; y += LINE_COUNT)
    {
        uint16_t* line_buffer = line_buffer_get();
        send_continue_line(line_buffer, DISPLAY_WIDTH, LINE_COUNT);
    }

    display_unlock();
}

void display_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer)
{
    short x, y;

    if (left < 0 || top < 0) abort();
    if (width < 1 || height < 1) abort();

    //xTaskToNotify = xTaskGetCurrentTaskHandle();

    send_reset_drawing(left, top, width, height);

    if (buffer == NULL)
    {
        // clear the buffer
        for (int i = 0; i < LINE_BUFFERS; ++i)
        {
            memset(line[i], 0, DISPLAY_WIDTH * sizeof(uint16_t) * LINE_COUNT);
        }

        // clear the screen
        send_reset_drawing(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

        for (y = 0; y < DISPLAY_HEIGHT; y += LINE_COUNT)
        {
            uint16_t* line_buffer = line_buffer_get();
            send_continue_line(line_buffer, DISPLAY_WIDTH, LINE_COUNT);
        }
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            uint16_t* line_buffer = line_buffer_get();

            for (int i = 0; i < width; ++i)
            {
                uint16_t pixel = buffer[y * width + i];
                line_buffer[i] = pixel << 8 | pixel >> 8;
            }

            send_continue_line(line_buffer, width, 1);
        }
    }
}

void display_tasktonotify_set(int value)
{
    //xTaskToNotify = value;
}

void display_drain_spi()
{
    spi_transaction_t *t[SPI_TRANSACTION_COUNT];
    for (int i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
        xQueueReceive(spi_queue, &t[i], portMAX_DELAY);
    }
    for (int i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
        if (xQueueSend(spi_queue, &t[i], portMAX_DELAY) != pdPASS)
        {
            abort();
        }
    }
}

/*
void display_show_sderr(int errNum)
{
    switch(errNum)
    {
        case ODROID_SD_ERR_BADFILE:
            display_write_frame_rectangleLE(0, 0, image_sd_card_unknown.width, image_sd_card_unknown.height, image_sd_card_unknown.pixel_data); // Bad File image
            break;

        case ODROID_SD_ERR_NOCARD:
            display_write_frame_rectangleLE(0, 0, image_sd_card_alert.width, image_sd_card_alert.height, image_sd_card_alert.pixel_data); // No Card image
            break;

        default:
            abort();
    }

    // Drain SPI queue
    display_drain_spi();
}
*/

void inline display_lock()
{
    if (!display_mutex)
    {
        display_mutex = xSemaphoreCreateMutex();
        if (!display_mutex) abort();
    }

    if (xSemaphoreTake(display_mutex, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }
}

void inline display_unlock()
{
    if (!display_mutex) abort();

    display_drain_spi();
    xSemaphoreGive(display_mutex);
}

void inline display_stop()
{
		xSemaphoreTake(display_mutex,portMAX_DELAY);
}
void inline display_resume()
{
		xSemaphoreGive(display_mutex);
}
