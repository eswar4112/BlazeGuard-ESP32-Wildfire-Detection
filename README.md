### Source Code
All logic is in a single, well-commented Arduino sketch:

→ [`blynk_blazeguard.ino`](src/BlazeGuard.ino)

Key parts include:
- Pin definitions
- Sensor initialization (DHT22, MQ-7, OLED, buzzer)
- Dynamic baseline calculation (EWMA, only when risk ≤ 40)
- Real-time risk score formula
- OLED display updates
- Buzzer alert patterns using `tone()`

Example core risk calculation snippet:

```cpp
// Inside loop() or a dedicated function
humFactor  = 0.30 * (baseHum - humidity);
tempFactor = 1.50 * (temperature - baseTemp);
gasFactor  = 0.50 * (gasValue / 2.0);          // tuned divisor
riskScore  = constrain(humFactor + tempFactor + gasFactor, 0, 100);
