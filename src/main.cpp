extern "C"{
    #include "driver/include/m2m_wifi.h"
    #include "driver/include/m2m_types.h"
}

#include "Arduino.h"

//Declare receive buffer 
uint8 gmgmt[1600];

//Callback functions
void wifi_cb(uint8 u8WiFiEvent, void * pvMsg)
{
    ; 
}
void wifi_monitoring_cb(tstrM2MWifiRxPacketInfo *pstrWifiRxPacket, uint8 *pu8Payload, uint16 u16PayloadSize)
{
    if((NULL != pstrWifiRxPacket) && (0 != u16PayloadSize)) {
        if(MANAGEMENT == pstrWifiRxPacket->u8FrameType) {
            M2M_INFO("***# MGMT PACKET #***\n");
            Serial.println("Management Packet");
        } else if(DATA_BASICTYPE == pstrWifiRxPacket->u8FrameType) {
            M2M_INFO("***# DATA PACKET #***\n");
            Serial.println("Data Packet");
        } else if(CONTROL == pstrWifiRxPacket->u8FrameType) {
            M2M_INFO("***# CONTROL PACKET #***\n");
            Serial.println("Control Packet");
        }
    }
}

int main()
{
    Serial.begin(115200);
    Serial.println("Yaaaaay! :party:");
    //Register wifi_monitoring_cb 
    tstrWifiInitParam param;
    param.pfAppWifiCb = wifi_cb;
    param.pfAppMonCb  = wifi_monitoring_cb;
    
    // init Board Support Package
    nm_bsp_init();
    
    if(!m2m_wifi_init(&param)) {
        //Enable Monitor Mode with filter to receive all data frames on channel 1
        tstrM2MWifiMonitorModeCtrl	strMonitorCtrl = {0};
        strMonitorCtrl.u8ChannelID		= M2M_WIFI_CH_ALL;
        strMonitorCtrl.u8FrameType		= DATA_BASICTYPE;  // M2M_WIFI_FRAME_TYPE_ANY
        strMonitorCtrl.u8FrameSubtype	= M2M_WIFI_FRAME_SUB_TYPE_ANY; //Receive any subtype of data frame
        m2m_wifi_enable_monitoring_mode(&strMonitorCtrl, gmgmt, sizeof(gmgmt), 0);
        
        while(1) {
            m2m_wifi_handle_events(NULL);
        }

        uint8 txBuffer[100];
        uint16 headerLength = 14;
        for(uint16 i = 0;i<14;i++)
            txBuffer[i] = 0xFF;
        m2m_wifi_send_wlan_pkt( txBuffer, headerLength, 100);
    }
    return 0;
}