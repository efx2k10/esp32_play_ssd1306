#include "header.h"

void lcd_init();
void lcd_show_audio_info(byte resset = 0);
void lcd_show_audio_title();
void lcd_show_player_status();
void lcd_show_progress_bar(byte resset = 0);
void lcd_show_system_info(String in_string);
void lcd_show_setting_mode(int8_t value = 0);
void lcd_show_wifi_info();
void lcd_clear();
void lcd_update();
void lcd_on();
void lcd_off();