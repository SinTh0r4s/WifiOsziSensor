//libs
#include <WiFi101.h>
#include <WiFiUdp.h>

// local includes
#include "network.h"
#include "debug.h"
#include "constants.h"
#include "secrets.h"
#include "networkStructs.h"


// Send a network message to advertise this devise and its properties
void sendBeacon();

Beacon beaconBlueprint;
uint32_t lastBeaconTransmissionMillis = 0;
setTriggerCallback _setTriggerCallback;
WiFiUDP Udp;
uint8_t rxBuffer[ETHERNET_MAX_BUFFER_SIZE];

void Network_registerSetTriggerCallback(setTriggerCallback _cb)
{
    _setTriggerCallback = _cb;
}

void Network_init()
{
    beaconBlueprint.magicNumber = MAGIC_ID;
    strcpy(beaconBlueprint.model, BOARD_DESCRIPTION);
    strcpy(beaconBlueprint.adc, ADC_DESCRIPTION);
    beaconBlueprint.v_ref = 1650;
    beaconBlueprint.channels = 1;
    beaconBlueprint.frequency = 100000;
    beaconBlueprint.numSamples = 10000;
    beaconBlueprint.resolution = 8;
    beaconBlueprint.sampleTime = ((float)beaconBlueprint.frequency) / ((float)beaconBlueprint.numSamples);
    beaconBlueprint.port = BOARD_LISTENING_PORT;
}

void Network_handleEvents()
{
    int packetSize = Udp.parsePacket();
    if (packetSize > 0)
    {
        logDebug("Received packet");
        // read the packet into packetBufffer
        int len = Udp.read(rxBuffer, ETHERNET_MAX_BUFFER_SIZE);
        if(len > 0)
            logTrace("Buffer read successfully");
        if(len == packetSize)
            logTrace("Bytes read == packetSize");
        // send a reply, to the IP address and port that sent us the packet we received
    }

    const uint32_t currentMillis = millis();
    if(currentMillis - BEACON_SEPARATION_TIME > lastBeaconTransmissionMillis)
    {
        sendBeacon();
        lastBeaconTransmissionMillis = currentMillis;
    }
}

void Network_connectWifi()
{
    int status = WL_IDLE_STATUS;
    while(status != WL_CONNECTED)
    {
        logDebug("Attempting to connect to SSID: ");
        logDebug(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(WIFI_SSID, WIFI_SSID_PWD);
    }
    logInfo("Connected to wifi");
    /*IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);*/
}

void Network_beginListen()
{
    Udp.begin(BOARD_LISTENING_PORT);
    logTrace("Begin to listen");
}

void sendBeacon()
{
    Udp.beginPacket(BROADCAST_IP, UI_LISTENING_PORT);
    Udp.write((uint8_t*)&beaconBlueprint, sizeof(Beacon));
    Udp.endPacket();
}