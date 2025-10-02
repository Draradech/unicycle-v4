#include "stdinc.h"

float ramp(float from, float to, float gradient)
{
  if (to > from)
  {
    return from + gradient > to ? to : from + gradient;
  }
  return from - gradient < to ? to : from - gradient;
}

void setupControl()
{
  controlPara.speedPitchFilter = 100.0f;
  controlPara.pPartSpeedPitch = 1.0f;
  controlPara.iPartSpeedPitch = 1.0f;
  controlPara.pPartPitch = 7.0f;
  controlPara.dPartPitch = -0.15f;

  controlPara.speedRollFilter = 200.0f;
  controlPara.pPartSpeedRoll = 5.0f;
  controlPara.iPartSpeedRoll = 0.5f;
  controlPara.pPartRoll = 10.0f;
  controlPara.dPartRoll = 0.6f;

  controlPara.dPartYaw = 0.15f;

  controlState.activePitch = false;
  controlState.activeRoll = false;

  controlPara.gyroFilter.x = 50.0f;
  controlPara.gyroFilter.y = 50.0f;
  controlPara.gyroFilter.z = 20.0f;

  controlPara.curveRoll = 6.0f;
}

void loopControl()
{
  float maxvel = MAX(fabs(sensorData.wheelVel[1]), fabs(sensorData.wheelVel[2]));
  float inputDamp = 2.0f - maxvel / 20.0f;
  inputDamp = LIMIT(inputDamp, 0.0f, 1.0f);
  sensorData.gyroFilt.x = PT1(sensorData.gyro.x, sensorData.gyroFilt.x, controlPara.gyroFilter.x);
  sensorData.gyroFilt.y = PT1(sensorData.gyro.y, sensorData.gyroFilt.y, controlPara.gyroFilter.y);
  sensorData.gyroFilt.z = PT1(sensorData.gyro.z, sensorData.gyroFilt.z, controlPara.gyroFilter.z);

  float targetSpeedPitch = 0;
  static float effectivePPitch = controlPara.pPartPitch;
  effectivePPitch = ramp(effectivePPitch, controlPara.pPartPitch, 0.05f);
  if(input.c || BUTTON(3)) effectivePPitch = 1.0f;
  if(joystickTimeout) targetSpeedPitch += (joystickReport.y  / 127.0f - 1.0f) * -8.0f;
  targetSpeedPitch += (input.a  / 127.0f) * 8.0f;
  targetSpeedPitch *= inputDamp;
  controlState.targetSpeedPitch = ramp(controlState.targetSpeedPitch, targetSpeedPitch, 0.025f);
  float speedDev = controlState.targetSpeedPitch - sensorData.wheelVel[0];
  controlState.targetPitchI -= speedDev * controlPara.iPartSpeedPitch / 500.0f;
  controlState.targetPitchI = LIMIT(controlState.targetPitchI, -5.0f, 5.0f);
  controlState.speedPitchFilt = PT1(sensorData.wheelVel[0], controlState.speedPitchFilt, controlPara.speedPitchFilter);
  speedDev = controlState.targetSpeedPitch - controlState.speedPitchFilt;
  controlState.targetPitch = controlState.targetPitchI - speedDev * controlPara.pPartSpeedPitch;
  controlState.targetPitch = LIMIT(controlState.targetPitch, -15.0f, 15.0f);
  float pitchDev = controlPara.pitchWP + controlState.targetPitch - sensorData.pitchAngle;
  actuator.torque[0] = pitchDev * effectivePPitch - sensorData.gyroFilt.z * controlPara.dPartPitch;
  //if(fabs(sensorData.rollAngle) < 10.0f && fabs(actuator.torque[0]) < 2.0f) controlState.activePitch = true;

  static float targetYawRamped = 0.0f;
  float curveRoll = sensorData.wheelVel[0] * targetYawRamped / 255.0f * controlPara.curveRoll;
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
  //if(controlState.activePitch && fabs(targetTorqueRoll) < 1.0f) controlState.activeRoll = true;

  float targetYaw = 0.0f;
  if(joystickTimeout) targetYaw -= joystickReport.z - 127;
  targetYaw += input.b;
  targetYaw *= inputDamp;
  float speedRef = MAX(fabs(sensorData.wheelVel[0]), fabs(controlState.targetSpeedPitch)) / 3.0f;
  targetYaw /= LIMIT(speedRef, 1.0f, 10.0f);
  targetYawRamped = ramp(targetYawRamped, targetYaw, 5.0f);
  float targetYawTorque = (sensorData.gyro.x - targetYaw) * controlPara.dPartYaw;
  actuator.torque[1] += targetYawTorque;
  actuator.torque[2] += targetYawTorque;

  if(BUTTON(4))
  {
    controlState.activePitch = false;
  }

  if (fabs(sensorData.pitchAngle) > 20.0f || fabs(sensorData.rollAngle) > 20.0f)
  {
    controlState.activePitch = false;
    controlState.activeRoll = false;
  }

  if (BUTTON(3) || input.c)
  {
    controlState.activePitch = true;
    controlState.activeRoll = true;
  }

  if (!controlState.activePitch || BUTTON(3)) controlState.targetPitchI = 0.0f;
  if (!controlState.activeRoll) controlState.targetRollI = 0.0f;

  for (int i = 0; i < 3; i++) actuator.torque[i] = LIMIT(actuator.torque[i], -40.0f, 40.0f);

  static int noTorqueCnt = 0;
  if (sensorData.acc.x < 0.5f) noTorqueCnt = 25;
  if (noTorqueCnt)
  {
    if (noTorqueCnt > 20) actuator.torque[0] = 0;
    float targetP = controlPara.pPartPitch - noTorqueCnt / 4.0f;
    if (effectivePPitch > targetP) effectivePPitch = targetP;
    noTorqueCnt--;
  }

  for (int i = 0; i < 3; i++)
  {
    actuator.torque[i] -= sensorData.wheelVel[i] / 20.0f;
  }

  if (!actuator.disabled && controlState.activePitch)
  {
    sendTorque(0, actuator.torque[0]);
  } else {
    sendTorque(0, 0);
  }

  if (!actuator.disabled && controlState.activeRoll)
  {
    sendTorque(1, actuator.torque[1]);
    sendTorque(2, actuator.torque[2]);
  } else {
    sendTorque(1, 0);
    sendTorque(2, 0);
  }

}
