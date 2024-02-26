#include "sht21.h"

HAL_StatusTypeDef sht21_last_error = HAL_OK;

/********************************************************************************************
 *  Transmit the passed command to the SHT21 and reads the response. Response data is
 *  parsed in the get functions.
 *******************************************************************************************/
static HAL_StatusTypeDef SHT21_transmit_receive(UInt8* rx_buf, UInt8 len, SHT21_Commands_TypeDef cmd)
{
	// Create the struct with the passed command
	SHT21_Request_TypeDef sht21_request = SHT21_Request_Buf(cmd);

	/*
	 *  Shifting the address 1 bit to the left and then setting bit 0 to write(0)
	 *  Address will be 0b10000000 after.
	 */
	UInt8 address = (sht21_request.data.address << 1U);
	address &= ~(0x2U);

	UInt8 tx_buf = sht21_request.data.command;
	// Transmit the command
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(SHT21_I2C_HANDLE, address, &tx_buf, 1, SHT21_READ_TIMEOUT);

    // Wait for given time if performing temp or humidity readings
    switch (cmd)
    {
        case SHT21_TEMP_MEASURE_HOLD:
        HAL_Delay(90);
        break;
        case SHT21_RH_MEASURE_HOLD:
        HAL_Delay(40);
        break;
        default:
        break;
    }

	// Check if transmit was successful
	if (status != HAL_OK)
		return status;

	// Setting the read bit, Address will be 0b10000001 after.
	address |= (1U << 0U);

	status =  HAL_I2C_Master_Receive(SHT21_I2C_HANDLE, address, rx_buf, len, SHT21_READ_TIMEOUT);

	return status;
}

/********************************************************************************************
 *  Returns the humidity reading of the SHT21
 *******************************************************************************************/
float SHT21_get_humidity(void)
{
    UInt8 rx_buf[3] = {0};

    sht21_last_error = SHT21_transmit_receive(rx_buf, 3, SHT21_RH_MEASURE_HOLD);
    if (sht21_last_error == HAL_OK)
        return SHT21_Parse_RH(rx_buf);
    else
        return 0.0f;
}

/********************************************************************************************
 *  Returns the temperature reading of the SHT21
 *******************************************************************************************/
float SHT21_get_temp(void)
{
    UInt8 rx_buf[3] = {0};

    sht21_last_error = SHT21_transmit_receive(rx_buf, 3, SHT21_TEMP_MEASURE_HOLD);
    if (sht21_last_error == HAL_OK)
        return SHT21_Parse_Temp(rx_buf);
    else
        return 0.0f;
}

/********************************************************************************************
 *  Returns the user register of the SHT21
 *******************************************************************************************/
SHT21_User_Reg_TypeDef SHT21_get_user_reg(void)
{
    UInt8 rx_buf[1] = {0};
    SHT21_User_Reg_TypeDef reg = {0};

    sht21_last_error = SHT21_transmit_receive(rx_buf, 1, SHT21_READ_USER_REG);
    if (sht21_last_error == HAL_OK)
    	reg = SHT21_Parse_User_Reg(rx_buf);

    return reg;
}

/********************************************************************************************
 *  Updates the user register on the SHT21
 *******************************************************************************************/
HAL_StatusTypeDef SHT21_update_user_reg(SHT21_User_Reg_TypeDef new_reg)
{
    SHT21_Request_TypeDef sht21_request = SHT21_Request_Buf(SHT21_WRITE_USER_REG);
 	
    /*
    *  Shifting the address 1 bit to the left and then setting bit 0 to write(0)
    *  Address will be 0b10000000 after.
    */
    UInt8 address = (sht21_request.data.address << 1U);
    address &= ~(0x2U);

    UInt8 tx_buf[2];
    tx_buf[0] = sht21_request.data.command;
    tx_buf[1] = new_reg.reg;
    sht21_last_error = HAL_I2C_Master_Transmit(SHT21_I2C_HANDLE, address, tx_buf, 2, SHT21_READ_TIMEOUT);
    return sht21_last_error;
}

/********************************************************************************************
 *  Send a reset command to the SHT21 for a soft reset. This will reset the SHT21 to
 *  default settings, except the heat enabled bit.
 *******************************************************************************************/
HAL_StatusTypeDef SHT21_reset(void)
{
    SHT21_Request_TypeDef sht21_request = SHT21_Request_Buf(SHT21_SOFT_RESET);

    UInt8 tx_buf = sht21_request.data.command;

    /*
    *  Shifting the address 1 bit to the left and then setting bit 0 to write(0)
    *  Address will be 0b10000000 after.
    */
    UInt8 address = (sht21_request.data.address << 1U);
    address &= ~(0x2U);

    sht21_last_error = HAL_I2C_Master_Transmit(SHT21_I2C_HANDLE, address, &tx_buf, 1, SHT21_READ_TIMEOUT);
    return sht21_last_error;
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
SHT21_Error_TypeDef SHT21_selftest(void)
{
    // Store the current temp and hum values
    float tempAtStart = SHT21_get_temp();
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    float humAtStart = SHT21_get_humidity();
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    // Enable the heater
    SHT21_User_Reg_TypeDef sht21User = SHT21_get_user_reg(); // Read the current user register
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    sht21User.data.chip_heater = 1U;    // Set the chip heater enabled
    SHT21_update_user_reg(sht21User);   // Update SHT21 with the modified register
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    // Wait for temp to rise and humidity to fall
    HAL_Delay(10000);

    // Get the current readings
    float tempAfterTest = SHT21_get_temp();
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    float  humAfterTest = SHT21_get_humidity();
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    // Check if the changes after heater has been on had its initial values
    // change above thresholds
    // Variable to store status
    HAL_StatusTypeDef status = SHT21_OK;
    if (tempAfterTest - tempAtStart > SHT21_SELFTEST_TEMP_THRESHOLD &&
        humAtStart - humAfterTest > SHT21_SELFTEST_HUM_THRESHOLD)
    {
        status = SHT21_OK;
    }
    else 
        status = SHT21_SELFTEST_FAILED;

    // Disable the heater
    sht21User = SHT21_get_user_reg();
    if (sht21_last_error != HAL_OK)
        return sht21_last_error;

    sht21User.data.chip_heater = 0U;
    SHT21_update_user_reg(sht21User);

    return status;
}
