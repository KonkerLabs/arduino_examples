/*
  Exemplo básico de conexão a Konker Plataform via MQTT, enviando dados de um LDR.
  Este exemplo se utiliza das bibliotecas do ESP8266 programado via Arduino IDE 
  (https://github.com/esp8266/Arduino) e a biblioteca PubSubClient que pode ser 
  obtida em: https://github.com/knolleary/pubsubclient/
*/

//Inserindo os dados da aquisição de dados
        
//Quantas amostras usaremos para calcular a tensao media (um numero entre 4 e 10 eh apropriado)
#define AMOSTRAS 4
#define THRESHOLD 1000 //O threshold (limiar) eh o numero entre 0 e 1024 registrado pelo ADC que usaremos como limiar entre luz acesa e apagada

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 

// Vamos primeiramente conectar o ESP8266 com a rede Wireless (mude os parâmetros abaixo para sua rede).

// Dados da rede WiFi
const char* ssid = "";
const char* password = "";

// Dados do servidor MQTT
const char* mqtt_server = "";

const char* USER = "";
const char* PWD = "";

const char* PUB_a = "";
const char* PUB_d = "";
const char* SUB = "";


//Variaveis gloabais desse codigo
char bufferJ[256];
char *mensagem;

//Variaveis do LDR
int luz;
float tensao;
int i = 0;

//Vamos criar uma funcao para formatar os dados no formato JSON
char *jsonMQTTmsgDATA(const char *metric, float value) {
  const int capacity = JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> jsonMSG;
  jsonMSG["metric"] = metric;
  jsonMSG["value"] = value;
  serializeJson(jsonMSG, bufferJ);
  return bufferJ;
}

//Criando os objetos de conexão com a rede e com o servidor MQTT.
WiFiClient espClient;
PubSubClient client(espClient);

//Criando a funcao de callback
//Essa funcao eh rodada quando uma mensagem eh recebida via MQTT.
//Nesse caso ela eh muito simples: imprima via serial o que voce recebeu
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Entra no Loop ate estar conectado
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Usando um ID unico (Nota: IDs iguais causam desconexao no Mosquito)
    // Tentando conectar
    if (client.connect(USER, USER, PWD)) {
      Serial.println("connected");
      // Subscrevendo no topico esperado
      client.subscribe(SUB);
    } else {
      Serial.print("Falhou! Codigo rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Esperando 5 segundos para tentar novamente
      delay(5000);
    }
  }
}

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
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  //O programa em si eh muito simples: 
  //se nao estiver conectado no Broker MQTT, se conecte!
  if (!client.connected()) {
    reconnect();
  }
  
  tensao = 0;
  
  //Tirando a media do valor lido no ADC
  for (i=0; i< AMOSTRAS; i++) {
   tensao += analogRead(0)/AMOSTRAS;
   delay(10);
  }
  if (tensao > THRESHOLD) {
    luz = 0;
  }
  else {
    luz = 1;
  }
  tensao = 3.3*tensao/1024.0;
  Serial.print("Tensao Analogica: "); 
  Serial.println(tensao);
  Serial.print("Luz: "); 
  Serial.println(luz);
  
  //Enviando via MQTT o resultado analogico e digital
  mensagem = jsonMQTTmsgDATA("Volts", tensao);
  client.publish(PUB_a, mensagem); 
  mensagem = jsonMQTTmsgDATA("Luz", luz);
  client.publish(PUB_d, mensagem); 
  client.loop();
  
  //Gerando um delay de 2 segundos antes do loop recomecar
  delay(2000);
}
