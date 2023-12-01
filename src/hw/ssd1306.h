#include "header.h"
#include "Adafruit_SSD1306.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define FONT_1_WIDTH 5
#define FONT_1_HEIGHT 12

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define CONST_LCD_ROW_PROGRESS 52
#define CONST_LCD_ROW_TITLE 24
#define CONST_LCD_ROW_AUDIO_INFO 0
#define CONST_LCD_ROW_MODE 3
#define CONST_LCD_ROW_INFO 1

Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



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

    String line = utf8rus(play.title) + " - " + utf8rus(play.artist);
    static int16_t line_pos = 0;
    int16_t line_len = line.length();

    lcd.setTextSize(2);
    lcd.setTextWrap(0);

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

void lcd_clear(){
    lcd.clearDisplay();
}

void lcd_update(){
    lcd.display();
}

void lcd_on(){
    lcd.ssd1306_command(SSD1306_DISPLAYON);
}

void lcd_off(){
    lcd.ssd1306_command(SSD1306_DISPLAYOFF);
}

