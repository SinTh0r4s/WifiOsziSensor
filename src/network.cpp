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

// Parse received packet and, if successful, invoke setTrigger()
void parsePacket();

// Global Beacon instance to provide a filled struct at will
Beacon beaconBlueprint;
// Timestamp of last Beacon transmission
uint32_t lastBeaconTransmissionMillis = 0;
// Which ip to send data to
IPAddress responseTargetIp;
// Which port to send data to
uint32_t responseTargetPort = 0;
setTriggerCallback _setTriggerCallback;
WiFiUDP Udp;
// Buffer for received ethernet frames
uint8_t rxBuffer[ETHERNET_MAX_BUFFER_SIZE];

void Network_registerSetTriggerCallback(setTriggerCallback _cb)
{
    _setTriggerCallback = _cb;
}

void Network_init()
{
    beaconBlueprint.magicNumber = MAGIC_ID;
    strcpy(beaconBlueprint.model, BOARD_DESCRIPTION);
    strcpy(beaconBlueprint.adc, BOARD_ADC_DESCRIPTION);
    beaconBlueprint.v_ref = 1650;
    beaconBlueprint.channels = BOARD_CHANNELS;
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
        logTrace("Received packet");
        int len = Udp.read(rxBuffer, ETHERNET_MAX_BUFFER_SIZE);
        if(len != packetSize)
            logError("Could not read packet!");
        parsePacket();
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
    Udp.beginPacket(BROADCAST_IP, BEACON_LISTENING_PORT);
    Udp.write((uint8_t*)&beaconBlueprint, sizeof(Beacon));
    Udp.endPacket();
    logTrace("Beacon send");
}

void parsePacket()
{
    Command* command = (Command*)&rxBuffer;
    logTrace("magicNumber");
    if(command->magicNumber != MAGIC_ID)
    {
        logTrace("Received packet is not part of this project. Discarding...");
        return;
    }
    responseTargetPort = command->port;

    if(_setTriggerCallback == NULL)
    {
        logWarning("No callback is set!");
        return;
    }

    logDebug("Received valig command. Applying settings...");
    TriggerSetting* triggerSetting = (TriggerSetting*)(command+1);
    for(uint32_t i=0;i<command->numSettings;i++)
    {
        const TriggerSetting ts = triggerSetting[i];
        _setTriggerCallback(ts.channel, ts.active, ts.triggerVoltage);
    }
}