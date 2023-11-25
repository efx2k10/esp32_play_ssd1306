#include "Arduino.h"
#include "WiFiMulti.h"
#include "HTTPClient.h"
#include "Audio.h"
#include "Arduino_JSON.h"
#include "Adafruit_SSD1306.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <ir_code.h>
#include <const.h>
#include <helper.h>
#include <AsyncElegantOTA.h>
#include <web_server.h>
#include <main.h>

// define WIFI_SSID WIFI_PSW
#include <const_wifi.h>

WiFiMulti wifiMulti;
Audio *audio = nullptr;
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
Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void init()
{
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
}
void audio_init()
{

    if (is_audio < 1)
        ESP.restart();
    if (audio != nullptr)
    {
        audio->stopSong();
        delete audio;
        audio = nullptr;
        delay(1000);
    }
    if (audio == nullptr)
        audio = new Audio(false);

    audio->setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

    if (time_hour < MUTE_HOUR_MIN || time_hour > MUTE_HOUR_MAX)
        audio_set_volume(1);
    else
        audio_set_volume(play.audio_volume);

    is_audio = 1;
    player_status = PLAYER_READY;
}
void console_init()
{
    Serial.println("");
    Serial.println("------------------------------------------------------");
    Serial.println("|               ESP32 Yandex Music Module            |");
    Serial.println("------------------------------------------------------");
    Serial.print(F("CPU: "));
    Serial.print(ESP.getChipModel());
    Serial.print(F(" rev = "));
    Serial.print(ESP.getChipRevision());
    Serial.print(F(", cores = "));
    Serial.println(ESP.getChipCores());
    Serial.print(F("CPU: freq = "));
    Serial.print(ESP.getCpuFreqMHz());
    Serial.print(F(", cicle = "));
    Serial.println(ESP.getCycleCount());
    Serial.print(F("FlashChipSize: "));
    Serial.println(ESP.getFlashChipSize());
    Serial.print(F("FreePsram: "));
    Serial.println(ESP.getFreePsram());
}

void web_init()
{
    initWebSocket();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.on("/command", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                ws_command = "";
                if (request->hasParam("name")) ws_command = request->getParam("name")->value();
                if (request->hasParam("name") && request->hasParam("value"))  {
                    ws_command =  ws_command + "=" + request->getParam("value")->value();
                }
                request->send_P(200, "text/html", "ok"); });

    server.on("/get_status", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                ws_json_player();
                ws_json_esp_status();  
                ws_json_user_setting();           
                request->send_P(200, "text/html", "ok"); });
    AsyncElegantOTA.begin(&server);
    server.begin();
}

void wifi_init()
{
    Serial.print(F("WIFI: init "));
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PSW);

#if defined(WIFI_SSID_1) && defined(WIFI_PSW_1)
    wifiMulti.addAP(WIFI_SSID_1, WIFI_PSW_1);
#endif

    wifiMulti.run();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(F("."));
        WiFi.disconnect(true);
        wifiMulti.run();
    }
    Serial.println(F(""));
    Serial.print(F("WIFI SSID: "));
    Serial.print(WiFi.SSID());
    Serial.println(F(", connected"));

    Serial.print(F("WIFI AP: "));
    Serial.println(WiFi.localIP());
}

