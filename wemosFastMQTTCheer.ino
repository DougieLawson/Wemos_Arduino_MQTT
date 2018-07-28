#include "FastLED.h"
#include <pixeltypes.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    D2
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    3
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_LEDS];
WiFiClient wemosCL;
PubSubClient mqttClient(wemosCL);

const char* MQTT_broker = "iot.eclipse.org";
const int led = LED_BUILTIN;
const String mqttColours[] = {
  "red"    , "green"  , "blue"     ,
  "cyan"   , "white"  , "purple"   ,
  "magenta", "yellow" , "orange"   ,
  "pink"   , "oldlace", "warmwhite"
};

// Keep the order the same as mqttColours[]
const CRGB colourArray[] = {
  0xFF0000, 0x00FF00, 0x0000FF,
  0x00FFFF, 0xFFFFFF, 0x800080,
  0xFF00FF, 0xFFFF00, 0xFFA500,
  0xFFC0CB, 0xFDF5E6, 0xFDF5E6
};

void reconnect()
{
  while (!mqttClient.connected())
  {
    if (mqttClient.connect("WemosMQTTclient"))
    {
      mqttClient.subscribe("cheerlights");
    }

    else
    {
      //    Serial.print("Conn failed rc=");
      //    Serial.println(mqttClient.state());
      delay(1000);
    }

  }

}

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  //wifiManager.resetSettings();
  if (!wifiManager.autoConnect("DL1-Ltd-Wemos", "configuration"))
  {
    delay(100);
    digitalWrite(led, 1);
    //ESP.reset();
    delay(100);
    digitalWrite(led, 0);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  mqttClient.setServer(MQTT_broker, 1883);
  mqttClient.setCallback(mqttCallBack);
  delay(3000); // 3 second delay for recovery
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void mqttCallBack(char* topic, byte* payload, unsigned int plLen)
{
  String mqttMsg = String((char*)payload);
  String mqttColour = mqttMsg.substring(0, plLen);
  //Serial.print(plLen);
  //Serial.print(" ");
  //Serial.println(mqttColour);

  int mqttColourMax = sizeof(mqttColours) / sizeof(mqttColours[0]);
  for (int index = 1; index < mqttColourMax; index++)
  {
    if (mqttColour.equals(mqttColours[index]))
    {
      // Shuffle the old colours along the array
      for (int indexO = NUM_LEDS; indexO > 0; indexO--)
      {
        leds[indexO] = leds[indexO - 1];
        //Serial.print(indexO);
        //Serial.print(" ");
        //Serial.println(leds[indexO]);
        FastLED.show();
      }

      //Serial.print("0 ");
      //Serial.println(colourArray[index]);
      leds[0] = colourArray[index];
      FastLED.show();
    }

  }

}

void loop()
{
  if (!mqttClient.connected())
  {
    reconnect();
  }

  mqttClient.loop();
}
