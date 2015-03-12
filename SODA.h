/*! \mainpage The SODA library for datalogging with Olympia Circuit's SODA HE board.
 *
 * \section intro_sec Introduction
 *
 * SODA is an Arduino library for data logging. It provides functions to simplify datalogging tasks using Olympia Circuit's SODA HE 1.0 board.
 * SODA stands for Simple Open Data Acquisition.  The goal of the project is to provide simple, high quality tools for the collection and analysis of environmental data.
 * The library was built by Peter Gould (peter@olympiacircuits.com). Some code was adapted from Petre Rodan's DS3231 library for Arduino.
 * Additional thanks go to William Greiman for his SD_FAT library.
 *
 * The SODA library consists of a single class SODA. 
 * \section dependencies_sec Dependencies
 * SdFat: library for SD card functions. This library needs to be added to your Arduino library along with SODA.
 *
 * EEPROM: standard Arduino library for EEPROM functions (comes with your Arduino installation).
 *
 * Wire: standard Arduino library for I2C communication (comes with your Arduino installation).
 * \section install_sec Installation
 * The contents of the SODA folder should be added to the library folder of your Arduino installation (e.g., C:\\Program Files (x86)\\Arduino\\libraries). 
 * Arduino must be restarted after the library has been added.
 */

//The output filename can be changed by changing the character string below
#ifndef FILENAME
#define FILENAME "DATA.CSV"
#endif
//Note that the SD functions can be removed by uncommenting the line below:
//#define NO_SD
#ifndef __SODA__h
#define __SODA__h
#include <Arduino.h>
#include <Wire.h>
//clock values
#define CLOCK_I2C_ADDR   0x68
#define CLOCK_CONTROL_ADDR 0x0E
#define CLOCK_TEMPERATURE_ADDR     0x11
#define CLOCK_TIME_CAL_ADDR        0x00
#define CLOCK_SETUP 0x5 //allow interrupts on alarm1
#define CLOCK_ALARM1_ADDR          0x07
#define CLOCK_ALARM_STATUS 0x0F
//ADC values
#define ADC_I2C_ADDR 0x6E
#define ADC_CONTROL 0x00
#define ADC_BASE 0x80  //one shot conversion model
#define ADC_CH1 0x00
#define ADC_CH2 0x20
#define ADC_CH3 0x40
#define ADC_CH4 0x60
#define ADC_18BITS 0x0C
#define ADC_16BITS 0x08
#define ADC_14BITS 0x04
#define ADC_12BITS 0x00
#define ADC_GAIN1  0x00
#define ADC_GAIN2  0x01
#define ADC_GAIN4  0x02
#define ADC_GAIN8  0x03
//MISC Values
#define LEDPIN 13   // LED connected to digital pin 13
#endif
/** 
* A class to handle basic datalogging functions using
* Olympia Circuit's SODA HE 1.0 Arduino-compatible board.
*/
class SODA
{
	private:
		void setAddress(uint8_t,uint8_t,uint8_t);
		unsigned char getAddress(uint8_t,uint8_t);
		int dectobcd(int val);
		int bcdtodec(int val);
		void runSD(int);
		void parseCommand(char);
		void testForConnection();
		void incrementName(char* inChar, int newVal, int nameLength = 4);
		unsigned char standby;
		//variables 
	//	static char buffer[];
	//	static int timeArray[];
	public:
		/**
       * Initializes an instance of the SODA class. Should be called in each sketch before any other SODA functions.
       */
		void begin();
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//@I Clock Functions
		/**
       * Updates the time array. Need to run setTime to send time array to clock.
       * @param val an int time values.
       * @param place an int specifying the place in the time array (0 = year, 1 = month, 2 = day, 3 = hour, 4 = minute, 5 = second).
       * @see setTime()
	   * @see getTIme()
       */
		void updateTime(int val,int place);
		/**
       * Returns an int value from the timeArray.
       * @param place an int specifying the place in the time array (0 = year, 1 = month, 2 = day, 3 = hour, 4 = minute, 5 = second).
       * @see setTime()
	   * @see updateTime()
       */
		int checkTime(int place);
		/**
		* Resets the time in the clock to the values from timeArray.
		* @see checkTime()
		* @see getTime()
	   * @see updateTime()
		*/
		void setTime();
		/**
		* Loads the time from the clock to the timeArray.
		* (0 = year, 1 = month, 2 = day, 3 = hour, 4 = minute, 5 = second).
	   * @see setTime()
		*/
		void getTime();
		/**
		* Loads the timeArray into a formatted character buffer.
		* Format = YYYY-MM-DD HH:MM:SS
		* @see getTime()
		*/
		void bufferTime();
		/**
		*Set the clock based on input from the Serial connection.
		*Serial data are first saved to the buffer[] array and then loaded to the timeArray before being sent to the clock.
		*Serial data format = 'YYYY-MM-DD HH:MM:SS'.
		* @see setTime()
		*/
		void serialSetTime();
		/**
		*Returns the value from the internal temperature sensor in the DS3231 real time clock.
		*@return temperature in Celsius as float
		*/
		float getClockTemp();
		/**
		*Sets the clock alarm.  Used to wake up the logger and begin a new measurement.
		*Example: setWake(10,2); sets the alarm to the next 10 minute interval.
		*@param val an int time value.
		*@param valType an int indicating the units of time 1= secs, 2 = mins, 3=hours.
		*@see turnOff
		*/
		void setWake(int val,int valType);
		/**
		* Turns of the datalogger board by resetting the clock alarm pin, thereby shuttting off the voltage regulator.
		*@see setWake()
		*/
		void turnOff();
		/**
		* Sets the standby variable to indicate whether the logger is in logging or communication mode.
		*@param val unsigned char
		*@see getStandby
		*/
		void setStandby(unsigned char val);
		/**
		* Retrieves the standby variable that's used to indicate whether the logger is in logging or communication mode.
		*@return standby as unsigned char
		*@see setStandby
		*/
		int getStandby();
		////////////////////////////////////////////////////////////////////////////////////////
		//@I ADC Functions
		/**
		*Returns a reading from the MCP3424 18-bit analog-digital converter.
		*@param ch an int argument specifying channel 1, 2, 3, or 4.
		*@param bit an int argument specifying the bit encoding 1 = 12, 2=14, 3=16, 4=18 bits.
		*@param gain as in argument specifying the level of gain from the programmable gain amplifier 1 = x1, 2 = x2, 3 = x4, 4 = x8. 
		*The return value is adjusted for gain so that a signal of 100 nV with gain = 4 will return a reading of 100 nV.
		*@return long ADC value in nanoVolts (1 * 10^-9 volts).
		*/
		long adcRead(int ch, int bit, int gain);
		/**
		*Returns a temperature reading from a type K themocouple.
		*@param ch an int argument specifying ADC channel 1,2,3, or 4
		*@return a int value of temperature in degrees C. Int is used instead of a float since the precision of the measurement cannot realistically support decimal numbers.
		*/
		int tcReadK(int ch);
		/**
		*An improved version of analogRead that reduces noise in the measurement
		*@param pin1 pin number to make reading
		*@return an int value of the average reading (between 0 and 1023).
		*/
		int smoothAnalogRead(int pin1);
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//@I Data output functions
		/**
		*Begins a new dataline and writes the loggerid and current time separated by a comma.
		*The clock is read by the functions so there's no need to make a seperate call to getTime().
		*The function typically begins writing the line to the SD card.  If the USB cable is connected it instead writes to the serial monitor.
		*If writing to the SD card, the file is opened and left open until a call to dataLineEnd().
		*@param binary a boolean indicating whether to write file in binary mode
		*@param set_end_on_connect a boolean indicating whether the file should be closed when a USB connection is detected
		*@param set_single_file a boolean indicating whether to save data to a single file or to create a new file each time.
		*@param
		*@param sd_cs_pin an int value for the chip-select pin for the SD card. Pin 17 is the default for the normal build.
	   
		*@param 
		*@see dataLineAdd()
		*@see dataLineEnd()
		*@see getID()
		*@see getTime()
		*@see setID()
		*/
		void dataLineBegin(boolean binary = false, boolean set_end_on_connect = false, boolean set_single_file = true, int sd_cs_pin = 17);
		/**
		*Adds an int value to the current data line. A comma is placed before the value.
		*param value an int value
		*@see dataLineBegin()
		*@see dataLineEnd()
		*/
		void dataLineAdd(int value);
		/**
		*Adds a long value to the current data line. A comma is placed before the value.
		*@param value an int value
		*@see dataLineBegin()
		*@see dataLineEnd()
		*/
		void dataLineAdd(long value);
		/**
		*Adds a float value to the current data line. A comma is placed before the value.
		*@param value an int value
		*@see dataLineBegin()
		*@see dataLineEnd()
		*/
		void dataLineAdd(float value);
		/**
		*Adds a series of bytes located at buffer and of length nbytes to the current data line. No comma is placed before the value.
		*@param buffer as const void*
		*@param nbytes as int value
		*@see dataLineAdd()
		*/
		void dataLineAddBytes(const void* buffer, int nbytes);
		/**
		*Terminates a data line. Adds a carriage return/line feed to the end of the data line and, if writing to the SD card, then closes the file.
		*@see dataLineBegin()
		*@see dataLineAdd()
		*/
		void dataLineEnd();
		/**
		*Reads the contents of the data file from the SD card and streams it through the serial connection.  The name of the file is set using 
		*#define filename definition at the top of SODA.h.
		*@see dataLineBegin()
		*@see dataLineAdd()
		*@see dataLineEnd()
		*/
		void dataDownload();
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//@I communication functions
		/**Handles communication between the SODA and a computer/tablet through the serial monitor.  
		*Commands are sent through a serial connection using the format [XMORE_INFO] where X is a one-character command
		*and MORE_INFO are optional, additional characters used to complete some commands such as setting the clock.
		*current commands are:
		*[D] downloads the logger file on the sd card
		*
		*[I] return the logger_id
		*
		*[R] runs through the sketch and normally ouputs a line of current readings to the serial connection.
		*
		*[t] prints the current clock time to the serial connection
		*
		*[TYYYY-MM-DD HH:MM:SS] sets the clock time
		*@see dataLineBegin()
		*@see dataLineAdd()
		*@see dataLineEnd()
		*/
		void communicate();
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		//@I misc function
		/**
		*Writes an logger ID number as a long value to the microcontroller's EEPROM (address 0 to 3).
		*@param ID a long value to be used as the logger ID
		*/
		void setID(long ID);
		/**
		*Returns the logger ID stored in microcontroller's EEPROM.
		*@return ID a long integer.
		*/
		long getID();
		/**
		*Prints the contents of the buffer[] array, usually a formatted time stamp.
		*/
		void printBuffer();
		/**
		*Tests to see if the USB is connected.  A USB connection causes pin 0 of the microcontroller to read as a digital high.
		*@return boolean values where connected = true, not connected = false.
		*/
		bool usbConnected();
		/**
		*Blinks the led connected to pin 13. Used for simple communications such as to show when a process in under way or finished.
		*@param n an int that sets the number of times to blink. Each blink = 100 ms on, 100 ms off.
		*/
		void blinks(int n);
};


