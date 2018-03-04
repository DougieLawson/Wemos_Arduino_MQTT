#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "max6675.h"
#include <LiquidCrystal.h>
#include <stdio.h>

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
float degC, degF;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

EthernetClient ethClient;
PubSubClient client(ethClient);

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
static byte customChar[8] = {
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte mac[]    = {  
  0x00, 0x08, 0xdc, 0xc0, 0x62, 0x80 };
IPAddress ip(10, 1, 1, 75);
IPAddress server(10, 1, 1, 11);
int mlth;
char message[80];
char strDegF[6];
char strDegC[6];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  degC = thermocouple.readCelsius();
  degF = thermocouple.readFahrenheit();

  lcd.setCursor(0, 0);
  lcd.print("Temp ");
  lcd.print(degC);
  lcd.write((uint8_t)0);
  lcd.print("C     ");
  lcd.setCursor(0, 1);
  lcd.print("Temp ");
  lcd.print(degF);
  lcd.write((uint8_t)0);
  lcd.print("F     ");

  dtostrf(degC, 4, 2, strDegC);
  dtostrf(degF, 4, 2, strDegF);

  if (strcmp(topic,"arduino/in/ReadC") == 0) {
    Serial.print(" !C! ");
    mlth = sprintf(message, "Temp: %s °C\r\n\0", strDegC);
    Serial.println(message);
  }
  else if (strcmp(topic,"arduino/in/ReadF") == 0) {
    Serial.print(" !F! ");
    mlth = sprintf(message, "Temp: %s °F\r\n\0", strDegF);
    Serial.println(message);
  }
  else {
    Serial.print(" !Other! ");
    mlth = sprintf(message, "Temp: %s °C %s °F\r\n\0", strDegC, strDegF);
    Serial.println(message); 
  }

  client.publish("arduino/out/6675",message);


  Serial.println(degC);
  Serial.println(degF);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.publish("arduino/out/6675","MAX6675 test");
      client.subscribe("arduino/in/+");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("MAX6675 test");

  lcd.begin(16,2);
  lcd.createChar(0, customChar);
  lcd.print("1234567890123456789012345678901234567890");
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}















