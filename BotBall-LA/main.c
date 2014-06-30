#include <stdio.h>
#include "header.h"

const int turningSpeed=80, blackLineCriticalValue=900, lightCriticalValue=512;
int oFlag=0,yFlag=0;
void lightDetection() // Light detection
{
	int reading;
	do
	{
		reading=analog10(lightSensorPortNum);
		printf("The reading from light sensor is %d\n",reading );
		msleep(50); // prevent too frequent operation
		
	}while (reading>lightCriticalValue);
}
void servoInit()
{
	enable_servo(catchingServoPort);
	enable_servo(sensorLiftingServo);
	closeHand;//set_servo_position(catchingServoPort,1000);
	msleep(600);
	printf("Servo initialized!\n");
}
void motorInit()
{
	motor(liftingMotorPort,100);
	msleep(7000);
	off(liftingMotorPort);
	printf("Motor initialized!\n");
}
int blackLine()
{
	int reading;
	while (1)
	{
		reading=analog10(blackLineSensorPort);
		if(reading>blackLineCriticalValue) return 1;// Return TRUE when black color is detected
		if(reading<blackLineCriticalValue) return 0;// Return FALSE when white color is detected
	}
}
void turnRight(int degrees)
{
	int rad=0.5*pi;
	create_drive(-200,0);
	msleep(rad*1000*degrees/90);
	create_stop();
}
void turnLeft(int degrees)
{
	int rad=0.5*pi;
	create_drive(200,0);
	msleep(rad*1000*degrees/90);
	create_stop();
}
void goAlongLine(double time)
{
	printf("Following the black line!\n");
	int left,right,speed=100;
	double sTime,cTime=0;
 	sTime=seconds();
	create_drive_straight(speed);
	while(cTime - sTime <= time)
	{
		left=get_create_lfcliff_amt();
		right=get_create_rfcliff_amt();
		if (left < 800)
		{
			turnLeft(1);
			cTime = cTime - 500;
			create_drive_straight(speed);
		}
		if (right < 800)
		{
			turnRight(1);
			cTime = cTime - 500;
			create_drive_straight(speed);
		}
		cTime=seconds();
	}
}
int identColor()
{
	camera_open();
	while (1)
	{
		camera_update();
		if (get_object_count(orangeChan) == 0 && get_object_count(yellowChan) == 0) return -1;
		if (get_object_count(orangeChan) > 0) return 0;
		if (get_object_count(yellowChan) > 0) return 1;
	}
	camera_close();
}
void goToHangerRack();
void putHangers();
void blueHangers();
void orangeCube();
void yellowCube();

