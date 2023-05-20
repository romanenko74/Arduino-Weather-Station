#define DEBUG 1

//time settings
#define RESET_CLOCK 1
#define TIME_FORMAT 1 
/*
0 - hh:mm
1 - hh:mm:ss
2 - hh:mm AM (12 hour format)
*/

#define DATE_FORMAT 4  
/*
  0 - 01.01.21
  1 - 01.01.2021
  2 - 01.01.21
      Monday
  3 - 01.01.2021
      Monday
  4 - January, 1
      Monday
  5 - Jan, 1
      Monday 
*/

#define TIME_ADJUSTMENT 18 // adjust seconds

//sensors polling period
#define SENSORS_PERIOD 1000*60*5
#define PRESSURE_PERIOD 1000*60*10

//pins
#define BTN_PIN 2
#define LED_R 6
#define LED_G 5
#define LED_B 3
#define LED_COM 4
#define LED_BRIGHT_MAX 250

#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_CCS811.h"

const char day0[] PROGMEM = "Sunday";
const char day1[] PROGMEM = "Monday";
const char day2[] PROGMEM = "Tuesday";
const char day3[] PROGMEM = "Wednesday";
const char day4[] PROGMEM = "Thursday";
const char day5[] PROGMEM = "Friday";
const char day6[] PROGMEM = "Saturday";
const char* const daysOfTheWeek[7] PROGMEM = {day0, day1, day2, day3, day4, day5, day6};

const char month0[] PROGMEM = "January";
const char month1[] PROGMEM = "February";
const char month2[] PROGMEM = "March";
const char month3[] PROGMEM = "April";
const char month4[] PROGMEM = "May";
const char month5[] PROGMEM = "June";
const char month6[] PROGMEM = "July";
const char month7[] PROGMEM = "August";
const char month8[] PROGMEM = "September";
const char month9[] PROGMEM = "October";
const char month10[] PROGMEM = "November";
const char month11[] PROGMEM = "December";
const char* const months[12] PROGMEM = {month0, month1, month2, month3, month4, month5, month6, month7, month8, month9, month10, month11,};

const char forecast0[] PROGMEM = "Settled fine";
const char forecast1[] PROGMEM = "Fine weather";
const char forecast2[] PROGMEM = "Becaming fine";
const char forecast3[] PROGMEM = "Fine, becoming less settled";
const char forecast4[] PROGMEM = "Fine, possible showers";
const char forecast5[] PROGMEM = "Fairly fine, improving";
const char forecast6[] PROGMEM = "Fairly fine, possible showers early";
const char forecast7[] PROGMEM = "Fairly fine, showery later";
const char forecast8[] PROGMEM = "Showery early, improving";
const char forecast9[] PROGMEM = "Changeable, mending";
const char forecast10[] PROGMEM = "Fairly fine, showers likely";
const char forecast11[] PROGMEM = "Rather unsettled clearing later";
const char forecast12[] PROGMEM = "Unsettled, probably improving";
const char forecast13[] PROGMEM = "Showery, bright intervals";
const char forecast14[] PROGMEM = "Showery, becoming less settled";
const char forecast15[] PROGMEM = "Changeable, some rain";
const char forecast16[] PROGMEM = "Unsettled, short fine intervals";
const char forecast17[] PROGMEM = "Unsettled, rain later";
const char forecast18[] PROGMEM = "Unsettled, some rain";
const char forecast19[] PROGMEM = "Mostly very unsettled";
const char forecast20[] PROGMEM = "Occasional rain, worsening";
const char forecast21[] PROGMEM = "Rain at times, very unsettled";
const char forecast22[] PROGMEM = "Rain at frequent intervals";
const char forecast23[] PROGMEM = "Rain, very unsettled";
const char forecast24[] PROGMEM = "Stormy, may improve";
const char forecast25[] PROGMEM = "Stormy, much rain";
const char* const forecast[] PROGMEM = {forecast0, forecast1, forecast2, forecast3, forecast4, forecast5, forecast6, forecast7, forecast8, forecast9, forecast10, forecast11, forecast12, forecast13, 
                                        forecast14, forecast15, forecast16, forecast17, forecast18, forecast19, forecast20, forecast21, forecast22, forecast23, forecast24, forecast25};
                
