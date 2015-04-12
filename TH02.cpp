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
//           V1.10 2015-04-13 - changed to Wire library instead of I2C
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include <TH02.h>
#include <Wire.h>
#include <math.h>

// Class Constructor
TH02::TH02(uint8_t address)
{           
  _address = address; // I2C Module Address
  _last_temp = TH02_UNINITIALIZED_TEMP;  // Last measured temperature (for linearization)
  _last_rh = TH02_UNINITIALIZED_RH;      // Last measured RH
}

/* ======================================================================
Function: writeCommand
Purpose : write the "register address" value on I2C bus 
Input   : register address
          true if we need to release the bus after (default yes)
Output  : Arduino Wire library return code (0 if ok)
Comments: 
====================================================================== */
uint8_t TH02::writeCommand(uint8_t command, boolean release)
{ 
  Wire.beginTransmission(_address);
  Wire.write(command) ;
  return Wire.endTransmission(release);
}

/* ======================================================================
Function: writeRegister
Purpose : write a value on the designed register address on I2C bus 
Input   : register address
          value to write
Output  : Arduino Wire library return code (0 if ok)
Comments: 
====================================================================== */
uint8_t TH02::writeRegister(uint8_t reg, uint8_t value)
{   
  boolean ret = false;

  Wire.beginTransmission(_address);   
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission();  
}

/* ======================================================================
Function: readRegister
Purpose : read a register address value on I2C bus 
Input   : register address
          pointer where the return value will be filled
Output  : Arduino Wire library return code (0 if ok)
Comments: 
====================================================================== */
uint8_t TH02::readRegister(uint8_t reg, uint8_t * value)
{
  uint8_t ret ;

  // Send a register reading command
  // but DO NOT release the I2C bus
  ret = writeCommand(reg, false);

  if ( ret == 0) 
  {
    Wire.requestFrom( _address, 1);  

    if (Wire.available() != 1)
      // Other error as Wire library
      ret = 4;
    else
      // grab the value
      *value = Wire.read();  
  } 
    
  // Ok now we have finished
  Wire.endTransmission();
  return ret;
} 

/* ======================================================================
Function: getId
Purpose : Get device ID register  
Input   : pointer where the return value will be filled
Output  : Arduino Wire library return code (0 if ok)
Comments: -
====================================================================== */
uint8_t TH02::getId(uint8_t * pvalue)
{
  return (readRegister(TH02_ID, pvalue));
}

/* ======================================================================
Function: getStatus
Purpose : Get device status register  
Input   : pointer where the return value will be filled
Output  : Arduino Wire library return code (0 if ok)
Comments: 
====================================================================== */
uint8_t TH02::getStatus(uint8_t * pvalue)
{
  return (readRegister(TH02_STATUS, pvalue));
}

/* ======================================================================
Function: isConverting
Purpose : Indicate if a temperature or humidity conversion is in progress  
Input   : -
Output  : true if conversion in progress false otherwise
Comments: 
====================================================================== */
boolean TH02::isConverting(void)
{
  uint8_t status;
  // Get status and check RDY bit
  if ( getStatus(&status) == 0)
    if ( (status & TH02_STATUS_RDY) ==1 )
      return true;

  return false;
}

/* ======================================================================
Function: getConfig
Purpose : Get device configuration register  
Input   : pointer where the return value will be filled
Output  : Arduino Wire library return code (0 if ok)
Comments: 
====================================================================== */
uint8_t TH02::getConfig(uint8_t * pvalue)
{
  return (readRegister(TH02_CONFIG, pvalue));
}

/* ======================================================================
Function: setConfig
Purpose : Set device configuration register  
Input   : value to set
Output  : true if succeded, false otherwise
Comments: 
====================================================================== */
uint8_t TH02::setConfig(uint8_t config)
{
  return (writeRegister(TH02_CONFIG, config));
}

