# OpenDSC 2.0.2

*****************************************************************
CAUTION: Does not work with Skysafari 7 Plus !!!!!!!!

(Something is wrong in this version)

Works fine with Skysafari 5 Plus

*****************************************************************

If you are owner of SkySafari 5 Plus and you cannot download the OBB file from its website (https://support.simulationcurriculum.com/hc/en-us/articles/115004749607-Data-Download-for-SkySafari-for-Android), here you can download it:

https://drive.google.com/drive/folders/1J6c5GILhAsH-bUJ5VxNBshDk-nba_C--?usp=sharing

Copy the file you downloaded in:

SkySafari Plus: <top level of SD card>/Android/obb/com.simulationcurriculum.skysafari5plus/
  
  *****************************************************************

ESP32 TTGO Digital Setting  Circles 

![Image description](https://github.com/xvedra/OpenDsc/blob/master/screenshots/MainWin.jpg)

Based in:

https://github.com/synfinatic/esp-dsc

https://github.com/synfinatic/teensy-dsc

https://eksfiles.net/digital-setting-circles/bluetooth-digital-setting-circles/

Dependencies:

https://github.com/Bodmer/TFT_eSPI

https://github.com/Xinyuan-LilyGO/TTGO-T-Display

https://github.com/neu-rah/ArduinoMenu

https://github.com/LennartHennigs/Button2

"Thanks for your great work"

12/05/2020. Presentation

Here another DSC over ESP32-TTGO, a low cost development tft board.

Support WiFi ,Bluetooth and Serial (USB).

Skysafari, Ascom and Stellarium compatible.

Support AS5600 (AS5048 encoders: soon). 

Quadrature encoders support are coming.

Encoders system can downloaded from :

https://www.thingiverse.com/xvedra/designs

for Orion XT8:  https://www.thingiverse.com/thing:4221923

for GSO 10":    https://www.thingiverse.com/thing:4221918

case:           https://www.thingiverse.com/thing:4358327

13/02/2022 Be aware!!!!!

Some Power Banks generate electrical noise and interferes with I2C communication.

Keep it away from wires and controller (ESP32).

(See commit file for updates)
