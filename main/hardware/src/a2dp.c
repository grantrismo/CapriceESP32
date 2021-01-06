/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "a2dp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/xtensa_api.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/queue.h"

#include "ringbuf.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "Routines.h"
#include "CPC.h"
#include "event.h"

#undef A2DP_DEBUG_ENABLED
//#define A2DP_DEBUG_ENABLED

#ifdef A2DP_DEBUG_ENABLED
#  define DBG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: " fmt, ##args)
#else /* CPC_DEBUG_ENABLED */
#  define DBG_PRINT(fmt, args...)
#endif /* CPC_DEBUG_ENABLED */

#define BT_APP_SIG_WORK_DISPATCH            (0x01)
#define BT_APP_CORE_TAG                     "BT_APP_CORE"
#define BT_APP_SIG_WORK_DISPATCH            (0x01)

#define BT_AV_TAG               		      	"BT_AV"
#define BT_RC_CT_TAG                        "RCCT"
#define BT_APP_TAG                          "BT_API"

#define APP_RC_CT_TL_GET_CAPS               (0)
#define APP_RC_CT_TL_RN_VOLUME_CHANGE       (1)
#define BT_APP_HEART_BEAT_EVT               (0xff00)

// 512 = 2 * regulare callback request for 22050 Hz sampling rate
#define BT_MIN_BYTES_PER_FRAME              (512)

/* event for handler "bt_av_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

enum {
    STATE_DRIVER_DISABLED = 0,
    STATE_DRIVER_ENABLED,
    STATE_SILENCE_FEEDING,
    STATE_ACTIVE_FEEDING,
};

// Prototypes
/**
 * @brief     work dispatcher for the application task
 */
static bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback);
static void bt_app_task_start_up(void);
static void bt_app_task_shut_down(void);

/// handler for bluetooth stack enabled events
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

/// callback function for A2DP source
static void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

/// callback function for A2DP source audio data stream
static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len);

static void a2d_app_heart_beat(void *arg);

/// A2DP application state machine
static void bt_app_av_sm_hdlr(uint16_t event, void *param);

/* A2DP application state machine handler for each state */
static void bt_app_av_state_unconnected(uint16_t event, void *param);
static void bt_app_av_state_connecting(uint16_t event, void *param);
static void bt_app_av_state_connected(uint16_t event, void *param);
static void bt_app_av_state_disconnecting(uint16_t event, void *param);
static void bt_app_av_state_reconnecting(uint16_t event, void *param);
static bool bt_app_send_msg(bt_app_msg_t *msg);
static void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
static void bt_av_hdl_avrc_ct_evt(uint16_t event, void *p_param);

