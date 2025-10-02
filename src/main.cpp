#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configurações
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_broker = "broker.hivemq.com";
const char* mqtt_topic_beacon = "mottu/beacon/status";
const char* mqtt_topic_alert = "mottu/beacon/alert";
const char* mqtt_topic_history = "mottu/beacon/history";
const char* mqtt_client_id = "beacon_device_001";

WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis globais
const int buzzerPin = 5;
bool alarmActive = false;
unsigned long lastStatusUpdate = 0;
const long statusInterval = 5000;
String historyData = "[]";
int historyCount = 0;

// ========== FUNÇÃO DE HISTÓRICO ==========
void addToHistory(String eventType, String details) {
  JsonDocument doc;
  deserializeJson(doc, historyData);
  
  JsonObject newEntry = doc.add<JsonObject>();
  newEntry["timestamp"] = millis();
  newEntry["event_type"] = eventType;
  newEntry["beacon_id"] = mqtt_client_id;
  newEntry["details"] = details;
  newEntry["alarm_status"] = alarmActive ? "active" : "inactive";
  newEntry["rssi"] = WiFi.RSSI();
  newEntry["battery"] = random(80, 101);
  
  historyData.clear();
  serializeJson(doc, historyData);
  historyCount++;
  
  Serial.println("📝 " + eventType + ": " + details);
}

// ========== FUNÇÃO DE HISTÓRICO PARA DASHBOARD ==========
void sendHistoryToDashboard() {
  JsonDocument doc;
  doc["beacon_id"] = mqtt_client_id;
  doc["command"] = "update_history";
  doc["history_count"] = historyCount;
  doc["history_data"] = serialized(historyData);
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  client.publish(mqtt_topic_history, jsonString.c_str());
  
  Serial.println("📤 Histórico enviado: " + String(historyCount) + " eventos");
}

// ========== SETUP WiFi ==========
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ========== CALLBACK MQTT ==========
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Mensagem: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.println(message);
  
  if (String(topic) == mqtt_topic_alert) {
    JsonDocument doc;
    deserializeJson(doc, message);
    
    String targetBeacon = doc["beacon_id"];
    String command = doc["command"];
    
    if (targetBeacon == mqtt_client_id) {
      if (command == "activate_alarm") {
        alarmActive = true;
        Serial.println("ALARME ATIVADO!");
        addToHistory("alarm_activated", "Alarme ativado via dashboard");
        
      } else if (command == "deactivate_alarm") {
        alarmActive = false;
        digitalWrite(buzzerPin, LOW);
        Serial.println("Alarme desativado");
        addToHistory("alarm_deactivated", "Alarme desativado via dashboard");
        
      } else if (command == "get_history") {
        sendHistoryToDashboard();
      }
    }
  }
}

// ========== ENVIAR STATUS ==========
void sendBeaconStatus() {
  JsonDocument doc;
  doc["beacon_id"] = mqtt_client_id;
  doc["alarm_active"] = alarmActive;
  doc["rssi"] = WiFi.RSSI();
  doc["timestamp"] = millis();
  doc["battery"] = random(80, 101);
  doc["history_count"] = historyCount;
  
  String jsonString;
  serializeJson(doc, jsonString);
  client.publish(mqtt_topic_beacon, jsonString.c_str());
  
  addToHistory("status_update", "Status periódico enviado");
}

// ========== CONTROLAR ALARME ==========
void handleAlarm() {
  if (alarmActive) {
    static unsigned long lastBuzzerChange = 0;
    static bool buzzerState = false;
    
    if (millis() - lastBuzzerChange > 500) {
      buzzerState = !buzzerState;
      digitalWrite(buzzerPin, buzzerState);
      lastBuzzerChange = millis();
    }
  }
}

// ========== RECONECTAR MQTT ==========
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_alert);
      addToHistory("mqtt_connected", "Conectado ao broker MQTT");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando em 5s");
      delay(5000);
    }
  }
}

// ========== SETUP PRINCIPAL ==========
void setup() {
  Serial.begin(115200);
  
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  
  setup_wifi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(mqtt_callback);
  
  // Histórico inicial
  addToHistory("system_start", "Sistema beacon inicializado");
  
  Serial.println("🚀 Beacon Mottu - Sistema de Persistência JSON");
}

// ========== LOOP PRINCIPAL ==========
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (millis() - lastStatusUpdate > statusInterval) {
    sendBeaconStatus();
    lastStatusUpdate = millis();
  }
  
  handleAlarm();
  delay(100);
}