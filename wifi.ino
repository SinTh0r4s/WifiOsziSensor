//#define DEBUG_SERIAL

// Sample Buffers
const uint32_t BUFFER_SIZE = 512;
const uint32_t NUM_BUFFERS = 10;
uint16_t b_1[BUFFER_SIZE],
         b_2[BUFFER_SIZE],
         b_3[BUFFER_SIZE],
         b_4[BUFFER_SIZE],
         b_5[BUFFER_SIZE],
         b_6[BUFFER_SIZE],
         b_7[BUFFER_SIZE],
         b_8[BUFFER_SIZE],
         b_9[BUFFER_SIZE],
         b_10[BUFFER_SIZE];
uint16_t* buffers[] = {b_1, b_2, b_3, b_4, b_5, b_6, b_7, b_8, b_9, b_10};

const float TRIGGER_THRESHHOLD = 0.1;

void setup() {
#ifdef DEBUG_SERIAL
  //Initialize serial and wait for port to open:
  Serial.begin(250000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif

  initWifi();
  initAdc();
  initDma();

  // Status LED init and off
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
}

void loop() {
  // check the network connection once every 10 seconds:
  delay(10000);
#ifdef DEBUG_SERIAL
  printCurrentNet();
#endif

  // sample until trigger is reached and buffers are full
  sampleData();
}
