/*
  End-node LoRa 915 MHz
  Monitoramento da qualidade de água (Temperatura, Turbidez, pH)
  Autor: Felipe Xavier

  Heltec 32 LoRa OLED 
                  ______________
             Gnd |    | USB |   | Gnd                                 Legenda:
              5v |    | port|   | 3v3                                       "<-" = Input Only!
             Vext|    |_____|   | 3v3
             Vext|              |  36<-     
              Rx |     GPIO     |  37<- Sensor de turbidez
              Tx |     3.3v     |  38<- Sensor de pH
             RST |   ________   |  39<- 
               0 |  |        |  |  34<-
  SD_CS       22 |  |        |  |  35<-
  LoRa_MISO   19 |  |        |  |  32<-
  SD_MOSI     23 |  |        |  |  33<-
  LoRa_CS     18 |  |        |  |  25  Sensor de temperatura
  LoRa_SCK     5 |  |  OLED  |  |  26  LoRa IRQ
              15 |  |        |  |  27  LoRa_MOSI
               2 |  |        |  |  14  LoRa Rst
               4 |  |        |  |  12  
  SD_CLK      17 |  |________|  |  13  SD_MISO
  Sens_ON     16 |______________|  21  

*/

#include "Arduino.h" 
#include "heltec.h" 
#include "OneWire.h"
#include "DallasTemperature.h"
#include "EEPROM.h"
#include "DFRobot_ESP_PH.h" // https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
#include "SPI.h" 
#include "FS.h"
#include "SD.h"

// Cartao SD pinout
#define  SD_CS      22
#define  SD_MOSI    23
#define  SD_MISO    13
#define  SD_CLK     17

// LoRa pinout
#define  LoRa_CS    18
#define  LoRa_MOSI  27
#define  LoRa_MISO  19
#define  LoRa_SCK    5
#define  LoRa_RST   14    
#define  DI0        26    //  LoRa_IRQ

// Sensores pinout
#define Sens_Temp  25
#define Sens_pH    38
#define Sens_Turb  37
#define Sens_ON    16

// Configuracoes Heltec ESP32 LoRa V2
//#define TempoOFF       120000000 // 2 min
#define TempoOFF         3000000 
#define TempoON          3000000 
#define DisplayAtivo     true 
#define LoRaAtivo        true  
#define SerialAtivo      true 
#define PABOOSTAtivo     true   // define se o amplificador de potência PABOOST estará ou não ativo
#define BandaTransmissao 915E6  // define a frequência média de transmissão: 868E6, 915E6

// Variaveis e objetos globais 
OneWire oneWire(Sens_Temp);
DallasTemperature dallasTemperature(&oneWire);
DFRobot_ESP_PH phsensor;
SPIClass spi1;
hw_timer_t * timer0 = NULL;
float temperatura = 0;
float ph = 0;
float turbidez = 0;

void setupLoRa()
{
  //Heltec.display -> drawString(0, 20, "Configurando LoRa...");
  //Heltec.display -> display();
  delay(100);
  LoRa.setSpreadingFactor(7);      // define o fator de espalhamento
  LoRa.setSignalBandwidth(250E3);  // define a largura de banda
  LoRa.setCodingRate4(5);          // define o Codding Rate
  LoRa.setPreambleLength(6);       // define o comprimento do preâmbulo
  LoRa.setSyncWord(0x12);          // define a palavra de sincronização
  //LoRa.disableCrc();             // desabilita o CRC
  LoRa.crc();                      // ativa o CRC
  //Heltec.display -> drawString(0, 30, "LoRa configurado!");
  //Heltec.display -> display();
  delay(100);
}

// Funcao de interrupcao do Timer0
void IRAM_ATTR onTimer0() {
  Serial.println("ESP entrando em deep sleep...");

  digitalWrite(Sens_ON, LOW);
  esp_deep_sleep_start(); // ESP32 entra em modo SLEEP
}

