/*
   RAK3172 LoRa P2P Receiver
   Receives SHT40 data from another RAK3172
   Forwards it to ESP32 over UART
   Prints continuous timeout information when link is lost
*/

#include <Arduino.h>

struct SensorPacket
{
    float temperature;  
    float humidity;
};

// ---------------- LoRa Settings ----------------
double myFreq = 868000000;
uint16_t sf = 7;       // Fastest spreading factor
uint16_t bw = 2;       // 500 kHz
uint16_t cr = 0;       // 4/5
uint16_t preamble = 6; // Shorter preamble
uint16_t txPower = 0;  // 0 dBm (Receiver doesn't transmit much, but good practice to match)

// ---------------- Link Monitor ----------------
// Increased timeout to 25 seconds because the TX now sends every 10 seconds.
// This allows for 1 missed packet before triggering a timeout warning.
const uint32_t LINK_TIMEOUT = 25000; 

uint32_t lastPacketTime = 0;

void recv_cb(rui_lora_p2p_recv_t data)
{
    if (data.Status != LORA_P2P_RXDONE)
        return;

    if (data.BufferSize != sizeof(SensorPacket))
    {
        Serial.println("Invalid packet");
        return;
    }

    SensorPacket packet;
    memcpy(&packet, data.Buffer, sizeof(packet));

    lastPacketTime = millis();

    // ---------- USB Serial ----------
    Serial.println("--------------------------------");

    Serial.print("Temperature : ");
    Serial.print(packet.temperature, 2);
    Serial.println(" C");

    Serial.print("Humidity    : ");
    Serial.print(packet.humidity, 2);
    Serial.println(" %");

    Serial.print("RSSI        : ");
    Serial.print(data.Rssi);
    Serial.println(" dBm");

    Serial.print("SNR         : ");
    Serial.print(data.Snr);
    Serial.println(" dB"); // Changed from printing nothing to "dB" for SNR

    Serial.println("--------------------------------");

    // ---------- UART to ESP32 ----------
    // Format:
    // DATA,temp,humidity,rssi,snr
    Serial1.print("DATA,");
    Serial1.print(packet.temperature, 2);
    Serial1.print(",");
    Serial1.print(packet.humidity, 2);
    Serial1.print(",");
    Serial1.print(data.Rssi);
    Serial1.print(",");
    Serial1.println(data.Snr);
}

void send_cb()
{
    // If the RX node ever transmits, it needs to go back to listening
    api.lora.precv(65534);
}

void setup()
{
    Serial.begin(115200);      // USB Debug
    Serial1.begin(115200);     // UART to ESP32

    delay(1000);

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

    api.lora.registerPRecvCallback(recv_cb);
    api.lora.registerPSendCallback(send_cb);

    // Enter continuous receive mode
    api.lora.precv(65534);

    lastPacketTime = millis();

    Serial.println("RAK3172 Receiver Ready");
}

void loop()
{
    uint32_t elapsed = millis() - lastPacketTime;

    if (elapsed > LINK_TIMEOUT)
    {
        Serial.print("No packet received for ");
        Serial.print(elapsed);
        Serial.println(" ms");

        // Send timeout to ESP32
        Serial1.print("TIMEOUT,");
        Serial1.println(elapsed);

        delay(1000);      // Print once every second while disconnected
    }

    delay(100);
}