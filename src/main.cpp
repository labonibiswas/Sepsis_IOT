#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 15
#define DHTTYPE DHT22
#define HEART_RATE_PIN 36 
#define SPO2_PIN 39       
#define SBP_PIN 34
#define DBP_PIN 35
#define RESP_RATE_PIN 32
#define GREEN_LED 2
#define RED_LED 4
#define BUZZER 5

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Sepsis Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  float temp = dht.readTemperature();
  int heartRate = map(analogRead(HEART_RATE_PIN), 0, 4095, 40, 180);
  int spo2 = map(analogRead(SPO2_PIN), 0, 4095, 70, 100);
  int sbp = map(analogRead(SBP_PIN), 0, 4095, 80, 180);
  int dbp = map(analogRead(DBP_PIN), 0, 4095, 40, 110);
  int respRate = map(analogRead(RESP_RATE_PIN), 0, 4095, 10, 40);

  int sepsisCriteriaCount = 0; //count for criteria matched
  String failedVitals = ""; // String to store which vitals failed

  // Check Criteria & Build Alert String
  if (sbp <= 100 || dbp <= 100) { sepsisCriteriaCount++; failedVitals += "BP "; }
  if (respRate >= 22) { sepsisCriteriaCount++; failedVitals += "RR "; }
  if (temp > 38.0 || temp < 36.0) { sepsisCriteriaCount++; failedVitals += "T "; }
  if (spo2 < 95) { sepsisCriteriaCount++; failedVitals += "SPO2 "; }
  if (heartRate > 90) { sepsisCriteriaCount++; failedVitals += "HR "; }

  lcd.clear();
  
  if (sepsisCriteriaCount >= 2) {
    // SEPSIS POSITIVE STATE
    digitalWrite(GREEN_LED, LOW);
    
    // Row 1: Status
    lcd.setCursor(0, 0);
    lcd.print("SEPSIS POSITIVE");
    
    // Row 2: Show which vitals caused it
    lcd.setCursor(0, 1);
    lcd.print("Failed: "); lcd.print(failedVitals);

    // Alert Actions: Blink Red LED and sound Buzzer
    for(int i = 0; i < 7; i++) { // Blink 3 times per loop
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 1000); // 1000Hz tone
      delay(200);
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER);
      delay(200);
    }
  } else {
    // SEPSIS NEGATIVE STATE
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);

    lcd.setCursor(0, 0);
    lcd.print("Criteria met: "); lcd.print(sepsisCriteriaCount);
    lcd.setCursor(0, 1);
    lcd.print("Sepsis Negative");
  }

  // Serial Monitor Log
  Serial.print("Count: "); Serial.print(sepsisCriteriaCount);
  if(sepsisCriteriaCount == 0){
    Serial.println(" | All vitals normal.");
  }else{
    Serial.print(" | Failed: "); Serial.println(failedVitals);
  }
  

  delay(2000); 
}