// globals
esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
static uint8_t s_peer_bdname[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
bt_app_bd_names* peer_candidates;

// default pin code
esp_bt_pin_code_t pin_code;
uint32_t pin_code_len = 0;

esp_bd_addr_t s_peer_bda = {0};
//m_audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
int s_a2d_state = APP_AV_STATE_IDLE;
int s_media_state = APP_AV_MEDIA_STATE_IDLE;
int s_emu_state = EMU_AV_MEDIA_STATE_IDLE;
int s_driver_state = STATE_DRIVER_DISABLED;
int s_mode_state = APP_AV_MODE_MANUAL_CONNECT;
int s_intv_cnt = 0;
int s_connecting_intv = 0;
uint32_t s_pkt_cnt = 0;

char* bt_sinkname = NULL;
ringbuf_handle_t rb = NULL;
// initialization
bool nvs_init = true;
bool reset_ble = true;
bool ssp_enabled = false;

// freertos
TimerHandle_t s_tmr = NULL;
xQueueHandle s_bt_app_task_queue = NULL;
xTaskHandle s_bt_app_task_handle = NULL;

static void a2dp_create_heart_beat_timer(void)
{
  /* create and start heart beat timer */
  do {
      int tmr_id = 0;
      s_tmr = xTimerCreate("connTmr", (5000 / portTICK_RATE_MS),
                         pdTRUE, (void *)tmr_id, a2d_app_heart_beat);
      xTimerStart(s_tmr, portMAX_DELAY);
  } while (0);
}

static void a2dp_destroy_heart_beat_timer(void)
{
  if (s_tmr != NULL)
    xTimerDelete(s_tmr,0);

  s_tmr = NULL;
}

static void bt_app_notify_event(void)
{
  if ((s_a2d_state == APP_AV_STATE_CONNECTED) && (s_media_state == APP_AV_MEDIA_STATE_IDLE))
  {
    event_t ev = {.a2dp.head.type = EVENT_TYPE_A2DP, .a2dp.event = A2dpEventDeviceConnected};
    push_event(&ev);
    s_driver_state = STATE_DRIVER_ENABLED;
    return;
  }

  if (s_a2d_state == APP_AV_STATE_UNCONNECTED)
  {
    event_t ev = {.a2dp.head.type = EVENT_TYPE_A2DP, .a2dp.event = A2dpEventDeviceUnconnected};
    push_event(&ev);
    return;
  }

  if (s_media_state == APP_AV_MEDIA_STATE_STARTED)
  {
    event_t ev = {.a2dp.head.type = EVENT_TYPE_A2DP, .a2dp.event = A2dpEventMediaStarted};
    push_event(&ev);
    s_driver_state = STATE_SILENCE_FEEDING;
    return;
  }
  if (s_media_state == APP_AV_MEDIA_STATE_STOPPING)
  {
    event_t ev = {.a2dp.head.type = EVENT_TYPE_A2DP, .a2dp.event = A2dpEventMediaStopped};
    push_event(&ev);
    s_driver_state = STATE_DRIVER_DISABLED;
    return;
  }

  if (s_media_state == APP_AV_MEDIA_STATE_SUSPEND)
  {
    event_t ev = {.a2dp.head.type = EVENT_TYPE_A2DP, .a2dp.event = A2dpEventMediaSuspend};
    push_event(&ev);
    return;
  }
}

static bool isConnected(){
    return s_a2d_state == APP_AV_STATE_CONNECTED;
}

static void setPinCode(char *new_pin_code, esp_bt_pin_type_t new_pin_type){
    pin_type = new_pin_type;
    pin_code_len = strlen(new_pin_code);
    strcpy((char*)pin_code, new_pin_code);
}

static int32_t bt_a2d_source_data_cb(uint8_t *data, int32_t len)
{
  if (len < 0 || data == NULL) {
     return 0;
  }

  if (s_driver_state == STATE_ACTIVE_FEEDING)
  {
    int32_t len_rd = rb_read(rb, (char *)data, len, 0);
    // regular return
    if (len_rd == len)
    {
      return (len_rd);
    }
    // on error, do some logs to help debug
    else if (len_rd <= 0)
    {
      switch (len_rd)
      {
        case RB_ABORT:
            DBG_PRINT("IN-[%s] AEL_IO_ABORT", "BT_SEV");
            break;
        case RB_DONE:
        case RB_OK:
            DBG_PRINT("IN-[%s] AEL_IO_DONE,%d", "BT_SEV", len_rd);
            break;
        case RB_FAIL:
            DBG_PRINT("IN-[%s] AEL_STATUS_ERROR_INPUT", "BT_SEV");
            break;
        case RB_TIMEOUT:
            // ESP_LOGD(TAG, "IN-[%s] AEL_IO_TIMEOUT", el->tag);
            break;
        default:
            DBG_PRINT("IN-[%s] Input return not support,ret:%d", "BT_SEV", len_rd);
            break;
        }
        return len;
    }
    // buffer underrun -> how to proceed?
    else if (len_rd < len)
    {
        DBG_PRINT("rb underrun %d,%d\n", len_rd, len);
        if (data != NULL)
          memset((char *)data,0,len);

        if (s_emu_state == EMU_AV_MEDIA_STATE_IDLE)
        {
            s_driver_state = STATE_SILENCE_FEEDING;
            rb_reset(rb);
        }
        return len;
    }
  }
  // start active feeding, or continue silence feeding?
  else
  {
    if ((rb_bytes_filled(rb) >= BT_MIN_BYTES_PER_FRAME) && (s_emu_state == EMU_AV_MEDIA_STATE_RUNNING))
        s_driver_state = STATE_ACTIVE_FEEDING;

    if (data != NULL)
      memset((char *)data,0,len);

    return len;
  }
}


static char *bda2str(esp_bd_addr_t bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x\n",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

void a2dp_service_destroy()
{
  esp_a2d_source_deinit();
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  free(peer_candidates);
}

// Core function to call the setup
void a2dp_service_start(bool is_ssp_enabled) {
    ssp_enabled = is_ssp_enabled;

    if (nvs_init){
        // Initialize NVS (Non-volatile storage library).
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK( ret );
    }

    if (reset_ble) {
        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

        if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
            DBG_PRINT("%s initialize controller failed\n", __func__);
            return;
        }

        if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
            DBG_PRINT("%s enable controller failed\n", __func__);
            return;
        }

        if (esp_bluedroid_init() != ESP_OK) {
            DBG_PRINT("%s initialize bluedroid failed\n", __func__);
            return;
        }

        if (esp_bluedroid_enable() != ESP_OK) {
            DBG_PRINT("%s enable bluedroid failed\n", __func__);
            return;
        }
    }

    /* create application task */
    bt_app_task_start_up();

    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);

    if (ssp_enabled) {
        /* Set default parameters for Secure Simple Pairing */
        esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
        esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
        esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
    }

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    //esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    //esp_bt_pin_code_t pin_code;
    strcpy((char*)pin_code, "1234");
    pin_code_len = 4;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    // allocate bt_canditates
    peer_candidates = (bt_app_bd_names*)heap_caps_malloc(sizeof(bt_app_bd_names),  MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    memset(peer_candidates, 0, sizeof(bt_app_bd_names));
}

