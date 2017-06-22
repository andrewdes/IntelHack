#include "DHT.h"
#include <Wire.h>
#include "rgb_lcd.h"
    
rgb_lcd lcd;
DHT dht;

void setup()
{
  Serial.begin(9600);
3q
  dht.setup(2); // data pin 2
  lcd.begin(16,2);
  
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  if(dht.getStatusString()){
    lcd.clear();
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.setCursor(0,1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
  }else{
    lcd.clear();
    lcd.print("Error reading temp");
  }
}

