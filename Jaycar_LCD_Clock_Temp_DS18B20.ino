// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
// Code based around demo in https://github.com/adafruit/RTClib/archive/master.zip
// pushbutton code from https://arduino-info.wikispaces.com/LCD-Pushbuttons
// Modified by Tim Blythman Decmeber 2015

#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//pin for buttons
#define KEYPIN A0

//button constants
#define btnRIGHT  6
#define btnUP     5
#define btnDOWN   4
#define btnLEFT   3
#define btnSELECT 2
#define btnNONE   (-1)

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

RTC_DS1307 rtc;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //LiquidCrystal lcd(RS, E, b4, b5, b6, b7);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char monthsOfTheYear[12][4]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
char dOfTheWeek[7][4]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char setphase=0;    //to know whether we are setting the clock or not
char setpos=0;      //setting y/m/d/h/m/s
char cursorx[6]={5,7,14,1,4,7};    //where to place cursor in set mode
char cursory[6]={0,0,0,1,1,1};

DateTime now;      //set as global so we can update between loops

void setup () {

  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Start up the library
  sensors.begin();

}

void loop () {
if(!setphase){now = rtc.now();}    //only update clock if we're not in setting mode

sensors.requestTemperatures(); // Send the command to get temperatures
double tempC = sensors.getTempCByIndex(0);
  
    dobuttons();    //respond to button presses

Serial.print(now.year(), DEC);
Serial.print('/');
Serial.print(now.month(), DEC);
Serial.print('/');
Serial.print(now.day(), DEC);
Serial.print(" (");
Serial.print(dOfTheWeek[now.dayOfTheWeek()]);
Serial.print(") ");
Serial.print(now.hour(), DEC);
Serial.print(':');
Serial.print(now.minute(), DEC);
Serial.print(':');
Serial.print(now.second(), DEC);
Serial.print(' ');
Serial.print("T: ");
Serial.print(tempC);
Serial.println();
  

    lcd.setCursor(0, 0);
lcd.print(dOfTheWeek[now.dayOfTheWeek()]);
lcd.print(" ");
lcd.print(tens(now.day()), DEC);
lcd.print(units(now.day()), DEC);
lcd.print(' ');
lcd.print(monthsOfTheYear[now.month()-1]);
lcd.print(' ');
lcd.print(now.year(), DEC);
lcd.print(' ');

    lcd.setCursor(0, 1);
lcd.print(tens(now.hour()), DEC);
lcd.print(units(now.hour()), DEC);
lcd.print(':');
lcd.print(tens(now.minute()), DEC);
lcd.print(units(now.minute()), DEC);
lcd.print(':');
lcd.print(tens(now.second()), DEC);
lcd.print(units(now.second()), DEC);
lcd.print(' ');

// add alternate between T and H here (Later)!

lcd.print("T: ");
lcd.print(tempC);

lcd.setCursor(cursorx[setpos],cursory[setpos]);  //place flashing cursor in set mode
delay(150);    //short enough to pick up brief keypresses
}

char tens(int n){return (n/10)%10;}    //to help show leading zeroes

char units(int n){return n%10;}        //to help show leading zeros

int read_LCD_buttons(){
int adc_key_in    = 0;
adc_key_in = analogRead(KEYPIN);      // read the value from the sensor
delay(5); //switch debounce delay. Increase this delay if incorrect switch selections are returned.
int k = (analogRead(KEYPIN) - adc_key_in); //gives the button a slight range to allow for a little contact resistance noise
if (5 < abs(k)) return btnNONE;  // double checks the keypress. If the two readings are not equal +/-k value after debounce delay, it tries again.
// my buttons when read are centered at these valies: 0, 144, 329, 504, 741
// we add approx 50 to those values and check to see if we are close
if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
if (adc_key_in < 50)   return btnRIGHT; 
if (adc_key_in < 195)  return btnUP;
if (adc_key_in < 380)  return btnDOWN;
if (adc_key_in < 555)  return btnLEFT;
if (adc_key_in < 790)  return btnSELECT;  
return btnNONE;  // when all others fail, return this...
}

void dobuttons(){
int key;
key = read_LCD_buttons();
if(key==btnSELECT){
if(setphase==0){setphase=1;lcd.blink();setpos=3;}    //phase 1 is starting set mode, but select key still pressed, turn cursor on, default to changing hour
if(setphase==2){setphase=3;lcd.noBlink();rtc.adjust(now);}    //phase 3 is ending set mode, but select key still pressed, turn cursor off, need to update RTC to edited time
}
if(key==btnNONE){
if(setphase==1){setphase=2;}    //phase 2 is in set mode select key released
if(setphase==3){setphase=0;}    //phase 0 is normal mode, select key released
}

    if(setphase==2){          //respond to keys in setting mode
if(key==btnLEFT){setpos=setpos-1;if(setpos<0){setpos=5;}}
if(key==btnRIGHT){setpos=setpos+1;if(setpos>5){setpos=0;}}

      int syear,smon,sday,shour,smin,ssec;
syear=now.year();
smon=now.month();
sday=now.day();
shour=now.hour();
smin=now.minute();
ssec=now.second();
if(key==btnUP){
if(setpos==0){sday=sday+1;if(sday>31){sday=1;}}
if(setpos==1){smon=smon+1;if(smon>12){smon=1;}}
if(setpos==2){syear=syear+1;}
if(setpos==3){shour=shour+1;if(shour>23){shour=0;}}
if(setpos==4){smin=smin+1;if(smin>59){smin=0;}}
if(setpos==5){ssec=0;}
rtc.adjust(DateTime(syear, smon, sday, shour, smin, ssec));    //update clock
now = rtc.now();                                              //reload into now
}
if(key==btnDOWN){
if(setpos==0){sday=sday-1;if(sday<1){sday=31;}}
if(setpos==1){smon=smon-1;if(smon<1){smon=12;}}
if(setpos==2){syear=syear-1;}
if(setpos==3){shour=shour-1;if(shour<0){shour=23;}}
if(setpos==4){smin=smin-1;if(smin<0){smin=59;}}
if(setpos==5){ssec=0;}
rtc.adjust(DateTime(syear, smon, sday, shour, smin, ssec));    //update clock
now = rtc.now();                                              //reload into now
}
}
}
