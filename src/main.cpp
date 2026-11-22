#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30105.h" // Works for MAX30102
#include "heartRate.h" // Included with SparkFun library
#include <Adafruit_MLX90614.h> // Added for IR Temperature

// --- WIFI SETTINGS ---
const char* ssid = "Your SSID";
const char* password = "Your Password";
const char* serverName = "https://sepsis-api-server.onrender.com/predict";
const char* patient_email = "Email";

// --- TWILIO SMS SETTINGS ---
const char* twilio_sid = "id";
const char* twilio_token = "token";
//const char* twilio_phone_number = "twilio_num"; 
//const char* destination_phone_number = "number"; 
unsigned long last_sms_time = 0; 

// Constants
#define WINDOW_SIZE 5
#define FEATURE_COUNT 8
#define DECISION_THRESHOLD 0.5887648463249207

// Pin Definitions
// DHT pins removed
#define SBP_PIN 34
#define DBP_PIN 35
#define RESP_RATE_PIN 32
#define GREEN_LED 2
#define RED_LED 4
#define BUZZER 5

// Pre-calculated Scaling Constants
float means[] = {86.51486771 , 97.52455547 , 36.89601335 ,122.51589324 , 63.50451336, 18.56893754 , 62.05139045 ,  0.57613999};
float stds[]  = {17.74786194 , 2.81514868 , 0.81231678, 23.06672409, 12.90534682 , 5.25856119, 16.04828767 , 0.47167752};

float input_buffer[WINDOW_SIZE * FEATURE_COUNT];

LiquidCrystal_I2C lcd(0x27, 16, 2);
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Initialize MLX sensor

// Variables for non-blocking loop and HR calculation
unsigned long last_update_time = 0;
const unsigned long UPDATE_INTERVAL = 3000; // 3 seconds
long lastBeat = 0; 
float beatsPerMinute = 0;
int current_hr = 75;   // Default baseline
int current_spo2 = 98; // Default baseline 

//performance mesurments
int testCount = 0;
const int MAX_TESTS = 15;

unsigned long totalLatency = 0;
float totalThroughput = 0;

unsigned long requestStartTime;
unsigned long requestEndTime;



//void sendSMS(String message);
String urlEncode(String str);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < WINDOW_SIZE * FEATURE_COUNT; i++) {
    input_buffer[i] = 0.0;
  }

  // Init MLX90614
  lcd.print("Init MLX90614...");
  if (!mlx.begin()) {
    Serial.println("MLX90614 not found. Check wiring.");
  }
  delay(500);
  lcd.clear();

  // Init MAX30102
  lcd.print("Init MAX30102...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
  } else {
    particleSensor.setup(); 
    particleSensor.setPulseAmplitudeRed(0x0A); 
    particleSensor.setPulseAmplitudeGreen(0); 
  }
  delay(500);
  lcd.clear();

  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Connected!");
  delay(1000);
}

