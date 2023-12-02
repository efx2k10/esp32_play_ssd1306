#include "header.h"

void audio_init();

void audio_play_pause();
void audio_stop_host();
bool audio_start_host(char* url);

void audio_stop();
void audio_set_eq();
int8_t audio_set_volume(int8_t vol);
int8_t audio_get_volume();

int32_t audio_get_bitrate();
int32_t audio_get_current_time();
int32_t audio_get_current_duration();
