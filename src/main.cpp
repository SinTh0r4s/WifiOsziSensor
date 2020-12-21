#define CONF_WINC_DEBUG 1
#define SERIAL_DEBUG

extern "C"{
    #include "driver/include/m2m_wifi.h"
    #include "driver/include/m2m_types.h"
}

#include "Arduino.h"


inline void LOG(const char* s)
{
#ifdef SERIAL_DEBUG
    Serial.println(s);
#endif
}

//Declare receive buffer 
uint8 gmgmt[1600];

//Callback functions
void wifi_cb(uint8 u8WiFiEvent, void * pvMsg)
{
    LOG("Why is the normal callback used?");
}
void wifi_monitoring_cb(tstrM2MWifiRxPacketInfo *pstrWifiRxPacket, uint8 *pu8Payload, uint16 u16PayloadSize)
{
    if((NULL != pstrWifiRxPacket) && (0 != u16PayloadSize)) {
        if(MANAGEMENT == pstrWifiRxPacket->u8FrameType) {
            LOG("Management Packet");
        } else if(DATA_BASICTYPE == pstrWifiRxPacket->u8FrameType) {
            LOG("Data Packet");
        } else if(CONTROL == pstrWifiRxPacket->u8FrameType) {
            LOG("Control Packet");
        }
    }
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
#ifdef SERIAL_DEBUG
    Serial.begin(115200);
    while(!Serial);
#endif
    LOG("Wifi custom driver test");
    M2M_INFO("Test log level");
    //Register wifi_monitoring_cb 
    tstrWifiInitParam param;
    param.pfAppWifiCb = wifi_cb;
    param.pfAppMonCb  = wifi_monitoring_cb;
    
    // init Board Support Package
    LOG("BSP init");
    nm_bsp_init();
    
    LOG("WIFI init");

    sint8 ret = m2m_wifi_init(&param);
    if(ret != M2M_SUCCESS) {
        //Enable Monitor Mode with filter to receive all data frames on channel 1
        tstrM2MWifiMonitorModeCtrl	strMonitorCtrl = {0};
        strMonitorCtrl.u8ChannelID		= M2M_WIFI_CH_1;//M2M_WIFI_CH_ALL;
        strMonitorCtrl.u8FrameType		= DATA_BASICTYPE;  // M2M_WIFI_FRAME_TYPE_ANY
        strMonitorCtrl.u8FrameSubtype	= M2M_WIFI_FRAME_SUB_TYPE_ANY; //Receive any subtype of data frame
        LOG("switch to monitoring mode");
        if(m2m_wifi_enable_monitoring_mode(&strMonitorCtrl, gmgmt, sizeof(gmgmt), 0) != M2M_SUCCESS)
        {
            LOG("Error initializing monitor mode!");
        }
    }
    else
    {
        char str[30];
        sprintf(str, "Error wifi init: %d", ret);
        LOG(str);
    }
    
}

void loop()
{
    /*
    uint8 txBuffer[100];
    uint16 headerLength = 14;
    for(uint16 i = 0;i<14;i++)
        txBuffer[i] = 0xFF;
    LOG("Send marker packet");
    m2m_wifi_send_wlan_pkt( txBuffer, headerLength, 100);
    */
    
    while(1) {
        if(m2m_wifi_handle_events(NULL) != M2M_SUCCESS)
        {
            LOG("Error handling events!");
        }
    }  
}