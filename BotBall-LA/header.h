#define pi 3.1415926

#define liftingMotorPort 0
#define catchingServoPort 3
#define lightSensorPortNum 0
#define etSensorPort 4
#define sensorLiftingServo 2
#define blackLineSensorPort 1
#define orangeChan 0
#define yellowChan 1

#define closeHand set_servo_position(catchingServoPort,1000)
#define openHand set_servo_position(catchingServoPort,0)