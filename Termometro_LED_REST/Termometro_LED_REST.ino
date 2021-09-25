/*
  Exemplo básico de conexão a Konker Plataform via HTTP, 
  Este exemplo se utiliza das bibliotecas do ESP8266 programado via Arduino IDE 
  (https://github.com/esp8266/Arduino).
*/
#define ARDUINOJSON_USE_LONG_LONG 1

//Inserindo os dados do termistor usado

// Resistencia nominal a 25C (Estamos utilizando um MF52 com resistencia nominal de 1kOhm)
#define TERMISTORNOMINAL 1000      
// Temperatura na qual eh feita a medida nominal (25C)
#define TEMPERATURANOMINAL 25   
//Quantas amostras usaremos para calcular a tensao media (um numero entre 4 e 10 eh apropriado)
#define AMOSTRAS 4
// Coeficiente Beta (da equacao de Steinhart-Hart) do termistor (segundo o datasheet eh 3100)
#define BETA 3100
// Valor da resistencia utilizada no divisor de tensao (para temperatura ambiente, qualquer resistencia entre 470 e 2k2 pode ser usada)
#define RESISTOR 470   

//Inserindo dados dos LEDs
#define PIN01 D1
#define PIN02 D2
#define PIN03 D3

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h> 
#include <ESP8266HTTPClient.h>

// Vamos primeiramente conectar o ESP8266 com a rede Wireless (mude os parâmetros abaixo para sua rede).

// Dados da rede WiFi
const char* ssid = "";
const char* password = "";

// Dados do dispositivo e servidor HTTP/REST
const char* USER = "";
const char* PWD = "";

const char* http_publication_url = "";
const char* http_subscription_url = "";


//Variaveis gloabais desse codigo
char bufferJ[256];
char *mensagem;
String payload;
int httpCode = 0;
unsigned long long timestamp = 0;
unsigned long long last_timestamp = 0;

//Variaveis do termometro
float temperature;
float received_temperature;
float tensao;
float resistencia_termistor;
int i = 0;


//Vamos criar uma funcao para formatar os dados no formato JSON
char *jsonMQTTmsgDATA(const char *device_id, const char *metric, float value) {
  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> jsonMSG;
  jsonMSG["deviceId"] = device_id;
  jsonMSG["metric"] = metric;
  jsonMSG["value"] = value;
  serializeJson(jsonMSG, bufferJ);
  return bufferJ;
}
//E outra funcao para ler os dados
float jsonMQTT_temperature_msg(String msg)
{
  StaticJsonDocument<384> doc;

  DeserializationError error = deserializeJson(doc, msg);
  
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return -1000;
  }
  
  JsonObject root_0_meta = doc[0]["meta"];
  timestamp = root_0_meta["timestamp"]; 
  
  JsonObject root_0_data = doc[0]["data"];
  float temperature = root_0_data["value"]; 
   
  return temperature;
}
//Criando os objetos de conexão com a rede e com o servidor HTTP.
WiFiClient espClient;
HTTPClient http_p;
HTTPClient http_g;

void setup_wifi() {
  delay(10);
  // Agora vamos nos conectar em uma rede Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Imprimindo pontos na tela ate a conexao ser estabelecida!
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereco de IP: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  //Configurando a porta Serial e escolhendo o servidor MQTT
  Serial.begin(115200);
  pinMode(PIN01, OUTPUT);
  pinMode(PIN02, OUTPUT);
  pinMode(PIN03, OUTPUT);
  digitalWrite(PIN01, LOW);
  digitalWrite(PIN02, LOW);
  digitalWrite(PIN03, LOW);
  setup_wifi();
  http_p.begin(espClient,"data.demo.konkerlabs.net",80,http_publication_url);
  http_g.begin(espClient,"data.demo.konkerlabs.net",80,http_subscription_url);
  http_p.addHeader("Content-Type", "application/json");
  http_p.addHeader("Accept", "application/json");
  http_p.setAuthorization(USER, PWD);
  http_g.addHeader("Content-Type", "application/json");
  http_g.addHeader("Accept", "application/json");
  http_g.setAuthorization(USER, PWD);
  
}

void loop()
{
 
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
  //Tirando a media do valor lido no ADC
  tensao = 0;
  for (i=0; i< AMOSTRAS; i++) {
   tensao += analogRead(0)/AMOSTRAS;
   delay(10);
  }
  //Calculando a resistencia do Termistor
  resistencia_termistor = RESISTOR*tensao/(1023-tensao);
  //Equacao de Steinhart-Hart
  temperature = (1 / (log(resistencia_termistor/TERMISTORNOMINAL) * 1/BETA + 1/(TEMPERATURANOMINAL + 273.15))) - 273.15;
  //Vamos imprimir via Serial o resultado para ajudar na verificacao
  Serial.print("Resistencia do Termistor: "); 
  Serial.println(resistencia_termistor);
  Serial.print("Temperatura: "); 
  Serial.println(temperature);
  
  mensagem = jsonMQTTmsgDATA("My_favorite_thermometer", "Celsius", temperature);
  
  //Enviando via HTTP o resultado calculado da temperatura
  httpCode=http_p.POST(mensagem);
  Serial.print("Codigo de resposta: ");
  Serial.println(httpCode);
  
  //Agora vamos consultar na plataforma via HTTP
  httpCode = http_g.GET();
  if (httpCode>0){
    payload = http_g.getString();
    Serial.print("Mensagem recebida da plataforma: ");
    Serial.println(payload); 
    received_temperature = jsonMQTT_temperature_msg(payload);
    if (received_temperature > -999){
      if (timestamp > last_timestamp){
        if (received_temperature>10.0) digitalWrite(PIN01, HIGH);
          else digitalWrite(PIN01, LOW);
        if (received_temperature>18.0) digitalWrite(PIN02, HIGH);
          else digitalWrite(PIN02, LOW);
        if (received_temperature>25.0) digitalWrite(PIN03, HIGH);
          else digitalWrite(PIN03, LOW);
      last_timestamp = timestamp;
      }
      else{
            digitalWrite(PIN01, LOW);
            digitalWrite(PIN02, LOW);
            digitalWrite(PIN03, LOW);
      }
    }
  }
  Serial.println("");
  //Gerando um delay de 2 segundos antes do loop recomecar
  delay(2000);
}
