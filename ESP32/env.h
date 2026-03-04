#ifndef ENV_H
#define ENV_H
#define led0 23
#define led1 21
#define led2 18
#define led3 5
#define led4 4
#define pot 15

#define WIFI_SSID "lele"
#define WIFI_PASS "20070318"
                                                          //led que já vem soldado na esp32 
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_client_id = "ESP32_leleca"; 

const char* topico_decimal = "dta/binario/lelana";
const char* topico_velocidade = "dta/velocidade/lelana";
const char* topico_status = "dta/status/lelana";
const int mqtt_port = 1883;


#endif