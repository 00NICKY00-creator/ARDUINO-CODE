#include <WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>
/*
const char* ssid = "nicki";
const char* password = "123456789";
const char* mqtt_server = "192.168.0.101"; // Adresse IP de votre ordinateur où Mosquitto est installé
*/

const char* ssid = "Airbox-B7CF";
const char* password = "q6WFYUGhXY3Q";
const char* mqtt_server = "192.168.1.25"; 

const char* voltage_topic = "home/esp32/voltage";
const char* current_topic = "home/esp32/current";
const char* power_topic = "home/esp32/power";
const char* energy_topic = "home/esp32/energy";
const char* frequency_topic = "home/esp32/frequency";
const char* pf_topic = "home/esp32/pf";
const char* command_topic = "home/esp32/command"; // Nouveau topic pour les commandes

const int RELAY = 27;
WiFiClient espClient;
PubSubClient client(espClient);
#define RXD2 16
#define TXD2 17

HardwareSerial pzemSerial(2);
PZEM004Tv30 pzem(&pzemSerial, RXD2, TXD2);

void setup_wifi() {
  delay(10);
  pinMode(RELAY, OUTPUT);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Traitement du message reçu sur le topic "command"
  if (strcmp(topic, command_topic) == 0) {
    if (message == "true") {
      Serial.println("Allumer");
      digitalWrite(RELAY, HIGH);  
      
    } else if (message == "false") {
      Serial.println("Éteindre");
      digitalWrite(RELAY, LOW);  

      
    } else {
      Serial.println("Commande invalide");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client","jacky","5678")) {
      Serial.println("connected");
      client.subscribe("prise");
      client.subscribe(command_topic); // Souscrire au topic "command"
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // Définir la fonction de rappel pour les messages MQTT entrants
  pzemSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();

  // Check if the data is valid
  if (isnan(voltage)) {
    client.publish(voltage_topic, "Error reading voltage");
  } else if (isnan(current)) {
    client.publish(current_topic,"Error reading current");
  } else if (isnan(power)) {
    client.publish(power_topic,"Error reading power");
  } else if (isnan(energy)) {
    client.publish(energy_topic,"Error reading energy");
  } else if (isnan(frequency)) {
    client.publish(frequency_topic,"Error reading frequency");
  } else if (isnan(pf)) {
    client.publish(pf_topic,"Error reading pf");
  } else {

    client.publish(voltage_topic, String(voltage).c_str());
    client.publish(current_topic, String(current).c_str());
    client.publish(power_topic, String(power).c_str());
    client.publish(energy_topic, String(energy).c_str());
    client.publish(frequency_topic, String(frequency).c_str());
    client.publish(pf_topic, String(pf).c_str());
  }

  // Check if the data is valid
  if (isnan(voltage)) {
    Serial.println("Error reading voltage");
  } else if (isnan(current)) {
    Serial.println("Error reading current");
  } else if (isnan(power)) {
    Serial.println("Error reading power");
  } else if (isnan(energy)) {
    Serial.println("Error reading energy");
  } else if (isnan(frequency)) {
    Serial.println("Error reading frequency");
  } else if (isnan(pf)) {
    Serial.println("Error reading power factor");
  } else {

    // Print the values to the Serial console
    Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
    Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
    Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
    Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
    Serial.print("PF: ");           Serial.println(pf);
  }

  Serial.println();
  delay(1000);
}
