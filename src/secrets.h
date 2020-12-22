#pragma once

// Please define your own SSID and PW here!
//#define WIFI_SSID      "your wifi SSID"
//#define WIFI_SSID_PWD  "your wifi password"

/*
FYI: Git is set up to ignore any changes to this file.
Your WiFi password will not be uploaded to the repository
*/

#if !defined(WIFI_SSID) || !defined(WIFI_SSID_PWD)
#error Please define your WiFi secrets first! (secrets.h)
#endif