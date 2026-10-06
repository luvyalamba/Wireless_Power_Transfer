#define BLYNK_TEMPLATE_ID "TMPL3vYL3U7tv"
#define BLYNK_TEMPLATE_NAME "WPT"
#define BLYNK_AUTH_TOKEN "nhcF15UrgtT27glKFdqsnhTKrFMzjNLq"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Luvya";
char pass[] = "Cancer@190706";

HardwareSerial RAK(2);  // UART2

void setup() {
  Serial.begin(115200);

  // UART2
  // RX = GPIO16
  // TX = GPIO17
  RAK.begin(115200, SERIAL_8N1, 16, 17);

  Serial.println();
  Serial.println("Connecting to WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V0, 99);
  Blynk.virtualWrite(V1, 88);
  Blynk.virtualWrite(V2, -55);
  Blynk.virtualWrite(V3, 7);
  Blynk.virtualWrite(V4, 1);

  Serial.println("Connected to Blynk!");
}

void loop() {
  Blynk.run();

  if (RAK.available()) {
    String line = RAK.readStringUntil('\n');
    line.trim();

    Serial.println(line);

    if (line.startsWith("DATA")) {
      float temperature, humidity;
      int rssi, snr;

      if (sscanf(line.c_str(),
                 "DATA,%f,%f,%d,%d",
                 &temperature,
                 &humidity,
                 &rssi,
                 &snr)
          == 4) {
        // Send to Blynk
        Blynk.virtualWrite(V0, temperature);
        Blynk.virtualWrite(V1, humidity);
        Blynk.virtualWrite(V2, rssi);
        Blynk.virtualWrite(V3, snr);
        Blynk.virtualWrite(V4, 1);

        // Serial Monitor
        Serial.println("---------------------------");
        Serial.print("Temperature : ");
        Serial.print(temperature);
        Serial.println(" C");

        Serial.print("Humidity    : ");
        Serial.print(humidity);
        Serial.println(" %");

        Serial.print("RSSI        : ");
        Serial.print(rssi);
        Serial.println(" dBm");

        Serial.print("SNR         : ");
        Serial.print(snr);

        Serial.println("---------------------------");
      }
    } else if (line.startsWith("TIMEOUT")) {
      unsigned long elapsed;

      if (sscanf(line.c_str(),
                 "TIMEOUT,%lu",
                 &elapsed)
          == 1) {
        Serial.print("Link Lost for ");
        Serial.print(elapsed);
        Serial.println(" ms");

        Blynk.virtualWrite(V4, 0);
      }
    }
  }
}