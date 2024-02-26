#ifndef __SHT21_H
#define __SHT21_H

#include "sht21_core.h"
#include "i2c.h"

#define SHT21_READ_TIMEOUT 1000
#define SHT21_SELFTEST_TEMP_THRESHOLD 0.3f
#define SHT21_SELFTEST_HUM_THRESHOLD 0.5f

// Note: Change this to your HAL I2C handler you are going to be using!
#define SHT21_I2C_HANDLE &hi2c1

extern HAL_StatusTypeDef sht21_last_error;

float SHT21_get_humidity(void);
float SHT21_get_temp(void);
SHT21_User_Reg_TypeDef SHT21_get_user_reg(void);
HAL_StatusTypeDef SHT21_update_user_reg(SHT21_User_Reg_TypeDef new_reg);
HAL_StatusTypeDef SHT21_reset(void);
SHT21_Error_TypeDef SHT21_selftest(void);

#endif // SHT21
