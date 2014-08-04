#define pi 3.1415926

#define liftingMotorPort1 0
#define liftingMotorPort2 3
#define catchingServoPort 3
#define lightSensorPortNum 0
#define blackLineSensorPort 6
#define orangeChan 0
#define yellowChan 1

#define closeHand set_servo_position(catchingServoPort,1850)
#define openHand set_servo_position(catchingServoPort,1)

#define PROGRAM_STOPHERE_FOR_DEBUGGING_____REMOVE_BEFORE_THE_MATCH return 0
