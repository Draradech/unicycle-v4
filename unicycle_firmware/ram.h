#ifdef _DEFINE_RAM_VARS_
#define globl
#else
#define globl extern
#endif

enum tt
{
  ttmin,
  ttmax,
  ttsum,
  ttnum,
  ttavg,
  tt_size
};

enum charge
{
  chg_normal,
  chg_low,
  chg_critical,
  chg_empty
};

typedef struct
{
  int8_t a;
  int8_t b;
  int8_t c;
  int8_t d;
} SInput;

typedef struct
{
  xyzFloat acc;
  xyzFloat gyro;
  xyzFloat gyroFilt;
  xyzFloat magnet;
  float pitchAngleAcc;
  float rollAngleAcc;
  float pitchAngle;
  float rollAngle;
  float wheelPos[3];
  float wheelVel[3];
  float iBus[3];
  float voltage;
  uint16_t taskTime[tt_size];
  uint8_t chargeState;
} SSensorData;

typedef struct
{
  float torque[3];
  bool disabled;
} SActuator;

typedef struct
{
  xyzFloat gyroOffset;
  xyzFloat gyroFilter;
  float pPartSpeedPitch;
  float iPartSpeedPitch;
  float pPartPitch;
  float dPartPitch;
  float pitchWP;
  float speedPitchFilter;
  float pPartSpeedRoll;
  float iPartSpeedRoll;
  float pPartRoll;
  float dPartRoll;
  float rollWP;
  float speedRollFilter;
  float dPartYaw;
  float rollOffset;
  float pitchOffset;
} SControlPara;

typedef struct
{
  float targetSpeedPitch;
  float speedPitchFilt;
  float targetSpeedRoll;
  float speedRollFilt;
  float targetPitchI;
  float targetPitch;
  float targetRollI;
  float targetRoll;
  bool activePitch;
  bool activeRoll;
} SControlState;

typedef struct __attribute__((__packed__)) {
  uint8_t hat;
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t rz;
  uint8_t brake;
  uint8_t accel;
  uint32_t buttons;
} joystick_t;

globl joystick_t joystickReport;
globl uint8_t joystickTimeout;
globl bool wifiOn;
globl SSensorData sensorData;
globl SActuator actuator;
globl SControlPara controlPara;
globl SControlState controlState;
globl SInput input;
