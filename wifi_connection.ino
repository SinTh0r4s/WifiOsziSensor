#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

//char ssid[] = "SchadeDeutschland";     //  your network SSID (name)
//char pass[] = "allesistvorbei";  // your network password
char ssid[] = "Sinthospot";     //  your network SSID (name)
char pass[] = "5c56a75bb0234";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

IPAddress plotterIp(192,168,43,134);
uint32_t plotterPort = 7567;
uint32_t localPort = 12345;

WiFiUDP Udp;

void initWifi() {
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_SERIAL
    Serial.println("WiFi shield not present");
#endif
    while (true);
  }

#ifdef DEBUG_SERIAL
  /*String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
    Serial.print("Currently installed: ");
    Serial.println(fv);
  }*/
#endif

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
#ifdef DEBUG_SERIAL
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
#endif
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

#ifdef DEBUG_SERIAL
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
#endif

  uint32_t status = Udp.begin(localPort);
#ifdef DEBUG_SERIAL
  Serial.print("UDP init: ");
  Serial.println(status ? "success" : "fail");
#endif
}

void sendUdp(const uint8_t* buf, size_t len){
  uint32_t status = Udp.beginPacket(plotterIp, plotterPort);
#ifdef DEBUG_SERIAL
  Serial.print("UDP packet init : ");
  Serial.println(status ? "success" : "fail");
#endif
  Udp.write((byte*)buf, len);
  status = Udp.endPacket();
#ifdef DEBUG_SERIAL
  Serial.print("UDP transmit : ");
  Serial.println(status ? "success" : "fail");
#endif
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
  Serial.println("-----------------------------");
  Serial.println();
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println("----");
  Serial.println();
}