const byte forecastIndex[] = {0, 1, 3, 7, 14, 17, 20, 21, 23, 0, 1, 4, 10, 13, 15, 18, 22, 23, 25, 0, 1, 2, 5, 6, 8, 9, 11, 12, 16, 19, 24, 25};

RTC_DS3231 rtc;
DateTime CurrentDateTime;
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_CCS811 ccs;
Adafruit_BME280 bme;

uint16_t pressure;
float temp;
byte hum;
int co2, tvoc;
long pressureSample[7];
byte pressureTime[7] = {0, 1, 2 ,3, 4, 5, 6}; // моменты измерения давления (нулевой момент времени, первый и т.д)
long sumX, sumY, sumX2, sumXY;
float a;
byte samples;
int Z;
byte mode = 0;

bool btn_state = false; 
bool btn_click = false;
bool btn_hold = false;
bool btn_isPressed = false;
bool btn_OneClick = false;
bool btn_isHolded = false;
uint32_t timer; 
uint16_t timeout = 500;

uint32_t sensorsTimer;
uint32_t pressureSampleTimer;
uint32_t clockTimer;
uint32_t airTimer;

void timeToString(int hours, int minutes, int seconds, char* timeString){
  int time[3];
  if(seconds > 59){
    seconds -= 60;
    minutes++;
    if (minutes > 59){
      minutes -= 60;
      hours++;
      if (hours > 23){
        hours = 0;
      }
    }
  }
  time[0] = hours;
  time[1] = minutes;
  time[2] = seconds;
  char buffer[3];
  int bufferLength;
  int j = 0;
  for (byte i = 0; i < 3; i++){
    itoa(time[i], buffer, 10);
    bufferLength = strlen(buffer);
    if (bufferLength < 2){
      timeString[j] = '0';
      timeString[j+1] = buffer[0];
    }
    else{
      timeString[j] = buffer[0];
      timeString[j+1] = buffer[1];
    }
    j += 3;
  }
}

void setDateTime(){
  char time[] = __TIME__;
  char date[] = __DATE__;
  byte hours = atoi(strtok(time, ":"));
  byte minutes = atoi(strtok(NULL, ":"));
  byte seconds = atoi(strtok(NULL, ":"));
  seconds += TIME_ADJUSTMENT; // adjust time
  timeToString(hours, minutes, seconds, time);
  rtc.adjust(DateTime(date, time));
}

void clearLcdRowSection(byte column, byte row, byte length){
  lcd.setCursor(column, row);
  for(int i = 0; i < length; i++){
    lcd.print(" ");
  }
}

void displayDateTime(){
   lcd.setCursor(0, 0);
   CurrentDateTime = rtc.now();
   if(isMidnight()) lcd.clear();
   
   #if(TIME_FORMAT == 0)
    char timeFormat[] = "hh:mm";
   #elif(TIME_FORMAT == 1)
    char timeFormat[] = "hh:mm:ss";
   #elif(TIME_FORMAT == 2)
    char timeFormat[] = "hh:mm AP";
   #else
    char timeFormat[] = "hh:mm";
   #endif

   lcd.print(CurrentDateTime.toString(timeFormat));
   lcd.setCursor(0, 1);
  
   #if(DATE_FORMAT == 0)
    char dateFormat[] = "DD.MM.YY";
    lcd.print(CurrentDateTime.toString(dateFormat));
   #elif(DATE_FORMAT == 1)
    char dateFormat[] = "DD.MM.YYYY";
    lcd.print(CurrentDateTime.toString(dateFormat));
   #elif(DATE_FORMAT == 2)
    char dateFormat[] = "DD.MM.YY";
    lcd.print(CurrentDateTime.toString(dateFormat));
    clearLcdRowSection(0, 2, strlen_P(pgm_read_word(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()])));
    lcd.setCursor(0,2);
    lcdPrintStringFromPGM(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()]);
   #elif(DATE_FORMAT == 3)
    char dateFormat[] = "DD.MM.YYYY";
    lcd.print(CurrentDateTime.toString(dateFormat));
    clearLcdRowSection(0, 2, strlen_P(pgm_read_word(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()])));
    lcd.setCursor(0,2);
    lcdPrintStringFromPGM(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()]);
   #elif(DATE_FORMAT == 4)  
    lcd.setCursor(0,1);
    lcdPrintStringFromPGM(&months[CurrentDateTime.month()-1]);
    lcd.print(",");
    lcd.print(CurrentDateTime.day());
    lcd.setCursor(0,2);
    lcdPrintStringFromPGM(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()]);
   #elif(DATE_FORMAT == 5)  
    char dateFormat[] = "MMM, DD";
    lcd.print(CurrentDateTime.toString(dateFormat));
    clearLcdRowSection(0, 2, strlen_P(pgm_read_word(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()])));
    lcd.setCursor(0,2);
    lcdPrintStringFromPGM(&daysOfTheWeek[CurrentDateTime.dayOfTheWeek()]);
  #endif
}

