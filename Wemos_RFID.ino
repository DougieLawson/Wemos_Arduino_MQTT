#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_PCF8574.h>

#define RST_PIN 2
#define SS_PIN 15
#define NR_KNOWN_KEYS   8

const char* ssid = "WiFi-SSID";
const char* password = "secretWiFiPasswd";
const char* MQTT_broker = "192.168.3.14";
const char* door_num = "door2";

unsigned long previousMillis = 0;
long readTime = 5000;

WiFiClient wemosCL;
PubSubClient client(wemosCL);
LiquidCrystal_PCF8574 lcd(0x27);

byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
  {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
  {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
  {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
  {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
  {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
  {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

MFRC522 rfid(SS_PIN, RST_PIN);

void printReady() {
  delay(1500);
  lcd.clear();
  lcd.print("Ready");
}

void subCallback(char* topic, byte* payload, unsigned int payloadLen) {
  String message = String((char*)payload);
  Serial.print("message:");
  Serial.println(message);

  String cardHolder;
  String last_seen;
  String admit;

  int i1 = message.indexOf(':');
  cardHolder = message.substring(0, i1);
  int i2 = message.indexOf(':', i1 + 1);
  last_seen = message.substring(i1 + 1, i2);
  admit = message.substring(i2 + 1, payloadLen);
  //Serial.println(cardHolder);
  //Serial.println(last_seen);
  //Serial.println(admit);
  lcd.clear();
  if (cardHolder != "Unknown") {
    lcd.setCursor(0, 0);
    lcd.print("Welcome ");
    lcd.print(cardHolder);
  }
  lcd.setCursor(0, 1);
  lcd.print(admit);
  printReady();
}

void setup() {
  char topic[12];
  Serial.begin(115200);
  Serial.println("Wemos active");
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  while (!Serial);
  SPI.begin();
  rfid.PCD_Init();
  lcd.setCursor(0, 0);
  //Serial.println(F("Try the most used default keys to print block 0"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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
  client.setServer(MQTT_broker, 1883);
  if (client.connect("wemos290DE6")) {
    client.setCallback(subCallback);
    sprintf(topic, "%s/%s", "doors", door_num);
    client.subscribe(topic);
  }
  printReady();
}

void dump_byte_array(byte *buf, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buf[i] < 0x10 ? " 0" : " ");
    Serial.print(buf[i], HEX);
    Serial.print(buf[i]);
  }
}
void sendMQTT(byte *buf) {

  char message[9];
  char topic[11];
  sprintf(message, "%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3]);
  sprintf(topic, "%s/%s", "auth", door_num);
  //Serial.println(message);
  client.connect("wemosClient");
  client.publish(topic, message);
  printReady();
}

boolean try_key(MFRC522::MIFARE_Key *key)
{
  boolean result = false;
  byte buffer[18];
  byte block = 0;
  MFRC522::StatusCode status;

  if ( ! rfid.PICC_IsNewCardPresent())
    return false;
  if ( ! rfid.PICC_ReadCardSerial())
    return false;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  byte byteCount = sizeof(buffer);
  status = rfid.MIFARE_Read(block, buffer, &byteCount);
  if (status != MFRC522::STATUS_OK) {
  }
  else {
    result = true;
    //Serial.print(F("Success with key:"));
    //    dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
    sendMQTT((*key).keyByte);
    //Serial.println();
    //Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
    dump_byte_array(buffer, 16);
    //Serial.println();
  }
  //Serial.println();

  rfid.PICC_HaltA();       // Halt PICC
  rfid.PCD_StopCrypto1();  // Stop encryption on PCD
  return result;
}

void loop() {
  client.loop();
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  //  dump_byte_array(rfid.uid.uidByte, rfid.uid.size);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= readTime) {
    //Serial.print(F("Card UID:"));
    sendMQTT(rfid.uid.uidByte);
    previousMillis = currentMillis;
    //Serial.println();
  }

  //  Serial.print(F("PICC type: "));
  //  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  //  Serial.println(rfid.PICC_GetTypeName(piccType));

  MFRC522::MIFARE_Key key;
  for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
      key.keyByte[i] = knownKeys[k][i];
    }
    if (try_key(&key)) {
      break;
    }
  }
}
