void readSensors(){
  bme.takeForcedMeasurement();
  //temp = bme.readTemperature();
  temp = rtc.getTemperature();
  pressure = bme.readPressure() * 0.0075; // Pa => mmHg
  hum = bme.readHumidity();
  if(mode == 0) displayShortSensors();
  if(mode == 1) displaySensors();
  printSensorsSerial();
}

void checkAirQuality(){
if(ccs.available()){
    if(!ccs.readData()){
      co2 = ccs.geteCO2();
      tvoc = ccs.getTVOC();
      ccs.setEnvironmentalData(temp, hum);

       if(co2 < 600 && tvoc < 200)
          setLEDColor(1);
       else
          if(co2 > 1000 || tvoc > 600) 
            setLEDColor(3);
          else setLEDColor(2);
    }
  }
  if(mode == 2) displayAir();
  printAirSerial();
}

//temp, hum, pressure
void displayShortSensors(){
  lcd.setCursor(14, 0);
  lcd.print(String(temp, 1));
  lcd.write(0xDF);
  lcd.print("C");
  lcd.setCursor(12, 1);
  lcd.print(String(pressure) + " " + "mmHg");
  if(hum < 100) lcd.setCursor(17, 2);  
  else lcd.setCursor(16, 2);
  lcd.print(String(hum) + "%");
}

void displayAir(){
  lcd.setCursor(4, 0);
  lcd.print("Air Quality");
  lcd.setCursor(0, 2);
  lcd.print("TVOC: ");
  lcd.print("              ");
  lcd.setCursor(6, 2);
  lcd.print(String(tvoc) + " ppb");
  lcd.setCursor(0, 3);
  lcd.print("CO2: ");
  lcd.print("               ");
  lcd.setCursor(5, 3);
  lcd.print(String(co2) + " ppm");
}

//**************DEBUG********************//
void printAirSerial(){
  Serial.println("******Air Quality******");
  Serial.print("TVOC: ");
  Serial.println(tvoc);
  Serial.print("CO2: ");
  Serial.println(co2);
  Serial.println("********************");
  Serial.println();
}
//**************************************//

void displaySensors(){
  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  lcd.print(String(temp, 1));
  lcd.write(0xDF);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Pressure: ");
  lcd.print(String(pressure) + " " + "mmHg");
  lcd.setCursor(0,2);
  lcd.print("Humidity: ");
  lcd.print(String(hum) + "%");
}

//**************DEBUG********************//
void printSensorsSerial(){
  Serial.println("********************");
  Serial.print("Temperature: ");
  Serial.println(String(temp, 2));
  Serial.print("Pressure: ");
  Serial.println(String(pressure) + " " + "mmHg");
  Serial.print("Humidity: ");
  Serial.println(String(hum) + "%");
  Serial.println("*********************");
  Serial.println();
}
//**************************************//


void lcdPrintStringFromPGM(char** stringPointerAddress){
    uint16_t stringPtr = pgm_read_word(stringPointerAddress); 
    while(pgm_read_byte(stringPtr) != NULL){
      lcd.print(char(pgm_read_byte(stringPtr)));
      stringPtr++;
    }
}

void modeChange(){
  bool changeMode = false;
  if(isOneClick()){
    changeMode = true;
    mode++;
    if(mode > 2) mode = 0;
  }
  if(isBtnHolded()){
    changeMode = true;
    mode = 0;
  }
  if(changeMode == true){
    lcd.clear();
    switch(mode){
      case 0:
        displayDateTime();
        displayShortSensors();
        break;
      case 1: displaySensors();
        break;
      case 2: displayAir();
        break;
    }
  }
}

bool isMidnight(){
  if(CurrentDateTime.hour() == 0 && CurrentDateTime.minute() == 0 && CurrentDateTime.second() == 0)
    return true;
  else return false;
}

float callback_temp(const char *itemID, const opcOperation opcOP, const float value){
  return temp;
}

int callback_press(const char *itemID, const opcOperation opcOP, const int value){
  return pressure;
}

int callback_hum(const char *itemID, const opcOperation opcOP, const int value){
  return hum;
}

int callback_co2(const char *itemID, const opcOperation opcOP, const int value){
  return co2;
}

int callback_tvoc(const char *itemID, const opcOperation opcOP, const int value){
  return tvoc;
}
