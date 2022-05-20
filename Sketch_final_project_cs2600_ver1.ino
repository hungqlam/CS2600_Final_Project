#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

const char* ssid = "NETGEAR82";
const char* password = "Chloe0329";

// MQTT Broker
const char *mqtt_broker = "broker.mqttdashboard.com";
const char *topic = "esp32/FinalProject";
const char *mqtt_username = "hunglam";
const char *mqtt_password = "66668888";
const int mqtt_port = 1883;
const char *topicTemperature ="esp32/dht11/temperature";
const char *topicHumidity = "esp32/dht11/humidity";






#define LEDPIN 2
#define DHTPIN 13
#define DHTTYPE DHT11

unsigned long previousMillis = 0; 
const long interval = 5000;
int current_ledState = LOW;
int last_ledState = LOW;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
DHTesp dht;
void setup_dht() {
  dht.setup(DHTPIN, DHTesp::DHTTYPE);//Initialize the dht pin and dht object
  Serial.begin(115200);              //Set the baud rate as 115200
}
void setup_led()
{
  //initialize digital pin PIN_LED as an output
  pinMode(LEDPIN,OUTPUT);
  pinMode(current_ledState,INPUT);
}
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
   WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connect_to_broker() {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 client.subscribe(topic);
}

void callback(char* topic, byte *payload, unsigned int length) {
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  if (*payload == '1') current_ledState = HIGH;
  if (*payload == '0') current_ledState = LOW;
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);
  setup_wifi();
  connect_to_broker();
  //dht.begin();
  setup_dht();
  setup_led();
}


void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
  if (last_ledState != current_ledState) {
    last_ledState = current_ledState;
    digitalWrite(LEDPIN, current_ledState);
    Serial.print("LED state is ");
    Serial.println(current_ledState);
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    flag:TempAndHumidity newValues = dht.getTempAndHumidity();//Get the Temperature and humidity
  if (dht.getStatus() != 0) {//Judge if the correct value is read
    goto flag;               //If there is an error, go back to the flag and re-read the data
  }
  float h = newValues.humidity;
  float t = newValues.temperature;
  Serial.println(" Temperature:" + String(newValues.temperature) + 
  " Humidity:" + String(newValues.humidity));
  delay(1000);
    
   // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    } else {
      client.publish(topicTemperature, String(t).c_str());
      client.publish(topicHumidity, String(h).c_str());
      previousMillis = currentMillis;
    }
  }
}
