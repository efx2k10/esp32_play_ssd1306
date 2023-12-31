#include "../header/header.h"
#ifdef DAC_VS1053
#include <SPI.h>
#include "vs1053_ext.h"

VS1053 audio(VS1053_CS, VS1053_DCS, VS1053_DREQ, VSPI, VS1053_MOSI, VS1053_MISO, VS1053_SCK);

void audio_init()
{

    SPI.begin();
    delay(1000);
    audio.begin();

    if (time_hour < MUTE_HOUR_MIN || time_hour > MUTE_HOUR_MAX)
        audio_set_volume(1);
    else
        audio_set_volume(play.audio_volume);

    is_audio = 1;
    player_status = PLAYER_READY;
}

void audio_stop_host()
{
    if (audio.getFileSize()>0) audio.stop_mp3client();
}

bool audio_start_host(char* url)
{
    return audio.connecttohost(url);
}

void audio_play_pause()
{

    if (player_status == PLAYER_PLAY)
    {
        audio.stop_mp3client();
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
    audio_play_next(PLAY_TRACK_NEXT);
}
void audio_stop()
{

    if (player_status == PLAYER_PLAY)
    {
        audio_stop_host();
        player_status = PLAYER_PAUSE;
        ws_json_player();
        delay(100);
        return;
    }
}
void audio_set_eq()
{

    // audio.setTone(play.audio_eq1, play.audio_eq2, play.audio_eq3);
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
    return audio.getBitRate();
}

int32_t audio_get_current_time()
{
    return audio.getFilePos();
};

int32_t audio_get_current_duration()
{
    return audio.getFileSize();
};

void vs1053_eof_stream(const char *info)
{
    ws_json_player_status(info, "audio_eof_stream");
    audio_play_next(PLAY_TRACK_NEXT);
}
void vs1053_info(const char *info)
{
    Serial.print("audio_info     ");
    Serial.println(info);
    ws_json_player_status(info, "audio_info");
}
void vs1053_id3data(const char *info)
{ // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
    ws_json_player_status(info, "audio_id3data");
}
void vs1053_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    ws_json_player_status(info, "audio_eof_mp3");
}
void vs1053_showstation(const char *info)
{
    //? load
    if (strlen(play.artist))
        strcpy(play.artist, info);
    ws_json_player_status(info, "audio_showstation");
    ws_json_player();
}
void vs1053_showstreamtitle(const char *info)
{
    if (strlen(play.title))
        strcpy(play.title, info);
    ws_json_player_status(info, "audio_showstreamtitle");
    ws_json_player();
}
void vs1053_bitrate(const char *info)
{
    ws_json_player_status(info, "audio_bitrate");
}
void vs1053_lasthost(const char *info)
{ // stream URL played
    ws_json_player_status(info, "audio_lasthost");
}

#endif