void loop() {
  // ========================================================
  // 0. CONTINUOUS SENSOR POLLING (Runs as fast as possible)
  // ========================================================
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute > 40 && beatsPerMinute < 180) {
      current_hr = (current_hr * 0.7) + (beatsPerMinute * 0.3);
    }
  }

  if (irValue < 50000) { 
    current_hr = 0;
    current_spo2 = 0; 
  } else {
    current_spo2 = 98; 
  }

  // ========================================================
  // 1. TIMED SYSTEM PIPELINE (Runs every 3 seconds)
  // ========================================================
  if (millis() - last_update_time >= UPDATE_INTERVAL) {
    last_update_time = millis(); // Reset timer

    float raw_vitals[FEATURE_COUNT];

    raw_vitals[0] = current_hr; 
    raw_vitals[1] = current_spo2;       
    
    // Read Object Temperature from MLX90614
    float temp_read = mlx.readObjectTempC();                            
    if (isnan(temp_read)) {
      raw_vitals[2] = 37.0; // Default to normal if sensor fails
    } else {
      raw_vitals[2] = constrain(temp_read, 34.0, 42.0); 
    }
    
    raw_vitals[3] = map(analogRead(SBP_PIN), 0, 4095, 80, 180);        
    raw_vitals[4] = map(analogRead(DBP_PIN), 0, 4095, 40, 110);        
    raw_vitals[5] = map(analogRead(RESP_RATE_PIN), 0, 4095, 10, 40);   
    raw_vitals[6] = 65.0; 
    raw_vitals[7] = 1.0;  

    // 2. SHIFT & SCALE BUFFER
    for (int i = 0; i < (WINDOW_SIZE - 1) * FEATURE_COUNT; i++) {
      input_buffer[i] = input_buffer[i + FEATURE_COUNT];
    }
    for (int i = 0; i < FEATURE_COUNT; i++) {
      input_buffer[((WINDOW_SIZE - 1) * FEATURE_COUNT) + i] = (raw_vitals[i] - means[i]) / stds[i];
    }

    // 3. LOCAL VITALS EVALUATION
    int qsofa_score = 0;
    int abnormal_count = 0;
    String abnormal_list = ""; 

    if(raw_vitals[0] > 90 || raw_vitals[0] < 50) { abnormal_count++; abnormal_list += "HR "; }
    if(raw_vitals[1] < 95) { abnormal_count++; abnormal_list += "O2 "; }
    if(raw_vitals[2] > 38.0 || raw_vitals[2] < 36.0) { abnormal_count++; abnormal_list += "Temp "; }
    if(raw_vitals[3] <= 100 ) { qsofa_score++; abnormal_count++; abnormal_list += "SBP "; }
    if(raw_vitals[4] > 90 || raw_vitals[4] < 60) { abnormal_count++; abnormal_list += "DBP "; }
    if(raw_vitals[5] >= 22 ) { qsofa_score++; abnormal_count++; abnormal_list += "Resp "; }

    abnormal_list.trim(); 
    String comma_list = abnormal_list;
    comma_list.replace(" ", ","); 

    Serial.printf("\n--- CURRENT VITALS ---\n");
    Serial.printf("HR: %.0f | O2: %.0f%% | Temp: %.1fC | BP: %.0f/%.0f | Resp: %.0f\n", 
                  raw_vitals[0], raw_vitals[1], raw_vitals[2], raw_vitals[3], raw_vitals[4], raw_vitals[5]);
    
    if (abnormal_count > 0) {
      Serial.print("⚠️ ABNORMAL vitals: ");
      Serial.println(comma_list);
    } else {
      Serial.println("✅ All vitals normal.");
    }

    // 4. CLOUD INFERENCE
    float prediction = 0.0;
    bool ai_positive = false;
    bool cloud_success = false;

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;     
      client.setInsecure();        
      
      HTTPClient http;
      http.begin(client, serverName); 
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(20000); 

      DynamicJsonDocument doc(2048);
      
      JsonArray scaled_data = doc.createNestedArray("scaled_data");
      for (int i = 0; i < WINDOW_SIZE * FEATURE_COUNT; i++) { 
        scaled_data.add(input_buffer[i]); 
      }
      
      JsonObject raw_vitals_obj = doc.createNestedObject("raw_vitals");

      raw_vitals_obj["patient_email"] = patient_email;
      raw_vitals_obj["hr"] = raw_vitals[0];
      raw_vitals_obj["spo2"] = raw_vitals[1];
      raw_vitals_obj["temp"] = raw_vitals[2];
      raw_vitals_obj["sbp"] = raw_vitals[3];
      raw_vitals_obj["dbp"] = raw_vitals[4];
      raw_vitals_obj["rr"] = raw_vitals[5];
      
      String requestBody; 
      serializeJson(doc, requestBody);
      
      requestStartTime = millis();

      int httpResponseCode = http.POST(requestBody);

      requestEndTime = millis();

      unsigned long latency = requestEndTime - requestStartTime;

      float latencySeconds = latency / 1000.0;
      int dataSize = requestBody.length();
      
      float throughput = 0;
      if(latency > 0){
        throughput = dataSize / latencySeconds;
      } 


      if (httpResponseCode == 200) {

        if(testCount < MAX_TESTS){
          testCount++;
          totalLatency += latency;
          totalThroughput += throughput;
        }
        Serial.println("\n========== PERFORMANCE TEST ==========");
        Serial.print("Test Number       : ");
        Serial.println(testCount);

        Serial.print("Data Size         : ");
        Serial.print(dataSize);
        Serial.println(" bytes");

        Serial.print("Latency           : ");
        Serial.print(latency);
        Serial.println(" ms");

        Serial.print("Throughput        : ");
        Serial.print(throughput, 2);
        Serial.println(" bytes/sec");

        Serial.println("======================================");

        if(testCount == MAX_TESTS){
          Serial.println("\n====================================");
          Serial.println("15 TESTS COMPLETED");
          Serial.println("====================================");

          Serial.print("Average Latency: ");
          Serial.print((float)totalLatency / MAX_TESTS);
          Serial.println(" ms");

          Serial.print("Average Throughput: ");
          Serial.print(totalThroughput / MAX_TESTS, 2);
          Serial.println(" bytes/sec");

          Serial.println("====================================");
        }



        Serial.println("🌐 ✅ Connected to Cloud Server successfully!");
        
        String response = http.getString();
        StaticJsonDocument<256> responseDoc;
        deserializeJson(responseDoc, response);
        prediction = responseDoc["risk"];
        ai_positive = (prediction >= DECISION_THRESHOLD);
        cloud_success = true;
      } else {
        Serial.print("❌ Server crashed! Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Server said: ");
        Serial.println(http.getString());
      }
      http.end();
    }

    // 5. DISPLAY & ALERTS
    lcd.clear();
    
    if (abnormal_count >= 2) {
      lcd.setCursor(0, 0); lcd.print("qSOFA Score: ");
      lcd.print(qsofa_score);
      lcd.setCursor(0, 1); 
      
      if (ai_positive) {
        lcd.print("AI Risk: "); lcd.print(prediction * 100, 1); lcd.print("%"); 
      } else {
        String msg = "Bad: " + comma_list;
        if (msg.length() > 16) msg = msg.substring(0, 16); 
        lcd.print(msg); 
      }
      
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      digitalWrite(BUZZER, HIGH); 

      if(qsofa_score == 2){ 
        if(millis() - last_sms_time > 60000) { 
          String sms_msg = "Alert Abnormal  Vitals. qSOFA Score: " + String(qsofa_score)  + (ai_positive ? ". AI Risk: " + String(prediction * 100, 1) + "%" : "Bad: " + comma_list);
          //sendSMS(sms_msg);
          last_sms_time = millis();
        }
      }
    } else {
      lcd.setCursor(0, 0); 
      lcd.print("Vitals Normal");
      
      lcd.setCursor(0, 1); 
      lcd.print("Status: stable");
      
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER, LOW); 
    }
  }
}

// --- TWILIO SMS HELPER FUNCTION ---
/*void sendSMS(String message) {
  if(String(twilio_sid) == "YOUR_TWILIO_ACCOUNT_SID") {
    Serial.println("⚠️ SMS Skipped: Put your real Twilio credentials in the code!");
    return; 
  }

  HTTPClient http;
  String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(twilio_sid) + "/Messages.json";
  http.begin(url);
  http.setAuthorization(twilio_sid, twilio_token);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "To=" + urlEncode(destination_phone_number) + "&From=" + urlEncode(twilio_phone_number) + "&Body=" + urlEncode(message);

  int httpResponseCode = http.POST(postData);
  if (httpResponseCode == 201) {
    Serial.println("📱 ✅ SMS Sent Successfully!");
  } else {
    Serial.print("❌ Error sending SMS: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}*/

String urlEncode(String str) {
  String encodedString = "";
  char c, code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) { encodedString += c; }
    else if (c == ' ') { encodedString += '+'; }
    else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encodedString += '%'; encodedString += code0; encodedString += code1;
    }
  }
  return encodedString;
}
