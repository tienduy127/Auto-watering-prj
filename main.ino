#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ==== Pin configuration ====
#define DHTPIN A2
#define DHTTYPE DHT11
#define SOIL_PIN A3
#define RELAY_PIN 6

// ==== Timing configuration (ms) ====
unsigned long pumpTestDuration = 3000;  // Pump test duration (ms)
unsigned long checkDelay = 15000;       // Wait time after pump test (ms)
unsigned long displayDelay = 2000;      // Display info delay (ms)

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==== State variables ====
bool pumpState = false;

void displayInfo() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int rawSoil = analogRead(SOIL_PIN);
  int soilPercent = map(rawSoil, 1023, 200, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // Serial Monitor output
  Serial.print("T: "); 
  if (isnan(t)) {
    Serial.print("Error");
  } else {
    Serial.print(t);
  }
  Serial.print("C, H: "); 
  if (isnan(h)) {
    Serial.print("Error");
  } else {
    Serial.print(h);
  }
  Serial.print("%, Soil: "); Serial.print(soilPercent);
  Serial.print("%, Mode: AUTO");
  Serial.print(", Pump: "); Serial.println(pumpState ? "ON" : "OFF");

  // LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isnan(t) || isnan(h)) {
    lcd.print("DHT11 Error!");
  } else {
    lcd.print("T:");
    lcd.print((int)t);
    lcd.print("C H:");
    lcd.print((int)h);
    lcd.print("%  ");
  }
  lcd.setCursor(0, 1);
  lcd.print("Soil:");
  lcd.print(soilPercent);
  lcd.print("% ");
  lcd.print("Au ");
  lcd.print(pumpState ? "On" : "Off");
}

void handleSoilCheck() {
  int raw = analogRead(SOIL_PIN);
  int soilPercent = map(raw, 1023, 200, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  if (soilPercent < 25) {
    Serial.println("[AUTO] Soil dry. Starting pump...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pump running 3s...");
    digitalWrite(RELAY_PIN, HIGH);
    pumpState = true;
    delay(pumpTestDuration);
    digitalWrite(RELAY_PIN, LOW);
    pumpState = false;

    Serial.println("[AUTO] Waiting for check...");

    // Display countdown during wait
    for (int i = checkDelay / 1000; i > 0; i--) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please wait!  ");
      lcd.setCursor(0, 1);
      lcd.print("> ");
      lcd.print(i);
      lcd.print("s checking   ");
      delay(1000);
    }

    // Check soil again after wait
    raw = analogRead(SOIL_PIN);
    soilPercent = map(raw, 1023, 200, 0, 100);
    soilPercent = constrain(soilPercent, 0, 100);
    Serial.print("[AUTO] Soil after pump: "); 
    Serial.print(soilPercent);
    Serial.println("%");
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  // Startup display
  lcd.setCursor(1, 0);
  lcd.print("Irrigation ");
  lcd.setCursor(1, 1);
  lcd.print("System");
  delay(3000);
  lcd.clear();
}

void loop() {
  // ==== Main cycle ====
  // Step 1: Delay 2s
  delay(displayDelay);

  // Step 2: Check soil and display info
  displayInfo();
  handleSoilCheck();

  // Step 3: Delay 2s
  delay(displayDelay);

  // Step 4: Display info again
  displayInfo();
}

