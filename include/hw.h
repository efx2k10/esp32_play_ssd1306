// ESP =  ESP_PLATFORM, ESP_PLATFORM_S3
#define  ESP_PLATFORM_S3
// DAC =  DAC_PCM5102, DAC_VS1053
#define DAC_PCM5102
// DISPLAY =  DISPLAY_NO, DISPLAY_SSD1306
#define DISPLAY_SSD1306


#ifndef ESP_PLATFORM_S3
#define I2S_DOUT 27
#define I2S_BCLK 26
#define I2S_LRC 25
#else 
#define I2S_BCLK      7
#define I2S_DOUT      6
#define I2S_LRC       5
#endif


#ifndef ESP_PLATFORM_S3
#define IR_PIN 15
#else 
#define IR_PIN 4
#endif

#ifndef ESP_PLATFORM_S3
#define RELAY_PIN 19
#else 
#define RELAY_PIN 15
#endif
