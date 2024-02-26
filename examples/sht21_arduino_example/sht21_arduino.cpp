/********************************************************************************************
 *  Filename: sht21_arduino.cpp
 *  Author: Erik Fagerland
 *  Created On: 25/02/2024
 * 
 *  Brief:
 *  Universal driver in C for SHT21.
 * 
 *******************************************************************************************/
#include "sht21_arduino.h"
#include <Wire.h>
#include <Arduino.h>

/********************************************************************************************
 *  Initliazes the Arduino I2C, to be called in setup()
 *******************************************************************************************/
void SHT21::init()
{
  Wire.begin();
}

/********************************************************************************************
 *  Returns the humidity reading of the SHT21
 *******************************************************************************************/
float SHT21::getHumidity(SHT21_Error_TypeDef* error)
{
  UInt8 rxBuf[3] = {0};

  if (error == nullptr) // No error checking enabled
  {
    if (transmitReceiveSht21(rxBuf, 3, SHT21_RH_MEASURE_HOLD) == SHT21_OK)
      return SHT21_Parse_RH(rxBuf);
  }
  else
  {
    *error = transmitReceiveSht21(rxBuf, 3, SHT21_RH_MEASURE_HOLD);
    if (*error == SHT21_OK)
      return SHT21_Parse_RH(rxBuf);
  }
  return 0.0f;
}

/********************************************************************************************
 *  Returns the temperature reading of the SHT21
 *******************************************************************************************/
float SHT21::getTemp(SHT21_Error_TypeDef* error)
{
  UInt8 rxBuf[3] = {0};
  if (error == nullptr) // No error checking enabled
  {
    if (transmitReceiveSht21(rxBuf, 3, SHT21_TEMP_MEASURE_HOLD) == SHT21_OK)
      return SHT21_Parse_Temp(rxBuf);
  }
  else
  {
    *error = transmitReceiveSht21(rxBuf, 3, SHT21_TEMP_MEASURE_HOLD);
      if (*error == SHT21_OK)
        return SHT21_Parse_Temp(rxBuf);
  }
  return 0.0f;
}

/********************************************************************************************
 *  Returns the user register of the SHT21
 *******************************************************************************************/
SHT21_User_Reg_TypeDef SHT21::getUserReg(SHT21_Error_TypeDef* error)
{
  UInt8 rxBuf[1] = {0};
  if (error == nullptr) // No error checking enabled
  {
    if (transmitReceiveSht21(rxBuf, 1, SHT21_READ_USER_REG) == SHT21_OK)
      return SHT21_Parse_User_Reg(rxBuf);
  }
  else
  {
    *error = transmitReceiveSht21(rxBuf, 1, SHT21_READ_USER_REG);
    if (*error == SHT21_OK)
      return SHT21_Parse_User_Reg(rxBuf);
  }
  return SHT21_User_Reg_TypeDef{0};
}

/********************************************************************************************
 *  Updates the user register on the SHT21
 *******************************************************************************************/
void SHT21::updateUserReg(SHT21_User_Reg_TypeDef new_reg)
{
  SHT21_Request_TypeDef sht21_request = SHT21_Request_Buf(SHT21_WRITE_USER_REG);
  Wire.beginTransmission(sht21_request.data.address);
  Wire.write(sht21_request.data.command);
  Wire.write(new_reg.reg);
  Wire.endTransmission();
}

/********************************************************************************************
 *  Send a reset command to the SHT21 for a soft reset. This will reset the SHT21 to
 *  default settings, except the heat enabled bit.
 *******************************************************************************************/
void SHT21::reset()
{
  SHT21_Request_TypeDef sht21_request = SHT21_Request_Buf(SHT21_SOFT_RESET);
  Wire.beginTransmission(sht21_request.data.address);
  Wire.write(sht21_request.data.command);
  Wire.endTransmission();
}

/********************************************************************************************
 *  Runs a function test on the SHT21. Before starting it will store the current temperature
 *  and humidity values. It then turns on the heating element. Wait some time and then check
 *  if the temperature has risen and humidity has decreased to certain threshold.
 *
 *  If any problems on the communication if SHT21 it will also return error. 
 *
 *  It is important the chip is at a stable temperature before starting this test.
 *******************************************************************************************/
SHT21_Error_TypeDef SHT21::selftest()
{
  // Variable to store status
  SHT21_Error_TypeDef error = SHT21_OK;

  // Store the current temp and hum values
  float tempAtStart = getTemp(&error);
  if (error != SHT21_OK)
    return error;

  float humAtStart = getHumidity(&error);
  if (error != SHT21_OK)
    return error;

  // Enable the heater
  SHT21_User_Reg_TypeDef sht21User = getUserReg(&error); // Read the current user register
  if (error != SHT21_OK)
    return error;

  sht21User.data.chip_heater = 1U;                 // Set the chip heater enabled
  updateUserReg(sht21User);                        // Update SHT21 with the modified register

  // Wait for temp to rise and humidity to fall
  delay(10000);

  // Get the current readings
  float tempAfterTest = getTemp(&error);
  if (error != SHT21_OK)
    return error;

  float  humAfterTest = getHumidity(&error);
  if (error != SHT21_OK)
    return error;

  // Check if the changes after heater has been on had its initial values
  // change above thresholds
  if (tempAfterTest - tempAtStart > SHT21_SELFTEST_TEMP_THRESHOLD &&
      humAtStart - humAfterTest > SHT21_SELFTEST_HUM_THRESHOLD)
  {
    error = SHT21_OK;
  }
  else 
    error = SHT21_SELFTEST_FAILED;

  // Disable the heater
  sht21User = getUserReg(&error);
  if (error != SHT21_OK)
    return error;

  sht21User.data.chip_heater = 0U;
  updateUserReg(sht21User);

  return error;
}

/********************************************************************************************
 *  Transmit the passed command to the SHT21 and reads the response. Response data is
 *  parsed in the get functions.
 *******************************************************************************************/
SHT21_Error_TypeDef SHT21::transmitReceiveSht21(UInt8* rxBuf, UInt8 len, SHT21_Commands_TypeDef cmd)
{
  SHT21_Error_TypeDef status = SHT21_OK;
  SHT21_Request_TypeDef sht21_request = SHT21_Request_Buf(cmd);
  Wire.beginTransmission(sht21_request.data.address);
  Wire.write(sht21_request.data.command);
  Wire.endTransmission();

  // Wait for given time if performing temp or humidity readings
  switch (cmd)
  {
    case SHT21_TEMP_MEASURE_HOLD:
    delay(90);
    break;
    case SHT21_RH_MEASURE_HOLD:
    delay(40);
    break;
    default:
    break;
  }

  Wire.requestFrom((int)sht21_request.data.address, (int)len);
  status = readSht21(rxBuf, len);
  return status;
}

/********************************************************************************************
 *  Reads the I2C bus and stores the data into the passed pointer to buffer.
 *  It will stay in loop until all bytes are read or a timeout occurs.
 *******************************************************************************************/
SHT21_Error_TypeDef SHT21::readSht21(UInt8* rxBuf, UInt8 len)
{
  UInt8 bytesReceived = 0;
  UInt32 millisStarted = millis();
  bool isTimeout = false;

  // Stay in loop until received correct amount of bytes, returns error if timeout
  while (Wire.available() && !isTimeout)
  {
    UInt8 readByte = Wire.read();
    rxBuf[bytesReceived] = readByte;
    bytesReceived++;
  }

  // Return timeout error if timeout occured
  if (isTimeout)
    return SHT21_TIME_OUT_ERROR;

  return SHT21_OK;
}