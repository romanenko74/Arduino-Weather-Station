void takePressureSample(){
  bme.takeForcedMeasurement();
  
  if(pressureSample[6] == 0) samples = 0;
  else samples++;
  
  Serial.println("Samples: " + String(samples));
  
  for(byte i = 0; i < 6; i++){
    pressureSample[i] = pressureSample[i+1];
    Serial.println(pressureSample[i]);
  }
  pressureSample[6] = bme.readPressure();
  Serial.println(pressureSample[6]);
  
  if(samples == 6) makeForecast();
}

void readSensors(){
  bme.takeForcedMeasurement();
  //temp = bme.readTemperature();
  temp = rtc.getTemperature();
  pressure = bme.readPressure() * 0.0075; // Pa => mmHg
  hum = bme.readHumidity();
  if(mode == 0) displayShortSensors();
  if(mode == 1) displaySensors();
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

//**************DEBUG********************//
//***************************************//

  Serial.println("******Air Quality******");
  Serial.print("TVOC: ");
  Serial.println(tvoc);
  Serial.print("CO2: ");
  Serial.println(co2);
  Serial.println("*********************");
  Serial.println();

//**************************************//
}

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

//**************DEBUG********************//

  Serial.println("********************");
  Serial.print("Temperature: ");
  Serial.println(String(temp, 2));
  Serial.print("Pressure: ");
  Serial.println(String(pressure) + " " + "mmHg");
  Serial.print("Humidity: ");
  Serial.println(String(hum) + "%");
  Serial.println("**********************");
  Serial.println();
  
//**************************************//
}

void makeForecast(){
  samples = 0;
  long pressureNow = bme.readPressure()/100; // Па => гПа
//****************DEBUG**********************//
  Serial.println("Pressure array:");
   for(byte i = 0; i < 7; i++){
    Serial.println(pressureSample[i]);
  }
  Serial.println();
//******************************************//
  
  // linear approximation
  sumX = 0;
  sumY = 0;
  sumX2 = 0;
  sumXY = 0;

  for(byte i = 0; i < 7; i++){
    sumX += pressureTime[i];
    sumY += pressureSample[i];  
    sumX2 += pressureTime[i] * pressureTime[i];
    sumXY += pressureSample[i] * pressureTime[i];
  }

  a = 7 * sumXY;
  a = a - (long)sumX*sumY;
  a = (float)(a / (7 * sumX2 - sumX * sumX));
  
  int delta = a*6; // разница между приближёнными значениями давления час назад и в текущий момент
  
//********DEBUG**********//
  Serial.print("a = ");
  Serial.println(a);
  Serial.print("delta = ");
  Serial.println(delta);
  Serial.println();
//************************//

  //Zambretti forecast:

  byte month = CurrentDateTime.month();
  //falling
  if(delta < 0 && abs(delta) > 15){                // if delta is less than 15 it is not considered as a pressure change
    Z = round(130 - (pressureNow / 8.1));
    if(month <= 12 && month > 9) Z++;
    if(month > 3 && month <10) Z += 2;

//***********DEBUG***************//
    Serial.println(F("FALLING"));
    Serial.print("Z = ");
    Serial.println(Z);
    Serial.println();
//******************************//
  }
  //raising
  if(delta > 0 && abs(delta) > 15){ 
    Z = round(179 - ((2*pressureNow) / 12.9));
    if(month <= 12 && month > 9) Z--;
    if(month > 3 && month <10) Z -= 2;
    
//***********DEBUG***************//
    Serial.println(F("RISING"));
    Serial.print(F("Z = "));
    Serial.print(Z);
    Serial.println();
//*******************************//
  }
  //settled
  if(delta > -15 && delta < 15){ 
    Z = round(147 - ((5*pressureNow) / 37.6));
    
//************DEBUG****************//
    Serial.println(F("SETTELED"));
    Serial.print("Z = ");
    Serial.print(Z);
    Serial.println();
//********************************//
  }  
  
  if(mode == 3) displayForecast();
}

void displayForecast(){
  lcd.clear();
  if(pressureSample[0] == 0)
     lcd.print("Forecast is not ready yet"); // if an hour hasn`t passsed and the forecast isn`t ready
  else lcdPrintStringFromPGM(&forecast[forecastIndex[Z-1]]);
}

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
    if(mode > 3) mode = 0;
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
      case 3: displayForecast();
        break;
    }
  }
}

bool isMidnight(){
  if(CurrentDateTime.hour() == 0 && CurrentDateTime.minute() == 0 && CurrentDateTime.second() == 0)
    return true;
  else return false;
}
