#include <SigFox.h>
#include <SPI.h>

typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t id1;
  uint8_t temp1;
  uint8_t hum1;
  uint8_t hour1;
  uint8_t min1;
  
  uint8_t id2;
  uint8_t wet1;
  uint8_t wet2;
  uint8_t wet3;
  uint8_t wet4;
  uint8_t hour2;
  uint8_t min2;
}

SigfoxMessage; //Sigfox packet 12 BYTES
SigfoxMessage msg;

typedef struct {
  uint8_t dlink_payload [2];
} 

dlink_data;
dlink_data downdata[2];

uint8_t ulink_payload[12]; //ID-TEMP-HUM-H-M
uint8_t dlink_payload[2]; //ID-TXTime (2 BYTES)
//int MAX_DEV = 1;
int numtx = 0;

void setup() {
  Serial.begin(9600); //Serial console
  Serial1.begin(9600); //Serial to LoRa GW
}

void loop() {
  //Serial.println("Waiting for serial connection with Lora Gateway");
  //delay(3000);
  if (Serial1.available()) { //uplink data from LoRa GW
    Serial.println("Receiving serial data from LoRa Gateway!");
    delay(100);
    msg.id1 = Serial1.read();
    msg.temp1 = Serial1.read();
    msg.hum1 = Serial1.read();
    msg.hour1 = Serial1.read();
    msg.min1 = Serial1.read();
    msg.id2 = Serial1.read();
    msg.wet1 = Serial1.read();
    msg.wet2 = Serial1.read();
    msg.wet3 = Serial1.read();
    msg.wet4 = Serial1.read();
    msg.hour2 = Serial1.read();
    msg.min2 = Serial1.read();
    Serial.println(msg.id1);
    Serial.println(msg.temp1);
    Serial.println(msg.hum1);
    Serial.println(msg.hour1);
    Serial.println(msg.min1);
    Serial.println(msg.id2);
    Serial.println(msg.wet1);
    Serial.println(msg.wet2);
    Serial.println(msg.wet3);
    Serial.println(msg.wet4);
    Serial.println(msg.hour2);
    Serial.println(msg.min2);
    delay(100);
    if (numtx == 35 || numtx == 70 || numtx == 105 || numtx == 138) {
      Downlink(); //We ask for Downlink data just 4 times per day
    }
    else {
      send_data();
    }
  }
}

void send_data() {
  SigFox.begin(); //Initialize Sigfox module
  delay(100);
  SigFox.debug();// Enable debug led and disable automatic deep sleep
  SigFox.status();// Delete pendent interruptions
  SigFox.beginPacket();// We begin writing the msg
  SigFox.write((uint8_t*)&msg, sizeof(msg));
  delay(5);
  //int ret = SigFox.endPacket(); //Buffer sending
  SigFox.endPacket(); //Buffer sending
  SigFox.end();

  Serial.println("Data sent to Sigfox! (just uplink)");
  numtx++; //Increment number of TXs
  Serial.println("Messages sent before reseting:");
  Serial.println(numtx);

  if (numtx == 140) {
    numtx = 0; //reset
  }
}

void Downlink() {
  //MANAGING SIGFOX DOWNLINK
  int dkmessage [2];
  int count8 = 0;

  if (!SigFox.begin()) {
    return;
  }
  SigFox.debug();
  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 12);
  SigFox.endPacket(true); //We ask for DL data
  Serial.println("Data sent to Sigfox!");

  if (SigFox.parsePacket()) {
    while (SigFox.available()) {
      uint8_t dev_id = SigFox.read();
      uint8_t payload = SigFox.read();
      Serial1.write(dev_id); //We send it through Serial to LoRa GW
      Serial1.write(payload);
    }
    Serial.println("Data sent to LoRa Gateway through Serial!");
  }
  numtx++; //Increment counter TXs

  if (numtx == 140) {
    numtx = 0; //reset
  }
}
