/*
   RAK3172 + SHT40 LoRa P2P Transmitter (FIXED DEEP SLEEP)
   Reads SHT40 and transmits temperature & humidity every 10 minutes.
   Uses RUI3 Hardware Timers so interrupts don't break the sleep cycle.
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

// ---------------- LoRa Settings ----------------
double myFreq = 868000000;
uint16_t sf = 7;       // Fastest spreading factor
uint16_t bw = 2;       // 500 kHz
uint16_t cr = 0;       // 4/5
uint16_t preamble = 6; // Shorter preamble
uint16_t txPower = 0;  // 0 dBm

const uint32_t TX_INTERVAL = 600000; // 10 minutes (in milliseconds)

// ------------------------------------------------

void send_cb()
{
    Serial.println("TX Done. Sleeping until next timer trigger.");
}

// This function is triggered by the hardware timer every 10 minutes
void take_reading_and_send(void *data)
{
    sensors_event_t humidity, temp;

    if (!sht4.getEvent(&humidity, &temp))
    {
        Serial.println("SHT40 Read Failed");
        return; // Will try again on the next timer tick
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
        Serial.println("Packet Queued for TX");
    }
    else
    {
        Serial.println("Send Failed");
    }
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
    
    // Turn off continuous receive mode immediately
    api.lora.precv(0); 

    // ---------- Timer Setup ----------
    // Create a periodic hardware timer
    api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)take_reading_and_send, RAK_TIMER_PERIODIC);
    
    // Start the timer with the 10-minute interval
    api.system.timer.start(RAK_TIMER_0, TX_INTERVAL, NULL);

    Serial.println("LoRa TX Ready. Sending first packet...");
    
    // Fire off the first packet manually so you don't have to wait 10 minutes to verify it works
    take_reading_and_send(NULL);
}

void loop()
{
    // Sleep indefinitely. 
    // The RAK3172 will only wake up when a hardware interrupt (like the timer or TX Done) occurs,
    // handle the interrupt, and then immediately return to this sleep state.
    api.system.sleep.all(); 
}