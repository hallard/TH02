// **********************************************************************************
// Example file reading temperature and humidity from TH02 sensor
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// For any explanation see TH02 sensor information at
// http://www.hoperf.com/sensor/app/TH02.htm

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

#include <arduino.h>
#include <Wire.h>
#include <TH02.h>

// Instantiate TH02 sensor
TH02 th02(TH02_I2C_ADDR);

boolean TH02_found = false;

/* ======================================================================
Function: printhex
Purpose : print hex value in 2 digit format
Input   : value
Output  : 
Comments: 
====================================================================== */
void printhex(uint8_t c)
{
  if (c<16) 
    Serial.print('0');
  Serial.print(c,HEX);
}

/* ======================================================================
Function: i2cScan
Purpose : scan I2C bus
Input   : -
Output  : number of I2C devices seens
Comments: global var TH02_found set to true if found TH02
====================================================================== */
int i2cScan()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning I2C bus ...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print(F("I2C device found at address 0x"));
      printhex(address);
      
      if (address == 0x40)
      {

        Serial.println(F("-> TH02 !"));
        TH02_found = true;
      }
      else 
        Serial.println(F("-> Unknown device !"));

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print(F("Unknow error at address 0x"));
      if (address<16) 
        Serial.print('0');
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println(F("No I2C devices found\n"));
  else
    Serial.println(F("Scan done"));

  return nDevices;
}

/* ======================================================================
Function: setup
Purpose : 
Input   : 
Output  : 
Comments: 
====================================================================== */
void setup()
{
  uint8_t devID;
  uint8_t err;
  uint8_t status;
  uint8_t config;
  
  Serial.begin(115200);
  Serial.println(F("\r\nTH02 Demo"));

  Wire.begin();

  // Loop until we found TH02 module
  while (!TH02_found)
  {
    // scan I2C bus
    i2cScan();

    // We found it ?
    if (TH02_found)
    {
      // TH02 ID 
      err = th02.getId(&devID);

      if (err)
      {
        Serial.print(F("TH02 getId error = 0x"));
        printhex(err);
        Serial.println();
      }
      else
      {
        Serial.print(F("TH02 device ID = 0x"));
        printhex(devID);
        Serial.println();

        if (devID == 0x50)
        {
          Serial.println(F("TH02 device ID match !"));

          if ( (err=th02.getStatus(&status)) != 0)
          {
            Serial.print(F("TH02 Status error = 0x"));
            printhex(err);
          }
          else
          {
            Serial.print(F("TH02 Status = 0x"));
            printhex(status);
          }
          Serial.println();

          if ( (err=th02.getConfig(&config)) != 0)
          {
            Serial.print(F("TH02 Config error = 0x"));
            printhex(err);
          }
          else
          {
            Serial.print(F("TH02 Config = 0x"));
            printhex(config);
          }
          Serial.println();
        }
      }
    }

    // wait until next search
    if (!TH02_found)
    {
      Serial.println(F("Will retry to find TH02 in 5 sec."));
      delay(5000);
    }

  } // While not found
}

/* ======================================================================
Function: loop
Purpose : 
Input   : 
Output  : 
Comments: 
====================================================================== */
void loop()
{
  int16_t temp, rh, rh_comp;
  uint8_t duration;
  //uint8_t status;

  Serial.print(F("Starting Temperature conversion."));
  th02.startTempConv();
  duration = th02.waitEndConversion();
  
  if (duration<=TH02_CONVERSION_TIME_OUT)
    Serial.print(F(".done in "));
  else
    Serial.print(F(".timed out "));

  Serial.print(duration);
  Serial.println(F("ms"));
    
  // Get temperature calculated and rounded
  temp = th02.getConversionValue();

  if (temp == TH02_UNDEFINED_VALUE)
  {
    Serial.print(F("Error reading value="));
    Serial.println(temp);
  }
  else
  {
    // Display unrounded value (raw)
    Serial.print(F("Temperature = "));
    // this call does not do any I2C reading, it use last reading
    Serial.print(th02.getLastRawTemp()/100.0);
    Serial.print(F(" C  =>  "));
    // Display now rounded value returned by 1st call
    Serial.print(temp/10.0);
    Serial.println(F(" C"));
  }
    
    
  Serial.print(F("Starting Humidity conversion."));
  
  // Convert humidity
  th02.startRHConv();
  duration = th02.waitEndConversion();

  if (duration<=TH02_CONVERSION_TIME_OUT)
    Serial.print(F(".done in "));
  else
    Serial.print(F(".timed out "));

  Serial.print(duration);
  Serial.println(F("ms"));

  // Get temperature calculated and rounded with no compensation
  rh = th02.getConversionValue();

  if (rh == TH02_UNDEFINED_VALUE)
  {
    Serial.print(F("Error reading value="));
    Serial.println(rh);
  }
  else
  {
    // Now display last reading with unrounded value 
    // this call does not do any I2C reading, it use last reading
    Serial.print(F("Raw Humidity = "));
    Serial.print(th02.getLastRawRH()/100.0);
    Serial.print(F("%  => Compensated "));
    Serial.print(th02.getConpensatedRH(false)/100.0);
    Serial.print(F("%  Rounded  "));
    Serial.print(th02.getConpensatedRH(true)/10.0);
    Serial.println(F("%"));
  }
  
  Serial.println(F("Now waiting 10 sec before next conversion."));
  delay(10000);
}