/* ======================================================================
Function: startTempConv
Purpose : Start a temperature conversion  
Input   : - fastmode true to enable fast conversion
          - heater true to enable heater
Output  : true if succeded, false otherwise
Comments: if heater enabled, it will not be auto disabled
====================================================================== */
uint8_t TH02::startTempConv(boolean fastmode, boolean heater)
{
  // init configuration register to start and temperature
  uint8_t config = TH02_CONFIG_START | TH02_CONFIG_TEMP;
  
  // set fast mode and heater if asked
  if (fastmode) config |= TH02_CONFIG_FAST;
  if (heater)   config |= TH02_CONFIG_HEAT;
  
  // write to configuration register
  return ( setConfig( config ) );
}

/* ======================================================================
Function: startRHConv
Purpose : Start a relative humidity conversion  
Input   : - fastmode true to enable fast conversion
          - heater true to enable heater
Output  : true if succeded, false otherwise
Comments: if heater enabled, it will not be auto disabled
====================================================================== */
uint8_t TH02::startRHConv(boolean fastmode, boolean heater)
{
  // init configuration register to start and no temperature (so RH)
  uint8_t config = TH02_CONFIG_START;
  
  // set fast mode and heater if asked
  if (fastmode) config |= TH02_CONFIG_FAST;
  if (heater)   config |= TH02_CONFIG_HEAT;
  
  // write to configuration register
  return ( setConfig( config ) );
}

/* ======================================================================
Function: waitEndConversion
Purpose : wait for a temperature or RH conversion is done  
Input   : 
Output  : delay in ms the process took. 
Comments: if return >= TH02_CONVERSION_TIME_OUT, time out occured
====================================================================== */
uint8_t TH02::waitEndConversion(void)
{
  // okay this is basic approach not so accurate
  // but avoid using long and millis()
  uint8_t time_out = 0;
  
  // loop until conversion done or duration >= time out
  while (isConverting() && time_out <= TH02_CONVERSION_TIME_OUT )
  {
    ++time_out;
    delay(1);
  }
  
  // return approx time of conversion
  return (time_out);
}

/* ======================================================================
Function: roundInt
Purpose : round a float value to int  
Input   : float value
Output  : int value rounded 
Comments: 
====================================================================== */
int16_t TH02::roundInt(float value)
{  

  // check positive number and do round
  if (value >= 0.0f)
    value = floor(value + 0.5f);
  else
    value = ceil(value - 0.5f);
    
  // return int value
  return (static_cast<int16_t>(value));
}

/* to avoid math library may I need to test something
   like that
float TH02::showDecimals(float x, int numDecimals) 
{
    int y=x;
    double z=x-y;
    double m=pow(10,numDecimals);
    double q=z*m;
    double r=round(q);

    return static_cast<double>(y)+(1.0/m)*r;
}
*/

