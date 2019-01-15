/*Ohjelma Excelvan LED-videoprojektorin etäkäynnistystä/sammutusta varten

*/

const int buttonPin = D5;     // the number of the pushbutton pin (tykin poweri-nappi)
const int ledPin =  D6;      // tykin vihreä ledi (1.85V)
int lastLedState = LOW;     //debouncea varten

int ledState = LOW;         // variable for reading the LED status
int powerUP = 0;          //tykin käynnistystila
int powerDOWN = 0;        //tykin sammutustila
int powerON = 0;          //power-tila
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 1000;

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "settings.h"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  pinMode(ledPin, INPUT);
  pinMode(buttonPin, INPUT);

}

void setup_wifi() {
  delay (10);
  Serial.println();
  Serial.print("Connecting ");
  Serial.print(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP adress:");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);

  if ((message == "on") and (powerON == 0)) {
    powerUP = 1;
  }
  if ((message == "off") and (powerON == 1)) {
    powerDOWN = 1;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int reading = digitalRead(ledPin); //power-tila luetaan siniseltä lediltä

  if (reading != lastLedState) { //debounce välkkyvää lediä varten
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != ledState) {
      ledState = reading;

      if (ledState == HIGH) {
        powerON = 1;
      }
      else powerON = 0;
    }
  }
  lastLedState = reading;

  //tykin käynnistys
  if ((powerON == 0) and (powerUP == 1)) {
    pinMode (buttonPin, OUTPUT);
    digitalWrite (buttonPin, LOW);
    delay(1000);
    digitalWrite (buttonPin, HIGH);
    delay(500);
    pinMode (buttonPin, INPUT);
    powerUP = 0;
    delay(4000);
  }

  //tykin sammutus
  if ((powerON == 1) and (powerDOWN == 1)) {
    pinMode (buttonPin, OUTPUT);
    digitalWrite (buttonPin, LOW);
    delay(1000);
    digitalWrite (buttonPin, HIGH);
    pinMode (buttonPin, INPUT);
    powerDOWN = 0;
    delay(4000);
    powerON = 0;
  }

  Serial.println("reading");
  Serial.println(reading);
  Serial.println("ledState");
  Serial.println(ledState);
  Serial.println("powerON");
  Serial.println(powerON);
  Serial.println("powerUP");
  Serial.println(powerUP);
  Serial.println("powerDOWN");
  Serial.println(powerDOWN);

  delay(10);

}