void a2dp_service_start_discovery(uint8_t inq_len)
{
  bt_sinkname = NULL;
  peer_candidates->length = 0;
  s_a2d_state = APP_AV_STATE_IDLE;
  s_peer_bdname[0] = '\0';
  s_mode_state = APP_AV_MODE_MANUAL_CONNECT;

  DBG_PRINT("Start device discovery...\n");
  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, inq_len, 0);
  esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
}

void a2dp_service_cancel_discovery()
{
  DBG_PRINT("Cancel device discovery ...\n");
  esp_bt_gap_cancel_discovery();
  s_a2d_state = APP_AV_STATE_IDLE;
}

void a2dp_service_start_discovery_connect(const char* device_name)
{
  /* start device discovery */
  s_peer_bdname[0] = '\0';
  bt_sinkname = device_name;
  DBG_PRINT("Starting %s discovery connect...\n", device_name);
  s_a2d_state = APP_AV_STATE_DISCOVERING;
  s_mode_state = APP_AV_MODE_MANUAL_CONNECT;
  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
  esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);

  a2dp_create_heart_beat_timer();
}

bt_app_bd_names* a2dp_get_peer_canditates()
{
  return peer_candidates;
}


static bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback)
{
    DBG_PRINT("%s event 0x%x, param len %d\n", __func__, event, param_len);

    bt_app_msg_t msg;
    memset(&msg, 0, sizeof(bt_app_msg_t));

    msg.sig = BT_APP_SIG_WORK_DISPATCH;
    msg.event = event;
    msg.cb = p_cback;

    if (param_len == 0) {
        return bt_app_send_msg(&msg);
    } else if (p_params && param_len > 0) {
        if ((msg.param = malloc(param_len)) != NULL) {
            memcpy(msg.param, p_params, param_len);
            /* check if caller has provided a copy callback to do the deep copy */
            if (p_copy_cback) {
                p_copy_cback(&msg, msg.param, p_params);
            }
            return bt_app_send_msg(&msg);
        }
    }

    return false;
}

static bool bt_app_send_msg(bt_app_msg_t *msg)
{
    if (msg == NULL) {
        return false;
    }

    if (xQueueSend(s_bt_app_task_queue, msg, 10 / portTICK_RATE_MS) != pdTRUE) {
        DBG_PRINT("%s xQueue send failed\n", __func__);
        return false;
    }
    return true;
}

static void bt_app_work_dispatched(bt_app_msg_t *msg)
{
    if (msg->cb) {
        msg->cb(msg->event, msg->param);
    }
}

static void bt_app_task_handler(void *arg)
{
    bt_app_msg_t msg;
    for (;;) {
        if (pdTRUE == xQueueReceive(s_bt_app_task_queue, &msg, (portTickType)portMAX_DELAY)) {
            DBG_PRINT("%s, sig 0x%x, 0x%x\n", __func__, msg.sig, msg.event);
            switch (msg.sig) {
            case BT_APP_SIG_WORK_DISPATCH:
                bt_app_work_dispatched(&msg);
                break;
            default:
                DBG_PRINT("%s, unhandled sig: %d", __func__, msg.sig);
                break;
            } // switch (msg.sig)

            if (msg.param) {
                free(msg.param);
            }
        }
    }
}

