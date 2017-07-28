# Alarm Buddy

Say goodnight to boring old alarm clocks, Alarm Buddy is here to wake you up at the perfect time every morning 

<b> Hardware: </b>

    - Genuino 101
    - Groove Starter Kit (2xLED kit, 1xButton, 1xLightSensor, 1xBuzzer, 1xGrooveBaseShield)
    - DHT22 Temperature Sensor
    - Pir Motion Sensor


<b>How to run it: </b>

Step 1) Plug the Groove Base Shield into the Arduino/Genuino101 and plug the Groove sensors in the following spots:

    Groove LED Socket Kit (Green): D8 (Groove Base Shield)
    Groove LED Socket Kit (Red): D4 (Groove Base Shield)
    Groove Button: D7 (Groove Base Shield)
    Groove Buzzer: D3 (Groove Base Shield)
    Groove LCD RGB Backlight : I2C (Groove Base Shield)
    Groove Light Sensor : A0 (Groove Base Shield)
    
Step 2) Follow the wiring diagram in the repo to connect the temperature sensor and motion sensor

Step 3) Upload the sketch from the repo to the Genuino/Arduino101

Step 4) To use the Android app, download the zip folder in the repo, extract it and open it up in Android Studio. From there you will be able to download the app onto an Android device



<b>Features: </b>

    - Takes traffic (live and historical averages) into account when waking you up (the starting location is defaulted to the users current location)
    - Takes preperation time into account (just set the alarm to the time you wish to be there)
    - Alarm can only be shut off when the lights are on (helps assure user will not fall back asleep)
    - If no motion is detected one minute after the alarm sounds, the alarm will re-trigger
    - At any time you can get the current temperature in the room
    - Night mode helps reduce light in the room and prevents the user from staring at the clock all night (which is said to make sleeping more difficult)
    - Red LED indicator when the alarm is 20 minutes or less away. So if the user wakes up and sees a red light, there is no purpose in attempting to fall back asleep.
    - Fully controllable from an Android application (adjust time and date, alarm time, LCD backlight colour, etc.)

  
<b>Devpost:</b> https://devpost.com/software/alarm-buddy-3052as
<b>Android App:</b> https://github.com/andrewdes/IntelHackApp
 
