#define ESP32S3

#ifndef ESP32S3
#define I2S_DOUT 27
#define I2S_BCLK 26
#define I2S_LRC 25
#else 
#define I2S_BCLK      7
#define I2S_DOUT      6
#define I2S_LRC       5
#endif


#ifndef ESP32S3
#define IR_PIN 15
#else 
#define IR_PIN 4
#endif

#ifndef ESP32S3
#define RELAY_PIN 19
#else 
#define RELAY_PIN 15
#endif

#define DELAY_IR_RECEVICE 150
#define DELAY_IR_POST 50
#define DELAY_DISPLAY 500
#define DELAY_AUTO_SAVE 30000
#define DELAY_SEND_STATS 10000


#define DELAY_DISPLAY_OFF 60000

#define SET_MODE_IDL 0
#define SET_MODE_EQ1 1
#define SET_MODE_EQ2 2
#define SET_MODE_EQ3 3
#define SET_MODE_VOLUME 4

#define SET_MODE_RES_MYWAVE 101
#define SET_MODE_RES_MYFAVORITE 102
#define SET_MODE_RES_ONLINE 103



#define CONST_HTTP_TIMEOUT 30000

#define CONST_VOLUME_MIN 0
#define CONST_VOLUME_MAX 21
#define CONST_EQ_MIN -40
#define CONST_EQ_MAX 6

#define CONST_LCD_ADDR 0x27
#define CONST_LCD_ROW 4
#define CONST_LCD_COL 10
#define CONST_LCD_ROW_PROGRESS 52
#define CONST_LCD_ROW_TITLE 24
#define CONST_LCD_ROW_AUDIO_INFO 0
#define CONST_LCD_ROW_MODE 3
#define CONST_LCD_ROW_INFO 1

#define CONST_SCREEN_OFF 0
#define CONST_SCREEN_IDL 1
#define CONST_SCREEN_PLAY 2
#define CONST_SCREEN_SETTING 3

#define FONT_1_WIDTH 5
#define FONT_1_HEIGHT 12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SITE_MAIN_URL "http://efx2k10gma.temp.swtest.ru/esp32/index.php?"


#define BT_HOST_NAME "ESP_Music"

#define MUTE_HOUR_MIN 9
#define MUTE_HOUR_MAX 20