/* ======================================================================
Function: getConversionValue
Purpose : return the last converted value to int * 10 to have 1 digit prec.  
Input   : float value
Output  : value rounded but multiplied per 10 or TH02_UNDEFINED_VALUE on err
Comments: - temperature and rh raw values (*100) are stored for raw purpose
          - the configuration register is checked to see if last conv was
            a temperature or humidity conversion
====================================================================== */
int16_t TH02::getConversionValue(void)
{
  int32_t result=0 ;
  uint8_t config;
  int16_t  ret = TH02_UNDEFINED_VALUE;
 
  // Prepare reading address of ADC data result
  if ( writeCommand(TH02_DATAh, false) == 0 )
  {
    // Read 2 bytes adc data result MSB and LSB from TH02
    Wire.requestFrom( (uint8_t) _address, (uint8_t) 2);

    // we got 2 bytes ?
    if (Wire.available() == 2)
    {
      // convert number
      result  = Wire.read() << 8;
      result |= Wire.read();

<<<<<<< HEAD
      // Get configuration to know what was asked last time
      if (getConfig(&config)==0)
      {
=======
  // Get configuration to know what was asked last time
  config = getConfig();
  
  // Error reading config ?
  if (config == TH02_I2C_ERR)
  {
    return TH02_UNDEFINED_VALUE;
  }
  
  // last conversion was temperature ?
  else  if( config & TH02_CONFIG_TEMP)
  {
    result >>= 2;  // remove 2 unused LSB bits
    result *= 100; // multiply per 100 to have int value with 2 decimal
    result /= 32;  // now apply datasheet formula
    result -= 5000;
    
    // now result contain temperature * 100
    // so 2134 is 21.34 C
    
    // Save raw value 
    temperature = result;
  }
  // it was RH conversion
  else
  {
    result >>= 4;  // remove 4 unused LSB bits
    result *= 100; // multiply per 100 to have int value with 2 decimal
    result /= 16;  // now apply datasheet formula
    result -= 2400;

    // now result contain humidity * 100
    // so 4567 is 45.67 % RH
    rh = result;
  }
>>>>>>> 3bbf7840a1bf4cef822943283e3229cd180a02ae

        // last conversion was temperature ?
        if( config & TH02_CONFIG_TEMP)
        {
          result >>= 2;  // remove 2 unused LSB bits
          result *= 100; // multiply per 100 to have int value with 2 decimal
          result /= 32;  // now apply datasheet formula
          if(result >= 5000)
          {
            result -= 5000;
          }
          else
          {
            result -= 5000;
            result = -result;
          }
          
          // now result contain temperature * 100
          // so 2134 is 21.34 C
          
          // Save raw value 
          _last_temp = result;
        }
        // it was RH conversion
        else
        {
          result >>= 4;  // remove 4 unused LSB bits
          result *= 100; // multiply per 100 to have int value with 2 decimal
          result /= 16;  // now apply datasheet formula
          result -= 2400;

          // now result contain humidity * 100
          // so 4567 is 45.67 % RH
          _last_rh = result;
        }

        // remember result value is multiplied by 10 to avoid float calculation later
        // so humidity of 45.6% is 456 and temp of 21.3 C is 213
        ret = roundInt(result/10.0f);
      }
    } // if got 2 bytes from I2C

    // Ok now we have finished with I2C, release
    Wire.endTransmission();

  } // if write command TH02_DATAh

  return ret;
}

/* ======================================================================
Function: getConpensatedRH
Purpose : return the compensated calulated humidity  
Input   : true if we want to round value to 1 digit precision, else 2
Output  : the compensed RH value (rounded or not)
Comments: 
====================================================================== */
int16_t TH02::getConpensatedRH(boolean round)
{
  float rhvalue  ;
  float rhlinear ;
  int16_t  ret = TH02_UNDEFINED_VALUE;

  // did we had a previous measure RH
  if (_last_rh != TH02_UNINITIALIZED_RH)
  {
    // now we're float restore real value RH value
    rhvalue = (float) _last_rh / 100.0 ;
    
    // apply linear compensation
    rhlinear = rhvalue - ((rhvalue*rhvalue) * TH02_A2 + rhvalue * TH02_A1 + TH02_A0);

    // correct value
    rhvalue = rhlinear;
    
    // do we have a initialized temperature value ?
    if (_last_temp != TH02_UNINITIALIZED_TEMP )
    {
      // Apply Temperature compensation
      // remember last temp was stored * 100
      rhvalue += ((_last_temp/100.0) - 30.0) * (rhlinear * TH02_Q1 + TH02_Q0);
    }
    
    // now get back * 100 to have int with 2 digit precision
    rhvalue *= 100;
    
    // do we need to round to 1 digit ?
    if (round)
    {
      // remember result value is multiplied by 10 to avoid float calculation later
      // so humidity of 45.6% is 456
      ret = roundInt(rhvalue/10.0f);
    }
    else
    {
      ret = (int16_t) rhvalue;
    }
  }
  
  return ret;
}

/* ======================================================================
Function: getLastRawRH
Purpose : return the raw humidity * 100  
Input   : 
Output  : int value (ie 4123 for 41.23%)
Comments: 
====================================================================== */
int32_t TH02::getLastRawRH(void)
{
  return _last_rh; 
}

/* ======================================================================
Function: getLastRawTemp
Purpose : return the raw temperature value * 100  
Input   : 
Output  : int value (ie 2124 for 21.24 C)
Comments: 
====================================================================== */
int32_t TH02::getLastRawTemp(void) 
{
  return _last_temp;
}

