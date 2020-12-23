//libs
#include <WiFi101.h>
#include <WiFiUdp.h>

// local includes
#include "network.h"
#include "debug.h"
#include "constants.h"
#include "secrets.h"


// Send a network message to advertise this devise and its properties
void sendBeacon();

setTriggerCallback _setTriggerCallback;
WiFiUDP Udp;
uint8_t rxBuffer[ETHERNET_MAX_BUFFER_SIZE];

void Network_registerSetTriggerCallback(setTriggerCallback _cb)
{
    _setTriggerCallback = _cb;
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
        //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        //Udp.write(txBuffer);
        //Udp.endPacket();
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
    Udp.begin(LISTENING_PORT);
    logTrace("Begin to listen");
}