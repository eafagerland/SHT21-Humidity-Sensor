#include "sht21_arduino.h"

SHT21 sht21;

void setup() 
{
  sht21.init(); // Start the I2C for SHT21
  Serial.begin(9600);
  delay(100);

  // Perform a selftest to see if sensor is working (Takes 10seconds)
  if (sht21.selftest() == SHT21_OK)
    Serial.println("Selftest OK!\n");
  else
    Serial.println("Selftest failed!\n");
}

void loop() 
{
  // Get the temperature reading
  float temp = sht21.getTemp();
  // Get the humidity reading
  float hum = sht21.getHumidity();
  // Get the user register
  SHT21_User_Reg_TypeDef sht21User = sht21.getUserReg();

  // Print the temperature and humidity readings
  Serial.print("### Temp: ");
  Serial.print(temp);
  Serial.print(" | Humidity: ");
  Serial.print(hum);
  Serial.println(" ###");

  // Print the user register
  char serialBuffer[200];
  sprintf(serialBuffer, "\n#User Register\nResolution Bit0: %d\nResolution Bit1: %d\nOTP Reload: %d\nChip Heater: %d\nVoltage Status: %d\n",
                         sht21User.data.resolution_bit1,
                         sht21User.data.resolution_bit2,
                         sht21User.data.otp_reload,
                         sht21User.data.chip_heater,
                         sht21User.data.voltage_status);

  Serial.println(serialBuffer);

  delay(1000);
}
