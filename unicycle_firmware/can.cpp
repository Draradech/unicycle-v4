#include "stdinc.h"
#include <ESP32-TWAI-CAN.hpp>

#define CAN_TX 16
#define CAN_RX 18

CanFrame rxFrame;

bool sendTorque(uint8_t axis, float tq)
{
    CanFrame tqFrame = { 0 };
    tqFrame.identifier = ((axis + 1) << 5) | 0x0e;
    tqFrame.data_length_code = 4;
    tqFrame.data[0] = ((uint8_t *)&tq)[0];
    tqFrame.data[1] = ((uint8_t *)&tq)[1];
    tqFrame.data[2] = ((uint8_t *)&tq)[2];
    tqFrame.data[3] = ((uint8_t *)&tq)[3];
    return ESP32Can.writeFrame(tqFrame, 0);
}

void setupCan()
{
  ESP32Can.begin(ESP32Can.convertSpeed(1000), CAN_TX, CAN_RX, 6, 6);
  sensorData.voltage = 13.2f;
}

void loopCan()
{
  while(ESP32Can.readFrame(rxFrame, 0))
  {
    uint8_t cmd = rxFrame.identifier & 0x1f;
    uint8_t axis = (rxFrame.identifier >> 5) - 1;

    if (cmd == 0x09)
    {
      float pos = (*(float*)(&rxFrame.data[0]));
      float vel = (*(float*)(&rxFrame.data[4]));
      sensorData.wheelPos[axis] = pos;
      sensorData.wheelVel[axis] = vel;
    }

    if (cmd == 0x17)
    {
      float volt = (*(float*)(&rxFrame.data[0]));
      float curr = (*(float*)(&rxFrame.data[4]));
      sensorData.voltage = volt + (sensorData.voltage - volt) * 0.95f;
      sensorData.iBus[axis] = curr;
    }
  }
}
