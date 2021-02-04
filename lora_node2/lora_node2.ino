#include <SPI.h> 
#include <RH_RF95.h>
#include <LowPower.h>

RH_RF95 rf95;

uint8_t dev_id = 138; //unique device ID
uint8_t payload[2];
bool rx_ok;
int tx_time = 10000; //TX time periodicity

void setup() {
  Serial.begin(9600);
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

  Serial.println("Lets Sleep...");
  //delay(4 * 60 * 1000); //4mins
  delay(240000);
  /* Activate this when setting the low power idle mode
    for (tx_time; tx_time > 0; sleepCounter--){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }*/
}

void preparePayload() {
  float wet = analogRead(A2); //Get analog measurement from leaf sensor (0-2V --> 0-409.2)
  float wet_percent = map(val, 0, 409.2, 0, 100); //Convert to Percentage
  uint8_t wetness = wet_percent;
  payload[0] = dev_id;
  payload[1] = wetness;
}
