#include "header.h"

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


    String mes = "{\"message_type\":\"audio_info\", \"BitRate\":\"" + String(audio_get_bitrate()) + "\" , \"BitsPerSample\":\"" + String(audio_get_bitrate()) + "\" , \"Codec\":\"" + String(audio.getCodec()) + "\" , \"Codecname\":\"" + String(audio.getCodecname()) + "\"  , \"Volume\":\"" + String(audio.getVolume()) + "\" , \"AudioCurrentTime\":\"" + String(audio_get_current_time()) + "\"  , \"AudioFileDuration\":\"" + String(audio_get_current_duration()) + "\" }";

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
        user_setting_get();

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
            audio.setVolume(audio_get_volume() + 1);
        if (val == "down")
            audio.setVolume(audio_get_volume() - 1);
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
