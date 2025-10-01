#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configurações WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações MQTT
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

// ID do beacon
String beaconId = "BEACON_001";

// Pino do buzzer
const int buzzerPin = 5;

// Estados
bool beaconAtivo = false;
bool buzzerLigado = false;

WiFiClient espClient;
PubSubClient client(espClient);

// Função simples para criar JSON manualmente
String criarJSON(String beacon_id, bool ativo, bool buzzer) {
  return "{\"beacon_id\":\"" + beacon_id + "\",\"ativo\":" + 
         (ativo ? "true" : "false") + ",\"buzzer_ligado\":" + 
         (buzzer ? "true" : "false") + ",\"timestamp\":" + 
         String(millis()) + "}";
}

void setup() {
  Serial.begin(115200);
  
  // Configurar pino do buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  
  // Conectar WiFi
  conectarWiFi();
  
  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callbackMQTT);
  
  Serial.println("🚨 SISTEMA BEACON COM BUZZER 🚨");
  Serial.println("Beacon ID: " + beaconId);
  Serial.println("Buzzer no pino: GPIO" + String(buzzerPin));
  Serial.println("Aguardando comandos...");
}

void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();
  
  // Controlar buzzer se beacon ativado
  if (beaconAtivo && buzzerLigado) {
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(300);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
  
  // Publicar status periodicamente
  static unsigned long ultimoStatus = 0;
  if (millis() - ultimoStatus > 5000) {
    publicarStatusBeacon();
    ultimoStatus = millis();
  }
}

void conectarWiFi() {
  Serial.print("📡 Conectando WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Conectado! IP: " + WiFi.localIP().toString());
}

void reconectarMQTT() {
  while (!client.connected()) {
    Serial.print("🔌 Conectando MQTT...");
    String clientId = "Beacon-" + beaconId;
    
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ MQTT Conectado!");
      
      String topicoComando = "beacon/" + beaconId + "/comando";
      client.subscribe(topicoComando.c_str());
      Serial.println("📩 Inscrito em: " + topicoComando);
      
      publicarStatusBeacon();
      
    } else {
      Serial.print("❌ Falha MQTT, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  Serial.print("📨 Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String mensagem;
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }
  Serial.println(mensagem);
  
  // Processar comando simples
  if (mensagem.indexOf("ativar_beacon") != -1) {
    ativarBeacon();
  } else if (mensagem.indexOf("desativar_beacon") != -1) {
    desativarBeacon();
  } else if (mensagem.indexOf("localizar") != -1) {
    localizarBeacon();
  } else if (mensagem.indexOf("testar_buzzer") != -1) {
    testarBuzzer();
  }
}

void ativarBeacon() {
  beaconAtivo = true;
  buzzerLigado = true;
  Serial.println("🚨 BEACON ATIVADO - Buzzer ligado!");
  publicarStatusBeacon();
}

void desativarBeacon() {
  beaconAtivo = false;
  buzzerLigado = false;
  digitalWrite(buzzerPin, LOW);
  Serial.println("✅ BEACON DESATIVADO - Buzzer desligado");
  publicarStatusBeacon();
}

void localizarBeacon() {
  Serial.println("📍 Comando de localização recebido");
  for (int i = 0; i < 5; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
}

void testarBuzzer() {
  Serial.println("🔊 Testando buzzer...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(200);
  }
  Serial.println("✅ Teste do buzzer concluído");
}

void publicarStatusBeacon() {
  String jsonString = criarJSON(beaconId, beaconAtivo, buzzerLigado);
  String topico = "beacon/" + beaconId + "/status";
  client.publish(topico.c_str(), jsonString.c_str());
  Serial.println("📊 Status publicado: " + jsonString);
}