# Mail Beacon

The Mailbox Beacon is a device that detects when a new letter or package has been delivered to your mailbox using a reed switch. It sends a signal to your LoRa gateway, then the gateway sends a message via WiFi to MQTT server.


Specification
--
- Microcontroller: Attiny1616
- Radio module: SX1278 LoRa 433 Mhz (Ai-Thinker-Ra-01)
- Sensor: Reed switch
- Voltage regulator: SPX3819M5-L-3-3
- Programming Protocol: UPDI
- Power consumption: in microamps range when in sleep
- Gateway: LILYGO TTGO LoRa32 V2.1


Battery monitoring
--
The beacon sends the battery voltage (in milivolts) in every LoRa packet, so that you know when you need to re-charge the battery.

Schematic
--
![schematic](images/schematic.png)
![pcb](images/pcb_routing.png)

Images
--
![manufactured](images/mail_beacon.jpg)
