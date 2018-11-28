#include <dht.h>
#include <PubSubClient.h>
#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <ArduinoJson.h>

dht DHT;
WiFiEspClient client;
PubSubClient mqttClient(client);
SoftwareSerial WifiSerial(2, 3); // RX, TX

long lastMsg = 0;
char msg[50];

const char* ssid = "dlink-6478@unifi";
const char* pass = "1029384756";
const char* mqtt_server = "broker.hivemq.com";
int status = WL_IDLE_STATUS;     // the Wifi radio's status

#define DHT22_PIN 4

void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
}

void setup_wifi() {
  WifiSerial.begin(9600);
  WiFi.init(&WifiSerial);
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

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
  
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect(NULL)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  //DHT.read22(DHT22_PIN);
  //Serial.print(DHT.humidity, 1);
  //Serial.println("");
  //Serial.print(DHT.temperature, 1);
  //Serial.println("");
  //delay(1000);

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    StaticJsonDocument<200> doc;
    JsonObject root = doc.to<JsonObject>();

    DHT.read22(DHT22_PIN);
    
    root["humidity"] = DHT.humidity;
    root["temperature"] = DHT.temperature;

    serializeJson(root, msg);
    
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttClient.publish("sensor", msg);
  }
}
