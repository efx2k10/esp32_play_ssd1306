#ifndef __HW_FILE__
#define __HW_FILE__

// ESP =  ESP32_PLATFORM, ESP32_PLATFORM_S3
#define ESP32_PLATFORM

// DAC =  DAC_PCM5102, DAC_VS1053
#define DAC_VS1053

// DISPLAY =  DISPLAY_NO, DISPLAY_SSD1306
#define DISPLAY_NO

#if defined(ESP32_PLATFORM) && defined(DAC_PCM5102)
#define I2S_DOUT 27
#define I2S_BCLK 26
#define I2S_LRC 25
#endif

#if defined(ESP32_PLATFORM_S3) && defined(DAC_PCM5102)
#define I2S_BCLK 7
#define I2S_DOUT 6
#define I2S_LRC 5
#endif

#if defined(ESP32_PLATFORM) && defined(DAC_VS1053)
#define VS1053_CS 27   /*  XCS pin */
#define VS1053_DCS 25  /*  XDCS pin.  */
#define VS1053_DREQ 26 /*  DREQ pin.  */
#define VS1053_RST -1
#define VS1053_MOSI 23
#define VS1053_MISO 19
#define VS1053_SCK 18
#endif

#ifdef ESP32_PLATFORM
#define IR_PIN 15
#define RELAY_PIN 16
#endif

#ifdef ESP32_PLATFORM_S3
#define IR_PIN 4
#define RELAY_PIN 15
#endif

#endif




