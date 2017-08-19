

/*
  DS3231: Real-Time Clock. Alarm simple
  Read more: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-DS3231.html
  GIT: https://github.com/jarzebski/Arduino-DS3231
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <DS3231.h>
#include <Servo.h>
#include "pitches.h"
#include <avr/sleep.h>
#include <avr/power.h>


DS3231 clock;
RTCDateTime dt;
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

byte pos = 0;    // variable to store the servo position
volatile boolean isAlarm = false;
volatile boolean canAlarm = false;
boolean alarmState = false;
byte alarmLED = 4;

const byte FirstAlarmHour = 11;
const byte SecondAlarmHour = 15;
const byte ThirdAlarmHour = 18;

const byte FirstAngle = 0;
const byte SecondAngle = 77;
const byte ThirdAngle = 152;

byte CurAlarm;
byte CurHr;

int melody[] = {
  NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
byte noteDurations[] = {
  4, 4
};

void alarmFunction()
{
  detachInterrupt(0);
  if (canAlarm)
  {
//  Serial.println("*** INT 0 ***");
  isAlarm = true;

  }
}

void sleepNow()         // here we put the arduino to sleep
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
    sleep_enable();          // enables the sleep bit in the mcucr register
    attachInterrupt(0, alarmFunction, LOW); // use interrupt 0 (pin 2) and run function
    ADCSRA = 0; // disable ADC                                    
    power_spi_disable();
    power_timer0_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_twi_disable(); 
    power_adc_disable();
    power_usart0_disable();
    sleep_mode();            // here the device is actually put to sleep!!
                             // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
    sleep_disable();         // first thing after waking from sleep:
                             // disable sleep...
}

void moveServo(int angle){
  myservo.attach(9);
  delay(1000);
  myservo.write(angle);
  delay(1800);
  myservo.detach();
  delay(500);
}

void playMelody() {
    for (int thisNote = 0; thisNote < 2; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(8, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(8);
    }
}

void playTime(RTCDateTime curtime) {
      delay(1000);
      for (int h = 0; h < curtime.hour; h++){ 
//        Serial.println(h,DEC);
        tone(8, NOTE_B3, 100);
        delay(250);
        noTone(8);
        delay(250);
      }
      delay(500);
      tone(8, NOTE_B3, 500);
      delay(500);
      noTone(8);     
}

void setup()
{
/*  
  Serial.begin(9600);
  Serial.println("Initialize DS3231");
*/
  clock.begin();

  clock.enableOutput(false);
  clock.armAlarm1(false);
  clock.armAlarm2(false);
  clock.clearAlarm1();
  clock.clearAlarm2();

  moveServo(ThirdAngle);
  moveServo(SecondAngle);
  moveServo(FirstAngle);

  dt = clock.getDateTime();
  //Serial.println(clock.dateFormat("d-m-Y H:i:s - l", dt));
  
  CurHr = dt.hour;
  if ((CurHr > FirstAlarmHour) and (CurHr < SecondAlarmHour) ) 
  {
    CurAlarm = SecondAlarmHour; 
  }
  else if ((CurHr > SecondAlarmHour) and (CurHr < ThirdAlarmHour) ) 
  {
    CurAlarm = ThirdAlarmHour;
  }
  else 
  {
    CurAlarm = FirstAlarmHour;
  }
  //clock.setAlarm1(0, 0, 0,CurAlarm, DS3231_MATCH_S);
  clock.setAlarm1(0, CurAlarm, 0, 0, DS3231_MATCH_H_M_S);
  
  isAlarm = false;
  // Attach Interrput 0. In Arduino UNO connect DS3231 INT to Arduino Pin 2
  
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, alarmFunction, LOW);
  
/*  Serial.println("Start"); 
  Serial.println(clock.dateFormat("d-m-Y H:i:s - l", dt));
  Serial.println(CurAlarm,DEC);
  delay(1000);
*/
  playMelody();
  playTime(dt);
  canAlarm=true;
  sleepNow();
}



void loop()
{
  if (isAlarm)
  {
 //   digitalWrite(13, HIGH);  
    power_timer0_enable();
    power_timer1_enable();
    power_timer2_enable();
    power_twi_enable(); 
    power_usart0_enable();
    clock.clearAlarm1();
    isAlarm = false;
   
    dt = clock.getDateTime();
/*    Serial.println(clock.dateFormat("d-m-Y H:i:s - l", dt));
    delay(1000);
*/  
  //  CurHr = dt.second;
    CurHr = dt.hour;
    switch (CurHr) 
    {
      case FirstAlarmHour:
          pos = SecondAngle;
          CurAlarm = SecondAlarmHour;
       break;
       case SecondAlarmHour:
          pos = ThirdAngle;
          CurAlarm = ThirdAlarmHour;
       break;
       case ThirdAlarmHour:
          pos = FirstAngle;
          CurAlarm = FirstAlarmHour;
       break;
    }
      
/*    Serial.println(dt.second,DEC);
    Serial.println(pos,DEC);
    Serial.println(CurAlarm,DEC);
 */
 //   digitalWrite(13, LOW); 
    moveServo(pos);
    playMelody();
 //   clock.setAlarm1(0, 0, 0,CurAlarm, DS3231_MATCH_S);
    clock.setAlarm1(0, CurAlarm, 0, 0, DS3231_MATCH_H_M_S);
    sleepNow();
  }
}
 
