/*
  End-node LoRa 915 MHz
  Monitoramento da qualidade de água (Temperatura, Turbidez, pH)
  Autor: Felipe Xavier
*/

#include "Arduino.h" 
#include "heltec.h" 
#include "OneWire.h"
#include "DallasTemperature.h"
#include "EEPROM.h"
#include "DFRobot_ESP_PH.h" //https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK

const int oneWireBus = 13;    
OneWire oneWire(oneWireBus);
DallasTemperature dallasTemperature(&oneWire);

DFRobot_ESP_PH phsensor;

#define DisplayAtivo true 
#define LoRaAtivo true 
#define SerialAtivo true 
#define PABOOSTAtivo true // define se o amplificador de potência PABOOST estará ou não ativo
#define BandaTransmissao 915E6 //define a frequência média de transmissão: 868E6, 915E6

// Variaveis monitoradas
float temperatura = 25;
float ph = 0;
float turbidez = 3000;

void SetupLoRa()
{
  //Sinaliza o início dos ajustes
  Heltec.display -> drawString(0, 20, "Configurando LoRa...");
  Heltec.display -> display();
  delay(100);
  LoRa.setSpreadingFactor(7); //define o fator de espalhamento
  LoRa.setSignalBandwidth(250E3); //define a largura de banda
  LoRa.setCodingRate4(5); //define o Codding Rate
  LoRa.setPreambleLength(6); //define o comprimento do preâmbulo
  LoRa.setSyncWord(0x12); //define a palavra de sincronização
  //LoRa.disableCrc(); //desabilita o CRC
  LoRa.crc(); //ativa o CRC
  //Sinaliza o fim dos ajustes do rádio LoRa
  Heltec.display -> drawString(0, 30, "LoRa configurado!");
  Heltec.display -> display();
  delay(100);
}

void setup() {
  //inicia os dispositivos do devKit da Heltec (DEVE SER INICIADO ANTES DE TUDO)
  Heltec.begin(DisplayAtivo, LoRaAtivo, SerialAtivo, PABOOSTAtivo, BandaTransmissao);
  Heltec.display->clear(); //Limpa qualquer mensagem inicial do display
  SetupLoRa(); //Configura o rádio LoRa
  //Sinaliza o fim dos ajustes
  Heltec.display -> drawString(0, 40, "Setup End-node concluído!");
  Heltec.display -> display();

  dallasTemperature.begin();
  EEPROM.begin(32); //Necessario para a calibracao do sensor de pH
  phsensor.begin();

  delay(1000);
}

float getTurbidez() {
  int sensorValue = analogRead(37);
  float voltage = sensorValue * (3.3 / 4096);

  // Debug...
  /*
  Serial.println("Sensorvalue = " + String(sensorValue));
  Serial.print("voltage = ");
  Serial.println(voltage,2);*/

  // O ESP permite ate 3,3V de entrada GPIO e o sensor trabalha com 5V
  // Divisor de tensao: 220k/(220k+100k)
  // Entrada max = 4,5V ==> Saida = 2,18V
  // Multiplica para do nivel de tensao 3.3V => 5V, pois a parametrizacao é para 5V
  voltage = voltage*2.1;

  //Serial.print("voltage = ");   // Debug
  //Serial.println(voltage,2);

  //voltage *= 0.92;

 if(voltage>4.2)
    return 0;
  else if (voltage < 2.56)
    return 3000;
  
  //y = -1120.4x² + 5742.3x - 4352.9
  // parabola com raizes x1 = 0.92, x2 = 4.2 e ponto y maximo em x= 2.56
  return -1120.4*voltage*voltage + 5742.3*voltage - 4352.9;
}

float getpH(float temperatura)
{
  float tensao = analogRead(38)/4096.0*3300; 
  return phsensor.readPH(tensao,temperatura); 
  phsensor.calibration(tensao,temperatura);           // Calibracao por comunicacao serial.
}

void sendPacoteLoRa()
{
  //Serial.println("Enviando: temp=" + String(int(temperatura)) + " turbidez=" + String(int(turbidez)) + " ph=" + String(int(ph)));

  char json[250];

  sprintf(json,  "{\"temperatura\":%02.02f, \"turbidez\":%02.02f, \"ph\":%02.02f }", 
            temperatura, turbidez, ph); 

  LoRa.beginPacket();
  LoRa.print(json);
  LoRa.endPacket(); 
}

void loop() {
  // Leitura da temperatura
  dallasTemperature.requestTemperatures(); 
  temperatura = dallasTemperature.getTempCByIndex(0);

  // Leitura da turbidez
  turbidez = getTurbidez();

  // Leitura de pH
  ph = getpH(temperatura);

  // Envio para o gateway LoRa
  sendPacoteLoRa();
  delay(15000);
  //delay(1000);
  //Serial.println("Enviando: temp=" + String(temperatura) + " turbidez=" + String(turbidez) + " ph=" + String(ph));
}