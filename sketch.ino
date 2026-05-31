#include <WiFi.h>
#include <PubSubClient.h>

// Configurações de rede virtuais do Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

// Configurações do Broker MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Definição dos pinos
const int PIR_PIN = 14;
const int LDR_PIN = 34;
const int LED_PIN = 12;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println("\n--- Conectando ao Wi-Fi ---");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado com sucesso!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao Broker MQTT...");
    String clientId = "ESP32Client-Emanuelle-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println(" Conectado!");
    } else {
      Serial.print(" Falhou, erro c = ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 3 segundos...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leitura dos sensores
  int valorLDR = analogRead(LDR_PIN);
  int presenca = digitalRead(PIR_PIN);

  Serial.print("Luminosidade (LDR): ");
  Serial.print(valorLDR);
  Serial.print(" | Presenca (PIR): ");
  Serial.println(presenca);

  // Lógica de Iluminação Inteligente (ODS 11)
  // No Wokwi, o pino analógico 2 lê valores mais altos no escuro
  if (valorLDR > 2000 && presenca == HIGH) {
    digitalWrite(LED_PIN, HIGH); // Liga o poste de luz
    Serial.println("Poste LIGADO: Ambiente escuro e com movimento.");
  } else {
    digitalWrite(LED_PIN, LOW);  // Mantém desligado economizando energia
  }

  // Envio dos dados via MQTT para o monitoramento remoto cobrado pelo professor
  String payload = "{\"luminosidade\":" + String(valorLDR) + ",\"movimento\":" + String(presenca) + "}";
  client.publish("mackenzie/emanuelle/iluminacao", payload.c_str());
  
  delay(2500); 
}
