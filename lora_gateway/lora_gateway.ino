#include <SPI.h>
#include <RH_RF95.h>
#include <Time.h>
#include <TimeLib.h>
uint8_t ulink_payload[12]; //ID-TEMP-HUM-H-M
uint8_t dlink_payload[2]; //ID-TXTime (2 BYTES)

typedef struct {
  int dev_id = NULL;
  uint8_t ulink_payload[12];
  int sensorsCount;
}

uplink_data;

typedef struct {
  int dev_id = NULL;
  uint8_t dlink_payload [2];
}

dlink_data;
uplink_data updata[2];
dlink_data downdata[2];

int MAX_DEV = 2; //number of node devices
bool clock_set = false;
int finalh;
int finalm;
int dev_count = 0;
bool flag132 = false;
bool flag125 = false;
RH_RF95 rf95; // Singleton instance of the radio driver
int led = 8;

void setup() {
  pinMode(led, OUTPUT);
  Serial1.begin(9600); //Sigfox device serial communication
  Serial.begin(9600);
  if (!rf95.init()) {
    Serial.println("init failed");
  }
  rf95.setFrequency(868.0);
}

void loop() { //
  if (rf95.available()) { //checking if there's a message coming from the nodes
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      digitalWrite(led, HIGH);

      Serial.println("got request: ");
      uint8_t dev_id = buf[0];
      if (dev_id == 125) {
        flag125 = true;
        Serial.println(buf[0]); //Just for debugging
        Serial.println(buf[1]);
        Serial.println(buf[2]);
        Serial.print("RSSI for node 1: ");
        Serial.print(rf95.lastRssi(), DEC);
        Serial.println("dBm");
      }
      if (dev_id == 132) {
        flag132 = true;
        Serial.println(buf[0]); //Just for debugging
        Serial.println(buf[1]);
        Serial.println(buf[2]);
        Serial.println(buf[3]);
        Serial.println(buf[4]);
        Serial.print("RSSI for node 2: ");
        Serial.print(rf95.lastRssi(), DEC);
        Serial.println("dBm");
      }
      //print signal strength
      //PREPARING THE REPLY TO THE DEVICE
      int i = 0;
      bool found = false;
      for (i; i < MAX_DEV; i++) {
        if (downdata[i].dev_id == dev_id) {
          dlink_payload[0] = downdata[i].dlink_payload[0]; //ID
          dlink_payload[1] = downdata[i].dlink_payload[1]; //TX freq time
          i = 0;
          found = true;
          break;
        }
      }
      if (!found) {
        dlink_payload[0] = dev_id; //ID
        dlink_payload[1] = 0; //TX freq time (in this case there is no update)
      }
      rf95.send(dlink_payload, sizeof(dlink_payload));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(led, LOW);
      storeUplinkData(buf, dev_id); //store data and add timestamp
      if (flag125 or flag132) { //We have data from both devices
        sendSerialData(); //send serial data to MKRFOX1200 and recieve downlink info
        flag125 = false;
        flag132 = false;
      }
    }
    else {
      Serial.println("rx failed");
    }
  }
}

void storeUplinkData(uint8_t buf[RH_RF95_MAX_MESSAGE_LEN], uint8_t dev_id) {
  if (buf[0] == 125) {
    updata[0].dev_id = dev_id;
    updata[0].ulink_payload[0] = buf[0]; //dev ID
    updata[0].ulink_payload[1] = buf[1]; //temp
    updata[0].ulink_payload[2] = buf[2]; //humidity
    updata[0].ulink_payload[3] = 0; //Hour (timestamp)
    updata[0].ulink_payload[4] = 0; //Minutes (timestamp)
    updata[0].sensorsCount = 2; //We have one sensor but we're getting Temp and Humidity, so we consider it as 2 for the payload
    Serial.println("Storing uplink data! DEV.125!");
  }
  if (buf[0] == 132) {
    updata[1].dev_id = dev_id;
    updata[1].ulink_payload[0] = buf[0]; //dev ID
    updata[1].ulink_payload[1] = buf[1]; //leaf wetness 1
    updata[1].ulink_payload[2] = buf[2]; //leaf wetness 2
    updata[1].ulink_payload[3] = buf[3]; //leaf wetness 3
    updata[1].ulink_payload[4] = buf[4]; //leaf wetness 4
    updata[1].ulink_payload[5] = 0; //Hour (timestamp)
    updata[1].ulink_payload[6] = 0; //Minutes (timestamp)
    updata[1].sensorsCount = 4;
    Serial.println("Storing uplink data! DEV.132");
  }
}

void sendSerialData() {
  Serial.println("Serial data sending to MKRFOX!");

  for (int i = 0; i < MAX_DEV; i++) { //will run depending on how many nodes we have
    // will run depending on how long is the payload we received for each node
    // We have +3 because it's number of sensors + 1.device ID + 2.HH + 3.MM
    // This will send each byte one by one to the sigfox module (and the sigfox module is also reading the bytes one by one)
    for (int j = 0; j < updata[i].sensorsCount + 3; j++) {
      Serial1.write(updata[i].ulink_payload[j]);
    }
  }


    Serial.println("Waiting some time for Sigfox serial downlink!");

    //delay(45000); 
////getting downlink message from sigfox device. Not needed anymore as the downlink message from sigfox backend has been removed. Leaving here just for future reference.
//    if (Serial1.available()) { //RX downlink data
//      delay(100); //allows all serial sent to be received together
//      while (Serial1.available() && i < MAX_DEV) {
//        uint8_t dev_id = Serial1.read();
//        uint8_t tx_time = Serial1.read();
//        if (dev_id != 0) {
//          Serial.println("Receiving dlink serial data from MKRFOX!");
//          Serial.println(dev_id);
//          Serial.println(tx_time);
//          downdata[i].dev_id = dev_id; //ID
//          downdata[i].dlink_payload[0] = dev_id; //ID payload
//          downdata[i].dlink_payload[1] = tx_time; //TX freq time payload
//          i++;
//        }
//      }
//    }
  }
