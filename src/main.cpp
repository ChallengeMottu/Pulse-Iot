// beacon.ino
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configurações WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações MQTT
const char* mqtt_broker = "broker.hivemq.com";
const char* mqtt_topic_beacon = "mottu/beacon/status";
const char* mqtt_topic_alert = "mottu/beacon/alert";
const char* mqtt_topic_history = "mottu/beacon/history";
const char* mqtt_client_id = "beacon_device_001";

WiFiClient espClient;
PubSubClient client(espClient);

// Pinos
const int buzzerPin = 5;
bool alarmActive = false;
unsigned long lastStatusUpdate = 0;
const long statusInterval = 5000;

// Sistema de persistência
const int MAX_HISTORY_ENTRIES = 100;
String historyData = "[]"; // Inicializa com array JSON vazio
int historyCount = 0;

// Declarações das funções
void setup_wifi();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void activateAlarm();
void deactivateAlarm();
void sendAlertStatus(String status);
void sendBeaconStatus();
void handleAlarm();
void reconnect();
void addToHistory(String eventType, String details);
void saveHistoryToFile();
void sendHistoryToDashboard();

void setup() {
  Serial.begin(115200);
  
  // Configurar pinos
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  
  setup_wifi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(mqtt_callback);
  
  // Inicializar histórico
  initializeHistory();
  
  Serial.println("Beacon Mottu inicializado com persistência!");
  Serial.println("Sistema de histórico JSON ativado");
}

void initializeHistory() {
  // Simular carregamento de arquivo (em sistema real seria SPIFFS)
  historyData = "[]";
  historyCount = 0;
  
  // Adicionar evento inicial
  addToHistory("system_start", "Beacon inicializado com persistência JSON");
}

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
  
  // Serializar de volta para string
  historyData.clear();
  serializeJson(doc, historyData);
  historyCount++;
  
  Serial.println("📝 Histórico atualizado: " + eventType);
  
  // Manter apenas últimas entradas (controle de memória)
  if (historyCount > MAX_HISTORY_ENTRIES) {
    trimHistory();
  }
  
  // Salvar periodicamente (em sistema real seria em arquivo)
  if (historyCount % 10 == 0) {
    saveHistoryToFile();
  }
}

void trimHistory() {
  JsonDocument doc;
  deserializeJson(doc, historyData);
  
  JsonArray arr = doc.as<JsonArray>();
  if (arr.size() > MAX_HISTORY_ENTRIES) {
    // Remove as entradas mais antigas
    for (int i = 0; i < arr.size() - MAX_HISTORY_ENTRIES; i++) {
      arr.remove(0);
    }
    historyData.clear();
    serializeJson(doc, historyData);
    historyCount = arr.size();
    Serial.println("✂️  Histórico truncado para " + String(MAX_HISTORY_ENTRIES) + " entradas");
  }
}

void saveHistoryToFile() {
  // Em sistema real, salvaria em SPIFFS
  // Aqui simulamos o salvamento
  Serial.println("💾 Salvando histórico (simulado)...");
  Serial.println("📊 Total de eventos: " + String(historyCount));
  
  // Enviar histórico para dashboard periodicamente
  sendHistoryToDashboard();
}

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
  Serial.println("📤 Histórico enviado para dashboard");
}

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  addToHistory("wifi_connected", "Conectado ao WiFi: " + String(WiFi.localIP()));
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Mensagem recebida: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.println(message);
  
  // Processar comandos de alerta
  if (String(topic) == mqtt_topic_alert) {
    JsonDocument doc;
    deserializeJson(doc, message);
    
    String targetBeacon = doc["beacon_id"];
    String command = doc["command"];
    
    if (targetBeacon == mqtt_client_id) {
      if (command == "activate_alarm") {
        activateAlarm();
      } else if (command == "deactivate_alarm") {
        deactivateAlarm();
      } else if (command == "get_history") {
        sendHistoryToDashboard();
      }
    }
  }
}

void activateAlarm() {
  alarmActive = true;
  Serial.println("ALARME ATIVADO! Buzzer tocando...");
  addToHistory("alarm_activated", "Alarme ativado remotamente via dashboard");
  sendAlertStatus("activated");
}

void deactivateAlarm() {
  alarmActive = false;
  digitalWrite(buzzerPin, LOW);
  Serial.println("Alarme desativado");
  addToHistory("alarm_deactivated", "Alarme desativado remotamente");
  sendAlertStatus("deactivated");
}

void sendAlertStatus(String status) {
  JsonDocument doc;
  doc["beacon_id"] = mqtt_client_id;
  doc["alarm_status"] = status;
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  client.publish(mqtt_topic_beacon, jsonString.c_str());
}

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
  
  // Adicionar ao histórico periodicamente
  addToHistory("status_update", "Atualização periódica de status");
}

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
      Serial.println(" tentando em 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Enviar status periodicamente
  if (millis() - lastStatusUpdate > statusInterval) {
    sendBeaconStatus();
    lastStatusUpdate = millis();
  }
  
  // Gerenciar alarme
  handleAlarm();
  
  delay(100);
}