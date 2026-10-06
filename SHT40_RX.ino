/*
   RAK3172 LoRa P2P Receiver
   Receives SHT40 data and reports link status.
*/

#include <Arduino.h>

struct SensorPacket
{
    float temperature;
    float humidity;
};

// ---------- LoRa Settings ----------
double myFreq = 868000000;
uint16_t sf = 12;
uint16_t bw = 0;
uint16_t cr = 0;
uint16_t preamble = 8;
uint16_t txPower = 22;

// ---------- Link Monitor ----------
const uint32_t LINK_TIMEOUT = 1500;    // Wait 3 s before declaring link lost

uint32_t lastPacketTime = 0;
bool linkLostPrinted = true;

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

    if (linkLostPrinted)
    {
        Serial.println();
        Serial.println("******** LINK RESTORED ********");
        Serial.println();
    }

    linkLostPrinted = false;

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
    Serial.println(" dB");

    Serial.println("--------------------------------");
}

void send_cb()
{
    api.lora.precv(65534);
}

void setup()
{
    Serial.begin(115200);
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

    api.lora.precv(65534);

    Serial.println("LoRa Receiver Ready");
}

void loop()
{
    if ((millis() - lastPacketTime > LINK_TIMEOUT))
    {
        Serial.println();
        Serial.println("******** LINK LOST ********");

        Serial.print("No packet received for ");
        Serial.print(millis() - lastPacketTime);
        Serial.println(" ms");

        Serial.println("***************************");
        Serial.println();
        delay(1000);
    }

    delay(100);
}