static void bt_app_task_start_up(void)
{
    s_bt_app_task_queue = xQueueCreate(10, sizeof(bt_app_msg_t));
    xTaskCreate(bt_app_task_handler, "BtAppT", 2048, NULL, configMAX_PRIORITIES - 3, &s_bt_app_task_handle);
    return;
}

static void bt_app_task_shut_down(void)
{
    if (s_bt_app_task_handle) {
        vTaskDelete(s_bt_app_task_handle);
        s_bt_app_task_handle = NULL;
    }
    if (s_bt_app_task_queue) {
        vQueueDelete(s_bt_app_task_queue);
        s_bt_app_task_queue = NULL;
    }
}


static bool get_name_from_eir(uint8_t *eir, uint8_t *bdname, uint8_t *bdname_len)
{
    uint8_t *rmt_bdname = NULL;
    uint8_t rmt_bdname_len = 0;

    if (!eir) {
        return false;
    }

    rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
    if (!rmt_bdname) {
        rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);
    }

    if (rmt_bdname) {
        if (rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN) {
            rmt_bdname_len = ESP_BT_GAP_MAX_BDNAME_LEN;
        }

        if (bdname) {
            DBG_PRINT("EIR: Name updated\n");
            memcpy(bdname, rmt_bdname, rmt_bdname_len);
            bdname[rmt_bdname_len] = '\0';
        }
        if (bdname_len) {
            *bdname_len = rmt_bdname_len;
        }
        return true;
    }

    return false;
}

static void filter_inquiry_scan_result(esp_bt_gap_cb_param_t *param)
{
    char bda_str[18];
    uint32_t cod = 0;
    int32_t rssi = -129; /* invalid value */
    uint8_t *eir = NULL;
    esp_bt_gap_dev_prop_t *p;

    DBG_PRINT("Scanned device: %s\n", bda2str(param->disc_res.bda, bda_str, 18));
    for (int i = 0; i < param->disc_res.num_prop; i++) {
        p = param->disc_res.prop + i;
        switch (p->type) {
        case ESP_BT_GAP_DEV_PROP_COD:
            cod = *(uint32_t *)(p->val);
            DBG_PRINT("--Class of Device: 0x%x\n", cod);
            break;
        case ESP_BT_GAP_DEV_PROP_RSSI:
            rssi = *(int8_t *)(p->val);
            DBG_PRINT("--RSSI: %d\n", rssi);
            break;
        case ESP_BT_GAP_DEV_PROP_EIR:
            eir = (uint8_t *)(p->val);
            break;
        case ESP_BT_GAP_DEV_PROP_BDNAME:
        default:
            break;
        }
    }

    /* search for device with MAJOR service class as "rendering" in COD */
    if (!esp_bt_gap_is_valid_cod(cod) ||
            !(esp_bt_gap_get_cod_srvc(cod) & ESP_BT_COD_SRVC_RENDERING)) {
        return;
    }

    /* search for device name in its extended inqury response */
    if (eir) {
      get_name_from_eir(eir, (uint8_t *)&s_peer_bdname, NULL);
      DBG_PRINT("Device discovery found: %s\n", s_peer_bdname);

      bool found = false;
      if (bt_sinkname != NULL)
      {
        DBG_PRINT("Checking name: %s\n", bt_sinkname);
        if (strcmp((char *)s_peer_bdname, bt_sinkname) == 0) {
            bt_sinkname = (char *) s_peer_bdname;
            found = true;
        }

        if (found){
            DBG_PRINT("Found a target device, address %s, name %s\n", bda_str, s_peer_bdname);
            s_a2d_state = APP_AV_STATE_DISCOVERED;
            memcpy(s_peer_bda, param->disc_res.bda, ESP_BD_ADDR_LEN);
            DBG_PRINT("Cancel device discovery ...\n");
            esp_bt_gap_cancel_discovery();
        }
      }
      else
      {
        DBG_PRINT("Found connection candidate %s\n", s_peer_bdname);
        if (peer_candidates->length < ESP_BT_GAP_MAX_BDNAME_NAMES)
        {
          for (int i=0;i<peer_candidates->length;i++)
          {
            if((strcmp(s_peer_bdname, peer_candidates->bt_canditates[i]) == 0) || (strlen(s_peer_bdname) < 2))
              {
                found = true;
                break;
              }
          }
          if (!found)
          {
            strcpy(peer_candidates->bt_canditates[peer_candidates->length],s_peer_bdname);
            peer_candidates->length++;
            DBG_PRINT("Candidate added %s \n",s_peer_bdname);
          }
        }
        else
          DBG_PRINT("Candidtate List full!!\n");
      }
    }
}


