#include <LiquidCrystal_PCF8574.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFiManager.h>

//const char* ssid = "WiFi-SSID";
//const char* password = "secretWiFiPasswd";
const char* MQTT_broker = "192.168.3.14";
const int led = LED_BUILTIN;

WiFiClient wemosCL;
PubSubClient mqttClient(wemosCL);
LiquidCrystal_PCF8574 lcd(0x00);

void printReady() {
  delay(1500);
  lcd.clear();
  lcd.print("Ready");
}

void mqttCallBack(char* topic, byte* payload, unsigned int plLen) {
//  Serial.println(topic);
  String mqttMsg = String((char*)payload);
//  Serial.println("----+----1----+----2----+----3");
//  Serial.println(mqttMsg);

  int ind1;
  int ind2;
  int ind3;
  ind1 = mqttMsg.indexOf('C');
  ind2 = mqttMsg.indexOf('F');
  ind3 = mqttMsg.indexOf(':');
  if (ind2 > plLen) {
    ind2 = plLen;
  }
//  Serial.print("ind1: ");
//  Serial.print(ind1);
//  Serial.print(" ind2: ");
//  Serial.print(ind2);
//  Serial.print(" ind3: ");
//  Serial.print(ind3);
//  Serial.print(" plLen: ");
//  Serial.println(plLen);

  String t1 = "";
  String t2 = "";
  String line1 = "";
  String line2 = "";
  String CorF = " ";

  if (ind2 == plLen - 3 && ind1 > 0) {
    line1 = "Temp:";
    line1 += mqttMsg.substring(ind3 + 1, ind1 - 3);
    line1 += "C";
    line2 = mqttMsg.substring(ind1 + 2, ind2 - 3);
    line2 += "F";
  } else {
    if (ind2 == plLen) {
      ind2 = ind2 - 3;
      CorF = "C";
    } else {
      CorF = "F";
    }
    line1 = "Temp:";
    line1 += mqttMsg.substring(ind3 + 1, ind2 - 3);
    line1 += CorF;
    line2 = "                ";
  }

  Serial.println(line1);
  Serial.println(line2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(6, 1);
  lcd.print(line2);
}

void reconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("WemosMQTTclient")) {
      mqttClient.subscribe("arduino/out/#");
    } else {
      Serial.print("Conn failed rc=");
      Serial.println(mqttClient.state());
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  while (!Serial);
  lcd.setCursor(0, 0);
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  //wifiManager.resetSettings();
  if (!wifiManager.autoConnect("Wemos AP", "password")) {
    delay(100);
    digitalWrite(led, 1);
    ESP.reset();
    delay(100);
    digitalWrite(led, 0);
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  mqttClient.setServer(MQTT_broker, 1883);
  mqttClient.setCallback(mqttCallBack);
  printReady();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}
