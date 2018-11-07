//#define Serial SerialUSB
#include <RFM69.h>
#include "tag.h"

// Addresses for this node. CHANGE THESE FOR EACH NODE!
#define NETWORKID     155   // Must be the same for all nodes (0 to 255)
#define MYNODEID      0   // My node ID (0 to 255)

#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):
#define ENCRYPT       false // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

// Use ACKnowledge when sending messages (or not):
#define USEACK        false // Request ACKs or not

// Create a library object for our RFM69HCW module:
// RFM69(uint8_t slaveSelectPin=RF69_SPI_CS, uint8_t interruptPin=RF69_IRQ_PIN, bool isRFM69HW=false, uint8_t interruptNum=RF69_IRQ_NUM)
RFM69 radio(10, 3, false, digitalPinToInterrupt(3));

void Init()
{    
  // Initialize the RFM69HCW:
  radio.setCS(10);
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);
}

void SendCoordinates(TagReading reading, uint8_t target = 0)
{
  if(!target) target = reading.id; //target defaults to the tag id
  
  uint8_t sendBuffer[62]; //62 is max length with AES
  memcpy(sendBuffer, &reading, 6);

//  Serial.print(target);
//  Serial.print('\t');
//  Serial.print(reading.id);
//  Serial.print('\t');
//  Serial.print(reading.x_loc);
//  Serial.print('\t');
//  Serial.print(reading.y_loc);
//  Serial.print('\n');

  if (USEACK)
  {
    if (radio.sendWithRetry(target, sendBuffer, sizeof(reading)))
      Serial.println("ACK received!");
    else
      Serial.println("no ACK received :(");
  }
  
  else // don't use ACK
  {
    radio.send(target, sendBuffer, sizeof(reading));
  }
}
      

