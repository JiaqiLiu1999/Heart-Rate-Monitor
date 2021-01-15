
#ifndef WARP_BUILD_ENABLE_DEVMAX30102
#define WARP_BUILD_ENABLE_DEVMAX30102
#endif

void    initMAX30102(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer);
WarpStatus  writeSensorRegisterMAX30102(uint8_t deviceRegister, uint8_t payload, uint16_t menuI2cPullupValue);
WarpStatus  readSensorRegisterMAX30102(uint8_t deviceRegister, int numberOfBytes);
WarpStatus configureSensorMAX30102(uint8_t payload_IE1, uint8_t payload_IE2, uint8_t payload_FIFO_WR, uint8_t payload_OVF_COUNTER,uint8_t payload_FIFO_RD, uint8_t payload_FIFO_CON,uint8_t payload_Mode_CON,uint8_t payload_SP02_CON,uint8_t payload_LED1, uint8_t payload_LED2, uint16_t menuI2cPullupValue);
void    resetMAX30102(uint16_t menuI2cPullupValue);
int    printSensorDataMAX30102(bool hexModeFlag);