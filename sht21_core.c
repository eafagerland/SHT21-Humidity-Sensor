/********************************************************************************************
 *  Filename: sht21.c
 *  Author: Erik Fagerland
 *  Created On: 24/02/2024
 * 
 *  Brief:
 *  Implementation of SHT21
 * 
 *******************************************************************************************/
#include "sht21_core.h"

/********************************************************************************************
 *  Calculate the checksum on passed data pointer. Length is the number of bytes passed.
 *  It will compare the calculated checksum with the passed one and return error if
 *  mismatch. 
 *******************************************************************************************/
static UInt16 SHT21_Check_Crc(UInt8* buf, UInt8 length, UInt8 checksum)
{
    UInt8 crc = 0;	

    //calculates 8-Bit checksum with given polynomial
    for (UInt8 i = 0; i < length; i++)
    {  
        crc ^= (buf[i]);
        for (UInt8 bit = 8; bit > 0; --bit)
        { 
            if (crc & 0x80) 
                crc = (crc << 1) ^ SHT21_CRC_POLYNOMIAL;
            else crc = (crc << 1);
        }
    }
    if (crc != checksum) return SHT21_CHECKSUM_ERROR;
    else return 0;
}

/********************************************************************************************
 *  Creates a data structure to be sent on I2C to the SHT21.
 *******************************************************************************************/
SHT21_Request_TypeDef SHT21_Request_Buf(SHT21_Commands_TypeDef cmd)
{
    UInt8 read_write_bit = SHT21_I2C_READ_BIT;

    // Set write bit if updating user register
    if (cmd == SHT21_WRITE_USER_REG)
        read_write_bit = SHT21_I2C_WRITE_BIT;

    // Set address and add the write/read bit
    UInt8 address = SHT21_I2C_ADDRESS;

    SHT21_Request_TypeDef buf;
    buf.reg |= ((address & 0x7FU) << 9U);       // Set the address
    buf.reg |= ((read_write_bit & 0x1U) << 8U); // Set the read/write bit
    buf.reg |= (cmd & 0xFFU);                   // Set the command

    return buf;
}

/********************************************************************************************
 *  Parses the 2 byte temp value received from SHT21
 *******************************************************************************************/
float SHT21_Parse_Temp(UInt8* buf)
{
    UInt16 reading = ((buf[0] << 8) | buf[1]);
    reading &= ~(0x3U); // Mask out the status bits

    // Check checksum and return error if present
    if (SHT21_Check_Crc(buf, 2, buf[2]) != 0)
        return SHT21_CHECKSUM_ERROR;

    // Calculate the ADC value to temperature
    float temp = -46.85f + 175.72f * ((float)reading / (float)65536);

    return temp;
}

/********************************************************************************************
 *  Parses the 2 byte RH value received from SHT21
 *******************************************************************************************/
float SHT21_Parse_RH(UInt8* buf)
{
    UInt16 reading = ((buf[0] << 8) | buf[1]);
    reading &= ~(0x3U); // Mask out the status bits

    // Check checksum and return error if present
    if (SHT21_Check_Crc(buf, 2, buf[2]) != 0)
        return SHT21_CHECKSUM_ERROR;

    // Calculate the ADC value to humidity
    float humidity = -6.0f + 125.0f * ((float)reading / (float)65536);

    return humidity;
}

/********************************************************************************************
 *  Parses the 1 byte User Register received from SHT21
 *******************************************************************************************/
SHT21_User_Reg_TypeDef SHT21_Parse_User_Reg(UInt8* buf)
{
    SHT21_User_Reg_TypeDef sht21;
    sht21.reg  |= ((UInt8)buf[0] & SHT21_MEAS_RESOLUTION_BIT1);
    sht21.reg  |= ((UInt8)buf[0] & SHT21_MEAS_RESOLUTION_BIT2);
    sht21.reg  |= ((UInt8)buf[0] & SHT21_ENABLE_CHIP_HEATER);
    sht21.reg  |= ((UInt8)buf[0] & SHT21_DISABLE_OTP_RELOAD);
    sht21.reg  |= ((UInt8)buf[0] & SHT21_STATUS);
    return sht21;
}