static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
        filter_inquiry_scan_result(param);
        break;
    }
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
            if (s_a2d_state == APP_AV_STATE_IDLE) {
              DBG_PRINT("Device canditates discovery stopped with %i candidtates\n", peer_candidates->length);
              for (int i=0;i<peer_candidates->length;i++)
                DBG_PRINT("%s\n",peer_candidates->bt_canditates[i]);

            } else if (s_a2d_state == APP_AV_STATE_DISCOVERED) {
                s_a2d_state = APP_AV_STATE_CONNECTING;
                DBG_PRINT("Device discovery stopped.\n");
                DBG_PRINT("a2dp connecting to peer: %s\n", s_peer_bdname);
                esp_a2d_source_connect(s_peer_bda);
            } else {
                // not discovered, continue to discover
                if (bt_sinkname != NULL)
                {
                  DBG_PRINT("Device discovery failed, continue to discover...\n");
                  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
                }
            }
        } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) {
            DBG_PRINT("Discovery started.\n");
        }
        break;
    }
    case ESP_BT_GAP_RMT_SRVCS_EVT:
    case ESP_BT_GAP_RMT_SRVC_REC_EVT:
        break;
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            DBG_PRINT("authentication success: %s\n", param->auth_cmpl.device_name);
            //esp_log_buffer_hex(BT_AV_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            DBG_PRINT("authentication failed, status:%d\n", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT: {
        DBG_PRINT("ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d\n", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            DBG_PRINT("Input pin code: 0000 0000 0000 0000\n");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            DBG_PRINT("Input pin code: 1234\n");
            esp_bt_gap_pin_reply(param->pin_req.bda, true, pin_code_len, pin_code);
        }
        break;
    }
    case ESP_BT_GAP_CFM_REQ_EVT:
        if (!ssp_enabled) break;
        DBG_PRINT("ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d\n", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        if (!ssp_enabled) break;
        DBG_PRINT("ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d\n", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        if (!ssp_enabled) break;
        DBG_PRINT("ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!\n");
        break;

    // case ESP_BT_GAP_MODE_CHG_EVT:
    //     DBG_PRINT("ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
    //     break;

    default: {
        DBG_PRINT("event: %d\n", event);
        break;
    }
    }
    return;
}

static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    DBG_PRINT("%s evt %d\n", __func__, event);
    switch (event) {
    case BT_APP_EVT_STACK_UP: {
        /* set up device name */
        char *dev_name = "Odroid Go";
        esp_bt_dev_set_device_name(dev_name);

        /* register GAP callback function */
        esp_bt_gap_register_callback(bt_app_gap_cb);

        /* initialize AVRCP controller */
        //esp_avrc_ct_init();
        //esp_avrc_ct_register_callback(bt_app_rc_ct_cb);

        //esp_avrc_rn_evt_cap_mask_t evt_set = {0};
        //esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
        //assert(esp_avrc_tg_set_rn_evt_cap(&evt_set) == ESP_OK);

        /* initialize A2DP source */
        esp_a2d_register_callback(&bt_app_a2d_cb);

        //esp_a2d_source_register_data_callback(bt_app_a2d_data_cb);
        esp_a2d_source_register_data_callback(bt_a2d_source_data_cb);

        esp_a2d_source_init();

        /* set discoverable and connectable mode */
        //esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);

        break;
    }
    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

static void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    bt_app_work_dispatch(bt_app_av_sm_hdlr, event, param, sizeof(esp_a2d_cb_param_t), NULL);
}

static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len)
{
    if (len < 0 || data == NULL) {
        return 0;
    }
    DBG_PRINT("S:%d,%d\n",TimGetTicks(),len);

    // generate random sequence
    int val = rand() % (1 << 16);
    for (int i = 0; i < (len >> 1); i++) {
        data[(i << 1)] = val & 0xff;
        data[(i << 1) + 1] = (val >> 8) & 0xff;
    }

    return len;
}

static void a2d_app_heart_beat(void *arg)
{
    DBG_PRINT("HB: States: a2d %d, media %d\n",s_a2d_state, s_media_state);
    bt_app_work_dispatch(bt_app_av_sm_hdlr, BT_APP_HEART_BEAT_EVT, NULL, 0, NULL);
}

