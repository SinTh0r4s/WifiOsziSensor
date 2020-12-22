#define SERIAL_DEBUG 1
#define CONF_MGMT 1

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

struct ethheader {
    unsigned char       dsta[6];
    unsigned char       srca[6];
    uint16_t            type;
};

static const uint8_t IMAGINARY_MAC_ADDR[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0x75, 0x67};
static const uint8_t LAPTOP_MAC_ADDR[6] = {0x84, 0x3A, 0x4B, 0x7B, 0xD2, 0x30};

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
            M2M_INFO("***# MGMT PACKET #***\n");
        } else if(DATA_BASICTYPE == pstrWifiRxPacket->u8FrameType) {
            M2M_INFO("***# DATA PACKET #***\n");
        } else if(CONTROL == pstrWifiRxPacket->u8FrameType) {
            M2M_INFO("***# CONTROL PACKET #***\n");
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
    if(ret == M2M_SUCCESS) {
        //Enable Monitor Mode with filter to receive all data frames on channel 1
        tstrM2MWifiMonitorModeCtrl	strMonitorCtrl = {0};
        strMonitorCtrl.u8ChannelID		= M2M_WIFI_CH_10;//M2M_WIFI_CH_ALL;    12, 11
        strMonitorCtrl.u8FrameType		= M2M_WIFI_FRAME_TYPE_ANY;//DATA_BASICTYPE;
        strMonitorCtrl.u8FrameSubtype	= M2M_WIFI_FRAME_SUB_TYPE_ANY;
        //memcpy(strMonitorCtrl.au8DstMacAddress, LAPTOP_MAC_ADDR, sizeof(LAPTOP_MAC_ADDR));
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
    uint8 txBuffer[100];
    uint16 headerLength = 14;
    ethheader* eth = (ethheader*)txBuffer;
    memcpy(eth->dsta, LAPTOP_MAC_ADDR, sizeof(LAPTOP_MAC_ADDR));
    memcpy(eth->srca, IMAGINARY_MAC_ADDR, sizeof(IMAGINARY_MAC_ADDR));
    eth->type = 0x8858;
    
    
    while(1) {
        if(m2m_wifi_handle_events(NULL) != M2M_SUCCESS)
        {
            LOG("Error handling events!");
        }
        static int i = 0;
        i = (i+1) % 1000000;
        if(i == 0)
        {
            LOG("Send marker packet");
            m2m_wifi_send_wlan_pkt( txBuffer, headerLength, 100);
        }
    }  
}