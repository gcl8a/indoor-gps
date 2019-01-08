//#define Serial SerialUSB

#include "tag.h"

#include <RFM69.h>
// Addresses for this node. CHANGE THESE FOR EACH NODE!
#define NETWORKID     155   // Must be the same for all nodes (0 to 255)
#define MYNODEID      10   // My node ID (0 to 255)

// RFM69 frequency, uncomment the frequency of your module:
//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):
#define ENCRYPT       false // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

// Use ACKnowledge when sending messages (or not):
#define USEACK        false // Request ACKs or not

// Create a library object for our RFM69HCW module:
//RFM69 radio(10, 9, false, digitalPinToInterrupt(9));
RFM69 radio(8, 3, false, digitalPinToInterrupt(3));

void setup()
{
  // Open a serial port so we can send keystrokes to the module:
  Serial.begin(115200);
  while(!Serial) {}

  Serial.print("Setting up node ");
  Serial.println(MYNODEID, DEC);
    
  // Initialize the RFM69HCW:
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); // Always use this for RFM69HCW
  radio.promiscuous(true);

  // Turn on encryption if desired:
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);

  Serial.println("done");
}

String inputString; //for receiving Serial input

void loop()
{
  // SENDING
  while(Serial.available())
  {
    char ch = Serial.read();
    
    if (ch != '\r' && ch != '\n') // not a carriage return or newline
    {
      inputString += ch;
    }

    // If the input is a carriage return, or the buffer is full:
    if ((ch == '\r') || (inputString.length() == 60)) // CR or max length
    {
      uint8_t sendBuffer[62]; //62 is max length with AES
      uint8_t sendLength = 0;

      int8_t colon = inputString.indexOf(':');
      if(colon == -1) // forgot the colon: reset the string and abort
      {
        Serial.println("Invalid input string.");
        inputString = ""; 
        break;
      }

      uint8_t target = inputString.substring(0).toInt();      
      String messageString = inputString.substring(colon + 1);

      if(target == 9) //camera node
      {  
        uint8_t firstDigit = messageString.substring(0).toInt();
        uint8_t secondDigit = messageString.substring(messageString.indexOf(',') + 1).toInt();

        sendLength = 2;
        sendBuffer[0] = firstDigit;
        sendBuffer[1] = secondDigit;
      }

      else
      {
        sendLength = messageString.length();
        for(int i = 0; i < sendLength; i++)
        {
          sendBuffer[i] = messageString[i];
        }
      }
        
      static bool useACK = USEACK;
      
      // Send the packet
      Serial.print("sending to node ");
      Serial.print(target, DEC);
      Serial.print(": [");
      for(int i = 0; i < sendLength; i++)
      {
        if(i) Serial.print(' ');
        Serial.print((char)sendBuffer[i]);
      }
      
      Serial.print("]\n");
        
      if (useACK)
      {
        if (radio.sendWithRetry(target, sendBuffer, sendLength))
          Serial.println("ACK received!");
        else
          Serial.println("no ACK received :(");
      }

      else // don't use ACK
      {
        radio.send(target, sendBuffer, sendLength);
      }
      
      inputString = ""; // reset the packet
    }
  }

  // RECEIVING
  if (radio.receiveDone()) // Got one!
  {
    // Print out the information:
    Serial.print(millis());
    Serial.print(':');
    Serial.print(radio.SENDERID, DEC);
    Serial.print(",");
    Serial.print(radio.DATALEN);
    Serial.print(":\t");

    // The actual message is contained in the DATA array,
    // and is DATALEN bytes in size:
    uint8_t recBuffer[62]; //max length is 62
    uint8_t recLength = radio.DATALEN;

    for (byte i = 0; i < radio.DATALEN; i++)
    {
      recBuffer[i] = radio.DATA[i];
      if(radio.SENDERID == 0)
      {   
        Serial.print((char)recBuffer[i]);
      }
    }

    if(radio.SENDERID == 9)
    {
      Tag tag;
      memcpy(&tag, recBuffer, 6);
      
      Serial.print(tag.id);
      Serial.print('\t');
      Serial.print(tag.x_loc, DEC);
      Serial.print('\t');
      Serial.print(tag.y_loc, DEC);
      Serial.print('\n');
    }

    else if(radio.SENDERID == 0 && radio.DATALEN == 12)
    {
      float pose[3];
      memcpy(pose, recBuffer, 12);

      Serial.print(pose[0]);
      Serial.print('\t');
      Serial.print(pose[1]);
      Serial.print('\t');
      Serial.print(pose[2]);
      Serial.print('\n');
    }

    // RSSI is the "Receive Signal Strength Indicator",
    // smaller absolute values mean higher power.
//    Serial.print("], RSSI ");
//    Serial.println(radio.RSSI);

    // Send an ACK if requested.
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println("ACK sent");
    }

  }
}