static void bt_app_av_sm_hdlr(uint16_t event, void *param)
{
    DBG_PRINT("%s state %d, evt 0x%x\n", __func__, s_a2d_state, event);
    esp_a2d_cb_param_t *a2d = NULL;
    switch (s_a2d_state) {
    case APP_AV_STATE_IDLE:
        bt_app_av_state_reconnecting(event, param);
        break;
    case APP_AV_STATE_DISCOVERING:
    case APP_AV_STATE_DISCOVERED:
        break;
    case APP_AV_STATE_UNCONNECTED:
        bt_app_av_state_unconnected(event, param);
        break;
    case APP_AV_STATE_CONNECTING:
        bt_app_av_state_connecting(event, param);
        break;
    case APP_AV_STATE_CONNECTED:
        bt_app_av_state_connected(event, param);
        break;
    case APP_AV_STATE_DISCONNECTING:
        bt_app_av_state_disconnecting(event, param);
        break;
    default:
        a2d = (esp_a2d_cb_param_t *)(param);

        DBG_PRINT("%s state %d not handled!!, event %d, (%d, %d)\n", __func__, s_a2d_state, event, a2d->conn_stat.state, a2d->media_ctrl_stat.status);
        break;
    }
}

static void bt_app_av_state_unconnected(uint16_t event, void *param)
{
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT:
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
        break;
    case BT_APP_HEART_BEAT_EVT: {
        uint8_t *p = s_peer_bda;
        DBG_PRINT("a2dp connecting to peer: %02x:%02x:%02x:%02x:%02x:%02x\n",
                 p[0], p[1], p[2], p[3], p[4], p[5]);
        esp_a2d_source_connect(s_peer_bda);
        s_a2d_state = APP_AV_STATE_CONNECTING;
        s_connecting_intv = 0;
        break;
    }
    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

static void bt_app_av_state_connecting(uint16_t event, void *param)
{
    esp_a2d_cb_param_t *a2d = NULL;
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(param);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
            DBG_PRINT("a2dp connected\n");
            s_a2d_state =  APP_AV_STATE_CONNECTED;
            s_media_state = APP_AV_MEDIA_STATE_IDLE;
            bt_app_notify_event();
            //esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_NONE);
        } else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            s_a2d_state =  APP_AV_STATE_UNCONNECTED;
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT:
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
        break;
    case BT_APP_HEART_BEAT_EVT:
        if (++s_connecting_intv >= 4) {
            DBG_PRINT("HB: Forced to unconnected after %d attempts \n", s_connecting_intv);
            s_a2d_state = APP_AV_STATE_UNCONNECTED;
            s_connecting_intv = 0;
        }
        break;
    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

static void bt_app_av_state_reconnecting(uint16_t event, void *param)
{
    esp_a2d_cb_param_t *a2d = NULL;
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(param);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTING) {
            DBG_PRINT("a2dp re-connecting\n");
            s_a2d_state =  APP_AV_STATE_CONNECTED;
            s_media_state = APP_AV_MEDIA_STATE_IDLE;
            s_mode_state = APP_AV_MODE_AUTO_CONNECT;
            bt_app_notify_event();
            //esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_NONE);
            a2dp_create_heart_beat_timer();

        } else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            s_a2d_state =  APP_AV_STATE_UNCONNECTED;
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT:
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
        break;
    case BT_APP_HEART_BEAT_EVT:
        if (++s_connecting_intv >= 4) {
            s_a2d_state = APP_AV_STATE_UNCONNECTED;
            s_connecting_intv = 0;
        }
        break;
    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

static void bt_app_av_media_proc(uint16_t event, void *param)
{
    esp_a2d_cb_param_t *a2d = NULL;
    switch (s_media_state) {
    case APP_AV_MEDIA_STATE_IDLE: {
        if (event == BT_APP_HEART_BEAT_EVT) {
            DBG_PRINT("a2dp media ready checking ...\n");
            esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY);
        } else if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT) {
            a2d = (esp_a2d_cb_param_t *)(param);
            if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY &&
                    a2d->media_ctrl_stat.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS) {
                DBG_PRINT("a2dp media ready, starting ...\n");
                esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START);
                s_media_state = APP_AV_MEDIA_STATE_STARTING;
            }
        }
        break;
    }
    case APP_AV_MEDIA_STATE_SUSPEND:
    case APP_AV_MEDIA_STATE_STARTING: {
        if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT) {
            a2d = (esp_a2d_cb_param_t *)(param);
            if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_START &&
                    a2d->media_ctrl_stat.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS) {
                DBG_PRINT("a2dp media start successfully.\n");
                s_intv_cnt = 0;
                s_media_state = APP_AV_MEDIA_STATE_STARTED;
                bt_app_notify_event();
            } else {
                // not started succesfully, transfer to idle state
                DBG_PRINT("a2dp media start failed.\n");
                s_media_state = APP_AV_MEDIA_STATE_IDLE;
            }
        }
        break;
    }
    case APP_AV_MEDIA_STATE_STARTED: {
      if (event == BT_APP_HEART_BEAT_EVT) {
        //  if (s_emu_state == EMU_AV_MEDIA_STATE_IDLE) {
        //    DBG_PRINT("a2dp media transition to suspend.\n");
        //    esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_SUSPEND);
        //    s_media_state = APP_AV_MEDIA_STATE_SUSPENDING;
        //  }
      } else if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT) {
          a2d = (esp_a2d_cb_param_t *)(param);
          if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_SUSPEND &&
                  a2d->media_ctrl_stat.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS) {
              DBG_PRINT("a2dp media suspend successfully.\n");
              s_media_state = APP_AV_MEDIA_STATE_SUSPEND;
              bt_app_notify_event();
          } else {
                // not started succesfully, transfer to idle state
                DBG_PRINT("a2dp media suspend failed.\n");
                s_media_state = APP_AV_MEDIA_STATE_IDLE;
          }
      }
      break;
    }

    case APP_AV_MEDIA_STATE_STOPPING: {
        if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT) {
            a2d = (esp_a2d_cb_param_t *)(param);
            if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_STOP &&
                    a2d->media_ctrl_stat.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS) {
                DBG_PRINT("a2dp media stopped successfully, disconnecting...\n");
                bt_app_notify_event();
                s_media_state = APP_AV_MEDIA_STATE_IDLE;
                esp_a2d_source_disconnect(s_peer_bda);
                s_a2d_state = APP_AV_STATE_DISCONNECTING;
            } else {
                DBG_PRINT("a2dp media stopping...\n");
                esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_STOP);
            }
        }
        break;
    }
  }
}

