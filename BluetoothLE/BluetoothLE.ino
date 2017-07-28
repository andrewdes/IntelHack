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

#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ))     // from time-lib

#define NOTE_D1  37 //alarm note

rgb_lcd lcd;
DHT dht;

int eventHour = -2; 
int eventMinute = -2; 
int eventDay = -2;
int eventMonth = -2;
int eventYear = -2;

int buttonState = 0;         // variable for reading the pushbutton status
boolean alreadyTriggered = false;
boolean trigger = false;
boolean light = false;
int currentMin = -2;
int nmStartH = -1;
int nmStartM = - 1;
int nmEndH = -1;
int nmEndM = -1;
boolean nm = false;
boolean changeLCD = true;
int pirState = LOW; 
boolean retrigger = false;
boolean alreadyRetriggered = false;

int r = 255;
int g = 255; 
int b = 255;


boolean days[7] = {false,false,false,false,false,false,false}; //Used to track which days the alarm is

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEUnsignedCharCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //LED
BLEUnsignedCharCharacteristic tempCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead); //Temperature sensor
BLEUnsignedCharCharacteristic hourCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Hour
BLEUnsignedCharCharacteristic minuteCharacteristic("19B10013-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Minute
BLEUnsignedCharCharacteristic dayCharacteristic("19B10004-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Day
BLEUnsignedCharCharacteristic monthCharacteristic("19B10014-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Month
BLEUnsignedCharCharacteristic yearCharacteristic("19B10024-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Year
BLEUnsignedCharCharacteristic event("19B10005-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Used for tracking what is being written to (setting date, setting time, alarm, etc.)
BLEUnsignedCharCharacteristic blueCharacteristic("19B10006-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); //Used for the color blue (RGB, R & G are recycled from other characteristics)


const int thresholdvalue=25; //The threshold for which light is detected
float Rsensor; //Resistance of sensor in K

const int ledPin = 8; //Green led pin
const int redPin = 4; //red led pin
const int bluePin = 6; //blue led pin
const int speakerPin = 3; //buzzer pin
const int buttonPin = 7; //pushbutton pin
const int motionPin = 5;


void setup() {
  Serial.begin(9600);

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
  ledService.addCharacteristic(blueCharacteristic);
  

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
  blueCharacteristic.setValue(0);

  // start advertising
  BLE.advertise();

  pinMode(speakerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(redPin, OUTPUT);

  pinMode(buttonPin, INPUT);  
  pinMode(motionPin, INPUT);
  
  //set time to 5:00:00 on March 18th, 2017. Please change to your time / date
  setTime(5, 0, 0, 18, 3, 2017);



}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();


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
      if (minuteCharacteristic.written()){

        if(!event.value()){
          setTime(hourCharacteristic.value(), minuteCharacteristic.value(), 0, day(), month(), year());
        }else if(event.value() == 10){
          nmStartH = hourCharacteristic.value();
          nmStartM = minuteCharacteristic.value(); 
          changeLCD = true;        
        }else if(event.value() == 11){
          nmEndH = hourCharacteristic.value();
          nmEndM = minuteCharacteristic.value();         
        }
        else{        
          eventHour = hourCharacteristic.value();
          eventMinute = minuteCharacteristic.value();
          eventDay = event.value()-2;
          days[eventDay] = true;
        }
      }

      //Year is the last characteristic to be written to from the app
      if (yearCharacteristic.written()){
          setTime(hour(), minute(), second(), dayCharacteristic.value(), monthCharacteristic.value(), yearCharacteristic.value()+2000);      
      }

      //Update LCD background
      if(blueCharacteristic.written()){
        
        r = dayCharacteristic.value();
        g = monthCharacteristic.value();
        b = blueCharacteristic.value();
        
        lcd.setRGB(r,g,b);

        changeLCD = true;
      }    


      checkNightMode(hour(), minute());


      if(nm && changeLCD){
        lcd.setRGB(0,0,0);
        changeLCD = false;
      }else if(changeLCD){
        lcd.setRGB(r,g,b);
        changeLCD = false;
      }

      reTriggerAlarm(minute());

      if(retrigger){
        trigger = true;
      }    

      checkTemperature();

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

      //If alarm isn't already sounded
      if(!trigger && !alreadyTriggered){
        checkEvent(hour(), minute(), dayOfWeek(year(), month(), day()));
        }else if(trigger){
        checkButton();
      }
      
      //If atleast 1 minute has passed, reset alreadyTriggered
      if((currentMin + 1) == minute()){
        alreadyTriggered = false;
        currentMin = -2;
        alreadyRetriggered = false;
      }
      
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


  //If alarm isn't already sounded
  if(!trigger && !alreadyTriggered){
    checkEvent(hour(), minute(), dayOfWeek(year(), month(), day()));
  }else if(trigger){
    checkButton();
  }

  //If atleast 2 minutes has passed
  if((currentMin + 2) == minute()){
    alreadyTriggered = false;
    currentMin = -2;
    alreadyRetriggered = false;
  }


  checkNightMode(hour(),minute());

  if(nm && changeLCD){
    lcd.setRGB(0,0,0);
    changeLCD = false;
  }else if(changeLCD){
    lcd.setRGB(r,g,b);
    changeLCD = false;
  }

  reTriggerAlarm(minute());



}

//Returns the day of the week based on the current day, month and year
int dayOfWeek(uint16_t year, uint8_t month, uint8_t day)
{
  uint16_t months[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365         };   // days until 1st of month

  uint32_t days = year * 365;        // days until year 
  for (uint16_t i = 4; i < year; i += 4) if (LEAP_YEAR(i) ) days++;     // adjust leap years, test only multiple of 4 of course

  days += months[month-1] + day;    // add the days of this year
  if ((month > 2) && LEAP_YEAR(year)) days++;  // adjust 1 if this year is a leap year, but only after febr

  return days % 7;   // remove all multiples of 7
}


void checkEvent(int h, int m, int d){

  int hourLED = eventHour;
  int minuteLED = eventMinute;
  int dayLED = eventDay;


  //Check if alarm should be triggered
  if(h == eventHour && m == eventMinute && days[d]){    
    trigger = true;
    alreadyTriggered = true;
    currentMin = m;
  }  


  //Check if LED should go off (20 mintues before alarm)
  if((minuteLED-20) < 0){

      int remainder = abs(minuteLED-20);
  
      if((hourLED - 1) < 0){
  
        if((dayLED - 1) < 0){
          dayLED = 6;        
        }else{
          dayLED -= 1;
        }
  
        hourLED = 23;
        
      }else{
        hourLED -= 1;
      }
      
      minuteLED = 60 - remainder;    
    
  }else{
    minuteLED -= 20;
  }  


  if(h == hourLED && m == minuteLED && d  == dayLED){
    digitalWrite(redPin, HIGH);
  }

}



void checkButton(){

    checkLight();
    buttonState = digitalRead(buttonPin); //read button state

    //If button is pressed while alarm is going off
    if (buttonState == HIGH && light) {
        noTone(speakerPin );
        digitalWrite(ledPin, LOW);  
        digitalWrite(redPin, LOW);
        trigger = false;      
    }else{
        digitalWrite(ledPin, HIGH);
        tone(speakerPin, NOTE_D1, 500);
        delay(1000);
    }
  
}

void checkTemperature(){

   float temperature = dht.getTemperature();

  if(dht.getStatusString()){
    tempCharacteristic.setValue(temperature); 
  }

  
}



void checkLight(){

  int sensorValue = analogRead(0);   
  Rsensor=(float)(1023-sensorValue)*10/sensorValue;
  
  if(Rsensor>thresholdvalue)
  {
    light = false;
  }
  else
  {
    light = true;
  }
  
}

void checkNightMode(int h, int m){


  if(h == nmStartH && m == nmStartM){
    nm = true;
    changeLCD = true;
  }else if(h == nmEndH && m == nmEndM){
    nm = false;
    changeLCD = true;
  }
  
}

void reTriggerAlarm(int m){

  pirState = digitalRead(motionPin);  // read input value

  if(!alreadyRetriggered){
    if((eventMinute + 1) == m && pirState == LOW){
      trigger = true; 
      alreadyRetriggered = true;
    }else{
      alreadyRetriggered = true;  
    }
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

