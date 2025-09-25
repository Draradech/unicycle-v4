#include "stdinc.h"

void setupControl()
{
  controlPara.speedPitchFilter = 100.0f;
  controlPara.pPartSpeedPitch = 1.0f;
  controlPara.iPartSpeedPitch = 1.0f;
  controlPara.pPartPitch = 10.0f;
  controlPara.dPartPitch = -0.1f;

  controlPara.speedRollFilter = 200.0f;
  controlPara.pPartSpeedRoll = 5.0f;
  controlPara.iPartSpeedRoll = 0.5f;
  controlPara.pPartRoll = 10.0f;
  controlPara.dPartRoll = 0.15f;

  controlPara.dPartYaw = 0.15f;

  controlState.activePitch = false;
  controlState.activeRoll = false;

  controlPara.gyroFilter.x = 50.0f;
  controlPara.gyroFilter.y = 50.0f;
  controlPara.gyroFilter.z = 30.0f;
}

void loopControl()
{
  sensorData.gyroFilt.x = PT1(sensorData.gyro.x, sensorData.gyroFilt.x, controlPara.gyroFilter.x);
  sensorData.gyroFilt.y = PT1(sensorData.gyro.y, sensorData.gyroFilt.y, controlPara.gyroFilter.y);
  sensorData.gyroFilt.z = PT1(sensorData.gyro.z, sensorData.gyroFilt.z, controlPara.gyroFilter.z);

  float targetSpeedPitch = 0;
  if(joystickTimeout) targetSpeedPitch += (joystickReport.y  / 127.0f - 1.0f) * -5.0f;
  targetSpeedPitch += (input.a  / 127.0f) * 5.0f;
  if(targetSpeedPitch > controlState.targetSpeedPitch) controlState.targetSpeedPitch += 0.025f;
  if(targetSpeedPitch < controlState.targetSpeedPitch) controlState.targetSpeedPitch -= 0.025f;
  float speedDev = controlState.targetSpeedPitch - sensorData.wheelVel[0];
  controlState.targetPitchI -= speedDev * controlPara.iPartSpeedPitch / 500.0f;
  controlState.speedPitchFilt = PT1(sensorData.wheelVel[0], controlState.speedPitchFilt, controlPara.speedPitchFilter);
  speedDev = controlState.targetSpeedPitch - controlState.speedPitchFilt;
  controlState.targetPitch = controlState.targetPitchI - speedDev * controlPara.pPartSpeedPitch;
  float pitchDev = controlPara.pitchWP + controlState.targetPitch - sensorData.pitchAngle;
  actuator.torque[0] = pitchDev * controlPara.pPartPitch - sensorData.gyroFilt.z * controlPara.dPartPitch;
  if(input.c) actuator.torque[0] = pitchDev * 1.0f - sensorData.gyroFilt.z * controlPara.dPartPitch;
  if(fabs(sensorData.rollAngle) < 10.0f && fabs(actuator.torque[0]) < 2.0f) controlState.activePitch = true;

  controlState.targetSpeedRoll = 0;
  speedDev = controlState.targetSpeedRoll - sensorData.wheelVel[1] + sensorData.wheelVel[2];
  controlState.targetRollI -= speedDev * controlPara.iPartSpeedRoll / 50000.0f;
  controlState.speedRollFilt = PT1(sensorData.wheelVel[1] - sensorData.wheelVel[2], controlState.speedRollFilt, controlPara.speedRollFilter);
  speedDev = controlState.targetSpeedRoll - controlState.speedRollFilt;
  controlState.targetRoll = controlState.targetRollI - speedDev * controlPara.pPartSpeedRoll * 0.01f;
  float rollDev = controlPara.rollWP + controlState.targetRoll - sensorData.rollAngle;
  float targetTorqueRoll = rollDev * controlPara.pPartRoll - sensorData.gyroFilt.y * controlPara.dPartRoll;
  actuator.torque[1] = targetTorqueRoll;
  actuator.torque[2] = -targetTorqueRoll;
  if(controlState.activePitch && fabs(targetTorqueRoll) < 1.0f) controlState.activeRoll = true;

  float targetYawTorque = (sensorData.gyro.x - input.b) * controlPara.dPartYaw;
  actuator.torque[1] += targetYawTorque;
  actuator.torque[2] += targetYawTorque;

  if (fabs(sensorData.pitchAngle) > 20.0f || fabs(sensorData.rollAngle) > 20.0f)
  {
    controlState.activePitch = false;
    controlState.activeRoll = false;
  }

  if (!controlState.activePitch) controlState.targetPitchI = 0.0f;
  if (!controlState.activeRoll) controlState.targetRollI = 0.0f;

  for (int i = 0; i < 3; i++) actuator.torque[i] = LIMIT(actuator.torque[i], -40.0f, 40.0f);

  if (!actuator.disabled && (controlState.activePitch || input.c))
  {
    sendTorque(0, actuator.torque[0]);
  } else {
    sendTorque(0, 0);
  }

  if (!actuator.disabled && (controlState.activeRoll || input.c))
  {
    sendTorque(1, actuator.torque[1]);
    sendTorque(2, actuator.torque[2]);
  } else {
    sendTorque(1, 0);
    sendTorque(2, 0);
  }

}
