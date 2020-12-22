#pragma once

// Please define your own SSID and PW here!
//#define SSID      "your wifi SSID"
//#define SSID_PWD  "your wifi password"

/*
FYI: Git is set up to ignore any changes to this file.
Your WiFi password will not be uploaded to the repository
*/

#if !defined(SSID) || !defined(SSID_PWD)
#error Please define your WiFi secrets first! (secrets.h)
#endif