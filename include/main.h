void init();
void audio_init();
void console_init();
void lcd_init();
void web_init();
void wifi_init();
void audio_play_next(int8_t idx = 0);
void audio_play_pause();
void audio_stop();
void audio_set_eq();
int8_t audio_get_volume();
int8_t audio_set_volume(int8_t vol);
bool ir_comand_validate(uint64_t ir_code);
void ir_command_audio(uint64_t ir_code);
void ir_command();

void lcd_show_audio_info(byte resset = 0);
void lcd_show_audio_title();
void lcd_show_player_status();
void lcd_show_progress_bar(byte resset = 0);
void lcd_show_system_info(String in_string);
void lcd_show_setting_mode(int8_t value = 0);
void lcd_show_wifi_info();

void ws_json_user_setting();
void ws_json_player();
void ws_json_player_status(String info, String from);
void ws_json_audio_info();
void ws_json_play_next(String mes);
void ws_json_esp_status();
String ws_command_send_http(String name, String value);
void ws_command_http_parse(String str);

String get_radio_mode_url();
String get_radio_mode_lcd();

void user_setting_update_int8(char *setting_name, int8_t setting_value);
void user_setting_get();

