    #define ROTARY_ANGLE_SENSOR A3
    #define ADC_REF 5//reference voltage of ADC is 5v.If the Vcc switch on the seeeduino
                     //board switches to 3V3, the ADC_REF should be 3.3
    #define GROVE_VCC 5//VCC of the grove interface is normally 5v
    #define FULL_ANGLE 255

    #include <Wire.h>
    #include "rgb_lcd.h"
    
    const int buttonPin = 5;     // the number of the pushbutton pin
    int buttonState = 0;         // variable for reading the pushbutton status
    int colorChange = 0;        // detecting which color is currently being changed
    
    rgb_lcd lcd;

    //RGB colors
    int colorR = 0; 
    int colorG = 0;
    int colorB = 0;
    
    void setup() 
    {
        Serial.begin(9600);
        pinsInit();

        lcd.begin(16,2);
        lcd.setRGB(colorR, colorG, colorB);

        delay(1000);
        
    }

    void loop() 
    {
        int degrees;
        degrees = getDegree();


        int color;
        /*The degrees is 0~300, should be converted to be 0~255 to control the*/
        /*brightness of LED                                                   */
        color = map(degrees, 0, FULL_ANGLE, 0, 255); 
        controlColor(color, degrees);

        // read the state of the pushbutton value:
        buttonState = digitalRead(buttonPin);

        // check if the pushbutton is pressed.
        // if it is, the buttonState is HIGH:
        if (buttonState == HIGH) {          
          if(colorChange == 2){
            colorChange = 0;
          }else{
            colorChange++;
          }

          delay(200);
        }
        else {
          //do nothing
        }

        delay(50);

    }
    
    void pinsInit(){
        pinMode(ROTARY_ANGLE_SENSOR, INPUT);
        pinMode(buttonPin, INPUT);
    }


    void controlColor (int color, int deg){

      if(colorChange == 0){
       colorR = color;
       }else if(colorChange == 1){        
        colorG = color;
      }else{
        colorB = color;
      }

      delay(100);
      
      lcd.setRGB(colorR, colorG, colorB);
      displayColor(colorR, colorG, colorB);

    }


    void displayColor(int c1, int c2, int c3){
      lcd.clear();
      lcd.print("R:");
      lcd.print(c1);
      lcd.print(" G:");
      lcd.print(c2);
      lcd.setCursor(0,1);
      lcd.print("B:");
      lcd.print(c3);
    }
    
    /*Function: Get the angle between the mark and the starting position    */
    /*Parameter:-void                                                       */
    /*Return:   -int,the range of degrees is 0~300                          */
    int getDegree()    {
        int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
        float voltage;
        voltage = (float)sensor_value*ADC_REF/1023;
        float degrees = (voltage*FULL_ANGLE)/GROVE_VCC;
        return degrees;
    }
