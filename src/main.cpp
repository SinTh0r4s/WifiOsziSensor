// libs
#include "Arduino.h"

// local includes
#include "debug.h"
#include "constants.h"
#include "network.h"
#include "adc.h"
#include "dma.h"

#ifdef ESP32
void networkHandleEventTask(void *parameters)
{
    while(true)
    {
        // Handle network events
        mNetwork::handleEvents();

        // Make room for system task for 50ms
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
#endif

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

    

#ifdef ESP32
    TaskHandle_t networkEventsTask;
    xTaskCreatePinnedToCore(
                networkHandleEventTask,
                "NetworkEvents",        // Name
                10000,                  // Stack size
                NULL,                   // Parameters
                1,                      // Priority
                &networkEventsTask,
                0);                     // Pin task to core 0
#endif
}

void loop()
{
#ifdef MKR1000
    mNetwork::handleEvents();
#endif
    mDMA::handleEvents();
    mADC::handleEvents();
}