static void bt_app_av_state_connected(uint16_t event, void *param)
{
    esp_a2d_cb_param_t *a2d = NULL;
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(param);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            DBG_PRINT("a2dp disconnected\n");
            s_a2d_state = APP_AV_STATE_UNCONNECTED;
            //esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(param);
        if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state) {
            s_pkt_cnt = 0;
        }
        break;
    }
    case ESP_A2D_AUDIO_CFG_EVT:
        // not suppposed to occur for A2DP source
        break;
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
    case BT_APP_HEART_BEAT_EVT: {
        bt_app_av_media_proc(event, param);
        break;
    }
    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

static void bt_app_av_state_disconnecting(uint16_t event, void *param)
{
    esp_a2d_cb_param_t *a2d = NULL;
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(param);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            DBG_PRINT("a2dp disconnected\n");
            s_a2d_state =  APP_AV_STATE_UNCONNECTED;
            bt_app_notify_event();
            //esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISa2dp connecting to peerCOVERABLE);
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT:
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
    case BT_APP_HEART_BEAT_EVT:
        break;
    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

static void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    switch (event) {
    case ESP_AVRC_CT_METADATA_RSP_EVT:
    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
    case ESP_AVRC_CT_REMOTE_FEATURES_EVT: {
    //case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT:
    //case ESP_AVRC_CT_SET_ABSOLUTE_VOLUME_RSP_EVT: {
        bt_app_work_dispatch(bt_av_hdl_avrc_ct_evt, event, param, sizeof(esp_avrc_ct_cb_param_t), NULL);
        break;
    }
    default:
        DBG_PRINT("Invalid AVRC event: %d\n", event);
        break;
    }
}

// void bt_av_volume_changed(void)
// {
//     if (esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_TEST, &s_avrc_peer_rn_cap,
//                                            ESP_AVRC_RN_VOLUME_CHANGE)) {
//         esp_avrc_ct_send_register_notification_cmd(APP_RC_CT_TL_RN_VOLUME_CHANGE, ESP_AVRC_RN_VOLUME_CHANGE, 0);
//     }
// }

