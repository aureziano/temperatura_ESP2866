/*   Comunicação com ThinkSpeak e sinric
 *  Sensor de temperatura --> D13
 *  Sensor de Umidade --> D2 -->GPIO 4
 *  Relé --> D4 --> GPIO 2
 * 
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h> 
#include <StreamString.h>
#include <ESP8266WiFi.h>  //ESP8266 Core WiFi Library  
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal    
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal ( https://github.com/zhouhan0126/DNSServer---esp32 )
#include <WiFiManager.h>  // WiFi Configuration Magic ( https://github.com/zhouhan0126/WIFIMANAGER-ESP32 ) >> htt

//--------Temperatura------------
#include "OneWire.h"
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
//--------Umidade----------------
#include <DHT.h>

//GPIO do NodeMCU que o pino de comunicação do sensor está ligado.
#define DHTPIN D1
#define DHTTYPE DHT11 // DHT 11
//-------------------------------
/* constantes e variáveis globais */
char endereco_api_thingspeak[] = "api.thingspeak.com";
String chave_escrita_thingspeak = "VCSWY9PEGFYSJTPD";  /* Coloque aqui sua chave de escrita do seu canal */
unsigned long last_connection_time;
DHT dht(DHTPIN, DHTTYPE);
 
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
 
//Instalar o Driver CG340M 
//Configurações-----------------------------------------
#define ssid1 "DZ-302_EXT"
#define password1 "dz11270107"
#define ssid2 "HughesNet_GERALDO"
#define password2 "AURE0603"
#define ssid3 "DZ-302"
#define password3 "dz11270107"
#define INTERVALO_ENVIO_THINGSPEAK 30000 /* intervalo entre envios de dados ao ThingSpeak (em ms) */ 
#define MyApiKey "90964883-6f35-4901-81df-3b8445158c0f"
 
#define DispositivoID "6096bb44c26766757ecd5374"
 
#define RelayPin 2 //Pino onde o Relé está conectado
//------------------------------------------------------
 
 
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 
 
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;
 
void turnOn(String deviceId);
void turnOff(String deviceId);
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
/* prototypes de temperatura e Umidade*/
void envia_informacoes_thingspeak(String string_dados);
void init_wifi(void);
void conecta_wifi(void);
void verifica_conexao_wifi(void);
 
