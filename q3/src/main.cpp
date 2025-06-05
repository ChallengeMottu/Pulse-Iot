#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Dados Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";


// Dados ThingSpeak
String apiKey = "E02DGIOX84KUSPGF";
const char* server = "http://api.thingspeak.com/update";

DHT dht(33, DHT22);

#define LED_Red 4
#define LED_Green 2
#define Buzzer 5
#define PIR_SENSOR 32  

float temperature;
float humidity;
const int pot1Pin = 35; 
int pot1Value = 0;
const int smokeThreshold = 500;  
int counter;
int estadoAnteriorPIR = LOW;     

void setup() {
  Serial.begin(9600);
  Serial.println("Olá, ESP32!");

  pinMode(LED_Red, OUTPUT);
  pinMode(LED_Green, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(PIR_SENSOR, INPUT); 

  dht.begin();

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");
}

void loop() {
  // Leitura dos sensores
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  pot1Value = analogRead(pot1Pin);
  int movimento = digitalRead(PIR_SENSOR); 

  Serial.println("=======================");
  Serial.println("Dado: " + String(counter));
  Serial.print("Temperatura:\t");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.println("Umidade:\t" + String(humidity) + "%");
  Serial.print("Valor do sensor de fumaça: ");
  Serial.println(pot1Value);

  if (movimento == HIGH && estadoAnteriorPIR == LOW) {
    Serial.println("Movimento detectado!");
    tone(Buzzer, 2000, 300);  
  }
  estadoAnteriorPIR = movimento; 

  if (temperature >= 30) {
    digitalWrite(LED_Red, HIGH);
    digitalWrite(LED_Green, LOW);
    tone(Buzzer, 440, 500);
  } else {
    digitalWrite(LED_Red, LOW);
    digitalWrite(LED_Green, HIGH);
  }

  if (pot1Value >= smokeThreshold) {
    Serial.println("Alerta: Nível alto de fumaça detectado!");
    tone(Buzzer, 1000, 500);
    digitalWrite(LED_Red, HIGH);
  }

  // Envio para o ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + apiKey;
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(humidity);
    url += "&field3=" + String(pot1Value);
    url += "&field4=" + String(movimento);

    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Enviado para ThingSpeak com sucesso!");
    } else {
      Serial.print("Erro ao enviar: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  counter++;
  delay(15000); // ThingSpeak permite 1 envio a cada 15 segundos (mínimo)
}