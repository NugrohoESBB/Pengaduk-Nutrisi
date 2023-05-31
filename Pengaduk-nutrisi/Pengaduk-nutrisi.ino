#include <Wire.h>
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <LiquidCrystal_I2C.h>

// Component pins
const int flowSensorPin = 10;
const int waterLevelSensorPin = 8;
const int solenoidValvePin = 9;
const int relayV1Pin = 7;
const int relayV2Pin = 6;
const int relayV3Pin = 5;
const int relayMPin = 4;
const int ledPin = 3;

// Configuration keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3};
byte colPins[COLS] = {4, 5, 6, 7};
int i2caddress = 0x20;
Keypad_I2C keypad = Keypad_I2C(makeKeymap(keys), rowPins, colPins, ROWS, COLS, i2caddress);

// Configuration lcd
LiquidCrystal_I2C lcd1(0x23, 20, 4);
LiquidCrystal_I2C lcd2(0x26, 20, 4);

// Input variable
float nutrisi1 = 0;
float nutrisi2 = 0;
float nutrisi3 = 0;
float air = 0;

void setup() {
  // Input output initialization
  pinMode(solenoidValvePin, OUTPUT);
  pinMode(relayV1Pin, OUTPUT);
  pinMode(relayV2Pin, OUTPUT);
  pinMode(relayV3Pin, OUTPUT);
  pinMode(relayMPin, OUTPUT);
  pinMode(flowSensorPin, INPUT);
  pinMode(waterLevelSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  // LCD initialization
  lcd1.init();
  lcd2.init();
  lcd1.backlight();
  lcd2.backlight();

  // Display LCD1
  lcd1.setCursor(0, 0);
  lcd1.print("====================");
  lcd1.setCursor(3, 1);
  lcd1.print("Pengaduk Nutrisi");
  lcd1.setCursor(4, 2);
  lcd1.print("Sistem Irigasi");
  lcd1.setCursor(0, 3);
  lcd1.print("====================");
  delay(2000);
  lcd1.clear();

  // Display LCD2
  lcd2.setCursor(0, 0);
  lcd2.print("====================");
  lcd2.setCursor(3, 1);
  lcd2.print("Pengaduk Nutrisi");
  lcd2.setCursor(4, 2);
  lcd2.print("Sistem Irigasi");
  lcd2.setCursor(0, 3);
  lcd2.print("====================");
  delay(2000);
  lcd2.clear();

  // Keypad initialization
  keypad.begin();
}

void loop() {

  // Menu keypad
  lcd2.setCursor(0, 0);
  lcd2.print("1 : Nilai Nutrisi A");
  lcd2.setCursor(0, 1);
  lcd2.print("2 : Nilai Nutrisi B");
  lcd2.setCursor(0, 2);
  lcd2.print("3 : Nilai Nutrisi C");
  lcd2.setCursor(0, 3);
  lcd2.print("4 : Nilai Air");
  
  char key = keypad.getKey();

  if (key != NO_KEY) {
    switch(key) {
      case '1':
        nutrisi1 = setValue();
        break;
      case '2':
        nutrisi2 = setValue();
        break;
      case '3':
        nutrisi3 = setValue();
        break;
      case '4':
        air = setValue();
        break;
      case 'A':
        openSolenoidValve();
        turnOnAgitator();
        break;
      case 'B':
        closeSolenoidValve();
        turnOffAgitator();
        break;
      case 'C':
        showNutrientValues();
        break;
      case 'D':
        resetNutrientValues();
        break;
    }
  }

  // Cek semua komponen
  componentCheck();
}

// Function untuk input nilai bilangan char
/*
float setValue() {
  float value = 0;
  lcd2.clear();
  lcd2.setCursor(0, 1);
  lcd2.print("Input Value :");
  while(1) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
      if (key >= '0' && key <= '9') {
        lcd2.setCursor(15, 1);
        lcd2.print(key);
        value = value * 10 + (key - '0');
      } else if (key == '#') {
        break;
      }
    }
  }
  lcd2.clear();
  return value;
}
*/

// Function untuk input nilai bilangan float tetapi
// nilai decimal didepan koma tampil 1 char bergantian
/*
float setValue() {
  float value = 0;
  float decimal = 0;
  bool decimalFlag = false;
  int decimalPlaces = 0;

  lcd2.clear();
  lcd2.setCursor(0, 1);
  lcd2.print("Input Value :");

  while (1) {
    char key = keypad.getKey();

    if (key != NO_KEY) {
      if (key >= '0' && key <= '9') {
        if (!decimalFlag) {
          value = value * 10 + (key - '0');
        } else {
          decimalPlaces++;
          decimal = decimal + ((key - '0') * pow(10, -decimalPlaces));
        }
        lcd2.setCursor(0 + decimalPlaces, 2);
        lcd2.print(key);
      } else if (key == '#') {
        break;
      } else if (key == '*') {
        decimalFlag = true;
        lcd2.setCursor(0, 2);
        lcd2.print('.');
      }
    }
  }

  value += decimal;
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("Value: ");
  lcd2.setCursor(0, 1);
  lcd2.print(value);
  delay(2000);
  lcd2.clear();
  
  return value;
}
*/

// Function untuk input nilai bilangan float tetapi
// (lihat comment dibawah)
float setValue() {
  char inputValue[16];
  int inputIndex = 0;
  float decimal = 0;
  bool decimalFlag = false;
  int decimalPlaces = 0;

  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("Input Value :");

  while (1) {
    char key = keypad.getKey();

    if (key != NO_KEY) {
      if (key >= '0' && key <= '9') {
        if (!decimalFlag) {
          inputValue[inputIndex] = key;
          inputIndex++;
          inputValue[inputIndex] = '\0';
        } else {
          decimal = decimal * 10 + (key - '0');
          decimalPlaces++;
        }
        lcd2.setCursor(0, 1);
        lcd2.print(inputValue);
        
        // jika kedua fungsi ini dimatikan,
        // maka input nilai dibelakang koma
        // akan menyatu dengan posisi semula
        // ====================================
        lcd2.print('.');
        lcd2.print(decimal);
        // ====================================
      } else if (key == '#') {
        break;
      } else if (key == '*') {
        decimalFlag = true;
        lcd2.setCursor(0, 1);
        lcd2.print(inputValue);
        lcd2.print('.');
      }
    }
  }

  float value = atof(inputValue);
  value += decimal * pow(10, -decimalPlaces);
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("Value :");
  lcd2.setCursor(0, 1);
  lcd2.print(value);
  delay(2000);
  lcd2.clear();
  
  return value;
}

void componentCheck() {
  lcd1.setCursor(3, 0);
  lcd1.print("Component State");
  
  int flowSensorValue = digitalRead(flowSensorPin);
  int waterLevelSensorValue = digitalRead(waterLevelSensorPin);
  
  if (flowSensorValue == HIGH) {
    lcd1.setCursor(0, 1);
    lcd1.print("Flow Sensor: OK  ");
    digitalWrite(ledPin, HIGH);
  } else {
    lcd1.setCursor(0, 1);
    lcd1.print("Flow Sensor: LOW");
    digitalWrite(ledPin, LOW);
  }
  
  if (waterLevelSensorValue == HIGH) {
    lcd1.setCursor(0, 2);
    lcd1.print("Water Level: OK  ");
    digitalWrite(ledPin, HIGH);
  } else {
    lcd1.setCursor(0, 2);
    lcd1.print("Water Level: LOW ");
    digitalWrite(ledPin, LOW);
  }
}

void openSolenoidValve() {
  digitalWrite(solenoidValvePin, HIGH);
  lcd2.clear();
  lcd2.print("Solenoid Valve");
  lcd2.setCursor(0, 1);
  lcd2.print("Terbuka");
  delay(1000);
}

void closeSolenoidValve() {
  digitalWrite(solenoidValvePin, LOW);
  lcd2.clear();
  lcd2.print("Solenoid Valve");
  lcd2.setCursor(0, 1);
  lcd2.print("Tertutup");
  delay(1000);
}

void turnOnAgitator() {
  digitalWrite(relayV1Pin, LOW);
  digitalWrite(relayV2Pin, LOW);
  digitalWrite(relayV3Pin, LOW);
  digitalWrite(relayMPin, LOW);
  lcd2.clear();
  lcd2.print("Pengaduk Aktif");
  delay(3000);
}

void turnOffAgitator() {
  digitalWrite(relayV1Pin, HIGH);
  digitalWrite(relayV2Pin, HIGH);
  digitalWrite(relayV3Pin, HIGH);
  digitalWrite(relayMPin, HIGH);
  lcd2.clear();
  lcd2.print("Pengaduk Mati");
  delay(3000);
}

void showNutrientValues() {
  lcd2.clear();
  lcd2.print("Nutrisi 1 : ");
  lcd2.print(nutrisi1);
  lcd2.setCursor(0, 1);
  lcd2.print("Nutrisi 2 : ");
  lcd2.print(nutrisi2);
  lcd2.setCursor(0, 2);
  lcd2.print("Nutrisi 3 : ");
  lcd2.print(nutrisi3);
  lcd2.setCursor(0, 3);
  lcd2.print("Air       : ");
  lcd2.print(air);
  delay(5000);
}

void resetNutrientValues() {
  nutrisi1 = 0;
  nutrisi2 = 0;
  nutrisi3 = 0;
  air = 0;
  lcd2.clear();
  lcd2.print("Nilai Nutrisi");
  lcd2.setCursor(0, 1);
  lcd2.print("dan air direset");
  delay(5000);
}
