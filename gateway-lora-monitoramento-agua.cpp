/* GATEWAY LoRa - Publisher MQTT
  Monitoramento de qualidade de agua
  Autor: Felipe Xavier
*/

#include "heltec.h" 
#include "WiFi.h" 
#include "PubSubClient.h"
#include "ArduinoJson.h"

// Configuracoes MQTT | WiFi
#define SSID "" //Identificador do serviço de rede WiFi
#define Senha "" //Senha de acesso ao serviço WiFi
#define TOKEN "" //TOKEN do Ubidots 
#define DEVICE_ID "" //ID do dispositivo 
#define SERVER "" //Servidor do Ubidots (broker)
#define PORT 1883 //Porta padrão Ubidots
#define TOPIC "" //Topico Ubidots

WiFiClient wiFiClient;
PubSubClient pubSubClient(wiFiClient);

// Configuracoes ESP32 LoRa
#define DisplayAtivo true //define se o display estará ou não ativo
#define LoRaAtivo true //define se o rádio LoRa estará ou não ativo
#define SerialAtivo true //define se a Serial do módulo estará ou não disponível (Baud Rate padrão 115200)
#define PABOOSTAtivo true // define se o amplificador de potência PABOOST estará ou não ativo
#define BandaTransmissao 915E6 //define a frequência média de transmissão: 868E6, 915E6

// Variaveis medidas
float temperatura, turbidez, pH = 0.00;

void SetupLoRa()
{
  LoRa.setSpreadingFactor(7); //define o fator de espalhamento
  LoRa.setSignalBandwidth(250E3); //define a largura de banda
  LoRa.setCodingRate4(5); //define o Codding Rate
  LoRa.setPreambleLength(6); //define o comprimento do preâmbulo
  LoRa.setSyncWord(0x12); //define a palavra de sincronização
  //LoRa.disableCrc(); //desabilita o CRC
  LoRa.crc(); //ativa o CRC
}

void SetupWiFi()
{
  //inicio dos ajustes do conexão WiFi
  WiFi.disconnect(true); //Desconecta de uma possível conexão prévia
  delay(100);

  IPAddress ip(192, 168, 0, 119); // Definição de um IP fixo
  IPAddress gateway(192, 168, 0, 1); //Gateway da rede
  IPAddress subnet(255, 255, 255, 0); //Máscara de rede
  WiFi.mode(WIFI_STA); //Ajusta para o modo STATION
  WiFi.setAutoConnect(true); //Ativa a auto reconexão
  WiFi.config(ip, gateway, subnet); //Configura os endereços fixos
  WiFi.begin(SSID, Senha); //Informa o id do Serviço WiFi e a senha
  delay(100);
  
  byte count = 0; // Contador das tentativas de conexão
  
 while (WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Heltec.display -> drawString(0, 0, "Conectando WiFi...");
    Heltec.display -> display();
  }
  if (WiFi.status() == WL_CONNECTED) //Se conseguiu conectar...
  {
    Heltec.display -> drawString(0, 10, "Conexão WiFi ok!");
    Heltec.display -> display();
    delay(1000);
  }
  else // ... caso falhe a conexão
  {
    Heltec.display -> drawString(0, 10, "Falha na conexão!");
    Heltec.display -> display();
    delay(1000);
    //pode aplicar um loop infinito ou reset AQUI;
  }
}

bool mqttInit()
{
  //Inicia WiFi com o SSID e a senha
  SetupWiFi();
 
  //Loop até que o WiFi esteja conectado
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Estabelecendo conexao WiFi..");
  }
 
  //Exibe no monitor serial
  Serial.println("Conectado a rede WiFi");

  //Seta servidor com o broker e a porta
  pubSubClient.setServer(SERVER, PORT);
  
  //Conecta no ubidots com o Device id e o token, o password é informado como vazio
  while(!pubSubClient.connect(DEVICE_ID, TOKEN, ""))
  {
    Serial.println("MQTT - Erro de conexao");
    return false;
  }

  Serial.println("MQTT - Conexao ok.");
  return true;
}

void reconnectMQTT() 
{  
  //Loop até que o MQTT esteja conectado
  while (!pubSubClient.connected()) 
  {
    //sinaliza desconexão do mqtt no display
    Serial.println("Reconectando MQTT...");
    
    //Tenta conectar
    if (pubSubClient.connect(DEVICE_ID, TOKEN,"")) 
      Serial.println("connected");
    else 
    {
      Serial.print("Falha, rc=");
      Serial.print(pubSubClient.state());
      delay(2000);
    }
  }
}

void setup() 
{
  //inicia o ESP WiFi LoRa 32 da Heltec
  Heltec.begin(DisplayAtivo, LoRaAtivo, SerialAtivo, PABOOSTAtivo, BandaTransmissao);

  //Inicializa mqtt (conecta o esp com o wifi, configura e conecta com o servidor da ubidots)
  if(!mqttInit())
  {        
    delay(3000);
    Heltec.display -> drawString(0, 0, "MQTT Falhou");
    Heltec.display -> drawString(0, 10, "ESP restart...");
    ESP.restart();
  }

  SetupLoRa(); //faz os ajustes do rádio LoRa
  delay(100);
  //Sinaliza o fim dos ajustes
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Setup concluído!");
  Heltec.display -> drawString(0, 10, "Aguardando dados...");
  Heltec.display -> display();
}

bool getDados()
{
  // tenta analisar o pacote
  int packetSize = LoRa.parsePacket();
  if (packetSize)  // se o pacote não tiver tamanho zero
  {
    // Leitura JSON recebido...
    String LoraData;
    while (LoRa.available())
    {
      LoraData = LoRa.readString();
      Serial.println(LoraData);
    }

    DynamicJsonDocument doc(200);
    deserializeJson(doc, LoraData);

    temperatura = doc["temperatura"];
    turbidez = doc["turbidez"];
    pH = doc["ph"];

    
    //Sinaliza os dados recebidos e o RSSI no display
    Heltec.display -> clear();
    Heltec.display -> drawString(0, 0, "Temperatura: " + String(temperatura));
    Heltec.display -> drawString(0, 10, "Turbidez: " + String(turbidez));
    Heltec.display -> drawString(0, 20, "pH: " + String(pH));
    Heltec.display -> drawString(0, 30, "RSSI: " + String(LoRa.packetRssi()));
    Heltec.display -> display();
    //Sinaliza os dados recebidos e o RSSI na serial 
    Serial.print("Temperatura: ");
    Serial.println(temperatura,2);
    Serial.print("Turbidez: " );
    Serial.println(turbidez,2);
    Serial.print("pH: ");
    Serial.println(pH,2);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));

    return true;
  }

  return false;
}


//Envia valores por mqtt
bool sendDados()
{
  Serial.println("Enviando dados MQTT...");
  char json[250];
 
  //Atribui para a cadeia de caracteres "json" os valores referentes a temperatura e os envia para a variável do ubidots correspondente
  sprintf(json,  "{\"temperatura\":{\"value\":%02.02f}}", temperatura);  

  if(!pubSubClient.publish(TOPIC, json))
    return false;

  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  sprintf(json,  "{\"turbidez\":{\"value\":%02.02f}}", turbidez);  
      
  if(!pubSubClient.publish(TOPIC, json))
    return false;

  sprintf(json,  "{\"ph\":{\"value\":%02.02f}}", pH);  
      
  if(!pubSubClient.publish(TOPIC, json))
    return false;

  //Se tudo der certo retorna true
  return true;
}

void loop() {
  if(getDados())
    sendDados();
}
