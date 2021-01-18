//libs
#ifdef ESP32
    #include <WiFi.h>
#elif MKR1000
    #include <WiFi101.h>
#endif

#include <WiFiUdp.h>

// local includes
#include "network.h"
#include "debug.h"
#include "constants.h"
#include "secrets.h"
#include "networkStructs.h"
#include "adc.h"


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
WiFiUDP Udp;
// Buffer for received ethernet frames
uint8_t rxBuffer[ETHERNET_MAX_BUFFER_SIZE];

void mNetwork::init()
{
    beaconBlueprint.magicNumber = MAGIC_ID;
    strcpy(beaconBlueprint.model, BOARD_DESCRIPTION);
    strcpy(beaconBlueprint.adc, BOARD_ADC_DESCRIPTION);
    beaconBlueprint.v_ref = BOARD_V_REF;
    beaconBlueprint.channels = BOARD_CHANNELS;
    beaconBlueprint.beaconId = 0;
    beaconBlueprint.frequency = ADC_FREQUENCY;
    beaconBlueprint.numSamples = ADC_BUFFER_SIZE * ADC_NUM_BUFFERS;
    beaconBlueprint.resolution = BOARD_RESOLUTION;
    beaconBlueprint.port = BOARD_LISTENING_PORT;

    sampleTransmissionBlueprint.magicNumber = MAGIC_ID;
    sampleTransmissionBlueprint.frameId = 0;
    sampleTransmissionBlueprint.numFrames = 1;
    sampleTransmissionBlueprint.transmissionGroupId = 0;
    sampleTransmissionBlueprint.resolution = BOARD_RESOLUTION;
    sampleTransmissionBlueprint.channels = BOARD_CHANNELS;
    sampleTransmissionBlueprint.frequency = ADC_FREQUENCY;
    sampleTransmissionBlueprint.v_ref = BOARD_V_REF;
    sampleTransmissionBlueprint.numSamples = SAMPLES_PER_PACKET;
}

void mNetwork::handleEvents()
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

void mNetwork::connectWifi()
{
    while(WiFi.status() != WL_CONNECTED)
    {
        logDebug("Attempting to connect to SSID: ");
        logDebug(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        WiFi.begin(WIFI_SSID, WIFI_SSID_PWD);
        const uint32_t t = millis();
        while(millis() - t < 5000)
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                break;
            }
        }
    }
    logInfo("Connected to wifi");

    // MAC is only available after init of WiFi module, which is integrated in WiFi.begin
    uint8_t mac[6];
    WiFi.macAddress(mac);
    beaconBlueprint.uid = (mac[1] << 8) + mac[0];
    sampleTransmissionBlueprint.uid = beaconBlueprint.uid;
}

void mNetwork::beginListen()
{
    Udp.begin(BOARD_LISTENING_PORT);
    logTrace("Begin to listen");
}

void sendBeacon()
{
    beaconBlueprint.beaconId = (beaconBlueprint.beaconId + 1) % 0x100;
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
    responseTargetIp = Udp.remoteIP();

    logDebug("Received valig command. Applying settings...");
    mADC::setTrigger(command->channel, command->active, command->triggerVoltage);
}

void mNetwork::sendFragmentedSamples(const uint8_t* samples, uint32_t numSamples)
{
    logDebug("Transmitting samples...");
    sampleTransmissionBlueprint.transmissionGroupId = (sampleTransmissionBlueprint.transmissionGroupId + 1) % 0x100;
    const uint32_t numFullFrames = numSamples / SAMPLES_PER_PACKET;
    sampleTransmissionBlueprint.numFrames = numFullFrames;
    const uint32_t remainder = numSamples % SAMPLES_PER_PACKET;
    if(remainder > 0)
        sampleTransmissionBlueprint.numFrames++;

    for(uint32_t frameId=0;frameId<numFullFrames;frameId++)
    {
        logTrace("Begin packet");
        Udp.beginPacket(responseTargetIp, responseTargetPort);
        sampleTransmissionBlueprint.frameId = frameId;
        sampleTransmissionBlueprint.numSamples = SAMPLES_PER_PACKET;
        Udp.write((uint8_t*)&sampleTransmissionBlueprint, sizeof(SampleTransmissionHeader));
        Udp.write(&samples[frameId * BYTES_PER_PACKET], BYTES_PER_PACKET);
        Udp.endPacket();
        delay(50);
        // TODO: Experiment with this delay. Without the transmission is highly unreliable!
    }
    if(remainder > 0)
    {
        logTrace("Begin packet");
        Udp.beginPacket(responseTargetIp, responseTargetPort);
        sampleTransmissionBlueprint.frameId = numFullFrames;
        sampleTransmissionBlueprint.numSamples = remainder;
        Udp.write((uint8_t*)&sampleTransmissionBlueprint, sizeof(SampleTransmissionHeader));
        Udp.write(&samples[numFullFrames * BYTES_PER_PACKET], remainder * BYTES_PER_SAMPLE);
        Udp.endPacket();
    }
    logDebug("Samples transmitted.");
}