void audio_play_next(const int8_t idx)
{
    String jsonBuffer;
    if (audio == nullptr)
        return;
    String serverPath = SITE_URL + get_radio_mode_url() + "_next&user=" + USER_ID;
    if (idx > PLAY_TRACK_NEXT)
        serverPath = serverPath + "&idx=ablum_" + idx;
    else if (idx == PLAY_ALBUM_NEXT)
        serverPath = serverPath + "&idx=ablum_next";
    else if (idx == PLAY_ALBUM_PREV)
        serverPath = serverPath + "&idx=ablum_prev";
    else if (idx == PLAY_TRACK_PREV)
        serverPath = serverPath + "&idx=track_prev";
    else if (idx == PLAY_TRACK)
        serverPath = serverPath + "&idx=track_set";

    jsonBuffer = httpGETRequest(serverPath.c_str());
    JSONVar myObject = JSON.parse(jsonBuffer);

    if (JSON.typeof(myObject) == "undefined")
    {
        Serial.println("Parsing input failed!");
        lcd_show_system_info("error play track");
        lcd.clearDisplay();
        audio_play_next();
        return;
    }
    audio->stopSong();
    player_status = PLAYER_STOP;

    if (play.radio_mode == SET_MODE_RES_ONLINE)
        audio_init();
    delay(500);
    bool res = false;
    if (myObject["directLink"] != null)
    {
        strcpy(play.play_url, myObject["directLink"]);
        res = audio->connecttohost(play.play_url);
    }
    if (res == false)
    {
        audio->stopSong();
        player_status = PLAYER_STOP;
        audio_play_next();
    }

    ws_json_play_next(jsonBuffer);

    player_status = PLAYER_PLAY;
    strcpy(play.title, myObject["title"]);
    strcpy(play.artist, myObject["artist"]);

    //????

    if (myObject["idx"] != null)
    {
        play.play_id = atol(myObject["idx"]);
    }

    screen_mode = CONST_SCREEN_PLAY;
    lcd.clearDisplay();
    lcd_show_progress_bar(1);
    lcd_show_audio_title();
    lcd_show_audio_info(1);

    ws_json_player();
}
void audio_play_pause()
{
    if (audio == nullptr)
        return;
    if (player_status == PLAYER_PLAY)
    {
        audio->stopSong();
        player_status = PLAYER_PAUSE;
        ws_json_player();
        delay(100);
        return;
    }
    if (player_status == PLAYER_PAUSE)
    {
        audio->connecttohost(play.play_url);
        player_status = PLAYER_PLAY;
        ws_json_player();
        delay(100);
        return;
    }
    delay(100);
    audio_play_next();
}
void audio_stop()
{
    if (audio == nullptr)
        return;
    if (player_status == PLAYER_PLAY)
    {
        audio->stopSong();
        player_status = PLAYER_PAUSE;
        ws_json_player();
        delay(100);
        return;
    }
}
void audio_set_eq()
{
    if (audio == nullptr)
        return;
    audio->setTone(play.audio_eq1, play.audio_eq2, play.audio_eq3);
}
int8_t audio_set_volume(int8_t vol)
{
    if (audio == nullptr)
        return 0;
    audio->setVolume(vol);
    return audio->getVolume();
}
int8_t audio_get_volume()
{
    if (audio == nullptr)
        return 0;
    return audio->getVolume();
}

