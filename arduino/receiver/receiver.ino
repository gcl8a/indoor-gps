#define Serial SerialUSB
#include <RFM69.h>
// Addresses for this node. CHANGE THESE FOR EACH NODE!
#define NETWORKID     155   // Must be the same for all nodes (0 to 255)
#define MYNODEID      1   // My node ID (0 to 255)

// RFM69 frequency, uncomment the frequency of your module:
//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):
#define ENCRYPT       false // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

// Use ACKnowledge when sending messages (or not):
#define USEACK        true // Request ACKs or not

// Create a library object for our RFM69HCW module:
// RFM69(uint8_t slaveSelectPin=RF69_SPI_CS, uint8_t interruptPin=RF69_IRQ_PIN, bool isRFM69HW=false, uint8_t interruptNum=RF69_IRQ_NUM)
RFM69 radio(10, 3, false, digitalPinToInterrupt(3));

void setup()
{
  // Open a serial port so we can send keystrokes to the module:
  Serial.begin(115200);
  while(!Serial) {}

  Serial.print("Setting up node ");
  Serial.println(MYNODEID, DEC);
    
  // Initialize the RFM69HCW:
  radio.setCS(10);
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);

  Serial.println("done");
}

String inputString; //for receiving Serial input

void loop()
{
  // SENDING
  if (Serial.available())
  {
    char ch = Serial.read();
    
    if (ch != '\r' && ch != '\n') // not a carriage return or newline
    {
      inputString += ch;
    }

    // If the input is a carriage return, or the buffer is full:
    if ((ch == '\r') || (inputString.length() == 60)) // CR or max length
    {
      static bool useACK = true;
      static uint8_t target = 0;
      
      {
        // Send the packet
        Serial.print("sending to node ");
        Serial.print(target, DEC);
        Serial.print(": [");
        Serial.print(inputString);
        Serial.println("]");

        uint8_t sendBuffer[62]; //62 is max length with AES
        uint8_t sendLength = 0;
        {
          for(uint8_t i = 0; i < inputString.length(); i++)
          {
            uint8_t b = (uint8_t)inputString[i];
            sendBuffer[i] = b;
            sendLength++;
          }          
        }

        for(int i = 0; i < sendLength; i++)
        {
          Serial.print(',');
          Serial.print(sendBuffer[i], DEC);
        }
        Serial.print('\n');
        
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
      }
      
      inputString = ""; // reset the packet
    }
  }

  // RECEIVING
  if (radio.receiveDone()) // Got one!
  {
    // Print out the information:
    Serial.print("Received ");
    Serial.print(radio.DATALEN);
    Serial.print(" bytes from node ");
    Serial.print(radio.SENDERID, DEC);
    Serial.print(": [");

    // The actual message is contained in the DATA array,
    // and is DATALEN bytes in size:
    uint8_t recBuffer[62]; //max length is 62
    uint8_t recLength = radio.DATALEN;

    for (byte i = 0; i < radio.DATALEN; i++)
    {
      Serial.print(',');
      Serial.print(radio.DATA[i], DEC);

      recBuffer[i] = radio.DATA[i];
    }

    // RSSI is the "Receive Signal Strength Indicator",
    // smaller absolute values mean higher power.
    Serial.print("], RSSI ");
    Serial.println(radio.RSSI);

    // Send an ACK if requested.
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println("ACK sent");
    }

  }
}

