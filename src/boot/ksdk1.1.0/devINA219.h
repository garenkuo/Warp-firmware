/* Written by Karen Guo */

#ifndef WARP_BUILD_ENABLE_DEVINA219
#define WARP_BUILD_ENABLE_DEVINA219
#endif

void		initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer);
WarpStatus	readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes);
WarpStatus	writeSensorRegisterINA219(uint8_t deviceRegister,
					uint8_t payloadBtye,
					uint16_t menuI2cPullupValue);
WarpStatus	configureSensorINA219(uint8_t payloadF_SETUP, uint8_t payloadCTRL_REG1, uint16_t menuI2cPullupValue);
// WarpStatus	readSensorSignalINA219(WarpTypeMask signal,
// 					WarpSignalPrecision precision,
// 					WarpSignalAccuracy accuracy,
// 					WarpSignalReliability reliability,
// 					WarpSignalNoise noise);
void		printSensorCurrentINA219(bool hexModeFlag);

// uint16_t value;
//
// // Sometimes a sharp load will reset the INA219, which will
// // reset the cal register, meaning CURRENT and POWER will
// // not be available ... avoid this by always setting a cal
// // value even if it's an unfortunate extra step
// writeSensorRegisterINA219(INA219_REG_CALIBRATION, ina219_calValue);
//
// // Now we can safely read the CURRENT register!
// readSensorRegisterINA219(INA219_REG_CURRENT, &value);
//
// return (int16_t)value;
