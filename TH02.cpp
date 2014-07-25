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

uint8_t TH02::getId(void)
{
  if ( I2c.read(TH02_I2C_ADDR, TH02_ID, 1) == 0 )
  {
    if (I2c.available() == 1)
    {
      return (I2c.receive());
    }
  }

  return 0;
}


uint8_t TH02::getStatus(void)
{
  if ( I2c.read(TH02_I2C_ADDR, TH02_STATUS, 1) == 0 )
  {
    if (I2c.available() == 1)
    {
      return (I2c.receive());
    }
  }
    
  // had a problem fake a conversion in progress
  return TH02_I2C_ERR;
}

bool TH02::isConverting(void)
{
  if ((getStatus() & TH02_STATUS_RDY)==1)
    return true;
  else
    return false;
}


uint8_t TH02::getConfig(void)
{
  if ( I2c.read(TH02_I2C_ADDR, TH02_CONFIG, 1) == 0 )
  {
    if (I2c.available() == 1)
    {
      return (I2c.receive());
    }
  }
    
  // had a problem fake a config all bit to 1
  // should never happen to have config at 0xFF
  return TH02_I2C_ERR;
}

bool TH02::setConfig(uint8_t config)
{
  if ( I2c.write( (uint8_t) TH02_I2C_ADDR, (uint8_t) TH02_CONFIG, (uint8_t) config ) == 0 )
    return true;
  else
    return false;
}


bool TH02::startTempConv(bool fastmode, bool heater)
{
  uint8_t config = TH02_CONFIG_START | TH02_CONFIG_TEMP;
  
  if (fastmode) config |= TH02_CONFIG_FAST;
  if (heater)   config |= TH02_CONFIG_HEAT;
  
  return ( setConfig( config ) );
}

bool TH02::startRHConv(bool fastmode, bool heater)
{
  uint8_t config = TH02_CONFIG_START;
  
  if (fastmode) config |= TH02_CONFIG_FAST;
  if (heater)   config |= TH02_CONFIG_HEAT;
  
  return ( setConfig( config ) );
}


uint8_t TH02::waitEndConversion(void)
{
  // okay this is basic approach not so accurate
  // but avoing using long and millis
  uint8_t time_out = 0;
  
  // loop until conversion done or duration >= time out
  while (isConverting() && time_out <= TH02_CONVERSION_TIME_OUT )
  {
    ++time_out;
    delay(1);
  }
  
  // return approx time of conversion
  // if >=50 an error has occured
  return (time_out);
  
}

int16_t TH02::roundInt(float value)
{  
  if (value >= 0.0f)
    value = floor(value + 0.5f);
  else
    value = ceil(value - 0.5f);
    
  // remember result value is multiplied by 10 to avoid float calculation later
  // so humidity of 45.6% is 456 and temp of 21.3 C is 213
  return (static_cast<int16_t>(value));
}

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
      // convert to word
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
    result /= 32;
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
    
    // Save value in case we want to calculate RH linear result
    temperature = result;
  }
  else
  {
    result >>= 4;  // remove 4 unused LSB bits
    result *= 100; // multiply per 100 to have int value with 2 decimal
    result /= 16;
    result -= 2400;

    // now result contain humidity * 100
    // so 4567 is 45.67 % RH
    rh = result;
  }

  // remember result value is multiplied by 10 to avoid float calculation later
  // so humidity of 45.6% is 456 and temp of 21.3 C is 213
  return (roundInt(result/10.0f));
}


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

    rhvalue = rhlinear;
    
    // do we have a initialized temperature value 
    if (temperature != TH02_UNINITIALIZED_TEMP )
    {
      // Apply Temperature compensation
      // remember last temp was stored * 100
      rhvalue += ((temperature/100.0) - 30.0) * (rhlinear * TH02_Q1 + TH02_Q0);
    }
    
    // now get back * 100 to have int with 2 digit precision
    rhvalue *= 100;
    
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

int32_t TH02::getLastRawRH(void)
{
  return rh; 
}

int32_t TH02::getLastRawTemp(void) 
{
  return temperature;
}

