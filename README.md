TH02 Library
------------
By Charles-Henri Hallard
<br/>
hoperf TH02 temperature and humidity sensor library for Arduino

##License
Creative Commons Attrib Share-Alike License
https://creativecommons.org/licenses/by-sa/4.0/
You are free to share and adapt. But you need to give attribution and use the same license to redistribute.

For any explanation see TH02 sensor information at http://www.hoperf.com/sensor/app/TH02.htm

Code based on following datasheet http://www.hoperf.com/upload/sensor/TH02_V1.1.pdf 

##Features
List of features implemented in this library:

- Reading device ID
- Starting a Temperature / Humidity conversion in normal/fast mode
- Keep in memory last Temperature and Humidity with 2 digits precision
- Calculate Linear and temperature compensation for Humidity
- Waiting end of conversion with time out 
- return data as int16 * 10 rounded to avoid float in your program

This is an initial release, it could contain bugs, but works fine for now. Please let me know if you find issues.

###Installation
Copy the content of this library in your Arduino libraries folder in your sketchbook location, you should have now "your_sketchbook_folder/libraries/TH02".
<br />
To find your Arduino sketchbook location folder, in Arduino IDE, go to File>Preferences in the Arduino IDE.
<br/>
See [this tutorial][1] on Arduino libraries.
<br/>
<strike>This library depends on fast and optimized I2C library since Arduino wire library can't do repeated start. This I2C library is really a nice piece of code. It's faster than original wire library and require less memory and flash. 
See DSS Circuit [page][2] for information and download. Lot of good readings on I2C stuff.  
<b>TH02 library will not work until you install I2C library.</b></strike>
<br/>   
<b>Updated April 2015</b>, This library now use Arduino Wire library.  

###Possible issues
- The library and examples are continuously improved as bugs and stability issues are discovered. Be sure to check back often for changes.
- Don't send any IC2 command to TH02 when starting conversion except the one for polling ready flag, if not could lock up the RDY flag to be always busy.


###Sample usage
- [GetTempHum][3] Display Temperature and Humidity using various library parameters

##Blog dedicated post
See this [post][5] for information

##Why
- I have spent a lot of time developing this library. I made it open source because I believe a lot of people can benefit from this new powerful sensor. I hope people will also contribute and build on my work.
- I have long tested other temperature and humidity sensor, but this one is small, cheap, factory calibrated, low power, breadboard friendly or easy PCB soldering, fast response and for all of this it is an excellent humidity and temperature sensor.

##Misc
 See news and other projects on my [blog][4] 
 
[1]: http://learn.adafruit.com/arduino-tips-tricks-and-techniques/arduino-libraries
[2]: http://www.dsscircuits.com/index.php/articles/66-arduino-i2c-master-library
[3]: https://github.com/hallard/TH02/blob/master/examples/GetTempHum/GetTempHum.ino
[4]: http://hallard.me
[5]: http://hallard.me/th02-library/