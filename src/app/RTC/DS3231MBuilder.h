#pragma once

#include "app/RTC/RTC.h"
#include "app/RTC/TimeDirector.h"
#include <Wire.h>
#include <DS3231M.h>
class DS3231MBuilder  :public RTCBase,public DS3231M_Class
{
public:
    explicit DS3231MBuilder (rtos::Mutex& mtx,TwoWire& wire,const uint8_t rst=-1):RTCBase(mtx,wire),DS3231M_Class(wire),_mtx(mtx),_rst(rst)
    {
   
    }
    explicit DS3231MBuilder (const DS3231MBuilder & builder):RTCBase(builder._mtx,builder.I2CBus::_wire),DS3231M_Class(builder.I2CBus::_wire)
    ,_mtx(builder._mtx),_rst(builder._rst)
    {
        
    }
    explicit  DS3231MBuilder (DS3231MBuilder && builder):RTCBase(builder._mtx,builder.I2CBus::_wire),DS3231M_Class(builder.I2CBus::_wire)
    ,_mtx(builder._mtx),_rst(builder._rst)
    {   
        if(this!=&builder){
            this->~DS3231MBuilder ();
        }
    }
    ~DS3231MBuilder ()=default;
  
    void power_on() override;
    bool init(uint8_t h_mode=CLOCK_H24) override;
    time_t epoch() override;
    void setEpoch(time_t epoch) override;
    bool setDateTime(const char* date,const  char* time) override;

    void readCommand() {
  /*!
   @brief    Read incoming data from the Serial port
   @details  This function checks the serial port to see if there has been any input. If there is
             data it is read until a terminator is discovered and then the command is parsed and
             acted upon
   @return   void
  */
  char text_buffer[SPRINTF_BUFFER_SIZE];  ///< Buffer for sprintf()/sscanf()
  static uint8_t text_index = 0;                    ///< Variable for buffer position
  while (Serial.available())                        // Loop while there is incoming serial data
  {
    text_buffer[text_index] = Serial.read();  // Get the next byte of data
    // keep on reading until a newline shows up or the buffer is full
    if (text_buffer[text_index] != '\n' && text_index < SPRINTF_BUFFER_SIZE) {
      text_index++;
    } else {
      text_buffer[text_index] = 0;              // Add the termination character
      for (uint8_t i = 0; i < text_index; i++)  // Convert the whole input buffer to uppercase
      {
        text_buffer[i] = toupper(text_buffer[i]);
      }  // for-next all characters in buffer
      Serial.print(F("\nCommand \""));
      Serial.write(text_buffer);
      Serial.print(F("\" received.\n"));
      /*********************************************************************************************
      ** Parse the single-line command and perform the appropriate action. The current list of    **
      ** commands understood are as follows:                                                      **
      **                                                                                          **
      ** SETDATE      - Set the device time                                                       **
      **                                                                                          **
      *********************************************************************************************/
      enum commands { SetDate, Unknown_Command };  // enumerate all commands
      commands command;                            // declare enumerated type
      char     workBuffer[SPRINTF_BUFFER_SIZE];    // Buffer to hold string compare
      sscanf(text_buffer, "%s %*s", workBuffer);   // Parse the string for first word
      if (!strcmp(workBuffer, "SETDATE")) {
        command = SetDate;  // Set command number when found
      } else {
        command = Unknown_Command;  // Otherwise set to not found
      }                             // if-then-else a known command
      unsigned int tokens, year, month, day, hour, minute,
          second;  // Variables to hold parsed date/time
      switch (command) {
        /*********************************
        ** Set the device time and date **
        *********************************/
        case SetDate:
          // Use sscanf() to parse the date/time into component variables
          tokens = sscanf(text_buffer, "%*s %u-%u-%u %u:%u:%u;", &year, &month, &day, &hour,
                          &minute, &second);
          if (tokens != 6)  // Check to see if it was parsed correctly
          {
            Serial.print(F("Unable to parse date/time\n"));
          } else {
            adjust(
                DateTime(year, month, day, hour, minute, second));  // Adjust the RTC date/time
            Serial.print(F("Date has been set."));
          }  // of if-then-else the date could be parsed
          break;
        /********************
        ** Unknown command **
        ********************/
        case Unknown_Command:  // Show options on bad command
        default:
          Serial.println(F("Unknown command. Valid commands are:"));
          Serial.println(F("SETDATE yyyy-mm-dd hh:mm:ss"));
      }                // of switch statement to execute commands
      text_index = 0;  // reset the counter
    }                  // of if-then-else we've received full command
  }                    // of if-then there is something in our input buffer
}  // of method readCommand

void RTC_TEST()
{
  while(1){
    static uint8_t secs;

    DateTime now = DS3231M_Class::now();  // get the current time from device
    if (secs != now.second())            // Output if seconds have changed
    {
      // Use sprintf() to pretty print the date/time with leading zeros
      char output_buffer[SPRINTF_BUFFER_SIZE];  ///< Temporary buffer for sprintf()
      sprintf(output_buffer, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(),
              now.hour(), now.minute(), now.second());
      Serial.println(output_buffer);

      secs = now.second();  // Set the counter variable
    }                       // of if the seconds have changed
    readCommand();      

    ThisThread::sleep_for(3000);
  }
}
private:
    rtos::Mutex& _mtx;
    uint8_t _rst;
    const uint8_t  SPRINTF_BUFFER_SIZE{32};  ///< Buffer size for sprintf()
};