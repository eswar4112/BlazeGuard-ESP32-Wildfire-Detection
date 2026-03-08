// These THREE lines MUST be FIRST — before ANY #include
#define BLYNK_TEMPLATE_ID   "TMPL3z8v080_a"
#define BLYNK_TEMPLATE_NAME "Blazeguard"
#define BLYNK_AUTH_TOKEN    "JXpq2PtVLUoR4SVDvkNWenxvRKeVxTNv"

#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ─── WiFi ─────────────────────────────────────────────────────────────────────
char ssid[] = "Eswar";
char pass[] = "87654321";

// ─── Hardware ─────────────────────────────────────────────────────────────────
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define GAS_PIN 34     // MQ-7 analog
#define BUZZER_PIN 25  // Buzzer

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

float baseTemp = 28.0;
float baseHum  = 40.0;
float baseGas  = 150.0;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 600000UL; // 10 min

String riskLevel = "Normal";

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nBlazeGuard starting...");

  dht.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // WiFi + Blynk
  Serial.print("Connecting to WiFi ");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  Serial.print("Connecting to Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  if (Blynk.connected()) {
    Serial.println(" Blynk connected!");
  } else {
    Serial.println(" Blynk connection failed.");
  }

  // OLED
  if (!u8g2.begin()) {
    Serial.println(F("OLED init failed!"));
    for(;;);
  }
  u8g2.setContrast(140);
  u8g2.setFontDirection(0);
  u8g2.setFontPosTop();

  // Startup screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(10, 25);
  u8g2.print("BlazeGuard AI");
  u8g2.setCursor(30, 45);
  u8g2.print("Online");
  u8g2.sendBuffer();
  delay(3000);
}

void loop() {
  Blynk.run();

  float humidity    = dht.readHumidity();
  float temperature = dht.readTemperature();
  int gasRaw        = analogRead(GAS_PIN);
  float gas         = (float)gasRaw;

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT22 error!");
    delay(1800);
    return;
  }

  // Risk calculation
  float gasRel    = gas / baseGas;
  float gasFactor = 60.0 * constrain(gasRel - 1.0, 0, 2.5);
  float humFactor = 0.35 * (baseHum - humidity);
  float tempFactor = 1.40 * (temperature - baseTemp);

  float riskScoreRaw = constrain(humFactor + tempFactor + gasFactor, 0, 100);

  static float smoothedRisk = 20.0;
  smoothedRisk = 0.65 * smoothedRisk + 0.35 * riskScoreRaw;
  float riskScore = smoothedRisk;

  String prevRisk = riskLevel;
  if (riskScore <= 38)      riskLevel = "Normal";
  else if (riskScore <= 72) riskLevel = "Medium";
  else                      riskLevel = "High";

  // Send to Blynk
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, (int)round(humidity));
  Blynk.virtualWrite(V2, gasRaw);
  Blynk.virtualWrite(V3, (int)round(riskScore));
  Blynk.virtualWrite(V4, riskLevel);

  // Events only on change
  if (riskLevel != prevRisk) {
    if (riskLevel == "Medium") {
      Blynk.logEvent("medium_risk", String("Medium risk: ") + (int)round(riskScore) + " | T:" + temperature + "°C");
      Serial.println("→ Medium risk event sent");
    } else if (riskLevel == "High") {
      Blynk.logEvent("high_risk", String("HIGH RISK! ") + (int)round(riskScore) + " | T:" + temperature + "°C | Gas:" + gasRaw);
      Serial.println("→ HIGH risk event sent");
    }
  }

  // Baseline update
  if (millis() - lastUpdate >= updateInterval) {
    if (riskLevel == "Normal") {
      baseTemp = 0.92 * baseTemp + 0.08 * temperature;
      baseHum  = 0.92 * baseHum  + 0.08 * humidity;
      baseGas  = 0.92 * baseGas  + 0.08 * gas;
      Serial.printf("Baselines updated → T=%.1f H=%.1f Gas=%.0f\n", baseTemp, baseHum, baseGas);
    }
    lastUpdate = millis();
  }

  // OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(0, 0);
  u8g2.print("T: "); u8g2.print(temperature, 1); u8g2.print("C");
  u8g2.print("  H: "); u8g2.print(humidity, 0); u8g2.print("%");
  u8g2.setCursor(0, 12);
  u8g2.print("CO raw: "); u8g2.print(gasRaw);

  String scoreStr = String((int)round(riskScore));
  u8g2.setFont(u8g2_font_logisoso32_tn);
  int width = u8g2.getStrWidth(scoreStr.c_str());
  int x = (128 - width) / 2;
  int y = 26;

  u8g2.setDrawColor(1);
  u8g2.drawBox(x-8, y-6, width+16, 44);

  if (riskLevel == "High") u8g2.setDrawColor(0);
  else                     u8g2.setDrawColor(1);

  u8g2.setCursor(x, y);
  u8g2.print(scoreStr);

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);
  u8g2.setCursor(x + (width/2) - 20, y + 40);
  u8g2.print("RISK");

  if (riskLevel == "High" && (millis() / 400) % 2 == 0) {
    u8g2.drawFrame(4, 4, 120, 56);
  }

  u8g2.sendBuffer();

  // Buzzer
  noTone(BUZZER_PIN);

  if (riskLevel == "Medium") {
    tone(BUZZER_PIN, 800, 80);
    delay(1000 - 80);
  }
  else if (riskLevel == "High") {
    tone(BUZZER_PIN, 950, 100);  delay(140);
    tone(BUZZER_PIN, 1100, 120); delay(160);
    tone(BUZZER_PIN, 1250, 140); delay(180);
    delay(500);
  }
  else {
    delay(1400);
  }
}