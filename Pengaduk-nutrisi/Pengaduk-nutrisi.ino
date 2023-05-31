#include <Wire.h>
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <LiquidCrystal_I2C.h>

// Component pins
const int flow1SensorPin = 2;
const int flow2SensorPin = 3;
const int relayV1Pin = 4;
const int relayV2Pin = 5;
const int relayPoPin = 7;
const int relayMoPin = 6;
const int waterLevelSensorPin = 9;
const int ledPin = 13;

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

// Variable nutritional input
float nutrisi1 = 0;
float nutrisi2 = 0;

// Variable flow sensors
volatile long pulseFlow1;
volatile long pulseFlow2;
unsigned long lastTimeFlow1;
unsigned long lastTimeFlow2;
float volumeFlow1;
float volumeFlow2;

void setup() {
  // Input output initialization
  pinMode(relayV1Pin, OUTPUT);
  pinMode(relayV2Pin, OUTPUT);
  pinMode(relayPoPin, OUTPUT);
  pinMode(relayMoPin, OUTPUT);
  pinMode(flow1SensorPin, INPUT);
  pinMode(flow2SensorPin, INPUT);
  pinMode(waterLevelSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Relay Low
  digitalWrite(relayV1Pin, HIGH);
  digitalWrite(relayV2Pin, HIGH);
  digitalWrite(relayPoPin, HIGH);
  digitalWrite(relayMoPin, HIGH);
  
  // LCD initialization
  lcd1.init();
  lcd2.init();
  lcd1.backlight();
  lcd2.backlight();

  // Display LCD1
  lcd1.setCursor(0, 0);
  lcd1.print("====================");
  lcd1.setCursor(2, 1);
  lcd1.print("Pengaduk Nutrisi");
  lcd1.setCursor(3, 2);
  lcd1.print("Sistem Irigasi");
  lcd1.setCursor(0, 3);
  lcd1.print("====================");
  delay(2000);
  lcd1.clear();

  // Display LCD2
  lcd2.setCursor(0, 0);
  lcd2.print("====================");
  lcd2.setCursor(2, 1);
  lcd2.print("Pengaduk Nutrisi");
  lcd2.setCursor(3, 2);
  lcd2.print("Sistem Irigasi");
  lcd2.setCursor(0, 3);
  lcd2.print("====================");
  delay(2000);
  lcd2.clear();

  // Keypad initialization
  keypad.begin();

  // Flow sensor initialization
  attachInterrupt(digitalPinToInterrupt(flow1SensorPin), increase1, RISING);
  attachInterrupt(digitalPinToInterrupt(flow2SensorPin), increase2, RISING);  

  // Cek semua komponen
  componentCheck();
  delay(4000);
  lcd1.clear();
}

void loop() {
  // Menu keypad
  lcd2.setCursor(0, 0);
  lcd2.print("  Masukkan Pilihan  ");
  lcd2.setCursor(0, 2);
  lcd2.print("1 : Nilai Nutrisi A");
  lcd2.setCursor(0, 3);
  lcd2.print("2 : Nilai Nutrisi B");
  
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
        turnOnPump();
        break;
      case 'A':
        turnOnAgitator();
        break;
      case 'B':
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

  // Flow 1 dan 2
  lcd1.setCursor(4, 0);
  lcd1.print("Sensor Value");
  volFlow1();
  volFlow2();
  waterLevel();

  if (volumeFlow1 >= 1000 and volumeFlow1 <= 5000) {
    digitalWrite(relayV1Pin, HIGH);
  } else if (volumeFlow2 >= 1000 and volumeFlow2 <= 5000) {
    digitalWrite(relayV2Pin, HIGH);
  }
}

// =============== PR CODING ===============
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
  
  int flow1SensorValue = digitalRead(flow1SensorPin);
  int flow2SensorValue = digitalRead(flow2SensorPin);
  int waterLevelSensorValue = digitalRead(waterLevelSensorPin);
  
  if (flow1SensorValue == HIGH and flow2SensorValue == HIGH) {
  //if (flow1SensorValue == HIGH) {
    lcd1.setCursor(0, 2);
    lcd1.print("Flow Sen :OK ");
    digitalWrite(ledPin, HIGH); 
  } else {
    lcd1.setCursor(0, 2);
    lcd1.print("Flow Sen :LOW ");
    digitalWrite(ledPin, LOW);
  }
  
  if (waterLevelSensorValue == HIGH) {
    lcd1.setCursor(0, 3);
    lcd1.print("Water Lev:OK ");
    digitalWrite(ledPin, HIGH);
  } else {
    lcd1.setCursor(0, 3);
    lcd1.print("Water Lev:LOW ");
    digitalWrite(ledPin, LOW);
  }
}

