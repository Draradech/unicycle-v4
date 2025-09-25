#include "stdinc.h"

#define CS_PIN 12
#define MOSI_PIN 10
#define SCK_PIN 11
#define MISO_PIN 13
bool spi = true;

SPIClass SPI3(HSPI);
ICM20948_WE gIMU = ICM20948_WE(&SPI3, CS_PIN, MOSI_PIN, MISO_PIN, SCK_PIN, spi);

void setupSensors()
{
  analogSetAttenuation(ADC_11db);
  gIMU.init();
  gIMU.initMagnetometer();
  gIMU.setAccDLPF(ICM20948_DLPF_7);
  gIMU.setAccSampleRateDivider(0);
  gIMU.setGyrDLPF(ICM20948_DLPF_7);
  gIMU.setGyrSampleRateDivider(0);
  gIMU.setGyrRange(ICM20948_GYRO_RANGE_500);
  gIMU.setTempDLPF(ICM20948_DLPF_6);
  controlPara.gyroOffset.z = -0.28f;
  controlPara.gyroOffset.y = -0.54f;
  controlPara.pitchOffset = 1.35f;
  controlPara.rollOffset = -1.1f;

  sensorData.rollAngle = 45.0f;
  sensorData.pitchAngle = 45.0f;
}

void loopSensors()
{
  gIMU.readSensor();
  gIMU.getGValues(&sensorData.acc);
  gIMU.getGyrValues(&sensorData.gyro);
  gIMU.getMagValues(&sensorData.magnet);
  float pitchAcc = atan2(sensorData.acc.y, sensorData.acc.x) * 180.0f / 3.1416f + controlPara.pitchOffset;
  sensorData.pitchAngleAcc = PT1(pitchAcc, sensorData.pitchAngleAcc, 200);
  float rollAcc = atan2(sensorData.acc.z, sensorData.acc.x) * 180.0f / 3.1416f + controlPara.rollOffset;
  sensorData.rollAngleAcc = PT1(rollAcc, sensorData.rollAngleAcc, 200);
  sensorData.pitchAngle -= (sensorData.gyro.z + controlPara.gyroOffset.z) * LOOP_S;
  sensorData.rollAngle += (sensorData.gyro.y + controlPara.gyroOffset.y) * LOOP_S;
  sensorData.pitchAngle = PT1(sensorData.pitchAngleAcc, sensorData.pitchAngle, 3000);
  sensorData.rollAngle = PT1(sensorData.rollAngleAcc, sensorData.rollAngle, 3000);
}
