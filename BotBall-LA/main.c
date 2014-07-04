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
	closeHand;
	msleep(200);
	printf("Servo initialized!\n");
}
void motorInit()
{
	motor(liftingMotorPort1,100);
	motor(liftingMotorPort2,100);
	msleep(8200);
	ao();
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
	int area;
	camera_update();
	msleep(300);
	if (get_object_count(orangeChan) > 0) return get_object_area(orangeChan,0);
	return -1;
}
void goToHangerRack();
void putHangers();

int main()
{
	int cTime=0,sTime,Time2,foundOrange=0;
	printf("Start!\n");
	//printf("Wait for light!\n");
	//lightDetection();

	set_a_button_text("Start!\n");
	while (!a_button_clicked()){}

	shut_down_in(120);

	camera_open();
	display_clear();
	servoInit(); // Supply power to all the servos
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
	msleep(1000);
	create_stop();

	turnRight(70);
	create_drive_straight(200);
	msleep(400);
	create_stop();

	motor(liftingMotorPort1,-100);
	motor(liftingMotorPort2,-100);
	msleep(2900);
	ao();

	sTime = seconds();
	while (cTime - sTime <= 5.0)
	{
		cTime = seconds();
		create_spin_CW(50);
		msleep(300);
		create_stop();
		if (identColor() >= 200)
		{
			foundOrange = 1;
			create_spin_CCW(50);
			msleep(600);
			break;
		}
	}
	if (foundOrange = 0)
	{
		motor(liftingMotorPort1,-100);
		motor(liftingMotorPort2,-100);
		msleep(500);
		ao();
		create_drive_straight(-100);
		msleep(500);
		sTime = seconds();
		cTime =  seconds();
		while (cTime - sTime <= 5.0)
		{
			cTime = seconds();
			create_spin_CCW(50);
			msleep(300);
			create_stop();
			if (identColor >= 250)
			{
				create_spin_CW(50);
				msleep(600);
				break;
			}
		}
	}
	motor(liftingMotorPort1,-100);
	motor(liftingMotorPort2,-100);
	msleep(300);
	ao();
	Time2 = seconds();
	create_drive_straight(200);
	while (analog_et(etSensorPort < 500)){}
	create_stop();
	Time2 = Time2 - seconds();
	motor(liftingMotorPort1,-100);
	motor(liftingMotorPort2,-100);
	msleep(200);
	ao();
	closeHand; // Close the hand
	msleep(200);
	motor(liftingMotorPort1,100);
	motor(liftingMotorPort2,100);
	msleep(200);
	ao();
	create_drive_straight(-200);
	msleep(Time2);
	create_stop();
	create_spin_CCW(200);
	if (foundOrange == 1) msleep((cTime - sTime)/4);
	if (foundOrange == 0) msleep((20 - (cTime - sTime))/4);
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
	motor(liftingMotorPort1,-100);
	motor(liftingMotorPort2,-100);
	msleep(1000);
	ao();
	openHand;

	return 0;
}
void goToHangerRack()
{
	printf("Going to the hanger rack!\n");
	create_drive_straight(200);
	while(!blackLine()){}
	create_stop();
	msleep(50);
	create_drive_straight(-200);
	msleep(800);
	turnLeft(90);
	create_drive_straight(200);
	msleep(2000);
	turnRight(90);
	create_stop();
}
void putHangers()
{
	printf("Start putting hangers!\n");
	create_drive_straight(200);
	msleep(1800);
	create_stop();
	openHand; // open the "hand" to put the hangers
	msleep(500);
}
