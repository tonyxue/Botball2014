#include <stdio.h>
#define pi 3.1415926

#define liftingMotorPort 0
#define catchingServoPort 3
#define lightSensorPortNum 0
#define cubeButtonPort 15
#define sensorLiftingServo 2
#define blackLineSensorPort 1

const int turningSpeed=80, blackLineCriticalValue=900;
int oFlag,yFlag;
void lightDetection() // Light detection, checked
{
	int reading=1024,lightCriticalValue=512; //define Light Sensor port number, critical value and initialize the reading
	while (reading>lightCriticalValue)
	{
		msleep(50); // prevent too frequent operation
		reading=analog10(lightSensorPortNum);
		printf("The reading from light sensor is %d\n",reading );
	}
}
void servoInit()
{
	enable_servo(catchingServoPort);
	enable_servo(sensorLiftingServo);
	set_servo_position(catchingServoPort,1024);
	set_servo_position(sensorLiftingServo,100);// checked
	msleep(600);
	printf("Servo initialized!\n");
}
void motorInit()
{
	motor(liftingMotorPort,100);
	msleep(7000);//2700
	off(liftingMotorPort);
	printf("Motor initialized!\n");
}
int blackLine()
{
	int reading;
	reading=analog10(blackLineSensorPort);
	while (1)
	{
		if(analog10(blackLineSensorPort)>blackLineCriticalValue) return 1;// Return TRUE when black color is detected
		if(analog10(blackLineSensorPort)<blackLineCriticalValue) return 0;// Return FALSE when white color is detected
	}
}
void turnLeftDegrees(int degrees) //checked
{
	int angle;
	set_create_normalized_angle(0);
	angle=get_create_normalized_angle();
	create_spin_CCW(turningSpeed);// turn counter clock wise(CCW)
	while (angle<degrees)
		angle=get_create_normalized_angle();
}
void turnRightDegrees(int degrees) //checked
{
	int angle;
	set_create_normalized_angle(0);
	angle=get_create_normalized_angle();
	create_spin_CW(turningSpeed);// turn clock wise(CW)
	while (angle>degrees*-1)
		angle=get_create_normalized_angle();
}
void Right90()
{
	int rad=0.5*pi;
	create_drive(-200,0);
	msleep(rad*1000);
	create_stop();
}
void Left90()
{
	int rad=0.5*pi;
	create_connect();
	create_drive(200,0);
	msleep(rad*1000);
	create_stop();
}
void goAlongLine(double time)
{
	int left,right,speed=200;
	double sTime,cTime=0;
 	sTime=seconds();
	create_drive_straight(speed);
	while(cTime-sTime<=time)
	{
		left=get_create_lfcliff_amt();
		right=get_create_rfcliff_amt();
		if (left<800)
		{
			turnLeftDegrees(2);
			cTime=cTime-500;
			create_drive_straight(speed);
		}
		if (right<800)
		{
			turnRightDegrees(2);
			cTime=cTime-500;
			create_drive_straight(speed);
		}
		cTime=seconds();
	}
}
void goToHangerRack() //checked
{
	create_drive_straight(200);
	while(!blackLine()){}
	//turnLeftDegrees(84);
	Left90();
	goAlongLine(3.2);
	//turnRightDegrees(80);
	Right90();
	create_drive_straight(-100);
	msleep(900);
	create_stop();
}
void putHangers() //checked
{
	//put the hangers on to the PVC
	motor(liftingMotorPort,-100);
	msleep(1000);
	off(liftingMotorPort);
	create_drive_straight(200);
	msleep(450);
	create_stop();
	set_servo_position(catchingServoPort,600); // open the "hand" to put the hangers
	msleep(500);
	motor(liftingMotorPort,-100);
	msleep(280);
	off(liftingMotorPort);
}
void orangeCube()
{
	//the variable oFlag, which is defined as a global variable, is used in this function as an indicator for whether the robot has successfully gotten the orange cube after several trials.
	double cTime=0,sTime;// cTime stands for Current Time, sTime stands for Start Time
	create_drive_straight(200);// go straight a bit to pass the hanger rack, avoid collision
	msleep(500);
	create_stop();
	set_servo_position(sensorLiftingServo,1100);// extend the sensor to its working position
	create_drive_straight(200);// go straight till the button sensor is triggered
	sTime=seconds();// record the time of start
	while(!digital(cubeButtonPort))// while the long lever sensor is NOT triggered
	{
		if (oFlag==2) break;// give up when no result after two trials
		cTime=seconds();// initialize the timer
		if (cTime-sTime>3.0) // timeout reached. Adjust the position and repeat searching
		{
			set_servo_position(sensorLiftingServo,50); // retract the long lever sensor
			msleep(500);
			create_drive_straight(-200);// drive back to the start point
			msleep(3000);
			//turnLeftDegrees(80);// move closer to the shelf a bit
			Left90();
			create_drive_straight(200);
			msleep(80);
			//turnRightDegrees(80);// heading parallel to the black line
			Right90();
			create_drive_straight(200);// start detection again
			if(oFlag==0) set_servo_position(sensorLiftingServo,1110);// extend the sensor to its working position
			if(oFlag==1) set_servo_position(sensorLiftingServo,1130);// extend the sensor to its working position
			msleep(300);
			oFlag++;
			sTime=seconds();// reset the start time for next trial
		}
	}
	
	if (oFlag!=2)// if the attempt(s) didn't fail
	{
		create_drive_straight(-200);// repositioning, move back a little little bit. leave space for "hand" operations later
		msleep(230);
		create_stop();

		//turnLeftDegrees(75); // turn toward the shelf
		Left90();
		create_drive_straight(-200);// step back, move the "hand" above the cube detected
		msleep(250);
		create_stop();

		set_servo_position(catchingServoPort,300); // open the "hand"
		msleep(500);

		motor(liftingMotorPort,-100); // put the hand down
		msleep(2000);
		off(liftingMotorPort);
		msleep(500);

		set_servo_position(catchingServoPort,1400); // close the "hand"
		motor(liftingMotorPort,100);// lift the hand
		msleep(1500);
		off(liftingMotorPort);
		// got the cube

		//turnLeftDegrees(75);
		Left90();
		goAlongLine(2);// make the create drive straightly

		create_drive_straight(500);// rush to the other end of the game board
		while(!get_create_rbump() && !get_create_lbump()){} // reach the other side of the board
		create_drive_straight(-100);// step back, leave space for future "hand" operations
		msleep(200);

		//turnLeftDegrees(77); // turn toward the container
		Left90();
		create_drive_straight(100);
		while(!get_create_lbump() && !get_create_rbump()){}
		create_drive_straight(-100);// step back, leave space for future "hand" operations
		msleep(1500);
		create_stop();
		motor(liftingMotorPort,-100); // put the hand down
		msleep(1000);
		off(liftingMotorPort);
		set_servo_position(catchingServoPort,300); // open the "hand"
		msleep(500);
		motor(liftingMotorPort,100);// lift the "hand" up to the highest position, avoid collision in the rush later
		msleep(5000);
		off(liftingMotorPort);
		set_servo_position(catchingServoPort,1400); // close the "hand" again
		msleep(500);
	}
}
void yellowCube() // old version of getting yellow cube, no enough time, so no go
{
	double cTime=0,sTime;
	create_drive_straight(200);
	msleep(1000);
	create_stop();
	set_servo_position(sensorLiftingServo,1450);// extend the button sensor again
	msleep(1000);
	create_drive_straight(200);
	sTime=seconds();
	while(!digital(cubeButtonPort))
	{
		if (yFlag==3) break;
		cTime=seconds();
		if (cTime-sTime>2.7)
		{
			create_drive_straight(-200);
			msleep(3200);
			//turnLeftDegrees(80);
			Left90();
			create_drive_straight(100);
			msleep(500);
			//turnRightDegrees(80);
			Right90();
			create_drive_straight(200);
			yFlag++;
			sTime=seconds();
		}
	}
	if (yFlag!=4)
	{
		create_stop();
		motor(liftingMotorPort,100);// lift the hand a bit
		msleep(3000);
		//turnLeftDegrees(80);
		Left90();
		create_drive_straight(-200);
		msleep(500);
		create_stop();
		set_servo_position(catchingServoPort,300); // open the "hand"
		msleep(1000);
		motor(liftingMotorPort,-100);// put the hand down
		msleep(2000);
		off(liftingMotorPort);
		msleep(500);
		set_servo_position(catchingServoPort,1400); // close the "hand"
		create_drive_straight(-200);
		msleep(500);
		create_stop();
		motor(liftingMotorPort,100);// lift the hand a bit
		msleep(3000);
		off(liftingMotorPort);
	
		create_drive_straight(200);
		while(!blackLine()){}
		//turnLeftDegrees(75);
		Left90();
		goAlongLine(2);

		create_drive_straight(500);
		while(!get_create_rbump() && !get_create_lbump()){}
		//turnLeftDegrees(77);
		Left90();
		create_drive_straight(200);
		while(!get_create_rbump() && !get_create_lbump()){}
		create_drive_straight(-200);
		msleep(1300);
		create_stop();
		motor(liftingMotorPort,-100);
		msleep(1000);
		off(liftingMotorPort);
		set_servo_position(catchingServoPort,300); // open the "hand"
		msleep(500);
		set_servo_position(catchingServoPort,1400); // close the "hand" again
		msleep(500);
	}
}
void yellowCubeV2() // sweep the yellow cube to the board
{
	//the variable yFlag, which is defined as a global variable, is used in this function as an indicator for whether the robot has successfully detected the yellow cube after several trials.
	double cTime=0,sTime;
	create_drive_straight(200);// go straight a bit to pass the hanger rack, avoid collision
	msleep(1000);
	create_stop();
	set_servo_position(sensorLiftingServo,1450);// extend the button sensor again
	msleep(1000);
	create_drive_straight(200);// go straight till the button sensor is triggered
	while(!digital(cubeButtonPort))
	{
		if (yFlag==3) break;
		cTime=seconds();
		if (cTime-sTime>2.7)
		{
			create_drive_straight(-200);
			msleep(3200);
			//turnLeftDegrees(80);
			Left90();
			create_drive_straight(100);
			msleep(500);
			//turnRightDegrees(80);
			Right90();
			create_drive_straight(200);
			yFlag++;
			sTime=seconds();
		}
	}
	if (yFlag!=4)// if the long lever sensor is triggered
	{
		create_drive_straight(500); // rush to sweep the cubes down from the shelf
		msleep(6000);
		create_stop();
	}
}
int main()
{
	printf("Start!\n");
	//lightDetection();
	shut_down_in(120);

	servoInit(); // Supply power to all the servos
	motorInit(); // set the motor position
	create_connect();// connect the create
	create_full();// FULL mode, the create will execute commands under any circumstance
	printf("Create connected!\n Battery: %d\n",get_create_battery_charge());
	msleep(1000);
	
	//turnLeftDegrees(80); // adjust heading in startup area
	Left90();
	goToHangerRack(); // create move to the hanger rack
	putHangers(); // the "hand" put hangers on the rack
	
	create_drive_straight(-200);// drive away from hanger rack
	msleep(1200);
	create_stop();
	motor(liftingMotorPort,100);// lift the hand to the highest position, avoid collision
	msleep(2200);
	off(liftingMotorPort);
	create_drive_straight(200); // reach the black line
	while(!blackLine()){}
	create_drive_straight(-200);// move back a bit after reaching the black line
	msleep(300);
	//turnRightDegrees(77);// turn right onto the black line
	Right90();
	goAlongLine(1.5);// drive a long the black line for a few seconds
	//turnLeftDegrees(81);// turn left face towards the PVC
	Left90();
	create_drive_straight(200);// reach the PVC
	while(!get_create_rbump() && !get_create_lbump()){}
	create_drive_straight(-200); // drive away from the PVC on the board, leave space for the long lever sensor
	msleep(400);
	//turnRightDegrees(78);// turn right, heading parallel to the PVC
	Right90();
	orangeCube();// execute the procedures of getting the orange cube
	if(oFlag!=2)// if successfully got and delivered the orange cube, then move back to the shelf for the yellow cubes
	{
		create_drive_straight(-200);// move away from the containers
		msleep(500);
		//turnLeftDegrees(78);
		Left90();
		create_drive_straight(500); //rush
		msleep(2700);
		//turnLeftDegrees(76);
		Left90();
		create_drive_straight(200);
		while(!blackLine()){} // reach the black line
		create_drive_straight(-200);// step back, leave space for future operations
		msleep(400);
		create_stop();
		set_servo_position(sensorLiftingServo,50);// retract the button sensor to avoid collision
		msleep(2000);
		//turnRightDegrees(76);
		Right90();
	}
	if(oFlag==2) // if failed to get the orange cube, adjust the position for the yellow cubes
	{
		set_servo_position(sensorLiftingServo,50); // retract the long lever sensor
		msleep(500);
		create_drive_straight(-200);// drive back to the start point
		msleep(3000);
		//turnLeftDegrees(77);// turn toward the shelf(or PVC on the board)
		Left90();
		create_drive_straight(200);// touch the PVC
		while(!get_create_lbump() && !get_create_rbump()){}
		create_drive_straight(-200);// step back, leave enough space for future operations
		msleep(1500);
		//turnRightDegrees(77);// heading parallel to the black line
		Right90();
	}

	yellowCubeV2();// execute the procedures of sweeping down the yellow cubes

	return 0;
}
