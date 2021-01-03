// libs
#include "Arduino.h"

// local includes
#include "debug.h"
#include "constants.h"
#include "network.h"
#include "adc.h"
#include "dma.h"


void setup()
{
    startInit();
    setLogLevel(LogLevel::TRACE);

    mADC::init();
    mDMA::init();
    mNetwork::init();
    
    mDMA::start();
    mNetwork::connectWifi();
    mNetwork::beginListen();

    endInit();
}

void loop()
{
    mNetwork::handleEvents();
    mDMA::handleEvents();
    mADC::handleEvents();
}