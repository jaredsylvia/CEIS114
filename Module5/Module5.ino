#include "WiFi.h"
#include "PubSubClient.h"

const char* ssid = "INSERTSSID";
const char* passphrase = "INSERTPASSPHRASE";

const char* mqtt_server = "10.10.0.1";
const char* sub_channel = "ceis114/trafficLight/control";
const char* pub_channel = "ceis114/trafficLight/status";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

const int nsRed = 13;
const int nsYellow = 12;
const int nsGreen = 14;
const int ewRed = 4;
const int ewYellow = 5;
const int ewGreen = 18;

const int nsCrossing = 27;
const int ewCrossing = 23;


String crossing = "no";
String trafficDirection = "ns";
String newDirection = "ns";

unsigned long yellowTimer = 1000;
unsigned long redTimer = 1000;
unsigned long fourWayTimer = 500;

unsigned long changeTime = millis();

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  setup_mqtt();


  pinMode(nsRed, OUTPUT);
  pinMode(nsYellow, OUTPUT);
  pinMode(nsGreen, OUTPUT);
  pinMode(ewRed, OUTPUT);
  pinMode(ewYellow, OUTPUT);
  pinMode(ewGreen, OUTPUT);
  pinMode(ewCrossing, INPUT_PULLUP);
  pinMode(nsCrossing, INPUT_PULLUP);
  
  
  digitalWrite(nsGreen, HIGH);
  digitalWrite(ewRed, HIGH);
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
    
    if(messageTemp == "ns"){
      changeLights("ns");
    }
    else if(messageTemp == "ew"){
      changeLights("ew");
    
    } else {
      changeLights("4way");
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
    client.publish(pub_channel, "connected");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in a second");
  
    delay(1000);
    }
  }
}
void lights() {
  if(trafficDirection == "ns"){
    digitalWrite(ewGreen, LOW);
    
    if(millis() - yellowTimer <= changeTime) {
      digitalWrite(ewYellow, HIGH);
    } else if(millis() - yellowTimer >= changeTime) {
      digitalWrite(ewYellow, LOW);
      digitalWrite(ewRed, HIGH);
    }
    if((millis() - yellowTimer) - redTimer >= changeTime) {
      digitalWrite(nsRed, LOW);
      digitalWrite(nsYellow, LOW);
      digitalWrite(nsGreen, HIGH);
      digitalWrite(ewRed, HIGH);
      digitalWrite(ewYellow, LOW);
      digitalWrite(ewGreen, LOW);
  }

  
 } else if(trafficDirection == "ew"){
    digitalWrite(nsGreen, LOW);
    
    if(millis() - yellowTimer <= changeTime) {
      digitalWrite(nsYellow, HIGH);
    } else if(millis() - yellowTimer >= changeTime) {
      digitalWrite(nsYellow, LOW);
      digitalWrite(nsRed, HIGH);
    }
    if((millis() - yellowTimer) - redTimer >= changeTime) {
      digitalWrite(ewRed, LOW);
      digitalWrite(ewYellow, LOW);
      digitalWrite(ewGreen, HIGH);
      digitalWrite(nsRed, HIGH);
      digitalWrite(nsYellow, LOW);
      digitalWrite(nsGreen, LOW);
  }
  
 } else if(trafficDirection == "4way"){
  if(digitalRead(nsGreen) == HIGH) {
    digitalWrite(nsGreen, LOW);
    digitalWrite(nsYellow, HIGH);
    changeTime = millis();
  }
  if(digitalRead(ewGreen) == HIGH) {
    digitalWrite(ewGreen, LOW);
    digitalWrite(ewYellow, HIGH);
    changeTime = millis();
  }
  if(digitalRead(nsYellow) == HIGH and millis() - yellowTimer >= changeTime) {
    digitalWrite(nsGreen, LOW);
    digitalWrite(ewGreen, LOW);
    digitalWrite(nsYellow, LOW);
    digitalWrite(ewYellow, LOW);
    digitalWrite(nsRed, HIGH);
    digitalWrite(ewRed, HIGH);
    changeTime = millis();
  }

  if(digitalRead(nsRed) == LOW and digitalRead(ewRed) == LOW and millis() - fourWayTimer >= changeTime){
    digitalWrite(nsRed, HIGH);
    digitalWrite(ewRed, HIGH);
    changeTime = millis();
  }
  if(digitalRead(nsRed) == HIGH and digitalRead(ewRed) == HIGH and millis() - fourWayTimer >= changeTime){
    digitalWrite(nsRed, LOW);
    digitalWrite(ewRed, LOW);
    changeTime = millis();
  }
  
 }
}
void changeLights(String flowDirection) {
  if(flowDirection == "ns" and trafficDirection != "ns"){
    trafficDirection = "ns";
    client.publish(pub_channel, "nsFlow");
    client.publish(pub_channel, "ewStop");
    changeTime = millis();
    
    
  } else if(flowDirection == "ew" and trafficDirection != "ew"){
    trafficDirection = "ew";
    client.publish(pub_channel, "ewFlow");
    client.publish(pub_channel, "nsStop");
    changeTime = millis();
    
  } else if(flowDirection =="4way" and trafficDirection != "4way") {
    trafficDirection = "4way";
    client.publish(pub_channel, "4wayStop");
    changeTime= millis();
    
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

  lights();
  if(digitalRead(nsCrossing) == LOW){
    client.publish(pub_channel, "nsCross");
    delay(500);
  }
  if(digitalRead(ewCrossing) == LOW){
    client.publish(pub_channel, "ewCross");
    delay(500);
  }
  
}
