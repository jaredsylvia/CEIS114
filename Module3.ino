#include "WiFi.h"
#include "PubSubClient.h"

const char* ssid = "SSIDNAME";
const char* passphrase = "SSIDPASSPHRASE";

const char* mqtt_server = "MQTTADDRESS";
const char* sub_channel = "ceis114/trafficLight";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

const int redPin = 13;
const int yellowPin = 12;
const int greenPin = 14;

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  setup_mqtt();


  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}


void setup_wifi() { // Set WiFi mode and connect to SSID

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, passphrase);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int i = 1;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(i);
    Serial.print(" ");
    i = i + 1;
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Setup done");
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  
  if (String(topic) == sub_channel) {
    Serial.print("Changing output to ");
    if(messageTemp == "red"){
      Serial.println("red");
      digitalWrite(redPin, HIGH);
      digitalWrite(yellowPin, LOW);
      digitalWrite(greenPin, LOW);
    }
    else if(messageTemp == "yellow"){
      Serial.println("yellow");
      digitalWrite(redPin, LOW);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(greenPin, LOW);
    
    } else if(messageTemp == "green"){
      Serial.println("green");
      digitalWrite(redPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(greenPin, HIGH);
    }
  }
}

void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
  Serial.print("Attempting MQTT connection...");
    
  if (client.connect("ceis114ESP32")) {
    Serial.println("connected");
  
    client.subscribe(sub_channel);
    client.publish(sub_channel, "Connected to MQTT.");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in a second");
  
    delay(1000);
    }
  }
}

void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  if(!client.connected()) {
    setup_mqtt();
  }
  client.loop();
  
}
