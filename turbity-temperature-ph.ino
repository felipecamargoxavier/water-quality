#include <OneWire.h>
 #include <EEPROM.h>
#include "DFRobot_PH.h"

#define  DS18S20_Pin 2 // Sensor temperatura
#define  PH_PIN A1     // Sensor pH

OneWire ds(DS18S20_Pin);  
DFRobot_PH ph;

void setup(void) {
  Serial.begin(9600);
  ph.begin();
}

void loop(void) {
  // Leitura temperatura
  float temperature = getTemp();

  // Leitura turbidez
  float turbity = getTurbity();

  // Leitura pH
  float phValue = getpH(temperature);
  
  Serial.print("Temp: ");
  Serial.print(temperature,1);
  Serial.print("^C  pH:");
  Serial.print(phValue,2);
  Serial.print(" Turbidez: ");
  Serial.println(turbity); 
  
  delay(1000);
}

float getpH(float temperature){
  float voltage = analogRead(PH_PIN)/1024.0*5000; 
  return ph.readPH(voltage,temperature); 
  //ph.calibration(voltage,temperature);           // calibration process by Serail CMD
}

float getTurbity() {
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1024.0);
  
  if(voltage>4.2)
    return 0;
  else if (voltage < 2.56)
    return 3000;
    
  //y = -1120.4x² + 5742.3x - 4352.9
  // raizes x1 = 0.92 e x2 = 4.2, ponto max: xmax = 2.56
  return -1120.4*voltage*voltage + 5742.3*voltage - 4352.9;
}

float getTemp(){
  // Retorna a temperatura do sensor DS18S20 em Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;
}