int main()
{
	int cTime=0,sTime,foundOrange=0;
	printf("Start!\n");
	printf("Wait for light!\n");
	//lightDetection();
	shut_down_in(120);

	servoInit(); // Supply power to all the servos
	motorInit(); // set the motor position
	create_connect();// connect the create
	create_full();// FULL mode, the create will execute commands under any circumstance
	printf("Create connected!\n Battery: %d\n",get_create_battery_charge());
	msleep(1000);
	
	turnLeft(90);// adjust heading in startup area
	goToHangerRack(); // create move to the hanger rack
	putHangers(); // the "hand" put hangers on the rack
	
	create_drive_straight(-200); // drive away from hanger rack
	msleep(1500);
	create_stop();

	turnRight(45);
	create_drive_straight(200);
	msleep(500);
	create_stop();

	openHand; // set_servo_position(catchingServoPort, 0); // Open the hand
	sTime = seconds();
	create_spin_CW(200);
	while (cTime - sTime <= 5.0)
	{
		cTime = seconds();
		if (identColor() == 0)
		{
			foundOrange = 1;
			break;
		}
	}
	if (foundOrange = 0)
	{
		motor(liftingMotorPort,-100);
		msleep(500);
		off(liftingMotorPort);

		sTime = seconds();
		cTime =  seconds();
		create_spin_CCW(200);
		while (cTime - sTime <= 5.0)
		{
			cTime = seconds();
			if (identColor == 0)
			{
				foundOrange = 1;
				break;
			}
		}
	}
	create_stop();
	set_servo_position(catchingServoPort,500);
	sTime = seconds();
	create_drive_straight(200);
	while (analog_et(etSensorPort<500)){}
	create_stop();
	cTime = seconds();
	closeHand; // set_servo_position(catchingServoPort,1000); // Close the hand
	motor(liftingMotorPort,100);
	msleep(200);
	off(liftingMotorPort);
	create_drive_straight(-200);
	msleep(cTime - sTime);
	create_stop();
	create_spin_CCW(200);
	msleep(cTime - sTime);
	create_stop();
	create_drive_straight(200);
	while (!blackLine()){}
	create_stop();
	turnLeft(90);
	create_drive_straight(500);
	while (!get_create_lbump() && !get_create_rbump()){}
	create_stop();
	create_drive_straight(-200);
	msleep(200);
	create_stop();
	turnLeft(90); // turn toward the containers
	create_drive_straight(-200);
	msleep(1000);
	create_stop();
	motor(liftingMotorPort,-100);
	msleep(1000);
	off(liftingMotorPort);
	set_servo_position(catchingServoPort,0);
	
	/*
	motor(liftingMotorPort,100);// lift the hand to the highest position, avoid collision
	msleep(2200);
	off(liftingMotorPort);
	create_drive_straight(200); // reach the black line
	while(!blackLine()){}
	turnRight(90); // turn right onto the black line
	goAlongLine(3.0);// drive a long the black line for a few seconds
	turnLeft(90);// turn left face towards the PVC
	create_drive_straight(200);// reach the PVC
	while(!get_create_rbump() && !get_create_lbump()){}
	create_drive_straight(-200); // drive away from the PVC on the board, leave space for the long lever sensor
	while(!blackLine()){}
	create_stop();
	turnRight(90);// turn right, heading parallel to the PVC
	orangeCube();// execute the procedures of getting the orange cube
	if(oFlag!=2)// if successfully got and delivered the orange cube, then move back to the shelf for the yellow cubes
	{
		create_drive_straight(-200);// move away from the containers
		msleep(500);
		turnLeft(90);
		create_drive_straight(500); //rush
		msleep(2700);
		turnLeft(90);
		create_drive_straight(200);
		while(!blackLine()){} // reach the black line
		create_drive_straight(-200);// step back, leave space for future operations
		msleep(400);
		create_stop();
		set_servo_position(sensorLiftingServo,50);// retract the button sensor to avoid collision
		msleep(2000);
		turnRight(90);
	}
	if(oFlag==2) // if failed to get the orange cube, adjust the position for the yellow cubes
	{
		set_servo_position(sensorLiftingServo,50); // retract the long lever sensor
		msleep(500);
		create_drive_straight(-200);// drive back to the start point
		msleep(3000);
		turnLeft(90); // turn toward the shelf(or PVC on the board)
		create_drive_straight(200);// touch the PVC
		while(!get_create_lbump() && !get_create_rbump()){}
		create_drive_straight(-200);// step back, leave enough space for future operations
		msleep(1500);
		turnRight(90); // heading parallel to the black line
	}

	yellowCube();// execute the procedures of sweeping down the yellow cubes

	return 0;
	*/
}
void goToHangerRack()
{
	printf("Going to the hanger rack!\n");
	create_drive_straight(200);
	while(!blackLine()){}
	turnLeft(90);
	goAlongLine(5.6);
	turnRight(90);
	//create_drive_straight(-100);
	//msleep(900);
	create_stop();
}
void putHangers()
{
	printf("Start putting hangers!\n");
	//put the hangers on to the PVC
	//motor(liftingMotorPort,-100);
	//msleep(1200);
	//off(liftingMotorPort);
	create_drive_straight(200);
	msleep(450);
	create_stop();
	openHand;//set_servo_position(catchingServoPort,600); // open the "hand" to put the hangers
	msleep(500);
	motor(liftingMotorPort,-100);
	msleep(280);
	off(liftingMotorPort);
}
void orangeCube()
{
	printf("Start getting orange cube!\n");
	//the variable oFlag, which is defined as a global variable, is used in this function as an indicator for whether the robot has successfully gotten the orange cube after several trials.
	double cTime=0,sTime;// cTime stands for Current Time, sTime stands for Start Time
	goAlongLine(1.0);
	create_stop();
	set_servo_position(sensorLiftingServo,1100);// extend the sensor to its working position
	create_drive_straight(200);// go straight till the button sensor is triggered
	sTime=seconds();// record the time of start
	while(!digital(cubeButtonPort))// while the long lever sensor is NOT triggered
	{
		if (oFlag==2) break;// give up when no result after two trials
		cTime=seconds();// initialize the timer
		if (cTime-sTime>2.8) // timeout reached. Adjust the position and repeat searching
		{
			set_servo_position(sensorLiftingServo,50); // retract the long lever sensor
			msleep(500);
			create_drive_straight(-200);// drive back to the start point
			msleep(3000);
			// move closer to the shelf a bit
			turnLeft(90);
			create_drive_straight(200);
			msleep(80);
			turnRight(90); // heading parallel to the black line
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

		turnLeft(90); // turn toward the shelf
		create_drive_straight(-200);// step back, move the "hand" above the cube detected
		msleep(200);
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

		turnLeft(90);
		goAlongLine(4.0);// make the create drive straightly

		create_drive_straight(500);// rush to the other end of the game board
		while(!get_create_rbump() && !get_create_lbump()){} // reach the other side of the board
		create_drive_straight(-100);// step back, leave space for future "hand" operations
		msleep(200);

		turnLeft(90); // turn toward the container
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
void yellowCube() // sweep the yellow cube to the board
{
	printf("Start sweeping the yellow cubes!\n");
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
			turnLeft(90);
			create_drive_straight(100);
			msleep(500);
			turnRight(90);
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
void cube()
{
	int yellowCount=0,colorResult;
	printf("Start getting orange cube!\n");
	//the variable oFlag, which is defined as a global variable, is used in this function as an indicator for whether the robot has successfully gotten the orange cube after several trials.
	double cTime,sTime;// cTime stands for Current Time, sTime stands for Start Time
	goAlongLine(1.5);
	create_stop();
	//lift the arm
	sTime = seconds();
	create_spin_CCW(200);
	while (yellowCount != 2)
	{
		colorResult = identColor();
		if (colorResult == 0) break;
		if (colorResult == 1) yellowCount++;
	}
	if (yellowCount == 0) // No yellow cube on the upper shelf
	{}
}
