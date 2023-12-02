void init();

void console_init();

void web_init();
void wifi_init();

bool ir_comand_validate(uint64_t ir_code);
void ir_command_audio(uint64_t ir_code);
void ir_command();

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
void audio_play_next(int8_t idx);

void user_setting_update_int8(char *setting_name, int8_t setting_value);
void user_setting_get();
