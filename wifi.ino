#define DEBUG_SERIAL

// Sample Buffers
const uint32_t BUFFER_SIZE = 12000;
const uint32_t NUM_BUFFERS = 2;
uint8_t b_1[BUFFER_SIZE],
         b_2[BUFFER_SIZE];
uint8_t* buffers[] = {b_1, b_2};

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
#ifdef DEBUG_SERIAL
  // check the network connection
  printCurrentNet();
#endif

  // sample until trigger is reached and buffers are full
  sampleData();
}
