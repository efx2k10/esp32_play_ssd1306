#include "header/header.h"

void init()
{
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
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
#ifdef ESP32_PLATFORM_S3
    AsyncElegantOTA.begin(&server);
#endif
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

void audio_play_next(int8_t idx = PLAY_TRACK_NEXT)
{
    String jsonBuffer;

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
        lcd_clear();
        audio_play_next(PLAY_TRACK_NEXT);
        return;
    }
    audio_stop_host();
    player_status = PLAYER_STOP;

    if (play.radio_mode == SET_MODE_RES_ONLINE)
        audio_init();
    delay(500);
    bool res = false;
    if (myObject["directLink"] != null)
    {
        strcpy(play.play_url, myObject["directLink"]);
        res = audio_start_host(play.play_url);
    }
    if (res == false)
    {
        audio_stop_host();
        player_status = PLAYER_STOP;
        audio_play_next(PLAY_TRACK_NEXT);
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
    lcd_clear();
    lcd_show_progress_bar(1);
    lcd_show_audio_title();
    lcd_show_audio_info(1);

    ws_json_player();
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
        audio_play_next(PLAY_TRACK_NEXT);
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
        audio_play_next(PLAY_TRACK_NEXT);
        break;
    case IR_CODE_CHANGE_RESOUCE_WAVE:
        play.set_mode = SET_MODE_RES_MYWAVE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next(PLAY_TRACK_NEXT);
        break;
    case IR_CODE_CHANGE_RESOUCE_FAVORITE:
        play.set_mode = SET_MODE_RES_MYFAVORITE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next(PLAY_TRACK_NEXT);
        break;
    case IR_CODE_CHANGE_RESOUCE_RADIO:
        play.set_mode = SET_MODE_RES_ONLINE;
        play.radio_mode = play.set_mode;
        lcd_show_setting_mode();
        audio_play_next(PLAY_TRACK_NEXT);
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
            user_setting_get();
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
            lcd_on();
        }
        need_display_off = 1;
        if (is_audio == 1)
        {
            ir_command_audio(ir_code);
        }
    }
    irrecv.resume();
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
        lcd_clear();
        lcd_show_system_info("USER NOT FOUND " + USER_ID);
        lcd_update();
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
    lcd_clear();
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
    lcd_clear();
    lcd_show_system_info("wifi connected");
    lcd_clear();
    USER_ID = WiFi.macAddress();
    Serial.print("USER_ID = ");
    Serial.println(USER_ID);
    audio_init();
    user_setting_get();

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
        lcd_off();
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
        lcd_clear();
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

    audio.loop();
}
