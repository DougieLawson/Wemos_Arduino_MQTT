#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "WiFi-SSID";
const char* password = "secretWiFiPasswd";
const char* mqtt_server = "192.168.3.14";

WiFiClient espClient;
PubSubClient client(espClient);

const int ledGPIO0 = 0;
const int ledGPIO2 = 2;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (topic == "esp8266/0") {
    Serial.print("Changing GPIO 0 to ");
    if (messageTemp == "1" | messageTemp == "true") {
      digitalWrite(ledGPIO0, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "0" | messageTemp == "false") {
      digitalWrite(ledGPIO0, LOW);
      Serial.print("Off");
    }
  }
  if (topic == "esp8266/2") {
    Serial.print("Changing GPIO 2 to ");
    if (messageTemp == "1" | messageTemp == "true") {
      digitalWrite(ledGPIO2, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "0" | messageTemp == "false") {
      digitalWrite(ledGPIO2, LOW);
      Serial.print("Off");
    }
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("esp8266/0");
      client.subscribe("esp8266/2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(ledGPIO0, OUTPUT);
  pinMode(ledGPIO2, OUTPUT);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect("ESP8266Client");
}
