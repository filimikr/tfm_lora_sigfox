#include <SPI.h> 
#include <RH_RF95.h>
#include <LowPower.h>

RH_RF95 rf95;

bool rx_ok;
int tx_time = 10000; //TX time periodicity

//Leaf sensors Setup
int leafSupply = 4; //digital port 4 supply
//set analogRead ports for each sensor
int leaf0 = 0; //First leaf will send output to port A0 and so on
int leaf1 = 1; 
int leaf2 = 2;
int leaf3 = 3;
//int leaf4 = 4; //uncomment if additional sensors are added
//int leaf5 = 5; //uncomment if additional sensors are added
const int sensors = 4; //number of leaf sensors
int leafs[sensors] = {leaf0, leaf1, leaf2, leaf3}; //add the analogRead ports in the list, IF MORE SENSORS ARE ADDED, add more like {leaf4, leaf5, leaf6...}
int leafMeasure[sensors]; //set measurements list length depending on the number of sensors we set before

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
              Serial.println("Received Downlink Data!: ");
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

  Serial.println("Lets Sleep...");
  delay(600000); //10 mins - adding delay instead of low power mode, for testing
  /* Activate this when setting the low power idle mode
    for (tx_time; tx_time > 0; sleepCounter--){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }*/
}

//preparing the payload to be sent to LoRa gateway
void preparePayload() {
  payload[0] = dev_id;
  digitalWrite(leafSupply, HIGH); // sets the digital pin 4 on
  delay(100);
  Serial.println("=====PREPARING PAYLOAD=====");
  Serial.print("Device ID: ");
  Serial.println(payload[0]);
  for (int i = 0; i < sensors; i++) {
    leafMeasure[i] = analogRead(leafs[i]);
    //convert measure to percentage and add it in payload list. 
    //420 is considered as the highest digital value we can get when the leaf is fully wet (0-2V --> 0-420 digital values). Can be between 413-420.
    payload[i+1] = map(leafMeasure[i], 0, 420, 0 , 100); 
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
    Serial.println("%");
 }
  digitalWrite(leafSupply, LOW);  // sets the digital pin off
  Serial.println("=======END OF PAYLOAD=======");
}
