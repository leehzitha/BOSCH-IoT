#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long tempo = 0;
bool isConnected = false;
int leds[] = {led0, led1, led2, led3, led4};
int bin[] = { 0, 0, 0, 0, 0 };
int dec[] = { 16, 8, 4, 2, 1 };
int decimal = 0;
int count = 0;
int tempoLed = 0;
int duration = 0;
hw_timer_t *timer = NULL;
hw_timer_t *timerWifi = NULL;
bool publicar = false;

//callback do wifi
void WiFiEvent(WiFiEvent_t event) {                                              
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP: 
            Serial.println("WiFi Conectado! IP: " + WiFi.localIP().toString());              
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi perdido. O driver tentará reconectar sozinho...");
            break;
    }
}

void callback(char* topic, byte* payload, unsigned int lenght) {
  String guardar = String(topic);

  Serial.print("Lendo tópico: ");
  Serial.println(guardar);

  String msg = "";

  for (unsigned int i = 0; i < lenght; i++) {                     //transforma o payload recebido do mqtt para string
    msg += (char)payload[i];
  }

  Serial.println(msg);
}

boolean attemptMqttConnection() {                                     //função de Tentativa de Conexão (Retorna true/false, não trava o código)
 
  String clientId = String(mqtt_client_id) + "_" + String(WiFi.macAddress()); //gera ID Único (Evita colisão entre alunos)
  
  Serial.print("Tentando MQTT como: " + clientId + "... ");

  if (client.connect(
        clientId.c_str(),                                               //ID DO CLIENTE: Quem sou eu? (Ex: "ESP32_Patrick_MAC")
        NULL,                                                           //USUÁRIO: Tem login? (NULL pois o HiveMQ é público/anônimo)
        NULL,                                                           //SENHA: Tem senha? (NULL pois o HiveMQ é público/anônimo)
        topico_status,                                                  //TÓPICO DO TESTAMENTO (LWT): Onde publicar se a conexão cair?
        0,                                                              //QoS DO TESTAMENTO: Qual a prioridade? (0 = mais rápido/leve)
        true,                                                           //RETAIN (FIXAR): true = A mensagem fica "colada" no tópico
        "OFFLINE"                                                       //MENSAGEM DO TESTAMENTO: O que o Broker deve gritar por mim?
     )) {
    
    Serial.println("CONECTADO!");
    client.subscribe(topico_decimal);
    client.subscribe(topico_velocidade);                               //assina o tópico de comando
    return true;
  } 
  else {
    Serial.print("Falha. rc=");
    Serial.print(client.state());
    return false;
  }
}

void IRAM_ATTR wifi() {
  if (!isConnected)                                                      //interrupção a cada 1ms, só incrementa se não estiver conectado
    tempo++;
}

void IRAM_ATTR onTimer() {
  tempoLed ++;
  if (tempoLed >= duration) {
    // Contador binário
    for (int i = 4; i >= 0; i --){
      if (bin[i] == 0) {
        bin[i] = 1;
        break;
      }
      bin[i] = 0; 
    }

    // Acendimento dos leds
    for (int i = 0; i < 5; i ++){
      if (bin[i] == 1) {
        digitalWrite(leds[i], 1);
        decimal += dec[i];
      }
      else {
        digitalWrite(leds[i], 0);
      }
    }
    publicar = true;
    
    tempoLed = 0;

  }
}
void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 5; i++){
    pinMode(leds[i], OUTPUT);
  }

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000, true, 0);

  //configuração do wifi
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);                                               
  
  //configuração do mqtt
  timerWifi = timerBegin(1000000);
  timerAttachInterrupt(timerWifi, &wifi);
  timerAlarm(timerWifi, 1000, true, 0);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);         

}

void loop() {
  taConectado();

  // for (int i = 0; i < 5; i ++){
  //   Serial.print(bin[i]);
  // }
  // Serial.println();
  // Serial.println(decimal);

  duration = analogRead(pot);
  if (publicar) {
    client.publish(topico_decimal, String(decimal).c_str(), true);
    client.publish(topico_velocidade, String(duration).c_str(), true);
    publicar = false;
    decimal = 0;
  }
  // delay(100);
}


void taConectado(){
if (!client.connected()) {
    isConnected = false;
    if (tempo > 5000) {                                     // Só tenta conectar se já passaram 5 segundos desde a última vez
      tempo = 0;
      if (attemptMqttConnection())                          // Tenta conectar. Se falhar, o loop continua rodando (não trava o Wi-Fi)
        tempo = 0;
    }
  } 
  else {
    isConnected = true;
    client.loop();                                          // Se está conectado, mantém o Keep Alive
  }
}