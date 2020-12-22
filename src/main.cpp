// libs
#include "Arduino.h"
#include <WiFi101.h>
#include <WiFiUdp.h>

// local includes
#include "secrets.h"
#include "debug.h"
#include "constants.h"


WiFiUDP Udp;
uint8_t rxBuffer[ETHERNET_MAX_BUFFER_SIZE];
uint8_t txBuffer[ETHERNET_MAX_BUFFER_SIZE];

void setup()
{
    initLogging();
    setLogLevel(LogLevel::TRACE);

    while ( status != WL_CONNECTED)
    {
        logInfo("Attempting to connect to SSID: ");
        logInfo(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
        // wait 10 seconds for connection:
        delay(10000);
        // TODO: I don't like this delay. Maybe conside a while timeout not reached && connection not established?
    }
    logInfo("Connected to wifi");
    printWiFiStatus();
    logInfo("\nStarting connection to server...");
    Udp.begin(LISTENING_PORT);
}

void loop()
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
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(ReplyBuffer);
        Udp.endPacket();
    }
}