#include <FS.h>
#include <LiquidCrystal_PCF8574.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Wire.h>
#include "Adafruit_MAX31855.h"
#include <PubSubClient.h>

#define MAXDO   13
#define MAXCS   15
#define MAXCLK  14
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

//const char* ssid = "WiFi-SSID-none";
//const char* password = "secretWiFiPasswd";
const char* MQTT_broker = "192.168.3.14";

int notConnected = true;
unsigned long mqttSavedMillis = 0;
unsigned long loopSavedMillis = 0;
const long intervalMillis = 500;

byte customChar[8] = {
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

ESP8266WebServer webserver(80);
WiFiClient WemosCL;
PubSubClient client(WemosCL);

const int led = LED_BUILTIN;
double currentC;

LiquidCrystal_PCF8574 lcd(0x27);

char* readTcouple() {
  unsigned long mqttMillis = millis();
  char degC[20] = {"?"};
  double c = thermocouple.readCelsius();
  dtostrf(c, 6, 2, degC);
  if (c != currentC) {
    //Serial.println(degC);
    if (notConnected) {
      waitForConnection();
    }
    if (mqttMillis - mqttSavedMillis >= intervalMillis) {
      client.connect("WemosClient");
      client.publish("wemos/out/31855", degC);
      currentC = c;
      mqttSavedMillis = mqttMillis;
    }
  }
  return degC;
}

void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];
  char* C = readTcouple();
  char degC[20];
  strcpy(degC, C);
  Serial.print("handleRoot:");
  Serial.print(degC);
  snprintf(temp, 400, "<html><head><meta http-equiv=\"Refresh\" content=\"5\"><title>MAX31855 Thermocouple</title><style>body{background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;}</style></head><body><h1>MAX31855 Thermocouple</h1><h2>Temp: %s</h2></body></html>", degC);
  webserver.send(200, "text/html", temp);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";
  for (uint8_t i = 0; i < webserver.args(); i++) {
    message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
  }
  webserver.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void onDisconnected(const WiFiEventStationModeDisconnected& event) {
  notConnected = true;
  waitForConnection();
}

void waitForConnection() {
  webserver.stop();
  WiFi.begin(); // ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    client.setServer(MQTT_broker, 1883);
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print(".");
  }
  webserver.begin();
  webserver.on("/", handleRoot);
  webserver.onNotFound(handleNotFound);
  notConnected = false;
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  // wifiManager.resetSettings();
  if (!wifiManager.autoConnect("Wemos AP", "password")) {
    delay(100);
    digitalWrite(led, 1);
    ESP.reset();
    delay(100);
    digitalWrite(led, 0);
  }

  lcd.begin(16, 2);
  lcd.createChar(0, customChar);
  lcd.setBacklight(255);

  waitForConnection();

  lcd.setCursor(0, 1);
  lcd.print("http://");
  lcd.setCursor(7, 1);
  lcd.print(WiFi.localIP());
  client.setServer(MQTT_broker, 1883);

  digitalWrite(led, 1);
}

void loop(void) {
  unsigned long loopMillis = millis();
  webserver.handleClient();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  if (loopMillis - loopSavedMillis >= intervalMillis) {
    char* temp = readTcouple();
    char degC[20];
    strcpy(degC, temp);
    lcd.print(degC);
    lcd.write((uint8_t)0);
    lcd.print("C ");
    loopSavedMillis = loopMillis;
  }
  delay(10);
}
