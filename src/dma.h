#include "Arduino.h"


namespace mDMA
{
    void init();
    void start();
    void suspend();
    void triggerHit();
    void resume();
    void handleEvents();
    uint8_t* getLastCompletedBuffer();
};