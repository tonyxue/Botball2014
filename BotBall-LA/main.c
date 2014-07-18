#include <stdio.h>
#include "header.h"

const int turningSpeed=80, blackLineCriticalValue=900, lightCriticalValue=512;
int oFlag=0,yFlag=0;
void lightDetection() // Light detection
{
	int reading;
	printf("Wait for light!\n");
	do
	{
		reading = analog10(lightSensorPortNum);
		printf("The reading from light sensor is %d\n",reading );
		msleep(50); // prevent too frequent operation
		
	}while (reading>lightCriticalValue);
}
void twoMotors(int speed, int time)
{
	motor(liftingMotorPort1,speed);
	motor(liftingMotorPort2,speed);
	msleep(time);
	ao();
}
void servoInit()
{
	enable_servo(catchingServoPort);
	enable_servo(sensorLiftingServo);
	closeHand;
	msleep(200);
	printf("Servo initialized!\n");
}
void motorInit()
{
	twoMotors(100,7400);
	printf("Motor initialized!\n");
}
int blackLine()
{
	int reading;
	while (1)
	{
		reading=analog10(blackLineSensorPort);
		if(reading>blackLineCriticalValue) return 1; // Return TRUE when black color is detected
		if(reading<blackLineCriticalValue) return 0; // Return FALSE when white color is detected
	}
}
void turnRight(int degrees)
{
	int rad=0.5*pi;
	create_drive(-50,0);
	msleep(rad*4000*degrees/90);
	create_stop();
}
void turnLeft(int degrees)
{
	int rad=0.5*pi;
	create_drive(50,0);
	msleep(rad*4000*degrees/90);
	create_stop();
}
void goAlongLine(double time)
{
	printf("Following the black line!\n");
	int left,right,speed=100;
	double sTime,cTime=0;
 	sTime = seconds();
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
		cTime = seconds();
	}
}
int identColor() // awaiting fixation
{
	int area,i;
	camera_update();
	msleep(100);
	for (i=0;i<=2;i++)
	{
		if (get_object_count(orangeChan) > 0) 
		{
			area = get_object_area(orangeChan,0);
			printf("Area reading:%d\n", area);
			return area;
		}
	}
	return -1;
}
void appCube()
{
	int distance,dCount=0,errCount=0;
	create_drive_straight(30);
	do
	{
		distance = analog_et(etSensorPort);
		printf("Distance reading: %d\n", distance);
		if (distance >= 500)
		{
			create_drive_straight(-200);
			msleep(500);
			create_drive_straight(30);
			continue;
		}
		if (distance >= 420 && dCount < 3)
		{
			dCount++;
			distance = 0;
			msleep(100);
		}
	}
	while (distance < 480 || dCount < 3);
	create_stop();
}
void goToHangerRack();
void putHangers();

int main()
{
	int foundOrange=0,identColorReading,distance,cCount=0;
	double cTime=0,sTime,Time2;
	printf("Program Begin!\n");
	//lightDetection();

	set_a_button_text("Start");
	while (!a_button_clicked()){}

	shut_down_in(120);
	printf("Start!\n");
	camera_open();
	display_clear();
	servoInit(); // Supply electricity to all the servos
	motorInit(); // set the motor position
	create_connect(); // connect the create
	create_full(); // FULL mode, the create will execute commands under any circumstance
	printf("Create connected!\n Battery: %d\n",get_create_battery_charge());
	create_drive_straight(100); // avoid scratching with the tube
	msleep(300);
	turnLeft(90); // adjust heading in startup area
	
	goToHangerRack(); // create move to the hanger rack
	putHangers(); // the "hand" put hangers on the rack

	create_drive_straight(-200); // drive away from hanger rack
	msleep(800);
	create_stop();
	turnRight(90);
	create_drive_straight(200);
	msleep(3000);
	turnLeft(90);
	create_drive_straight(-200);
	msleep(1800);
	create_spin_CCW(200);
	msleep(200);
	create_stop();

	twoMotors(-100,2450);
	sTime = seconds();
	create_spin_CW(10);
	while (cTime - sTime <= 8.0)
	{
		// unfinished.
		cTime = seconds();
		identColorReading = identColor();
		if (identColorReading >= 410)
		{
			cCount++;
			if (cCount == 1)
			{
				foundOrange = 1;
				break;
			}
		}
	}
	if (foundOrange = 0)
	{
		create_drive_straight(-100);
		msleep(500);
		create_stop();
		twoMotors(-100,520);
		sTime = seconds();
		cTime = 0;
		while (cTime - sTime <= 10.0)
		{
			cTime = seconds();
			create_spin_CCW(10);
			identColorReading = identColor();
			if (identColorReading >= 410) break;
		}
	}
	create_stop();
	twoMotors(-100,550);
	distance = analog_et(etSensorPort);
	Time2 = seconds();
	if (distance < 400)
	{
		appCube();
		Time2 = seconds() - Time2;
	}
	else if (distance > 480) //Retrial for abnormal value
	{
		create_drive_straight(-400);
		msleep(800);
		create_stop();
		appCube();
		Time2 = seconds() - Time2 - 300;
	}
	twoMotors(100,200);
	closeHand; // Close the hand
	msleep(200);
	twoMotors(100,200);
	create_drive_straight(200);
	msleep(300);

	create_drive(-50,0);
	if (foundOrange == 1) msleep(4713 - (cTime - sTime)/4);
	if (foundOrange == 0) msleep(4705 + (cTime - sTime)/4); // todo
	turnRight(180);
	create_stop();

	PROGRAM_STOPHERE_FOR_DEBUGGING_____REMOVE_BEFORE_THE_MATCH;
	
    create_drive_straight(100); // Increase the speed steadily
	msleep(100);
	create_drive_straight(150);
	msleep(100);
	create_drive_straight(250);
	msleep(100);
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
	twoMotors(-100,1000);
	openHand;

	return 0;
}
void goToHangerRack()
{
	printf("Going to the hanger rack!\n");
	create_drive_straight(200);
	while(!blackLine()){}
	create_stop();
	msleep(200);
	create_drive_straight(-200);
	msleep(200);
	turnLeft(90);
	create_drive_straight(200);
	msleep(2000);
	turnRight(95);
	create_stop();
}
void putHangers()
{
	printf("Start putting hangers!\n");
	twoMotors(-100,500);
	create_drive_straight(200);
	msleep(900);
	create_stop();
	openHand; // open the "hand" to put the hangers
	msleep(50);
	twoMotors(100,250);
}
