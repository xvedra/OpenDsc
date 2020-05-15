
****************  SCHEME ***********************

Board: ESP32 Dev Module
Partition escheme: No OTA (2 MB APP/2 MB SPIFFS)

************************************************
Can connect Wifi, Bluetooh and USB simultaneously
************************************************
** 14/05/2020 **********************************
Beta Version 1.0.0
Support AS5600 encoder at 3.3V power supply in 5V input.
AS5600 module has got pull-ups to 5V and ESP32 is not 5V tolerant.
In D1_R32 ESP32 board AS5600 module works at 5V but in TTGO-T-Display ther is some problem...However 5V pull-ups must be attach to 3.3V!
I will try to solve it soon.
Quadrature encoder needs 5V power supply and this board has 4.8V connected to USB.
For Battery use I will add a DcDC boost to solve it. Pull-ups must be attach to 3.3V too!

Add goals: 
1) AS5048 support
2) Quadrature encoder support

** 15/05/2020 **********************************
a) Solved resolution bug
b) Added USB for serial commands

Add goals:
a) Detect I2C encoders
b) Demo interface to test clients
