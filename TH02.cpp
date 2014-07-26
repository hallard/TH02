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
#include <TH02.h>
#include <I2C.h>
#include <math.h>

int32_t temperature = TH02_UNINITIALIZED_TEMP;  // Last measured temperature (for linearization)
int32_t rh = TH02_UNINITIALIZED_RH;             // Last measured RH


/* ======================================================================
Function: getId
Purpose : Get device ID register  
Input   : -
Output  : ID register (id is in 4 MSB bits)
Comments: 
====================================================================== */
uint8_t TH02::getId(void)
{
  // Read ID register
  if ( I2c.read(TH02_I2C_ADDR, TH02_ID, 1) == 0 )
  {
    // Got one byte ?
    if (I2c.available() == 1)
    {
      // sound we have the correct data
      return (I2c.receive());
    }
  }

  // not successfull 
  return 0;
}


/* ======================================================================
Function: getStatus
Purpose : Get device status register  
Input   : -
Output  : Status register
Comments: 
====================================================================== */
uint8_t TH02::getStatus(void)
{
  // Read Status register
  if ( I2c.read(TH02_I2C_ADDR, TH02_STATUS, 1) == 0 )
  {
    // Got one byte ?
    if (I2c.available() == 1)
    {
      // sound we have the correct data
      return (I2c.receive());
    }
  }
    
  // had a problem fake all bit at 1 (conversion in progress)
  return TH02_I2C_ERR;
}

/* ======================================================================
Function: isConverting
Purpose : Indicate if a temperature or humidity conversion is in progress  
Input   : -
Output  : true if conversion in progress false otherwise
Comments: 
====================================================================== */
bool TH02::isConverting(void)
{
  // Get status and check RDY bit
  if ((getStatus() & TH02_STATUS_RDY)==1)
    return true;
  else
    return false;
}

/* ======================================================================
Function: getConfig
Purpose : Get device configuration register  
Input   : -
Output  : configuration register
Comments: 
====================================================================== */
uint8_t TH02::getConfig(void)
{
  // Read Status register
  if ( I2c.read(TH02_I2C_ADDR, TH02_CONFIG, 1) == 0 )
  {
    // Got one byte ?
    if (I2c.available() == 1)
    {
      // sound we have the correct data
      return (I2c.receive());
    }
  }
    
  // had a problem fake a config all bit to 1
  // should never happen to have config at 0xFF
  return TH02_I2C_ERR;
}

/* ======================================================================
Function: setConfig
Purpose : Set device configuration register  
Input   : value to set
Output  : true if succeded, false otherwise
Comments: 
====================================================================== */
bool TH02::setConfig(uint8_t config)
{
  // Write the value to the configuration register
  if ( I2c.write( (uint8_t) TH02_I2C_ADDR, (uint8_t) TH02_CONFIG, (uint8_t) config ) == 0 )
    return true;
  else
    return false;
}

/* ======================================================================
Function: startTempConv
Purpose : Start a temperature conversion  
Input   : - fastmode true to enable fast conversion
          - heater true to enable heater
Output  : true if succeded, false otherwise
Comments: if heater enabled, it will not be auto disabled
====================================================================== */
bool TH02::startTempConv(bool fastmode, bool heater)
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
bool TH02::startRHConv(bool fastmode, bool heater)
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

/* ======================================================================
Function: getConversionValue
Purpose : return the last converted value to int * 10 to have 1 digit prec.  
Input   : float value
Output  : int value rounded but multiplied per 10
Comments: - temperature and rh raw values (*100) are stored for raw purpose
          - the configuration register is checked to see if last conv was
            a temperature or humidity conversion
====================================================================== */
int16_t TH02::getConversionValue(void)
{
  int32_t result=0 ;
  uint8_t  config;
 
  // Read 2 bytes adc data MSB and LSB from TH02
  if ( I2c.read(TH02_I2C_ADDR, TH02_DATAh, 2) == 0 )
  {
    // we got 2 bytes ?
    if (I2c.available() == 2)
    {
      // convert number
      result = I2c.receive() << 8;
      result |= I2c.receive();
    }
  }

  // Get configuration to know what was asked last time
  config = getConfig();
  
  // Error reading config ?
  if (config == TH02_I2C_ERR)
  {
    return 0;
  }
  
  // last conversion was temperature ?
  else  if( config & TH02_CONFIG_TEMP)
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

  // remember result value is multiplied by 10 to avoid float calculation later
  // so humidity of 45.6% is 456 and temp of 21.3 C is 213
  return (roundInt(result/10.0f));
}

/* ======================================================================
Function: getConpensatedRH
Purpose : return the compensated calulated humidity  
Input   : true if we want to round value to 1 digit precision, else 2
Output  : the compensed RH value (rounded or not)
Comments: 
====================================================================== */
int16_t TH02::getConpensatedRH(bool round)
{
  float rhvalue  ;
  float rhlinear ;

  // did we had a previous measure RH
  if (rh == TH02_UNINITIALIZED_RH)
  {
    return rh;
  }
  else
  {
    // now we're float restore real value RH value
    rhvalue = (float) rh / 100.0 ;
    
    // apply linear compensation
    rhlinear = rhvalue - ((rhvalue*rhvalue) * TH02_A2 + rhvalue * TH02_A1 + TH02_A0);

    // correct value
    rhvalue = rhlinear;
    
    // do we have a initialized temperature value ?
    if (temperature != TH02_UNINITIALIZED_TEMP )
    {
      // Apply Temperature compensation
      // remember last temp was stored * 100
      rhvalue += ((temperature/100.0) - 30.0) * (rhlinear * TH02_Q1 + TH02_Q0);
    }
    
    // now get back * 100 to have int with 2 digit precision
    rhvalue *= 100;
    
    // do we need to round to 1 digit ?
    if (round)
    {
      // remember result value is multiplied by 10 to avoid float calculation later
      // so humidity of 45.6% is 456
      return (roundInt(rhvalue/10.0f));
    }
    else
    {
      return rhvalue;
    }
  }
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
  return rh; 
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
  return temperature;
}

