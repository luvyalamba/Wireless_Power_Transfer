#Ambient power harvesting using a rectenna array to power an IoT application, demonstrated by powering a field deployed sensor node for stubble-burning detection.


#Lifecylce:

The system begins with a wideband antenna covering the 750 MHz–2.6 GHz range, which captures ambient RF energy from the surrounding environment.

The harvested RF signal is fed into a custom-designed rectifier, which converts the RF energy into DC power. The rectifier is capable of delivering approximately 7 V DC at 20 dBm input power.

The harvested energy is stored in a 1 F supercapacitor, which acts as the energy buffer for the sensor node.

The stored energy is then used to power a field-deployed sensor node built around a RAK3172 and SHT40. The node follows a sleep–transmit–sleep operating cycle, waking periodically to measure and transmit sensor data over 868 MHz LoRa, with transmissions occurring approximately once every minute.

These transmissions are received at the LoRaWAN gateway build around a RAK3172 and ESP32 and forwarded to the internet through API requests. The data can then be used to monitor the deployed nodes, view sensor statistics, and generate alerts for potential stubble-burning events.