// void bt_av_notify_evt_handler(uint8_t event_id, esp_avrc_rn_param_t *event_parameter)
// {
//     switch (event_id) {
//     case ESP_AVRC_RN_VOLUME_CHANGE:
//         DBG_PRINT("Volume changed: %d", event_parameter->volume);
//         DBG_PRINT("Set absolute volume: volume %d", event_parameter->volume + 5);
//         esp_avrc_ct_send_set_absolute_volume_cmd(APP_RC_CT_TL_RN_VOLUME_CHANGE, event_parameter->volume + 5);
//         bt_av_volume_changed();
//         break;
//     }
// }

static void bt_av_hdl_avrc_ct_evt(uint16_t event, void *p_param)
{
    DBG_PRINT("%s evt %d\n", __func__, event);
    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(p_param);
    switch (event) {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT: {
        uint8_t *bda = rc->conn_stat.remote_bda;
        DBG_PRINT("AVRC conn_state evt: state %d, [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                 rc->conn_stat.connected, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

         if (rc->conn_stat.connected) {
             // get remote supported event_ids of peer AVRCP Target
             //esp_avrc_ct_send_get_rn_capabilities_cmd(APP_RC_CT_TL_GET_CAPS);
         } else {
             // clear peer notification capability record
             //s_avrc_peer_rn_cap.bits = 0;
         }
        break;
    }
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
        DBG_PRINT("AVRC passthrough rsp: key_code 0x%x, key_state %d\n", rc->psth_rsp.key_code, rc->psth_rsp.key_state);
        break;
    }
    case ESP_AVRC_CT_METADATA_RSP_EVT: {
        DBG_PRINT("AVRC metadata rsp: attribute id 0x%x, %s\n", rc->meta_rsp.attr_id, rc->meta_rsp.attr_text);
        free(rc->meta_rsp.attr_text);
        break;
    }
    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT: {
        DBG_PRINT("AVRC event notification: %d\n", rc->change_ntf.event_id);
        //bt_av_notify_evt_handler(rc->change_ntf.event_id, (esp_avrc_rn_param_t *) &rc->change_ntf.event_parameter);
        break;
    }
    case ESP_AVRC_CT_REMOTE_FEATURES_EVT: {
        //DBG_PRINT("AVRC remote features %x, TG features %x", rc->rmt_feats.feat_mask, rc->rmt_feats.tg_feat_flag);
        DBG_PRINT("AVRC remote features\n");
        break;
    }
    // case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT: {
    //     DBG_PRINT("remote rn_cap: count %d, bitmask 0x%x", rc->get_rn_caps_rsp.cap_count,
    //              rc->get_rn_caps_rsp.evt_set.bits);
    //     s_avrc_peer_rn_cap.bits = rc->get_rn_caps_rsp.evt_set.bits;

    //     bt_av_volume_changed();
    //     break;
    // }
    // case ESP_AVRC_CT_SET_ABSOLUTE_VOLUME_RSP_EVT: {
    //     DBG_PRINT("Set absolute volume rsp: volume %d", rc->set_volume_rsp.volume);
    //     break;
    // }

    default:
        DBG_PRINT("%s unhandled evt %d\n", __func__, event);
        break;
    }
}

void setNVSInit(bool doInit){
    nvs_init = doInit;
}

void setResetBLE(bool doInit){
    reset_ble = doInit;
}

void a2dp_set_emu_state_idle()
{
  s_emu_state = EMU_AV_MEDIA_STATE_IDLE;
}

void a2dp_set_emu_state_running()
{
  s_emu_state = EMU_AV_MEDIA_STATE_RUNNING;
}

void a2dp_set_rb_read(ringbuf_handle_t rb_handle)
{
  rb = rb_handle;
}

bool a2dp_is_connected()
{
  return (s_a2d_state == APP_AV_STATE_CONNECTED);
}

bool a2dp_media_is_ready()
{
  return (s_media_state == APP_AV_MEDIA_STATE_STARTED);
}

int a2dp_get_states()
{
  return ((s_media_state<<4) | (s_a2d_state<<1) | (s_mode_state));
}

void a2dp_service_stop()
{
  a2dp_destroy_heart_beat_timer();
  DBG_PRINT("a2dp media stopping...\n");
  esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_STOP);
  s_media_state = APP_AV_MEDIA_STATE_STOPPING;
  s_intv_cnt = 0;
  bt_sinkname = NULL;
  s_peer_bdname[0] = '\0';

}