void setup() 
{
  pinMode( RelayPin, OUTPUT);
   
  Serial.begin(115200);
  last_connection_time = 0;
  DS18B20.begin();
  /* Inicializa e conecta-se ao wi-fi */
  init_wifi();
   
//  WiFiMulti.addAP(ssid, password);
//  Serial.println();
//  Serial.print("Conectando a Rede: ");
//  Serial.println(ssid);  
 
//  //Espera pela conexão WiFi
//  while(WiFiMulti.run() != WL_CONNECTED) 
//  {
//    delay(500);
//    Serial.print(".");
//  }
// 
//  if(WiFiMulti.run() == WL_CONNECTED) 
//  {
//    Serial.println("");
//    Serial.println("WiFi Conectado");
//    Serial.println("IP address: ");
//    Serial.println(WiFi.localIP());
//  }
 
  //Estabelece conexão com Sinric
  webSocket.begin("iot.sinric.com", 80, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  webSocket.setReconnectInterval(5000);

  /* Inicializa sensor de temperatura e umidade relativa do ar */
  dht.begin();

}
 
void loop()
{
  webSocket.loop();
  //dados da temperatura
  char fields_a_serem_enviados[100] = {0};
  float temperatura_lida = 0.0;
  float umidade_lida = 0.0;

  /* Força desconexão ao ThingSpeak (se ainda estiver conectado) */
  if (client.connected())
  {
      client.stop();
      Serial.println("- Desconectado do ThingSpeak");
      Serial.println();
  }

  /* Garante que a conexão wi-fi esteja ativa */
  verifica_conexao_wifi();
    
  /* Verifica se é o momento de enviar dados para o ThingSpeak */
  if( millis() - last_connection_time > INTERVALO_ENVIO_THINGSPEAK )
  {
      DS18B20.requestTemperatures(); 
      temperatura_lida = DS18B20.getTempCByIndex(0);
      umidade_lida = dht.readHumidity();
//      umidade_lida = DS18B20.getTempC(sensor);
      Serial.println(String("Temperatura 1:") + String(temperatura_lida, 2));
      sprintf(fields_a_serem_enviados,"field1=%.2f&field2=%.2f", temperatura_lida, umidade_lida);
      envia_informacoes_thingspeak(fields_a_serem_enviados);
  }

  delay(1000);
   
  if(client.connected()) 
  {
      Serial.println("Veio Aqui");
      uint64_t now = millis();
      //Mantem a conexão mesmo se houver mudança do IP
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) 
      {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
}
 
 
void turnOn(String deviceId) 
{
  if (deviceId == DispositivoID)
  {  
    Serial.print("Ligar o dispositivo ID: ");
    Serial.println(deviceId);
 
    digitalWrite( RelayPin, HIGH);
  } 
}
 
void turnOff(String deviceId) 
{
    if (deviceId == DispositivoID)
    {  
      Serial.print("Desligar o dispositivo ID: ");
      Serial.println(deviceId);
   
      digitalWrite( RelayPin, LOW);
    } 
}
 
//Lida com os pedidos recebidos pela Sinric
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) 
{
  if(type == WStype_TEXT)
  {
      Serial.printf("Comando Recebido: %s\n", payload);
#if ARDUINOJSON_VERSION_MAJOR == 5
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
      DynamicJsonDocument json(1024);
      deserializeJson(json, (char*) payload);      
#endif        
      String deviceId = json ["deviceId"];     
      String action = json ["action"];
       
      if(action == "action.devices.commands.OnOff")  //Comando para ligar/desligar 
      {
          String value = json ["value"]["on"];
          Serial.println(value); 
           
          if(value == "true") 
              turnOn(deviceId);
          else
              turnOff(deviceId);
      }
  }
}


/* Função: envia informações ao ThingSpeak
* Parâmetros: String com a informação a ser enviada
* Retorno: nenhum
*/
void envia_informacoes_thingspeak(String string_dados)
{
    if (client.connect(endereco_api_thingspeak, 80))
    {
        /* faz a requisição HTTP ao ThingSpeak */
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+chave_escrita_thingspeak+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(string_dados.length());
        client.print("\n\n");
        client.print(string_dados);
          
        last_connection_time = millis();
        Serial.println("- Informações enviadas ao ThingSpeak!");
    }
}

/* Função: inicializa wi-fi
* Parametros: nenhum
* Retorno: nenhum
*/
void init_wifi(void)
{
    WiFiMulti.addAP(ssid1, password1);
    Serial.println();
    Serial.print("Conectando a Rede: ");
    Serial.println(ssid1);  
  
    conecta_wifi();
}

/* Função: conecta-se a rede wi-fi
* Parametros: nenhum
* Retorno: nenhum
*/
void conecta_wifi(void)
{
    String ssid = "";
    /* Se ja estiver conectado, nada é feito. */
    if(WiFiMulti.run() == WL_CONNECTED)
    {
        return;
    }
      
    /* refaz a conexão */
    WiFiMulti.addAP(ssid1, password1);
    if(WiFiMulti.run() == WL_CONNECTED)
    {
        ssid = ssid1;
        return;
    }
    
    int tempo_wait = 0;  
    while (WiFiMulti.run() != WL_CONNECTED && tempo_wait < 10000)
    {
        delay(100);
        tempo_wait = tempo_wait + 100;
    }

    if(WiFiMulti.run() != WL_CONNECTED)
    {
      /* tenta a conexão wifi2 */
      WiFiMulti.addAP(ssid3, password3);
      int tempo_wait = 0;  
      while (WiFiMulti.run() != WL_CONNECTED && tempo_wait < 10000)
      {
          delay(100);
          tempo_wait = tempo_wait + 100;
      }
      ssid = ssid2;
    }
 
    if(WiFiMulti.run() != WL_CONNECTED)
    {
      /* tenta a conexão wifi3 */
      WiFiMulti.addAP(ssid3, password3);
      int tempo_wait = 0;  
      while (WiFiMulti.run() != WL_CONNECTED && tempo_wait < 10000)
      {
          delay(100);
          tempo_wait = tempo_wait + 100;
      }
      ssid = ssid3;
    }
    
    Serial.println("Conectado com sucesso a rede wi-fi \n");
    Serial.println(ssid);
}
  
/* Função: verifica se a conexao wi-fi está ativa
* (e, em caso negativo, refaz a conexao)
* Parametros: nenhum
* Retorno: nenhum
*/
void verifica_conexao_wifi(void)
{
    conecta_wifi();
}
