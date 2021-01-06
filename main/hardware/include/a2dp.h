/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef __A2DP_H__
#define __A2DP_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SIM
#include <ringbuf.h>
#endif

//#include "esp32-hal-log.h"
//#include "esp32-hal-bt.h"

#define ESP_BT_GAP_MAX_BDNAME_NAMES   (20)
#define ESP_BT_GAP_MAX_BDNAME_LENGTH (248)

/* A2DP global state */
enum {
    APP_AV_STATE_IDLE,
    APP_AV_STATE_DISCOVERING,
    APP_AV_STATE_DISCOVERED,
    APP_AV_STATE_UNCONNECTED,
    APP_AV_STATE_CONNECTING,
    APP_AV_STATE_CONNECTED,
    APP_AV_STATE_DISCONNECTING,
};

/* sub states of APP_AV_STATE_CONNECTED */
enum {
    APP_AV_MEDIA_STATE_IDLE,
    APP_AV_MEDIA_STATE_STARTING,
    APP_AV_MEDIA_STATE_STARTED,
    APP_AV_MEDIA_STATE_SUSPENDING,
    APP_AV_MEDIA_STATE_SUSPEND,
    APP_AV_MEDIA_STATE_STOPPING,
};

// stat information about the emulator
enum {
  EMU_AV_MEDIA_STATE_IDLE,
  EMU_AV_MEDIA_STATE_RUNNING,
};

enum {
  APP_AV_MODE_MANUAL_CONNECT,
  APP_AV_MODE_AUTO_CONNECT,
};

/**
 * @brief     handler for the dispatched work
 */
typedef void (* bt_app_cb_t) (uint16_t event, void *param);

/* message to be sent */
typedef struct {
    uint16_t             sig;      /*!< signal to bt_app_task */
    uint16_t             event;    /*!< message event id */
    bt_app_cb_t          cb;       /*!< context switch callback */
    void                 *param;   /*!< parameter area needs to be last */
} bt_app_msg_t;

/* holding the bt discovery candiates */
typedef struct {
  uint16_t              length;
  char                  bt_canditates[ESP_BT_GAP_MAX_BDNAME_NAMES][ESP_BT_GAP_MAX_BDNAME_LENGTH+1];
} bt_app_bd_names;

/**
 * @brief     parameter deep-copy function to be customized
 */
typedef void (* bt_app_copy_cb_t) (bt_app_msg_t *msg, void *p_dest, void *p_src);


#ifndef SIM
extern void a2dp_set_rb_read(ringbuf_handle_t rb_handle);
#endif

extern void a2dp_service_start(bool is_ssp_enabled);
extern void a2dp_service_destroy();
extern void a2dp_set_emu_state_running();
extern void a2dp_set_emu_state_idle();
extern void a2dp_service_cancel_discovery();
extern void a2dp_service_start_discovery(uint8_t inq_len);
extern void a2dp_service_start_discovery_connect(const char* device_name);
extern bt_app_bd_names* a2dp_get_peer_canditates();
extern bool a2dp_is_connected();
extern bool a2dp_media_is_ready();
extern void a2dp_service_stop();
extern int a2dp_get_states();
#endif /* __BLE_A2DP_SOURCE_H__ */