bool ir_comand_validate(uint64_t ir_code)
{
    switch (ir_code)
    {
    case IR_CODE_VOL_U:
        return 1;
    case IR_CODE_VOL_D:
        return 1;
    case IR_CODE_PLAY_NEXT:
        return 1;
    case IR_CODE_PLAY_PREV:
        return 1;
    case IR_CODE_PLAY_PLAY:
        return 1;
    case IR_CODE_SELECT_EQ:
        return 1;
    case IR_CODE_SET_UP:
        return 1;
    case IR_CODE_SET_DOWN:
        return 1;
    case IR_CODE_CHANGE_RESOUCE:
        return 1;
    case IR_CODE_CHANGE_RESOUCE_WAVE:
        return 1;
    case IR_CODE_CHANGE_RESOUCE_FAVORITE:
        return 1;
    case IR_CODE_CHANGE_RESOUCE_RADIO:
        return 1;
    case IR_CODE_CHANGE_RESOUCE_BT:
        return 1;
    case IR_CODE_ON_OFF:
        return 1;
    case IR_CODE_NUMBER_1:
        return 1;
    case IR_CODE_NUMBER_2:
        return 1;
    case IR_CODE_NUMBER_3:
        return 1;
    case IR_CODE_NUMBER_4:
        return 1;
    case IR_CODE_NUMBER_5:
        return 1;
    case IR_CODE_NUMBER_6:
        return 1;
    }
    return 0;
}
void ir_command_audio(uint64_t ir_code)
{

    switch (ir_code)
    {
    case IR_CODE_PLAY_PLAY:
        audio_play_pause();
        delay(500);
        break;
    case IR_CODE_PLAY_NEXT:
        audio_play_next();
        break;
    case IR_CODE_PLAY_PREV:
        if (play.set_mode != SET_MODE_RES_MYWAVE)
            audio_play_next(PLAY_TRACK_PREV);
        break;
    case IR_CODE_VOL_D:
        play.set_mode = SET_MODE_VOLUME;
        if (play.audio_volume > CONST_VOLUME_MIN)
            play.audio_volume = audio_get_volume() - 1;
        lcd_show_setting_mode(play.audio_volume);
        audio_set_volume(play.audio_volume);
        break;
    case IR_CODE_VOL_U:
        play.set_mode = SET_MODE_VOLUME;
        play.audio_volume = audio_get_volume() + 1;
        if (play.audio_volume > CONST_VOLUME_MAX)
            play.audio_volume = CONST_VOLUME_MAX;
        lcd_show_setting_mode(play.audio_volume);
        audio_set_volume(play.audio_volume);
        break;
    case IR_CODE_SELECT_EQ:
        if (play.set_mode > SET_MODE_EQ3)
            play.set_mode = SET_MODE_IDL;
        if (play.set_mode == SET_MODE_IDL)
        {
            play.set_mode = SET_MODE_EQ1;
            lcd_show_setting_mode(play.audio_eq1);
            break;
        }
        if (play.set_mode == SET_MODE_EQ1)
        {
            play.set_mode = SET_MODE_EQ2;
            lcd_show_setting_mode(play.audio_eq2);
            break;
        }
        if (play.set_mode == SET_MODE_EQ2)
        {
            play.set_mode = SET_MODE_EQ3;
            lcd_show_setting_mode(play.audio_eq3);
            break;
        }
        if (play.set_mode == SET_MODE_EQ3)
        {
            play.set_mode = SET_MODE_EQ1;
            lcd_show_setting_mode(play.audio_eq1);
            break;
        }
        break;
    case IR_CODE_SET_UP:
        //-- eq setup mode
        if (play.set_mode == SET_MODE_EQ1 or play.set_mode == SET_MODE_EQ2 or play.set_mode == SET_MODE_EQ3)
        {
            if (play.set_mode == SET_MODE_EQ1 && play.audio_eq1 < CONST_EQ_MAX)
            {
                play.audio_eq1++;
                lcd_show_setting_mode(play.audio_eq1);
            }
            if (play.set_mode == SET_MODE_EQ2 && play.audio_eq2 < CONST_EQ_MAX)
            {
                play.audio_eq2++;
                lcd_show_setting_mode(play.audio_eq2);
            }
            if (play.set_mode == SET_MODE_EQ3 && play.audio_eq3 < CONST_EQ_MAX)
            {
                play.audio_eq3++;
                lcd_show_setting_mode(play.audio_eq3);
            }
            audio_set_eq();
            break;
        }
        audio_play_next(PLAY_ALBUM_NEXT);
        break;
    case IR_CODE_SET_DOWN:
        //-- eq setup mode
        if (play.set_mode == SET_MODE_EQ1 or play.set_mode == SET_MODE_EQ2 or play.set_mode == SET_MODE_EQ3)
        {
            if (play.set_mode == SET_MODE_EQ1 && play.audio_eq1 > CONST_EQ_MIN)
            {
                play.audio_eq1--;
                lcd_show_setting_mode(play.audio_eq1);
            }
            if (play.set_mode == SET_MODE_EQ2 && play.audio_eq2 > CONST_EQ_MIN)
            {
                play.audio_eq2--;
                lcd_show_setting_mode(play.audio_eq2);
            }
            if (play.set_mode == SET_MODE_EQ3 && play.audio_eq3 > CONST_EQ_MIN)
            {
                play.audio_eq3--;
                lcd_show_setting_mode(play.audio_eq3);
            }
            audio_set_eq();
            break;
        }
        audio_play_next(PLAY_ALBUM_PREV);
        break;
    case IR_CODE_CHANGE_RESOUCE:
        if (play.set_mode != SET_MODE_RES_MYWAVE and play.set_mode != SET_MODE_RES_MYFAVORITE and play.set_mode != SET_MODE_RES_ONLINE)
            play.set_mode = SET_MODE_RES_MYWAVE;
        else if (play.set_mode == SET_MODE_RES_MYWAVE)
            play.set_mode = SET_MODE_RES_MYFAVORITE;
        else if (play.set_mode == SET_MODE_RES_MYFAVORITE)
            play.set_mode = SET_MODE_RES_ONLINE;
        else if (play.set_mode == SET_MODE_RES_ONLINE)
            play.set_mode = SET_MODE_RES_MYWAVE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next();
        break;
    case IR_CODE_CHANGE_RESOUCE_WAVE:
        play.set_mode = SET_MODE_RES_MYWAVE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next();
        break;
    case IR_CODE_CHANGE_RESOUCE_FAVORITE:
        play.set_mode = SET_MODE_RES_MYFAVORITE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next();
        break;
    case IR_CODE_CHANGE_RESOUCE_RADIO:
        play.set_mode = SET_MODE_RES_ONLINE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next();
        break;
    case IR_CODE_NUMBER_1:
        audio_play_next(1);
        break;
    case IR_CODE_NUMBER_2:
        audio_play_next(2);
        break;
    case IR_CODE_NUMBER_3:
        audio_play_next(3);
        break;
    case IR_CODE_NUMBER_4:
        audio_play_next(4);
        break;
    case IR_CODE_NUMBER_5:
        audio_play_next(5);
        break;
    case IR_CODE_NUMBER_6:
        audio_play_next(6);
        break;
    case IR_CODE_ON_OFF:
        digitalWrite(RELAY_PIN, !digitalRead(RELAY_PIN));
        if (digitalRead(RELAY_PIN) == LOW)
        {
            audio_play_next(PLAY_TRACK);
        }
        else
        {
            audio_stop();
        }
        delay(1000);
        break;
    default:
        break;
    }
}
void ir_command()
{

    if (irrecv.decode(&irResults))
    {
        uint64_t ir_code = irResults.value;
        if (IR_CODE_SHOW == 1)
        {

            Serial.println(ir_code);
        }
        if (ir_comand_validate(ir_code) < 1)
        {
            irrecv.resume();
            return;
        };
        if (need_display_off < 1)
        {
            lcd.ssd1306_command(SSD1306_DISPLAYON);
        }
        need_display_off = 1;
        if (audio != nullptr and is_audio == 1)
        {
            ir_command_audio(ir_code);
        }
    }
    irrecv.resume();
}

