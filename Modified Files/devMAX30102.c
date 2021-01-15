#include <stdlib.h>

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"
//#include "devMAX30102.h"


extern volatile WarpI2CDeviceState	deviceMAX30102State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;

void
initMAX30102(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer)
{
	deviceStatePointer->i2cAddress	= i2cAddress;
	deviceStatePointer->signalType	= (	kWarpTypeMaskRED|kWarpTypeMaskIR);
	return;
}

WarpStatus
writeSensorRegisterMAX30102(uint8_t deviceRegister, uint8_t payload, uint16_t menuI2cPullupValue)
{
	uint8_t		payloadByte[1], commandByte[1];
	i2c_status_t	status;

	switch (deviceRegister)
	{
		case 0x02: case 0x03: case 0x04: case 0x05: 
		case 0x06: case 0x07: case 0x08: case 0x09: 
		case 0x0A: case 0x0C: case 0x0D: case 0x11: 
		case 0x12: case 0x21: 
		{
			/* OK */
			break;
		}
		
		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}

	i2c_device_t slave =
	{
		.address = deviceMAX30102State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	commandByte[0] = deviceRegister;
	payloadByte[0] = payload;
	status = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C instance */,
							&slave,
							commandByte,
							1,
							payloadByte,
							1,
							gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		SEGGER_RTT_WriteString(0, "write----,");
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorMAX30102(uint8_t payload_IE1, uint8_t payload_IE2, uint8_t payload_FIFO_WR, uint8_t payload_OVF_COUNTER,uint8_t payload_FIFO_RD, 
	uint8_t payload_FIFO_CON,uint8_t payload_Mode_CON,uint8_t payload_SP02_CON,uint8_t payload_LED1, uint8_t payload_LED2, uint16_t menuI2cPullupValue)
{
	WarpStatus	Status1, Status2, Status3, Status4, Status5, Status6, Status7, Status8, Status9, Status10;

	Status1 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102InterruptEnable1,payload_IE1,menuI2cPullupValue);
	Status2 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102InterruptEnable2,payload_IE2,menuI2cPullupValue);
	Status3 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102FIFO_WR_PTR,payload_FIFO_WR,menuI2cPullupValue);
	Status4 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102OVF_COUNTER,payload_OVF_COUNTER,menuI2cPullupValue);
	Status5 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102FIFO_RD_PTR,payload_FIFO_RD,menuI2cPullupValue);
	Status6 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102FIFO_CON,payload_FIFO_CON,menuI2cPullupValue);
	Status7 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102Mode_CON,payload_Mode_CON,menuI2cPullupValue);
	Status8 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102SP02_CON,payload_SP02_CON,menuI2cPullupValue);
	Status9 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102LED1_PA,payload_LED1,menuI2cPullupValue);
	Status10 = writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102LED2_PA,payload_LED2,menuI2cPullupValue);

	return (Status1 | Status2 | Status3 | Status4 | Status5 | Status6 | Status7 | Status8 | Status9 | Status10);
}

WarpStatus
readSensorRegisterMAX30102(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	status;


	USED(numberOfBytes);
	switch (deviceRegister)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: 
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0A: case 0x0C: 
		case 0x0D: case 0x11: case 0x12: case 0x1F: 
		case 0x20: case 0x21: case 0xFE: case 0xFF:
		{
			/* OK */
			break;
		}
		
		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}


	i2c_device_t slave =
	{
		.address = deviceMAX30102State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};


	cmdBuf[0] = deviceRegister;

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceMAX30102State.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		SEGGER_RTT_WriteString(0, " read----,");
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
resetMAX30102(uint16_t menuI2cPullupValue)
{
	writeSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102Mode_CON,0x40,menuI2cPullupValue);//reset configuration
}

int
printSensorDataMAX30102(bool hexModeFlag)
{
	WarpStatus	i2cReadStatus;
	uint32_t	readSensorRegisterValueByte;
	uint32_t	readSensorRegisterValueByte3;
	uint32_t	readSensorRegisterValueByte2;
	uint32_t	readSensorRegisterValueByte1;;
	int32_t		readSensorRegisterValueCombined1;

	i2cReadStatus = readSensorRegisterMAX30102(kWarpSensorOutRegisterMAX30102FIFO_DATA, 3); // read 3 byte from FIFO, which are raw value from RED LED
	readSensorRegisterValueByte3 = deviceMAX30102State.i2cBuffer[0];
	readSensorRegisterValueByte2 = deviceMAX30102State.i2cBuffer[1];
	readSensorRegisterValueByte1 = deviceMAX30102State.i2cBuffer[2];
	readSensorRegisterValueCombined1 = ((readSensorRegisterValueByte3 << 16) | (readSensorRegisterValueByte2 << 8) | (readSensorRegisterValueByte1));
    readSensorRegisterValueCombined1 = readSensorRegisterValueCombined1 & 0x03ffff; /* value of [23:18] is unused*/

	/*i2cReadStatus = readSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102FIFO_RD_PTR,1); // check for FIFO read point
	readSensorRegisterValueByte = deviceMAX30102State.i2cBuffer[0];

	if (i2cReadStatus != kWarpStatusOK)
	{
		SEGGER_RTT_WriteString(0, " NO----,");
	}
	else
	{
		if (hexModeFlag)
		{
			SEGGER_RTT_printf(0, " 0x%02x 0x%02x 0x%02x,", readSensorRegisterValueByte3, readSensorRegisterValueByte2, readSensorRegisterValueByte1);
		}
		else
		{
			SEGGER_RTT_printf(0, "R= %d, ", readSensorRegisterValueByte);
		}
	}
	i2cReadStatus = readSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102FIFO_WR_PTR,1); // check for FIFO Write Point
	readSensorRegisterValueByte = deviceMAX30102State.i2cBuffer[0];

	if (i2cReadStatus != kWarpStatusOK)
	{
		SEGGER_RTT_WriteString(0, " NO----,");
	}
	else
	{
		if (hexModeFlag)
		{
			SEGGER_RTT_printf(0, " 0x%02x 0x%02x 0x%02x,", readSensorRegisterValueByte3, readSensorRegisterValueByte2, readSensorRegisterValueByte1);
		}
		else
		{
			SEGGER_RTT_printf(0, "W= %d, ", readSensorRegisterValueByte);
		}
	}*/

/*

	i2cReadStatus = readSensorRegisterMAX30102(kWarpSensorConfigurationRegisterMAX30102OVF_COUNTER,1); // check for overflow value
	readSensorRegisterValueByte = deviceMAX30102State.i2cBuffer[0];

	if (i2cReadStatus != kWarpStatusOK)
	{
		SEGGER_RTT_WriteString(0, " NO----,");
	}
	else
	{
		if (hexModeFlag)
		{
			SEGGER_RTT_printf(0, " 0x%02x,", readSensorRegisterValueByte);
		}
		else
		{
			SEGGER_RTT_printf(0, " %d", readSensorRegisterValueByte);
		}
	}*/

	return readSensorRegisterValueCombined1;
	

}
    
   