// **********************************************************************************
// Driver definition for HopeRF TH02 temperature and humidity sensor
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// For any explanation see TH02 sensor information at
// http://www.hoperf.com/sensor/app/TH02.htm
//
// Code based on following datasheet
// http://www.hoperf.com/upload/sensor/TH02_V1.1.pdf 
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2014-07-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#ifndef TH02_H
#define TH02_H

#include <Arduino.h>            //assumes Arduino IDE v1.0 or greater

// TH02 I2C Device address
#define TH02_I2C_ADDR 0x40

// TH02 Registers addresses
#define TH02_STATUS 0
#define TH02_DATAh  1
#define TH02_DATAl  2
#define TH02_CONFIG 3
#define TH02_ID     17

// TH02 custom error code return function
#define TH02_I2C_ERR 0xFF

// Unititialized values (arbitrary)
#define TH02_UNINITIALIZED_TEMP 55555 // int32_t internal value 
#define TH02_UNINITIALIZED_RH   1111  // int32_t internal value
#define TH02_UNDEFINED_VALUE    12345 // int16_t returned value

// we decide error if conversion is >= 50ms 
#define TH02_CONVERSION_TIME_OUT  50

// Bit definition of TH02 registers values
#define TH02_STATUS_RDY    0x01

#define TH02_CONFIG_START  0x01
#define TH02_CONFIG_HEAT   0x02
#define TH02_CONFIG_TEMP   0x10
#define TH02_CONFIG_HUMI   0x00
#define TH02_CONFIG_FAST   0x20

// THO2 Linearization Coefficients
#define TH02_A0   -4.7844
#define TH02_A1    0.4008
#define TH02_A2   -0.00393

// TH02 Temperature compensation Linearization Coefficients
#define TH02_Q0   0.1973
#define TH02_Q1   0.00237

class TH02 { 
  public:

    uint8_t getId(void);
    uint8_t getStatus(void);
    bool    isConverting(void);
    uint8_t waitEndConversion(void);
    uint8_t getConfig(void);
    bool    setConfig(uint8_t config);
    bool    startTempConv(bool fastmode = false, bool heater = false);
    bool    startRHConv(bool fastmode = false, bool heater = false);
    int16_t roundInt(float value);
    int16_t getConversionValue(void);
    int16_t getConpensatedRH(bool round);
    int32_t getLastRawRH(void);
    int32_t getLastRawTemp(void);

  protected:
  
    int16_t temperature; // Last measured temperature (for linearization)
    int16_t rh;          // Last mesuared RH
};

#endif
