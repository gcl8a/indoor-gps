#ifndef __TAG_H
#define __TAG_H

const uint32_t SEND_INTERVAL = 400;

struct TagReading
{
  uint16_t id;
  uint16_t x_loc;
  uint16_t y_loc;
//  uint16_t location[2];
};

uint32_t lastSendTime[12];

#endif
