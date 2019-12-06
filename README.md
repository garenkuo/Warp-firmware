Name: Karen Guo
College: St. Catharine's (Caltech Exchange)
CRSid: kg472

This fork of the Warp-firmare respository modifies the base Warp-firmare code
to implement a motion sensing bike light.

Comparison with the base repository can be found at:
https://github.com/physical-computation/Warp-firmware/compare/master...garenkuo:master

The following high-level changes were made to the base repository:
- added methods getSensorXMMA8451Q, getSensorYMMA8451Q, and getSensorZMMA8451Q
  in devMMA8451Q.c to use I2C to read data from the sensor at each axis of the
  accelerometer.
- added methods devSSD1331Stop, devSSD1331LeftTurn, devSSD1331RightTurn,
  devSSD1331Clear in decSSD1331.c to draw shapes on the OLED display for when
  a turn signal is recognized.
- modified main loop in warp-kl03-ksdk1.1-boot.c to configure the MMA8451Q and
  initialize the SSD1331 (OLED driver) and then continuously store data and
  look for variations in the incoming accelerometer data. If a hand signal is
  detected, the appropriate symbol is outputted to the OLED display.

For data collection purposes, you can uncomment out the parts that say "print for CSV"
to print CSV formatted x, y, z accelerometer data to the JLink RTT Client.