void lcd_init()
{
    if (!lcd.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    lcd.setTextSize(1);
    lcd.setTextColor(SSD1306_WHITE);
    lcd.clearDisplay();
    lcd.display();
}
void lcd_show_audio_info(const byte resset)
{
    if (audio == nullptr)
        return;
    if (screen_mode != CONST_SCREEN_PLAY)
        return;
    static uint16_t bitRate = 0;
    if (resset == 1)
    {
        bitRate = 0;
    }
    if (bitRate > 0)
        return;
    bitRate = audio->getBitRate() / 1000;
    if (bitRate < 1)
        return;
    lcd.setTextSize(1);
    lcd.setCursor(0, CONST_LCD_ROW_AUDIO_INFO);
    lcd.print(audio->getCodecname());
    lcd.print("-");
    lcd.print(bitRate);
    lcd.print("  ");
    lcd.print(get_radio_mode_lcd());
    lcd.display();
}
void lcd_show_audio_title()
{
    if (audio == nullptr)
        return;
    if (screen_mode != CONST_SCREEN_PLAY)
        return;
    lcd.setTextSize(2);
    lcd.setTextWrap(0);
    String line = utf8rus(play.title) + " - " + utf8rus(play.artist);
    static int16_t line_pos = 0;
    int16_t line_len = line.length();
    line_pos--;
    if (line_pos <= (-line_len - SCREEN_WIDTH / (FONT_1_WIDTH * 2)))
        line_pos = SCREEN_WIDTH / (FONT_1_WIDTH * 2);
    lcd.fillRect(0, CONST_LCD_ROW_TITLE, SCREEN_WIDTH, FONT_1_HEIGHT * 2, SSD1306_BLACK);
    lcd.setCursor(line_pos * FONT_1_WIDTH * 2, CONST_LCD_ROW_TITLE);
    lcd.print(line);
    lcd.display();
}
void lcd_show_player_status()
{
    lcd.clearDisplay();
    lcd.setTextSize(4);
    switch (player_status)
    {
    case PLAYER_PLAY:
        lcd.print("PLAY");
        break;
    case PLAYER_STOP:
        lcd.print("STOP");
        break;
    case PLAYER_PAUSE:
        lcd.print("PAUSE");
        break;
    case PLAYER_UNKNOW:
        lcd.print("UNKNOW");
        break;
    case PLAYER_READY:
        lcd.println("READY");
        lcd.setTextSize(1);
        lcd.println(WiFi.localIP());
        break;
    default:
        lcd.print("????");
        break;
        break;
    }
    lcd.display();
}
void lcd_show_progress_bar(const byte resset)
{
    if (audio == nullptr)
        return;
    if (screen_mode != CONST_SCREEN_PLAY)
        return;
    static int8_t bar_tmp = -1;
    if (resset == 1)
        bar_tmp = -1;
    int max = audio->getAudioFileDuration();
    int pos = audio->getAudioCurrentTime();
    int8_t bar = 0;
    if (max > 0)
        bar = (pos * (SCREEN_WIDTH / 2)) / max;
    lcd.setTextSize(1);
    lcd.fillRect(0, CONST_LCD_ROW_PROGRESS, SCREEN_WIDTH, CONST_LCD_ROW_PROGRESS + 12, SSD1306_BLACK);
    lcd.fillRect(0, CONST_LCD_ROW_PROGRESS, bar, CONST_LCD_ROW_PROGRESS + 12, SSD1306_WHITE);
    if (max > 0)
    {
        lcd.setCursor(SCREEN_WIDTH / 2 + 4, CONST_LCD_ROW_PROGRESS + 2);
    }
    else
    {
        lcd.setCursor(SCREEN_WIDTH / 2 - 16, CONST_LCD_ROW_PROGRESS + 2);
    }
    if (player_status == PLAYER_PLAY)
    {
        int8_t min = pos / 60;
        int8_t sec = pos % 60;
        if (min < 10)
            lcd.print(" ");
        lcd.print(min);
        lcd.print(":");
        if (sec < 10)
            lcd.print("0");
        lcd.print(sec);
        if (max > 0)
        {
            min = max / 60;
            sec = max % 60;
            lcd.print("/");
            lcd.print(min);
            lcd.print(":");
            if (sec < 10)
                lcd.print("0");
            lcd.print(sec);
        }
    }
    else if (player_status == PLAYER_PAUSE)
    {
        lcd.print("PAUSE");
    }
    else if (player_status == PLAYER_STOP)
    {
        lcd.print("STOP");
    }
    lcd.display();
}
void lcd_show_system_info(String in_string)
{
    lcd.setTextSize(1);
    lcd.setTextWrap(1);
    lcd.clearDisplay();
    lcd.display();
    int8_t pos = (SCREEN_WIDTH - utf8rus(in_string).length() * FONT_1_WIDTH) / 2;
    if (pos < 0)
        pos = 0;
    lcd.setCursor(pos, CONST_LCD_ROW_INFO);
    lcd.println(utf8rus(in_string));
    lcd.display();
    delay(100);
}
void lcd_show_setting_mode(const int8_t value)
{
    if (screen_mode != CONST_SCREEN_SETTING)
        screen_mode = CONST_SCREEN_SETTING;
    String set_mode_name = "";
    switch (play.set_mode)
    {
    case SET_MODE_IDL:
        set_mode_name = "IDL";
        break;
    case SET_MODE_EQ1:
        set_mode_name = "EQ1";
        break;
    case SET_MODE_EQ2:
        set_mode_name = "EQ2";
        break;
    case SET_MODE_EQ3:
        set_mode_name = "EQ3";
        break;
    case SET_MODE_VOLUME:
        set_mode_name = "VOLUME";
        break;
    case SET_MODE_RES_MYWAVE:
        set_mode_name = "WAV";
        break;
    case SET_MODE_RES_MYFAVORITE:
        set_mode_name = "FAV";
        break;
    case SET_MODE_RES_ONLINE:
        set_mode_name = "RAD";
        break;
    default:
        break;
    }
    lcd.clearDisplay();
    if (play.set_mode < SET_MODE_RES_MYWAVE)
    {
        lcd.setTextSize(2);
        lcd.setCursor(0, CONST_LCD_ROW_MODE);
        lcd.print(set_mode_name);
        lcd.setTextSize(4);
        lcd.setCursor(0, CONST_LCD_ROW_MODE + FONT_1_HEIGHT * 2);
        lcd.print(value);
    }
    else
    {
        lcd.setTextSize(7);
        lcd.setCursor(0, CONST_LCD_ROW_MODE);
        lcd.print(set_mode_name);
    }
    lcd.display();
    need_save = 1;
    delay(DELAY_IR_POST);
}
void lcd_show_wifi_info()
{
    if (screen_mode != CONST_SCREEN_PLAY)
        return;
    lcd.fillRect(SCREEN_WIDTH - FONT_1_WIDTH * 4, CONST_LCD_ROW_AUDIO_INFO, FONT_1_WIDTH * 4, FONT_1_HEIGHT, SSD1306_BLACK);
    lcd.setTextSize(1);
    lcd.setCursor(SCREEN_WIDTH - FONT_1_WIDTH * 4, CONST_LCD_ROW_AUDIO_INFO);
    lcd.print(WiFi.RSSI());
}

void ws_json_user_setting()
{
    String serverPath = SITE_URL + "get_esp32_setting&user=" + USER_ID;
    String jsonBuffer = httpGETRequest(serverPath.c_str());
    ws.textAll(jsonBuffer);
}
void ws_json_player()
{
    String mes = "{\"message_type\":\"player_info\", \"title\":\"" + String(play.title) + "\", \"artist\":\"" + String(play.artist) + "\"  , \"play.play_url\":\"" + String(play.play_url) + "\"  , \"radio_mode\":\"" + String(play.radio_mode) + "\", \"player_status\":\"" + String(player_status) + "\" , \"play.play_id\":\"" + String(play.play_id) + "\" }";
    ws.textAll(mes);
}
void ws_json_player_status(const String info, const String from)
{

    String mes = "{\"message_type\":\"player_status\", \"message\":\"" + String(info) + "\", \"from\":\"" + String(from) + "\" }";
    ws.textAll(mes);
}
void ws_json_audio_info()
{

    if (audio == nullptr)
        return;
    String mes = "{\"message_type\":\"audio_info\", \"BitRate\":\"" + String(audio->getBitRate()) + "\" , \"BitsPerSample\":\"" + String(audio->getBitsPerSample()) + "\" , \"Codec\":\"" + String(audio->getCodec()) + "\" , \"Codecname\":\"" + String(audio->getCodecname()) + "\"  , \"Volume\":\"" + String(audio->getVolume()) + "\" , \"AudioCurrentTime\":\"" + String(audio->getAudioCurrentTime()) + "\"  , \"AudioFileDuration\":\"" + String(audio->getAudioFileDuration()) + "\" }";

    ws.textAll(mes);
}
void ws_json_play_next(const String mes)
{
    ws.textAll(String(mes));
}
void ws_json_esp_status()
{
    String mes = "{\"message_type\":\"esp_status\", \"wifi_rssi\":\"" + String(WiFi.RSSI()) + "\", \"esp_free_psram\":\"" + String(ESP.getFreePsram()) + "\", \"esp_free_heap\":\"" + String(ESP.getFreeHeap()) + "\", \"esp_relay_pin\":\"" + digitalRead(RELAY_PIN) + "\" }";
    ws.textAll(mes);
}
String ws_command_send_http(String name, String value)
{
    String serverPath = SITE_URL + "esp32_command=" + name + "&value=" + value + "&user=" + USER_ID;
    String jsonBuffer = httpGETRequest(serverPath.c_str());
    return jsonBuffer;
}
void ws_command_http_parse(String str)
{
    str.trim();
    String name = str;
    String val = "";

    uint8_t pos = str.indexOf("=");

    if (pos > 0)
    {
        name = str.substring(0, pos);
        val = str.substring(pos + 1);
    }

    if (name == "play_next")
    {
        audio_play_next(PLAY_TRACK_NEXT);
        return;
    }

    if (name == "play")
    {
        audio_play_next(PLAY_TRACK);
        return;
    }

    if (name == "play_prev")
    {
        audio_play_next(PLAY_TRACK_PREV);
        return;
    }

    if (name == "play_album_next")
    {
        audio_play_next(PLAY_ALBUM_NEXT);
        return;
    }

    if (name == "play_album_prev")
    {
        audio_play_next(PLAY_ALBUM_PREV);
        return;
    }

    if (name == "play_pause")
    {
        audio_play_pause();
        return;
    }

    if (name == "stop")
    {
        audio_stop();
        return;
    }

    if (name == "off")
    {
        digitalWrite(RELAY_PIN, HIGH);
        ws_json_esp_status();
        audio_stop();
        return;
    }

    if (name == "on")
    {
        digitalWrite(RELAY_PIN, LOW);
        ws_json_esp_status();

        if (start_auto_play)
            audio_play_next(PLAY_TRACK);

        return;
    }
    if (name == "get_status")
    {
        ws_json_esp_status();
        return;
    }

    if (name == "set_mode_wave")
    {
        play.radio_mode = SET_MODE_RES_MYWAVE;
        need_save = 1;
        audio_play_next(PLAY_TRACK);
        return;
    }

    if (name == "set_mode_favorite")
    {
        play.radio_mode = SET_MODE_RES_MYFAVORITE;
        need_save = 1;
        audio_play_next(PLAY_TRACK);
        return;
    }

    if (name == "set_mode_online")
    {
        play.radio_mode = SET_MODE_RES_ONLINE;
        need_save = 1;
        audio_play_next(PLAY_TRACK);
        return;
    }

    if (name == "set_volume")
    {
        if (val == "up")
            audio->setVolume(audio_get_volume() + 1);
        if (val == "down")
            audio->setVolume(audio_get_volume() - 1);
        need_save = 1;
        return;
    }

    if (name == "get_audio_info")
    {
        ws_json_audio_info();
        return;
    }

    if (name == "reload_esp32")
    {
        ESP.restart();
        return;
    }

    if (name.startsWith("esp32_"))
    {
        String mes = ws_command_send_http(name, val);
        ws.textAll(mes);
    }
}

String get_radio_mode_url()
{
    switch (play.radio_mode)
    {
    case SET_MODE_RES_MYWAVE:
        return "my_wave";
    case SET_MODE_RES_MYFAVORITE:
        return "my_favorite";
    case SET_MODE_RES_ONLINE:
        return "online";
    default:
        return "my_wave";
    }
}
String get_radio_mode_lcd()
{
    switch (play.radio_mode)
    {
    case SET_MODE_RES_MYWAVE:
        return "WAVE";
    case SET_MODE_RES_MYFAVORITE:
        return "FAVOR";
    case SET_MODE_RES_ONLINE:
        return "RADIO";
    default:
        return "WAVE";
    }
}

void user_setting_update_int8(const char *setting_name, const int8_t setting_value)
{
    String serverPath = SITE_URL + "set_setting&setting_name=" + setting_name + "&setting_value=" + setting_value + "&user=" + USER_ID;
    httpGETRequest(serverPath.c_str());
}
void user_setting_get()
{

    String jsonBuffer;
    String serverPath = SITE_URL + "get_setting&user=" + USER_ID;
    jsonBuffer = httpGETRequest(serverPath.c_str());
    JSONVar myObject = JSON.parse(jsonBuffer);

    if (JSON.typeof(myObject) == "undefined")
    {
        Serial.println("Parsing input failed!");
        delay(1000);
        ESP.restart();
    }

    if (!myObject.hasOwnProperty("user_id"))
    {
        lcd.clearDisplay();
        lcd.setCursor(0, 0);
        lcd.setTextSize(1);
        lcd.println("USER NOT FOUND");
        lcd.println();
        lcd.println(USER_ID);
        lcd.display();
        delay(20000);
        ESP.restart();
    }
    if (myObject.hasOwnProperty("volume"))
        play.audio_volume = atoi(myObject["volume"]);

    if (myObject.hasOwnProperty("eq1"))
        play.audio_eq1 = atoi(myObject["eq1"]);

    if (myObject.hasOwnProperty("eq2"))
        play.audio_eq2 = atoi(myObject["eq2"]);

    if (myObject.hasOwnProperty("eq3"))
        play.audio_eq3 = atoi(myObject["eq3"]);

    if (myObject.hasOwnProperty("mode"))
        play.radio_mode = atoi(myObject["mode"]);

    if (myObject.hasOwnProperty("time_hour"))
        time_hour = atoi(myObject["time_hour"]);

    if (myObject.hasOwnProperty("start_auto_play"))
        start_auto_play = atoi(myObject["start_auto_play"]);

    Serial.println(myObject);

    if (time_hour < MUTE_HOUR_MIN || time_hour > MUTE_HOUR_MAX)
        audio_set_volume(1);
    else
        audio_set_volume(play.audio_volume);

    Serial.print(F("Audio: "));
    Serial.print(F("volume="));
    Serial.print(play.audio_volume);
    Serial.print(F(", tone="));
    Serial.print(play.audio_eq1);
    Serial.print(F(","));
    Serial.print(play.audio_eq2);
    Serial.print(F(","));
    Serial.println(play.audio_eq3);

    Serial.print(F("RADIO MODE: "));
    Serial.print(play.radio_mode);
    Serial.print(F(" , MUTE(hour):"));
    Serial.print(time_hour);
    Serial.print(F(" , MUTE_HOUR_MIN="));
    Serial.print(MUTE_HOUR_MIN);
    Serial.print(F(" , MUTE_HOUR_MAX="));
    Serial.println(MUTE_HOUR_MAX);

    audio_set_eq();
    lcd.clearDisplay();
}

void setup()
{
    init();
    console_init();
    irrecv.enableIRIn();
    play.set_mode = SET_MODE_IDL;
    lcd_init();
    lcd_show_system_info("start yandex music");
    wifi_init();
    lcd.clearDisplay();
    lcd_show_system_info("wifi connected");
    lcd.clearDisplay();
    USER_ID = WiFi.macAddress();
    Serial.print("USER_ID = ");
    Serial.println(USER_ID);
    user_setting_get();
    audio_init();
    lcd_show_player_status();

    web_init();

    if (start_auto_play)
    {
        audio_play_next(PLAY_TRACK);
        digitalWrite(RELAY_PIN, LOW);
    }
}
void loop()
{

    ws.cleanupClients(2);

    static unsigned long timing;
    static unsigned long ir_timing;
    static unsigned long save_timing;
    static unsigned long display_off_timing = millis();
    static unsigned long send_stats_timing;

    unsigned long now = millis();

    if (need_display_off < 1)
    {
        display_off_timing = now;
    }
    if (need_save < 1)
        save_timing = now;
    if (now - display_off_timing > DELAY_DISPLAY_OFF and need_display_off == 1)
    {
        lcd.ssd1306_command(SSD1306_DISPLAYOFF);
        need_display_off = 0;
    }

    if (now - send_stats_timing > DELAY_SEND_STATS)
    {
        ws_json_esp_status();
        send_stats_timing = now;
    }

    if (now - save_timing > DELAY_AUTO_SAVE and need_save == 1)
    {
        user_setting_update_int8("volume", play.audio_volume);
        user_setting_update_int8("eq1", play.audio_eq1);
        user_setting_update_int8("eq2", play.audio_eq2);
        user_setting_update_int8("eq3", play.audio_eq3);
        user_setting_update_int8("mode", play.radio_mode);
        Serial.println("save user setting");
        screen_mode = CONST_SCREEN_PLAY;
        lcd.clearDisplay();
        lcd_show_wifi_info();
        lcd_show_audio_info(1);
        lcd_show_audio_title();
        lcd_show_progress_bar();
        save_timing = now;
        need_save = 0;
    }
    if (now - ir_timing > DELAY_IR_RECEVICE)
    {
        ir_command();
        ir_timing = now;

        if (ws_command != "")
        {
            ws_command_http_parse(ws_command);

            ws_command = "";
        }
    }

    if ((now - timing > DELAY_DISPLAY))
    {
        if (player_status == PLAYER_PLAY || player_status == PLAYER_PAUSE || player_status == PLAYER_STOP)
        {
            lcd_show_wifi_info();
            lcd_show_audio_info();
            lcd_show_audio_title();
            lcd_show_progress_bar();
        }

        timing = now;
    }
    if (Serial.available())
    {
        String r = Serial.readString();
        r.trim();
        // format name=value
        if (r.length() > 2)
        {
            uint8_t pos = r.indexOf("=");
            String name = r.substring(0, pos);
            String val = r.substring(pos + 1);
            Serial.println("SET " + name + " = " + val);
            if (name == "IR_CODE_SHOW")
            {
                IR_CODE_SHOW = val.toInt();
            }
        }
    }
    if (is_audio and audio != nullptr)
        audio->loop();
}

void audio_eof_stream(const char *info)
{
    ws_json_player_status(info, "audio_eof_stream");
    audio_play_next();
}
void audio_info(const char *info)
{
    Serial.print("audio_info     ");
    Serial.println(info);
    ws_json_player_status(info, "audio_info");
}
void audio_id3data(const char *info)
{ // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
    ws_json_player_status(info, "audio_id3data");
}
void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    ws_json_player_status(info, "audio_eof_mp3");
}
void audio_showstation(const char *info)
{
    //? load
    if (strlen(play.artist))
        strcpy(play.artist, info);
    ws_json_player_status(info, "audio_showstation");
    ws_json_player();
}
void audio_showstreamtitle(const char *info)
{
    if (strlen(play.title))
        strcpy(play.title, info);
    ws_json_player_status(info, "audio_showstreamtitle");
    ws_json_player();
}
void audio_bitrate(const char *info)
{
    ws_json_player_status(info, "audio_bitrate");
}
void audio_lasthost(const char *info)
{ // stream URL played
    ws_json_player_status(info, "audio_lasthost");
}
