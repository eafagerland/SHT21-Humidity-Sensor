/********************************************************************************************
 *  Filename: sht21_arduino.h
 *  Author: Erik Fagerland
 *  Created On: 25/02/2024
 * 
 *  Brief:
 *  Universal driver in C for SHT21.
 * 
 *******************************************************************************************/
#pragma once

#include "sht21_core.h"

#define SHT21_READ_TIMEOUT 1000
#define SHT21_SELFTEST_TEMP_THRESHOLD 0.5f
#define SHT21_SELFTEST_HUM_THRESHOLD 0.5f

class SHT21
{
public:
  SHT21(){};
  void init();
  float getHumidity(SHT21_Error_TypeDef* error = nullptr);
  float getTemp(SHT21_Error_TypeDef* error = nullptr);
  SHT21_User_Reg_TypeDef getUserReg(SHT21_Error_TypeDef* error = nullptr);
  void updateUserReg(SHT21_User_Reg_TypeDef new_reg);
  void reset();
  SHT21_Error_TypeDef selftest();

private:
  SHT21_Error_TypeDef transmitReceiveSht21(UInt8* rxBuf, UInt8 len, SHT21_Commands_TypeDef cmd);
  SHT21_Error_TypeDef readSht21(UInt8* rxBuf, UInt8 len);
};