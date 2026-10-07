/*
   RAK3172 + SHT40 LoRa P2P Transmitter
   Reads SHT40 and transmits temperature & humidity every 10 seconds.
   Optimized for ultra-low power consumption.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>

Adafruit_SHT4x sht4;

struct SensorPacket
{
    float temperature;
    float humidity;
};  

SensorPacket packet;

// ---------------- LoRa Settings --- -------------
double myFreq = 868000000;
uint16_t sf = 7;       // Fastest spreading factor
uint16_t bw = 2;       // 500 kHz
uint16_t cr = 0;       // 4/5
uint16_t preamble = 6; // Shorter preamble
uint16_t txPower = 0;  // 0 dBm for lowest power consumption

// Transmission interval in milliseconds
const uint32_t TX_INTERVAL = 10000; // 10 seconds

// ------------------------------------------------

void send_cb()
{
    Serial.println("TX Done, going to sleep.");
    // Do NOT put the device into RX mode here to save power
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // ---------- SHT40 ----------
    Wire.begin();

    if (!sht4.begin())
    {
        Serial.println("Couldn't find SHT40");
        while (1);
    }

    // Lower precision can also save a tiny bit of power on the sensor side, 
    // but HIGH_PRECISION is generally fine for a 10s interval.
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);

    Serial.println("SHT40 Found");

    // ---------- LoRa ----------
    if (api.lora.nwm.get() != 0)
    {
        api.lora.nwm.set();
        api.system.reboot();
    }

    api.lora.pfreq.set(myFreq);
    api.lora.psf.set(sf);
    api.lora.pbw.set(bw);
    api.lora.pcr.set(cr);
    api.lora.ppl.set(preamble);
    api.lora.ptp.set(txPower);

    api.lora.registerPSendCallback(send_cb);
    
    // Turn off continuous receive mode to save power
    api.lora.precv(0); 

    Serial.println("LoRa TX Ready");
}

void loop()
{
    sensors_event_t humidity, temp;

    if (!sht4.getEvent(&humidity, &temp))
    {
        Serial.println("SHT40 Read Failed");
        api.system.sleep.all(1000);
        return;
    }

    packet.temperature = temp.temperature;
    packet.humidity = humidity.relative_humidity;

    Serial.print("Temperature: ");
    Serial.print(packet.temperature);
    Serial.print(" C   Humidity: ");
    Serial.print(packet.humidity);
    Serial.println(" %");

    if (api.lora.psend(sizeof(packet), (uint8_t *)&packet))
    {
        Serial.println("Packet Sent");
    }
    else
    {
        Serial.println("Send Failed");
    }

    // Enter deep sleep until the next transmission
    api.system.sleep.all(TX_INTERVAL);
}