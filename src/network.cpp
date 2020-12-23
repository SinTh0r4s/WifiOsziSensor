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

// Global Header instances to provide a prefilled struct at will
BeaconHeader beaconBlueprint;
SampleTransmissionHeader sampleTransmissionBlueprint;
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

    sampleTransmissionBlueprint.magicNumber = MAGIC_ID;
    sampleTransmissionBlueprint.frameId = 0;
    sampleTransmissionBlueprint.numFrames = 1;
    sampleTransmissionBlueprint.resolution = 8;
    sampleTransmissionBlueprint.channels = BOARD_CHANNELS;
    sampleTransmissionBlueprint.frequency = 100000;
    sampleTransmissionBlueprint.v_ref = 1650;
    sampleTransmissionBlueprint.numSamples = 1000;
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
    Udp.write((uint8_t*)&beaconBlueprint, sizeof(BeaconHeader));
    Udp.endPacket();
    logTrace("Beacon send");
}

void parsePacket()
{
    CommandHeader* command = (CommandHeader*)&rxBuffer;
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
    TriggerSettingHeader* triggerSetting = (TriggerSettingHeader*)(command+1);
    for(uint32_t i=0;i<command->numSettings;i++)
    {
        const TriggerSettingHeader ts = triggerSetting[i];
        _setTriggerCallback(ts.channel, ts.active, ts.triggerVoltage);
    }
}

void Network_sendSamples(uint8_t* samples, uint32_t numSamples)
{
    logDebug("Transmitting samples...");
    const uint32_t numFullFrames = numSamples / SAMPLES_PER_PACKET;
    const uint32_t remainder = numSamples % SAMPLES_PER_PACKET;
    if(remainder > 0)
        sampleTransmissionBlueprint.numFrames = numFullFrames + 1;

    for(uint32_t frameId=0;frameId<numFullFrames;frameId++)
    {
        Udp.beginPacket(responseTargetIp, responseTargetPort);
        sampleTransmissionBlueprint.frameId = frameId;
        Udp.write((uint8_t*)&sampleTransmissionBlueprint, sizeof(SampleTransmissionHeader));
        Udp.write(&samples[frameId * SAMPLES_PER_PACKET], SAMPLES_PER_PACKET);
        Udp.endPacket();
    }
    if(remainder > 0)
    {
        Udp.beginPacket(responseTargetIp, responseTargetPort);
        sampleTransmissionBlueprint.frameId = numFullFrames;
        Udp.write((uint8_t*)&sampleTransmissionBlueprint, sizeof(SampleTransmissionHeader));
        Udp.write(&samples[numFullFrames * SAMPLES_PER_PACKET], remainder);
        Udp.endPacket();
    }
    logDebug("Samples transmitted.");
}