#include "header.h"
#ifdef DAC_PCM5102
#include "Audio.h"

Audio audio;

void audio_init()
{

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

    if (time_hour < MUTE_HOUR_MIN || time_hour > MUTE_HOUR_MAX)
        audio_set_volume(1);
    else
        audio_set_volume(play.audio_volume);

    is_audio = 1;
    player_status = PLAYER_READY;
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
        audio_play_next();
        return;
    }
    audio.stopSong();
    player_status = PLAYER_STOP;

    if (play.radio_mode == SET_MODE_RES_ONLINE)
        audio_init();
    delay(500);
    bool res = false;
    if (myObject["directLink"] != null)
    {
        strcpy(play.play_url, myObject["directLink"]);
        res = audio.connecttohost(play.play_url);
    }
    if (res == false)
    {
        audio.stopSong();
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
    lcd_clear();
    lcd_show_progress_bar(1);
    lcd_show_audio_title();
    lcd_show_audio_info(1);

    ws_json_player();
}
void audio_play_pause()
{

    if (player_status == PLAYER_PLAY)
    {
        audio.stopSong();
        player_status = PLAYER_PAUSE;
        ws_json_player();
        delay(100);
        return;
    }
    if (player_status == PLAYER_PAUSE)
    {
        audio.connecttohost(play.play_url);
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

    if (player_status == PLAYER_PLAY)
    {
        audio.stopSong();
        player_status = PLAYER_PAUSE;
        ws_json_player();
        delay(100);
        return;
    }
}
void audio_set_eq()
{

    audio.setTone(play.audio_eq1, play.audio_eq2, play.audio_eq3);
}
int8_t audio_set_volume(int8_t vol)
{

    audio.setVolume(vol);
    return audio.getVolume();
}
int8_t audio_get_volume()
{

    return audio.getVolume();
}

int32_t audio_get_bitrate()
{
    return audio.getBitsPerSample();
}

int32_t audio_get_current_time()
{
    return audio.getAudioCurrentTime();
};

int32_t audio_get_current_duration()
{
    return audio.getAudioFileDuration();
};

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

#endif