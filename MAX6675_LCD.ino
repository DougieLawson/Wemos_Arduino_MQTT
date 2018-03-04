#include "max6675.h"
#include <LiquidCrystal.h>

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
static char buffer[80];
static char readCel[] = "ReadC";
static char readFar[] = "ReadF";
float degC, degF;
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


int readline(int readch, char *buffer, int len)
{
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len - 1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
}

void setup() {
  Serial.begin(9600);
  Serial.println("MAX6675 test");
  lcd.begin(16, 2);
  lcd.print("MAX6675 test");
  lcd.createChar(0, customChar);

  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  if (readline(Serial.read(), buffer, 80) > 0) {
    //    Serial.print("You entered: >");
    //    Serial.print(buffer);
    //    Serial.println("<");
    if (strcmp(readCel, buffer) == 0) {
      degC = thermocouple.readCelsius();
      Serial.println(degC);
      lcd.setCursor(0, 0);
      lcd.print("Temp ");
      lcd.print(degC);
      lcd.write((uint8_t)0);
      lcd.print("C     ");
    }
    if (strcmp(readFar, buffer) == 0) {
      degF = thermocouple.readFahrenheit();
      Serial.println(degF);
      lcd.setCursor(0, 1);
      lcd.print("Temp ");
      lcd.print(degF);
      lcd.write((uint8_t)0);
      lcd.print("F     ");
    }
  }
  delay(100);
}
