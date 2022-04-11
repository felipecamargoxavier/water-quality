
#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"
#include "Adafruit_ADS1X15.h"
#include "Adafruit_BusIO_Register.h"
#include "Adafruit_BMP280.h"
#include "RTClib.h" 
#include "SPI.h" 
#include "FS.h"
#include "SD.h"

SPIClass spi1;

#define I2C_SDA 21
#define I2C_SCL 22

// Cartao SD pinout
#define  SD_CS      5
#define  SD_MOSI    23
#define  SD_MISO    19
#define  SD_CLK     18

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */
Adafruit_BMP280 bmp;
RTC_DS1307 rtc;


float getLeituraADS() 
{
  int16_t adc0, adc1, adc2, adc3;
  float volts0, volts1, volts2, volts3;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);

  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);

  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
  Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
  Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");

  return adc3;
}

void getDadosBMP280()
{
  Serial.println("-----------------------------------------------------------");
  Serial.print("TEMPERATURA INTERNA: "); Serial.print(String(bmp.readTemperature())); Serial.println(" °C");
  Serial.print("PRESSAO INTERNA: "); Serial.print(String(bmp.readPressure()/101325)); Serial.println(" atm");

}

void getDataHora()
{
  Serial.println("-----------------------------------------------------------");
  Serial.print("DATA-HORA: "); 
  Serial.print(String(rtc.now().day())); 
  Serial.print("/");
  Serial.print(String(rtc.now().month())); 
  Serial.print("/");
  Serial.print(String(rtc.now().year())); 
  Serial.print("  ");
  Serial.print(String(rtc.now().hour())); 
  Serial.print(":");
  Serial.print(String(rtc.now().minute())); 
  Serial.print(":");
  Serial.println(String(rtc.now().second())); 
}

void setupADS()
{
  Serial.println("Hello!");

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
   ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  if (!ads.begin(0x48)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

void setupSensorBMP280()
{

if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
}

void setupRTC()
{
  rtc.begin();                                        // Inicia o módulo RTC
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));     // Ajuste Automático da hora e data
  //rtc.adjust(DateTime(2019, 11, 29, 10, 23, 00));   // Ajuste Manual (Ano, Mês, Dia, Hora, Min, Seg)
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

void salvarDatalogger() 
{ 
  String leitura = String(millis()) + ";" + String("temperatura") + ";" + String("turbidez") + ";" + String(" ") + "; \r\n"; 
  
  Serial.println(leitura); 

  appendFile(SD, "/data.csv", leitura.c_str());
}


void setup(void)
{
  Wire.begin(I2C_SDA, I2C_SCL);
 
  Serial.begin(115200);

  setupSD();

  setupRTC();

  setupADS();

  setupSensorBMP280();

}

void loop(void)
{
    getDataHora();

    getDadosBMP280();

    getLeituraADS(); 

    salvarDatalogger();

    delay(1000);
}



/*
void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}
 
void loop() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);          
}
*/