// 0 - turn off, 1 - green, 2 - yellow, 3 - red
void setLEDColor(byte color){
  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);
  analogWrite(LED_B, 0);

  switch(color){
    case 0: break;
    case 1: 
      analogWrite(LED_G, LED_BRIGHT_MAX);
      break;
    case 2:
      analogWrite(LED_R, LED_BRIGHT_MAX);
      analogWrite(LED_G, LED_BRIGHT_MAX-30);
      break;
    case 3: 
      analogWrite(LED_R, LED_BRIGHT_MAX);
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_COM, LOW);
  setLEDColor(0);
  
  lcd.begin();
  lcd.backlight();
  //lcd.noBacklight();
  lcd.clear();

  #if(DEBUG == 1)
    delay(50);
    if(rtc.begin()){
      Serial.println(F("RTC - OK"));
      lcd.print(F("RTC - OK"));
    }
    else{
      Serial.println(F("RTC - ERROR"));
      lcd.print(F("RTC - ERROR"));
    }
    lcd.setCursor(0, 1);
    delay(500);
    if(ccs.begin()){
      Serial.println(F("CCS811 - OK"));
      lcd.print(F("CCS811 - OK"));
    }
    else{
      Serial.println(F("CCS811 - ERROR"));
      lcd.print(F("CCS811 - ERROR"));
    }
    lcd.setCursor(0, 2);
    if(bme.begin(0x76)){
      Serial.println(F("BME280 - OK"));
      lcd.print(F("BME - OK"));
    }
    else{
      Serial.println(F("BME280 - ERROR"));
      lcd.print(F("BME - ERROR"));
    }
    delay(1000);
  #else
    rtc.begin();
    ccs.begin();
    bme.begin(0x76);
  #endif

  if (RESET_CLOCK || rtc.lostPower()){
    setDateTime();
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, //temperature
                  Adafruit_BME280::SAMPLING_X1, //pressure
                  Adafruit_BME280::SAMPLING_X1, //humidity
                  Adafruit_BME280::FILTER_OFF); 
   
  lcd.clear();

  takePressureSample();
  
  readSensors();
  
  if(mode == 0){
    displayShortSensors();
    displayDateTime();
  }
}


void buttonTick(){
  bool btn_flag = !digitalRead(2);
  if(btn_flag == true && btn_state == false && (millis() - timer) > 100){ 
    btn_state = true;
    btn_click = true;
    timer = millis();
  } 
  if(btn_flag == false && btn_state == true && (millis() - timer) > 100){
    btn_state = false;
    if(btn_click){
      btn_click = false;
      btn_OneClick = true;
    }
    btn_hold = false;
    timer = millis();
  }
  if(btn_flag == true && btn_state == true && (millis() - timer) > timeout && btn_hold == false){
    btn_hold = true;
    btn_isHolded = true;
    btn_click = false;
    timer = millis();
  }
}

bool isOneClick(){
  if(btn_OneClick){
    btn_OneClick = false;
    return true;
  } else return false;
}

bool isBtnHolded(){
  if(btn_isHolded){
    btn_isHolded = false;
    return true;
  } else return false;
}


void loop() {  

  if(mode == 0 && millis() - clockTimer >= 1000){
    clockTimer = millis();
    displayDateTime();
    if(isMidnight()) displayShortSensors();
  }

  if(millis() - sensorsTimer >(long)SENSORS_PERIOD){
    sensorsTimer = millis();
    readSensors();
  }

  if(millis() - pressureSampleTimer >= (long)PRESSURE_PERIOD){
    pressureSampleTimer = millis();
    takePressureSample();
  }

  if(millis() - airTimer >= 5000){
    airTimer = millis();
    checkAirQuality();
  }
  
  buttonTick();
  modeChange();
  
}
