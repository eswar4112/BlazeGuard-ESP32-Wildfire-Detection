<p align="center">
  <img src="https://img.shields.io/badge/ESP32-✓-orange?style=for-the-badge&logo=esp32&logoColor=white" alt="ESP32"/>
  <img src="https://img.shields.io/badge/IoT-Fire%20Safety-red?style=for-the-badge" alt="IoT"/>
  <img src="https://img.shields.io/badge/Wildfire%20Detection-Active-orange?style=for-the-badge" alt="Wildfire"/>
  <img src="https://img.shields.io/badge/Final%20Year%20Project-2025--2026-blueviolet?style=for-the-badge" alt="Final Year"/>
</p>

<h1 align="center">🔥 BlazeGuard - ESP32 Wildfire Detection</h1>
<h3 align="center">Low-Cost IoT Early Warning System with Dynamic Risk Scoring</h3>

<p align="center">
  <strong>Intelligent forest fire detection using ESP32 • DHT22 • MQ-7 • Adaptive baselines • Buzzer & OLED alerts</strong><br>
  Total build cost: ~₹1200–1800 • Presented at Tech Expo Abhiyanth 2026
</p>

### ✨ Core Features
- Real-time monitoring of **temperature**, **humidity** (DHT22) and **combustible gases/CO** (MQ-7)
- Dynamic risk score (0–100) with 3 levels: **Normal (0–40)** • **Medium (41–70)** • **High (71–100)**
- **Innovation**: Baselines auto-recalibrate every 10 minutes **only when risk ≤ 40** (EWMA smoothing) — avoids learning from fire events
- Loud buzzer alert with rhythmic `tone()` pattern on High risk
- 0.96" SSD1306 OLED showing live values + large centered risk score
- Fully adaptive to local diurnal/environmental changes

### Hardware in Action

<p align="center">
  <img src="images/full_hardware_setup.jpg" alt="Complete BlazeGuard Prototype" width="600"/>
  <br><em>Full wired prototype with ESP32, sensors, buzzer and OLED display</em>
</p>

<p align="center">
  <img src="images/oled_normal.jpg" alt="OLED - Normal Risk" width="320"/>
  &nbsp;&nbsp;&nbsp;
  <img src="images/oled_high.jpg" alt="OLED - High Risk Alert" width="320"/>
  <br><em>Left: Safe conditions • Right: High risk triggered (demo with heat/gas)</em>
</p>

<p align="center">
  <img src="images/expo_poster.jpg" alt="Abhiyanth 2026 Poster" width="500"/>
  <br><em>Project poster presented on March 5, 2026 at Tech Expo Abhiyanth</em>
</p>

### Source Code
All logic is in a single-file Arduino sketch:

→ [`blazeguard_code/BlazeGuard.ino`](blazeguard_code/BlazeGuard.ino)  
*(or replace BlazeGuard.ino with your actual filename if different)*

Key libraries used:
- `DHT sensor library` (by Adafruit)
- `Adafruit_GFX`
- `Adafruit_SSD1306`

Core risk calculation (tuned example):

```cpp
// Example from loop() or risk function
float humFactor  = 0.30 * (baseHum - humidity);
float tempFactor = 1.50 * (temperature - baseTemp);
float gasFactor  = 0.50 * (gasRaw / 2.0);          // divisor tuned for your MQ-7 range

riskScore = constrain(humFactor + tempFactor + gasFactor, 0, 100);

// Classification & alert
if (riskScore > 70) {
  // High alert: rhythmic buzzer pattern
  tone(BUZZER_PIN, 800, 200); delay(100);
  tone(BUZZER_PIN, 1000, 200); delay(100);
  // ... more tones or loop for pattern
}
