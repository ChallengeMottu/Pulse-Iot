# Pulse - Sistema de Rastreamento IoT com Beacon  

![Pulse](https://img.shields.io/badge/Pulse-IoT%2520Tracking-blue)  
![Platform](https://img.shields.io/badge/Platform-ESP32%2520%257C%2520Wokwi%2520%257C%2520MQTT-green)  
![Status](https://img.shields.io/badge/Status-Funcional%2520%E2%9C%85-success)  

---

## 📋 Sobre o Projeto  
O **Pulse** é um sistema inteligente de rastreamento e localização desenvolvido para a **Mottu**, utilizando tecnologia **IoT** para monitoramento em tempo real de ativos através de **beacons ESP32** com comunicação **MQTT**.  

---

## 🎯 Objetivo  
Implementar uma solução completa de rastreamento que permita localizar e monitorar ativos (como motos compartilhadas) através de beacons com buzzer, proporcionando **controle remoto** e **persistência de dados históricos**.  

---

## 👥 Grupo Desenvolvedor  
- **Gabriela de Sousa Reis** - RM558830  
- **Laura Amadeu Soares** - RM556690  
- **Raphael Lamaison Kim** - RM557914  

---

## 🚀 Funcionalidades Principais  
- 📍 **Localização por Beacon**: Sistema de beacon com ESP32 e buzzer para localização auditiva  
- 📡 **Comunicação em Tempo Real**: Protocolo MQTT para comunicação bidirecional  
- 📊 **Dashboard Web**: Interface gráfica para monitoramento em tempo real  
- 💾 **Persistência de Dados**: Sistema completo de histórico em JSON  
- 🔔 **Controle Remoto**: Ativação/desativação remota do alarme  
- 📈 **Monitoramento**: Telemetria de bateria, sinal WiFi e status do sistema  

---

## 🛠️ Tecnologias Utilizadas  

### **Hardware**  
- ESP32 DevKit C V4 - Microcontrolador principal  
- Buzzer Ativo - Alerta sonoro (GPIO 5)  
- Conexão WiFi - Comunicação wireless  

### **Software**  
- PlatformIO - Ambiente de desenvolvimento  
- Arduino Framework - Programação do ESP32  
- MQTT - Protocolo de comunicação (`broker.hivemq.com`)  
- ArduinoJson - Manipulação de dados JSON  
- Wokwi - Simulação e prototipagem  
- **Frontend**: HTML5 / CSS3 / JavaScript - Dashboard Web  
- Paho MQTT - Cliente MQTT no navegador  
- LocalStorage - Persistência local de dados  

---

## 🎮 Como Usar  

### **Configuração do Beacon (Wokwi)**  
1. Acesse o **Wokwi ESP32 Simulator**  
2. Cole o código do `main.cpp` no editor  
3. Configure o `diagram.json` com as conexões do buzzer  
4. Clique em **Play** para iniciar a simulação  

### **Execução do Dashboard**  
1. Abra o arquivo `dashboard.html` em um navegador moderno  
2. Aguarde a conexão automática com o broker MQTT  
3. O beacon aparecerá automaticamente quando conectado  

---

## 🎛️ Controles do Sistema  
- 🔔 **Ativar Alarme**: Aciona o buzzer no beacon  
- 🔇 **Silenciar**: Desativa o alarme  
- 📋 **Carregar Histórico**: Busca eventos do beacon  
- 📤 **Exportar JSON**: Salva histórico em arquivo  
- 📍 **Localizar**: Ativa padrão especial de localização  

---

## 🔧 Configuração Técnica  

### `platformio.ini`  
```ini
[env:esp32dev]  
platform = espressif32  
board = esp32dev  
framework = arduino  
monitor_speed = 115200  
upload_speed = 921600  

lib_deps =  
   knolleary/PubSubClient@^2.8  
   bblanchon/ArduinoJson@^7.4.2  
