# Проект Яндекс Музыка

## HW
- DAC PCM5102A, VS1053
- PLATFORM ESP32, ESP32S3
- DISPLAY SSD 1306


ESP32 PINS
| HW | ESP32 | ESP32 S3 | 
| :--- | :---: | :---: |
|| `DAC PCM5102A`|
| I2S_DOUT |27|7|
| I2S_BCLK |26|6|
| I2S_LRC |25|5|
|| `DAC VS1053`|
| VS1053_CS |27| - | /*  XCS pin */
| VS1053_DREQ |26| - | /*  DREQ pin.  */
| VS1053_DCS |25| - | /*  XDCS pin.  */
| VS1053_MOSI |23| - |
| VS1053_MISO |19| - |
| VS1053_SCK |18| - |
|| `DISPLAY SSD1306`|
| SDA |-| 8 |
| SDA |-| 9 |
|| `IR RECIV` |
| IN |15| 4 |
|| `RELAY PIN 1` |
| OUT |19(?)| 15 |