void setupTimer() 
{
  // INTERRUPCAO DO TIMER 0
  // (timer escolhido = 0, prescaler = 80 (80 MHz/80 = 1MHz), contador crescente)
  timer0 = timerBegin(0, 80, true); 
  // (A interrupcao do timer0 executa a funcao "onTimer0", funcao de interrupcao do timer, interrupcao na borda)
  timerAttachInterrupt(timer0, &onTimer0, true); 
  // Devido ao prescaler o timer0 tem periodo igual a 1us
  // Contando ate "TempoON" temos uma interrupcao do timer depois de "TempoON" segundos
  // autoreload = false
  timerAlarmWrite(timer0, TempoON, false);
  timerAlarmEnable(timer0);
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void setupSD() 
{
  SPIClass(1);
  spi1.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin( SD_CS, spi1)) 
  {
    Serial.println("Erro na leitura do arquivo não existe um cartão SD ou o módulo está conectado incorretamente...");
    return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("Nenhum cartao SD encontrado");
    return;
  }

  Serial.println("Inicializando cartao SD...");
  if (!SD.begin(SD_CS, spi1)) {
    Serial.println("ERRO - SD nao inicializado!");
    return; 
  }

  // Verifica se existe o arquivo de datalogger, casao nao, entao o arquivo e criado
  File file = SD.open("/data.csv");
  if(!file) 
  {
    Serial.println("SD: arquivo data.csv nao existe");
    Serial.println("SD: Criando arquivo...");
    writeFile(SD, "/data.csv", "Millis; Temperatura; Turbidez; pH; \r\n");
  }
  else {
    Serial.println("SD: arquivo ja existe");  
  }
  file.close();
}

void setupSensores(){
  dallasTemperature.begin();
  // Necessario para a calibracao do sensor de pH
  EEPROM.begin(32); 
  phsensor.begin();
}

void setup() 
{
  // DEVE SER INICIADO ANTES DE TUDO
  Heltec.begin(!DisplayAtivo, LoRaAtivo, SerialAtivo, PABOOSTAtivo, BandaTransmissao);
  Heltec.display->displayOff();
  Serial.println("Iniciando ESP32 ....");
  //Heltec.display->clear();

  pinMode(Sens_ON, OUTPUT);
  digitalWrite(Sens_ON, HIGH);

  // Configura o rádio LoRa
  setupLoRa(); 
  //Heltec.display -> drawString(0, 40, "Setup End-node concluído!");
  //Heltec.display -> display();

  // Tempo que ESP estara "dormindo" - deep sleep
  esp_sleep_enable_timer_wakeup( TempoOFF );
  
  // Configuracao inicial dos senores
  setupSensores();

  // Inicia o cartao SD
  setupSD();
  
  delay(1000);

  // Configuracao do timer - tempo em que o ESP estara ativo
  setupTimer();
}

float getTurbidez() 
{
  int sensorValue = analogRead(Sens_Turb);
  float voltage = sensorValue * (3.3 / 4096);

  /* O ESP permite ate 3,3V de entrada GPIO e o sensor trabalha com 5V
     Divisor de tensao: 220k/(220k+100k)
     Entrada max = 4,5V ==> Saida = 2,18V
     Multiplica o nivel de tensao lido por "2.1", pois a parametrizacao é para 5V (3.3V => 5V)
  */

  voltage = voltage*2.1;

 if(voltage>4.2)
    return 0;
  else if (voltage < 2.56)
    return 3000;

  /*  y = -1120.4x² + 5742.3x - 4352.9
      Parabola com raizes x1 = 0.92, x2 = 4.2 e ponto y maximo em x= 2.56
  */
  return -1120.4*voltage*voltage + 5742.3*voltage - 4352.9;
}

float getpH(float temperatura)
{
  float tensao = analogRead(Sens_pH)/4096.0*3300; 
  return phsensor.readPH(tensao,temperatura); 
  phsensor.calibration(tensao,temperatura);           // Calibracao por comunicacao serial.
}

void sendPacoteLoRa()
{
  Serial.println("Enviando: temp=" + String(int(temperatura)) + " turbidez=" + String(int(turbidez)) + " ph=" + String(int(ph)));

  char json[250];

  sprintf(json,  "{\"temperatura\":%02.02f, \"turbidez\":%02.02f, \"ph\":%02.02f }", 
            temperatura, turbidez, ph); 

  LoRa.beginPacket();
  LoRa.print(json);
  LoRa.endPacket(); 
}

void salvarDatalogger() 
{ 
  String leitura = String(millis()) + ";" + String(temperatura) + ";" + String(turbidez) + ";" + String(ph) + "; \r\n"; 
  
  Serial.println(leitura); 

  appendFile(SD, "/data.csv", leitura.c_str());
}

void loop() 
{
  // Leitura da temperatura
  dallasTemperature.requestTemperatures(); 
  temperatura = dallasTemperature.getTempCByIndex(0);

  // Leitura da turbidez
  turbidez = getTurbidez();

  // Leitura de pH
  ph = getpH(temperatura);

  // Salva os dados no cartao SD
  salvarDatalogger();

  // Envio para o gateway LoRa
  sendPacoteLoRa();
  //delay(15000);
  delay(1000);
  //Serial.println("Enviando: temp=" + String(temperatura) + " turbidez=" + String(turbidez) + " ph=" + String(ph));
}