void volFlow1() {
  volumeFlow1 = 2663 * pulseFlow1;
  if (millis() - lastTimeFlow1 > 1000) {
    pulseFlow1 = 0;
    lastTimeFlow1 = millis();
  }
  lcd1.setCursor(0, 1);
  lcd1.print("Flow 1: ");
  lcd1.print(volumeFlow1);
  lcd1.setCursor(16, 1);
  lcd1.print("mL/s");
}

void volFlow2() {
  volumeFlow2 = 2663 * pulseFlow2;
  if (millis() - lastTimeFlow2 > 1000) {
    pulseFlow2 = 0;
    lastTimeFlow2 = millis();
  }
  lcd1.setCursor(0, 2);
  lcd1.print("Flow 2: ");
  lcd1.print(volumeFlow2);
  lcd1.setCursor(16, 2);
  lcd1.print("mL/s");
}

void increase1() {
  pulseFlow1++;
}

void increase2() {
  pulseFlow2++;
}

void waterLevel() {
  int waterLevelSensorValue = digitalRead(waterLevelSensorPin);
  
  if (waterLevelSensorValue == HIGH) {
    lcd1.setCursor(0, 3);
    lcd1.print("Water : OK ");
    digitalWrite(ledPin, HIGH);
  } else {
    lcd1.setCursor(0, 3);
    lcd1.print("Water : LOW ");
    digitalWrite(ledPin, LOW);
  }
}

void turnOnAgitator() {
  digitalWrite(relayV1Pin, LOW);
  digitalWrite(relayV2Pin, LOW);
  // digitalWrite(relayPoPin, LOW);
  digitalWrite(relayMoPin, LOW);
  lcd2.clear();
  lcd2.print("Pengaduk Aktif dan");
  lcd2.setCursor(0, 1);
  lcd2.print("Solenoid Valve");
  lcd2.setCursor(0, 2);
  lcd2.print("Terbuka");
  delay(3000);
  lcd2.clear();
}

void turnOffAgitator() {
  digitalWrite(relayV1Pin, HIGH);
  digitalWrite(relayV2Pin, HIGH);
  digitalWrite(relayPoPin, HIGH);
  digitalWrite(relayMoPin, HIGH);
  lcd2.clear();
  lcd2.print("Pengaduk Mati dan");
  lcd2.setCursor(0, 1);
  lcd2.print("Solenoid Valve");
  lcd2.setCursor(0, 2);
  lcd2.print("Tetutup");
  delay(3000);
  lcd2.clear();
}

void turnOnPump() {
  digitalWrite(relayPoPin, LOW);
}

void showNutrientValues() {
  lcd2.clear();
  lcd2.print("Nutrisi 1 : ");
  lcd2.print(nutrisi1);
  lcd2.setCursor(0, 1);
  lcd2.print("Nutrisi 2 : ");
  lcd2.print(nutrisi2);
  lcd2.setCursor(0, 2);
  delay(5000);
  lcd2.clear();
}

void resetNutrientValues() {
  nutrisi1 = 0;
  nutrisi2 = 0;
  lcd2.clear();
  lcd2.print("Nilai Nutrisi");
  lcd2.setCursor(0, 1);
  lcd2.print("direset");
  delay(5000);
  lcd2.clear();
}
