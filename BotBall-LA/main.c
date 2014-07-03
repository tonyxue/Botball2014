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
	msleep(600);
	printf("Servo initialized!\n");
}
void motorInit()
{
	motor(liftingMotorPort1,100);
	motor(liftingMotorPort2,100);
	msleep(6000);
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
		if (get_object_area(orangeChan,0) >= 20) return 0;
		if (get_object_area(yellowChan,0) >= 20) return 1;
	}
	camera_close();
}
void goToHangerRack();
void putHangers();

int main()
{
	int cTime=0,sTime,foundOrange=0;
	printf("Start!\n");
	//printf("Wait for light!\n");
	//lightDetection();

	set_a_button_text("Start!\n");
	while (!a_button_clicked()){}

	shut_down_in(120);

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

	turnRight(35);
	create_drive_straight(200);
	msleep(500);
	create_stop();

	motor(liftingMotorPort1,-100);
	motor(liftingMotorPort2,-100);
	msleep(2000);
	ao();

	//openHand; // Open the hand
	sTime = seconds();
	create_spin_CW(50);
	while (cTime - sTime <= 20.0)
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
		motor(liftingMotorPort1,-100);
		motor(liftingMotorPort2,-100);
		msleep(300);
		ao();
		sTime = seconds();
		cTime =  seconds();
		create_spin_CCW(50);
		while (cTime - sTime <= 20.0)
		{
			cTime = seconds();
			if (identColor == 0)
			{
				break;
			}
		}
	}
	create_stop();
	set_servo_position(catchingServoPort,500);
	sTime = seconds();
	create_drive_straight(200);
	while (analog_et(etSensorPort < 500)){}
	create_stop();
	cTime = seconds();
	closeHand; // Close the hand
	motor(liftingMotorPort1,100);
	motor(liftingMotorPort2,100);
	msleep(200);
	ao();
	create_drive_straight(-200);
	msleep(cTime - sTime);
	create_stop();
	create_spin_CCW(200);
	msleep((cTime - sTime)/4);
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
	create_drive_straight(-200);
	msleep(600);
	turnLeft(90);
	//goAlongLine(5.6);
	create_drive_straight(200);
	msleep(2000);
	turnRight(90);
	create_stop();
}
void putHangers()
{
	printf("Start putting hangers!\n");
	//put the hangers on to the PVC
	create_drive_straight(200);
	msleep(1200);
	create_stop();
	openHand; // open the "hand" to put the hangers
	msleep(500);
}
