#include <stdio.h>
#include "header.h"

const int turningSpeed=80, blackLineCriticalValue=900, lightCriticalValue=512;
int oFlag=0,yFlag=0,bumpFlag = 0;
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
//	enable_servo(sensorLiftingServo);
	closeHand;
	msleep(200);
	printf("Servo initialized!\n");
}
void motorInit()
{
	twoMotors(100,7600);
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
	//create_stop();
    create_drive_direct(0,0);
}
void turnLeft(int degrees)
{
	int rad=0.5*pi;
	create_drive(50,0);
	msleep(rad*4000*degrees/90);
	//create_stop();
    create_drive_direct(0,0);
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
	msleep(150);
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
void goToHangerRack();
void putHangers();
void checkBump();
int main()
{
	int foundOrange=0,identColorReading;
	double cTime=0,sTime;
        thread bumpCheckTID;
        point2 point;
	printf("Program Begin!\n");
	create_connect(); // connect the create
	servoInit(); // Supply electricity to all the servo
	lightDetection();

    
	shut_down_in(120);
	printf("Start!\n");
	camera_open();
	display_clear();
	motorInit(); // set the motor position
	create_full(); // FULL mode, the create will execute commands under any circumstance
	printf("Create connected!\n Battery: %d\n",get_create_battery_charge());

        msleep(3000);

	create_drive_straight(100); // avoid scratching with the tube
	msleep(300);
	turnLeft(90); // adjust heading in startup area
	
	goToHangerRack(); // create move to the hanger rack
	putHangers(); // the "hand" put hangers on the rack

	create_drive_straight(-200); // drive away from hanger rack
        while (!blackLine()){}
        msleep(95);
        create_stop();
        //turnRight(103);
        turnRight(97);
	create_drive_straight(-200);
        msleep(500);
        create_stop();
        twoMotors(-100,2650);

    create_drive_straight(50); 
    sTime = seconds();
    cTime = seconds();
    set_servo_position(catchingServoPort,400);
    identColorReading = identColor();
    point = get_object_centroid(orangeChan,0);
    while (1)
    {
        //if (identColorReading >= 1020 && identColorReading <= 1100) break;
        if (identColorReading >= 500 && identColorReading <= 1700) break;
        cTime = seconds();
        if (cTime - sTime >= 12.2)
        {
            foundOrange = 0;
            break;
        }
        identColorReading = identColor();
        point = get_object_centroid(orangeChan,0);
    }
    if (cTime - sTime < 12.2) foundOrange = 1;
    set_servo_position(catchingServoPort,0); 
    if (foundOrange == 1)
    {
            create_drive_straight(100);
            msleep(5372);
            create_stop();
            twoMotors(100,1200);
            if (cTime - sTime <= 2.0) turnLeft(92);
            else turnLeft(90);
            create_drive_straight(-200);
            msleep(1200);
            create_stop();
	    twoMotors(-100,1250);
    }
    else
    {
        create_stop();
        twoMotors(-100,1200);
        sTime = seconds();
        cTime = seconds();
        identColorReading = identColor();
        point = get_object_centroid(orangeChan,0);
        create_drive_straight(-50);
        while (1)
            {
            if (identColorReading >= 1200 && identColorReading <= 1660)
            {
                if (point.x - 60 <= 5) break;
            }
            identColorReading = identColor();
            point = get_object_centroid(orangeChan,0);
        }
        create_drive_straight(100);
        msleep(4560);
        create_stop();
        motor(liftingMotorPort1,100);
        motor(liftingMotorPort2,100);
        turnLeft(90);
        ao();
        create_drive_straight(-200);
        msleep(1830);
        create_stop();
        twoMotors(-100,2800);
    }
    set_servo_position(catchingServoPort,2000);
	msleep(200);
    if (foundOrange == 1)
    {
        twoMotors(100,1100);
        create_drive_straight(200);
	    msleep(1500);
    }
    if (foundOrange == 0)
    {
        twoMotors(100,1950);
        create_drive_straight(200);
        msleep(2000);
        create_stop();
    }
    turnRight(299);
    
    create_stop();
	
    create_drive_straight(100); // Increase the speed steadily
	msleep(100);
	create_drive_straight(150);
	msleep(100);
	create_drive_straight(250);
	msleep(100);
	create_drive_straight(500);
    msleep(2500);
    create_drive_straight(200);
	
	while (!get_create_lbump() && !get_create_rbump()){}
	create_drive_straight(500);
    msleep(500);
    create_drive_straight(-200);
	msleep(100);
	create_stop();
	turnLeft(93); // turn toward the containers
    create_drive_straight(200);
    while (!get_create_lbump() && !get_create_rbump()) {}
	create_drive_straight(-200);
	msleep(2300);
	create_stop();
	twoMotors(-100,2000);
	openHand;
    msleep(100);
    twoMotors(100,2000);

	return 0;
}
void goToHangerRack()
{

        double cTime,sTime,tTime;
        thread bumpCheckTID;
        bumpCheckTID = thread_create(checkBump);
	printf("Going to the hanger rack!\n");
	create_drive_straight(200);
	while(!blackLine()){}
	//create_stop();
        create_drive_direct(0,0);
	msleep(200);
	create_drive_straight(-200);
	msleep(200);
	turnLeft(90);
	create_drive_straight(200);
        thread_start(bumpCheckTID);
        cTime = seconds();
        sTime = seconds();
        tTime = 2.2;
        while (cTime - sTime <= tTime)
        {
                cTime = seconds();
                if (bumpFlag == 0) continue;
            if (bumpFlag == 1)
            {
                bumpFlag = 0;
                create_drive_straight(400);
                msleep(100);
                create_drive_straight(200);
                tTime += 0.2;
            }
        }
        thread_destroy(bumpCheckTID);
	//msleep(2200);
	turnRight(98);
	//create_stop();
        create_drive_direct(0,0);
}
void putHangers()
{
    	printf("Start putting hangers!\n");
	twoMotors(-100,450);
	//create_drive_straight(200)
	//msleep(910);
	create_drive_straight(100);
        msleep(1835);
        //create_stop();
        create_drive_direct(0,0);
        msleep(100);
        twoMotors(-100,50);
        msleep(150);
	openHand; // open the "hand" to put the hangers
	msleep(200);
	twoMotors(100,250);
}
void checkBump()
{
    while (1)
    {
        if (get_create_lbump() || get_create_rbump()) bumpFlag = 1;              
        msleep(100);
    }
}
