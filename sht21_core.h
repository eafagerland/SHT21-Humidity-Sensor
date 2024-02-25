/********************************************************************************************
 *  Filename: sht21.h
 *  Author: Erik Fagerland
 *  Created On: 24/02/2024
 * 
 *  Brief:
 *  Universal driver in C for SHT21.
 * 
 *******************************************************************************************/
#ifndef __SHT21_CORE__H
#define __SHT21_CORE__H

#define UInt32 		unsigned int
#define UInt16 		unsigned short
#define UInt8 		unsigned char

#define SHT21_I2C_ADDRESS           (0x40U)
#define SHT21_I2C_READ_BIT          (1U)
#define SHT21_I2C_WRITE_BIT         (0U)

// Definitions for user register
#define SHT21_MEAS_RESOLUTION_BIT1  (1U << 7U)
#define SHT21_MEAS_RESOLUTION_BIT2  (1U << 0U)
#define SHT21_STATUS                (1U << 6U)
#define SHT21_ENABLE_CHIP_HEATER    (1U << 2U)
#define SHT21_DISABLE_OTP_RELOAD    (1U << 1U)

#define SHT21_CRC_POLYNOMIAL        (0x131)  //P(x)=x^8+x^5+x^4+1 = 100110001

/********************************************************************************************
 *  User Register of the SHT21 module. Unioned for direct register access.
 * 
 *  Bit 7,0     : Measurement resolution            Default: 00
 *                  00 - RH:12bit   T:14bit         
 *                  01 - RH:8bit    T:12bit
 *                  10 - RH:10bit   T:13bit
 *                  11 - RH:11bit   T:11bit
 *  
 *  Bit 6       : Status: End of battery            Default: 0
 *                  0 - VDD > 2.25V
 *                  1 - VDD < 2.25V
 * 
 *  Bit 3,4,5   : Reserved (DO NOT CHANGE)
 * 
 *  Bit 2       : Enable on-chip heater             Default: 0
 * 
 *  Bit 1       : Disable OTP Reload                Default: 1
 *******************************************************************************************/
typedef union 
{
    struct
    {
        UInt8 resolution_bit1   : 1;
        UInt8 otp_reload        : 1;
        UInt8 chip_heater       : 1;
        UInt8 reserved          : 3;
        UInt8 voltage_status    : 1;
        UInt8 resolution_bit2   : 1;
    } data;
    UInt8 reg;
} SHT21_User_Reg_TypeDef;

typedef union
{
    struct
    {
        UInt8 command           : 8;
        UInt8 read_write_bit    : 1;
        UInt8 address           : 7;
    } data;
    UInt16 reg;
} SHT21_Request_TypeDef;

/********************************************************************************************
 *  Enum containing all the commands that can be used with the SHT21
 *******************************************************************************************/
typedef enum
{
    SHT21_TEMP_MEASURE_HOLD     = (0xE3U),
    SHT21_RH_MEASURE_HOLD       = (0xE5U),
    SHT21_TEMP_MEASURE          = (0xF3U),
    SHT21_RH_MEASURE            = (0xF5U),
    SHT21_WRITE_USER_REG        = (0xE6U),
    SHT21_READ_USER_REG         = (0xE7U),
    SHT21_SOFT_RESET            = (0xFEU)
} SHT21_Commands_TypeDef;

/********************************************************************************************
 *  Enum for error/status types used
 *******************************************************************************************/
typedef enum
{
    SHT21_OK                    = (0x00U),
    SHT21_ACK_ERROR             = (0x01U),
    SHT21_TIME_OUT_ERROR        = (0x02U),
    SHT21_CHECKSUM_ERROR        = (0x04U),
    SHT21_UNIT_ERROR            = (0x08U),
    SHT21_SELFTEST_FAILED       = (0x09U)
} SHT21_Error_TypeDef;

#ifdef __cplusplus
extern "C" {
#endif

SHT21_Request_TypeDef SHT21_Request_Buf(SHT21_Commands_TypeDef cmd);
float SHT21_Parse_Temp(UInt8* buf);
float SHT21_Parse_RH(UInt8* buf);
SHT21_User_Reg_TypeDef SHT21_Parse_User_Reg(UInt8* buf);

#ifdef __cplusplus
}
#endif

#endif // __SHT21_CORE__H