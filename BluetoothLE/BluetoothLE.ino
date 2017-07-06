/*
 * Copyright (c) 2016 Intel Corporation.  All rights reserved.
 * See the bottom of this file for the license terms.
 */

/*
 * Sketch: led.ino
 *
 * Description:
 *   This is a Peripheral sketch that works with a connected Central.
 *   It allows the Central to write a value and set/reset the led
 *   accordingly.
 */

#include <CurieBLE.h>
#include "DHT.h"

#include <Wire.h>
#include "rgb_lcd.h"

#include <CurieTime.h>


rgb_lcd lcd;
DHT dht;



BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEUnsignedCharCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //LED
BLEUnsignedCharCharacteristic tempCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead); //TempSens
BLEUnsignedCharCharacteristic hourCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Hour
BLEUnsignedCharCharacteristic minuteCharacteristic("19B10013-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Minute
BLEUnsignedCharCharacteristic dayCharacteristic("19B10004-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Day
BLEUnsignedCharCharacteristic monthCharacteristic("19B10014-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Month
BLEUnsignedCharCharacteristic yearCharacteristic("19B10024-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Year
BLEUnsignedCharCharacteristic event("19B10005-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Year



const int ledPin = 8; // pin to use for the LED

void setup() {
  Serial.begin(9600);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  dht.setup(2);

  // begin initialization
  BLE.begin();
  lcd.begin(16,2);

  // set advertised local name and service UUID:
  BLE.setLocalName("Alarm Clock");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(tempCharacteristic);
  ledService.addCharacteristic(hourCharacteristic);
  ledService.addCharacteristic(minuteCharacteristic);
  ledService.addCharacteristic(dayCharacteristic);
  ledService.addCharacteristic(monthCharacteristic);
  ledService.addCharacteristic(yearCharacteristic);
  ledService.addCharacteristic(event);
  

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.setValue(0);
  tempCharacteristic.setValue(0);
  hourCharacteristic.setValue(0);
  minuteCharacteristic.setValue(0);
  dayCharacteristic.setValue(0);
  monthCharacteristic.setValue(0);
  yearCharacteristic.setValue(0);
  event.setValue(0);

  // start advertising
  BLE.advertise();

  
  //set time to 1:35:24 on April 4th, 2016. Please change to your time / date
  setTime(1, 35, 24, 4, 10, 2016);

}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  float temperature = dht.getTemperature();

  if(dht.getStatusString()){
    tempCharacteristic.setValue(temperature); 
  }

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }


      //minute characteristic is the last to be written to from app
      if (minuteCharacteristic.written() && !event.value()){
        setTime(hourCharacteristic.value(), minuteCharacteristic.value(), 0, day(), month(), year());
      }

      //Year is the last characteristic to be written to from the app
      if (yearCharacteristic.written()){

        if(event.value()){
          Serial.print (hourCharacteristic.value());
          Serial.print(" : ");
          Serial.print (minuteCharacteristic.value());
          Serial.print(" : ");
          Serial.print (yearCharacteristic.value() + 2000);
          Serial.print(" / ");
          Serial.print (monthCharacteristic.value());
          Serial.print(" / ");      
          Serial.print (dayCharacteristic.value());
        }else{
          setTime(hour(), minute(), second(), dayCharacteristic.value(), monthCharacteristic.value(), yearCharacteristic.value()+2000);
        }
        
      }

      //create a character array of 16 characters for the time
      char clockTime[16];
      //use sprintf to create a time string of the hour, minte and seconds
      sprintf(clockTime, "    %2d:%02d:%02d    ", hour(), minute(), second());

      
      //create a character array of 15 characters for the date
      char dateTime[16];
      //use sprintf to create a date string from month, day and year
      sprintf(dateTime, "   %02d/%02d/%4d   ", month(), day(), year());
      
      //set cursor to column 0, row 0
      lcd.setCursor(0, 0);
      //print the date string over lcd
      lcd.print(clockTime);
      //set cursor to column 0, row 1
      lcd.setCursor(0, 1);
      //print the time string over lcd
      lcd.print(dateTime);

      checkAlarm(hour(), minute(), second());

      
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }

   //create a character array of 16 characters for the time
  char clockTime[16];
  //use sprintf to create a time string of the hour, minte and seconds
  sprintf(clockTime, "    %2d:%02d:%02d    ", hour(), minute(), second());

  //create a character array of 15 characters for the date
  char dateTime[16];
  //use sprintf to create a date string from month, day and year
  sprintf(dateTime, "   %02d/%02d/%4d   ", month(), day(), year());
  
  //set cursor to column 0, row 0
  lcd.setCursor(0, 0);
  //print the date string over lcd
  lcd.print(clockTime);
  //set cursor to column 0, row 1
  lcd.setCursor(0, 1);
  //print the time string over lcd
  lcd.print(dateTime);

  checkAlarm(hour(), minute(), second());

}

void checkAlarm(int h, int m, int s){

  if(h == 12 && m == 0 && s == 0){
    digitalWrite(ledPin, HIGH);
  }
  
}


/*
   Copyright (c) 2016 Intel Corporation.  All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
