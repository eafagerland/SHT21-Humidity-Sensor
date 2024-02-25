# SHT21-Humidity-Sensor

Universal driver for the SHT21 Temperature and Humidity Sensor written in C. Easy to implement for any I2C handler.

Create a request buffer with "SHT21_Request_Buf" where you pass your wanted command. Data can then be transmitted with your I2C handler.

The received data from the I2C is passed into the appropriate parser based on your command. For example if you created a SHT21_TEMP_MEASURE_HOLD request you will use the SHT21_Parse_Temp function.

# Examples

## Arduino
For the Arduino example copy sht21_core.c and sht21_core.h into your sketch folder.