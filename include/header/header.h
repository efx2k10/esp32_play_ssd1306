#ifndef __HEADER_FILE__
#define __HEADER_FILE__

#include "Arduino.h"
#include "WiFiMulti.h"

#include "hw.h"

#include "Arduino_JSON.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include "../hw/ir_code.h"

#include "../const.h"
#include "../helper.h"
#ifdef ESP32_PLATFORM_S3
#include "AsyncElegantOTA.h"
#endif
#include "../web/web_server.h"
#include "main.h"

// define WIFI_SSID WIFI_PSW
#include "const_wifi.h"

WiFiMulti wifiMulti;

bool is_audio = 1;
int IR_CODE_SHOW = 0;

struct STATE
{
    int8_t audio_volume = 0;
    int8_t audio_eq1 = 0;
    int8_t audio_eq2 = 0;
    int8_t audio_eq3 = 0;
    int8_t set_mode = 0;
    uint8_t radio_mode = 0;
    char title[100] = {};
    char artist[100] = {};
    char play_url[255] = {};
    long play_id = 0;
} play;

bool need_save = 0;
bool need_display_off = 1;
bool start_auto_play = 1;

uint8_t time_hour = 0;
IRrecv irrecv(IR_PIN);
decode_results irResults;

const String SITE_URL = SITE_MAIN_URL PROGMEM;

enum STATE_PLAYER
{
    PLAYER_UNKNOW = 0,
    PLAYER_READY = 1,
    PLAYER_STOP = 2,
    PLAYER_PLAY = 3,
    PLAYER_PAUSE = 4
} player_status;

enum STATE_PLAYER_MODE
{
    PLAY_TRACK_NEXT = 0,
    PLAY_ALBUM_NEXT = -1,
    PLAY_ALBUM_PREV = -2,
    PLAY_TRACK_PREV = -3,
    PLAY_TRACK = -4,
} play_mode;

String USER_ID = "";
String ws_command = "";
int8_t screen_mode = CONST_SCREEN_IDL;

#include "display.h"
#include "dac.h"

#ifdef DAC_PCM5102
#include "hw/dac_pcm5102.h"
#endif

#ifdef DAC_VS1053
#include "../hw/dac_vs1053.h"
#endif

#ifdef DISPLAY_SSD1306
#include "../hw/ssd1306.h"
#endif

#ifdef DISPLAY_NO
#include "../hw/nodisplay.h"
#endif

#include "../web/http.h"
#include "../web/ws.h"



#endif
