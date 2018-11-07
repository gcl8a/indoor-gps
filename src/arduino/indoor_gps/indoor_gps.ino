/*
 * For radioing AprilTag coordinates as GPS coordinates
 */
//#define Serial SerialUSB

#include "radio.h"
#include "tag.h"

#include <event_timer.h>
Timer pingTimer;

void setup() 
{
  Serial.begin(115200);
  //while(!Serial) {}
  Serial.println("setup()");

  Serial1.begin(115200);

  Init();
  
  pingTimer.Start(2000); //for testing, send out a faux tag every 2 seconds

  Serial.println("setup() complete");
}

//packets consist of: FF 00 idL idH xL xH yL yH AA
//note that id can never be 00

uint8_t mvArray[9]; //array for receiving data from the OpenMV cam
uint8_t mvIndex = 0; //for counting bytes

void loop() 
{
  if(Serial1.available())
  {
    uint8_t b = Serial1.read();
    Serial.println(b, HEX);
    if(HandleUART(b))
    {
      TagReading reading;
      memcpy(&reading, &mvArray[2], 6);
      uint32_t timeStamp = millis();
      
      if(timeStamp - lastSendTime[reading.id] > SEND_INTERVAL)
      {  
        lastSendTime[reading.id] = timeStamp;
        SendCoordinates(reading);
      }
    }
  }
  
  if(pingTimer.CheckExpired())
  {
    TagReading reading;
    reading.id = 99;
    reading.x_loc = 99;
    reading.y_loc = 99;
    SendCoordinates(reading, 10); //send to node 10, which won't be a tag
    pingTimer.Restart();  
  }
}

bool HandleUART(uint8_t b)
{
  bool retVal = false;
  switch(mvIndex)
  {
    case 0:
      if(b == 0xff) mvIndex++; //first byte must be 0xff
      break;
    case 1:
      if(b == 0x00) mvIndex++;
      else mvIndex = 0; //didn't get the 00 byte, so restart
      break;
    case 8:
      if(b == 0xaa) //correct end byte, so process
      {
        retVal = true;
        mvIndex = 0;
      } 
      else mvIndex = 0; //didn't get the aa byte, so restart
      break;
    case 9:
      Serial.println("Something is very wrong!");
      //while(1) {}
      break;
    default:
      mvArray[mvIndex++] = b;
  }

  return retVal;
}


// if (radio.receiveDone()) // Got one!
//  {
//    // Print out the information:
//    Serial.print("Received ");
//    Serial.print(radio.DATALEN);
//    Serial.print(" bytes from node ");
//    Serial.print(radio.SENDERID, DEC);
//    Serial.print(": [");
//
//    // The actual message is contained in the DATA array,
//    // and is DATALEN bytes in size:
//    uint8_t recBuffer[62]; //max length is 62
//    uint8_t recLength = radio.DATALEN;
//
//    for (byte i = 0; i < radio.DATALEN; i++)
//    {
//      Serial.print(',');
//      Serial.print(radio.DATA[i], DEC);
//
//      recBuffer[i] = radio.DATA[i];
//    }
//
//    // RSSI is the "Receive Signal Strength Indicator",
//    // smaller absolute values mean higher power.
//    Serial.print("], RSSI ");
//    Serial.println(radio.RSSI);
//
//    // Send an ACK if requested.
//    if (radio.ACKRequested())
//    {
//      radio.sendACK();
//      Serial.println("ACK sent");
//    }
//  }

