#include <SPI.h> 
#include <RH_RF95.h>
#include <LowPower.h>

RH_RF95 rf95;

bool rx_ok;
int tx_time = 10000; //TX time periodicity

//Leaf sensors Setup
int leafSupply = 4; //digital port 2 supply
//declare analogRead ports for each sensor
int leaf0 = 0; //First leaf will send output to port A0 and so on
//int leaf1 = 1; //uncomment if additional sensors are added
//int leaf2 = 2;
//int leaf3 = 3;
//int leaf4 = 4;
//int leaf5 = 5;
const unsigned int sensors = 1; //number of leaf sensors
int leafs[sensors] = {leaf0}; //add the analogRead ports in the list, IF MORE SENSORS ARE ADDED, add more like {leaf0, leaf1,...}
unsigned int leafMeasure[sensors]; //set measurements list length
//unsigned int finalMeasureToPayload[sensors]; //initialize array to store measurements converted to percentage and 8bit

uint8_t payload[sensors+1]; //set payload list length (The length will be the number of sensors +1 for the device ID)
uint8_t dev_id = 132; //unique device ID

void setup() {
  Serial.begin(9600);
  pinMode(leafSupply, OUTPUT); // sets the digital port as output
  if (!rf95.init()) {
    Serial.println("init failed");
  }
  rf95.setFrequency(868.0);
  //13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
}

void loop() {
  rx_ok = false; //flag to verify the RX
  while (!rx_ok) {
    Serial.println("Sending to Lora Gateway");
    preparePayload();
    rf95.send(payload, sizeof(payload));
    rf95.waitPacketSent(); // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.waitAvailableTimeout(3000)) {
      if (rf95.recv(buf, &len)) {
        Serial.print("got reply: ");
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);
        if (buf[0] == dev_id) { //RX packet is for me
          Serial.println("Received something for me!");
          rx_ok = true;
          if (buf[1] != 0) {
            //tx_time = (buf[1]*60) / 8; //updating tx time interval
            if (buf[1] == 15) { //payload
              Serial.println("Receiveard Downlink Data!: ");
              Serial.println(buf[0]);                                       
              Serial.println(buf[1]);
            }
          }
        }
      } 
      else {
        Serial.println("RX failed");
      }
    }
    else {
      Serial.println("No reply from the Lora Gateway");
    }
    if (rx_ok == false) {
      delay(random(2, 5) * 1000); //we wait a random time before trying again the TX (CSMA non persistent)
    }
  }

 // Serial.println("Lets Sleep...");
  //delay(4 * 60 * 1000); //4mins
  delay(240000);
  /* Activate this when setting the low power idle mode
    for (tx_time; tx_time > 0; sleepCounter--){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }*/
}

void preparePayload() {
  digitalWrite(leafSupply, HIGH); // sets the digital pin 2 on
  delay(100);
  
  for (int i = 0; i < sensors; i++) {
    leafMeasure[i] = analogRead(leafs[i]);
    //finalMeasureToPayload[i] = map(leafMeasure[i], 0, 409, 0 , 100); //convert measure to percentage
    payload[i+1] = map(leafMeasure[i], 0, 409, 0 , 100); //convert measure to percentage and add it in payload list
    //debugging monitor
    Serial.print("Port: A");
    Serial.print(leafs[i]);
    Serial.print(" == ");
    Serial.print("Leaf Sensor #");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(leafMeasure[i]);
    Serial.print(" --> ");
    Serial.print(payload[i+1]);
    //Serial.print(finalMeasureToPayload[i]);
    Serial.println("%");
    
    //payload[i+1] = finalMeasureToPayload[i]; // We go +1 because in payload[0] we will store the device ID
    
 }
  //delay(100);
  //digitalWrite(leafSupply, LOW);  // sets the digital pin off
  //uint8_t wetness[i+1] = wet_percent[i];
  payload[0] = dev_id;
  //payload[1] = wetness;
  //payload[2] = 0;
}
