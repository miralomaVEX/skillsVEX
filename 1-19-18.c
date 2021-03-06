#pragma config(Sensor, in1,    gyrosensor,     sensorAnalog)
#pragma config(Sensor, in3,    lineFollower,   sensorLineFollower)
#pragma config(Sensor, dgtl1,  FRQE,           sensorQuadEncoder)
#pragma config(Sensor, dgtl4, limitswitch, sensorDigitalIn)
#pragma config(Sensor, dgtl5, touchsensor, sensorDigitalIn)
#pragma config(Sensor, dgtl6,  BRQE,           sensorQuadEncoder)
#pragma config(Sensor, dgtl8,  FLQE,           sensorQuadEncoder)
#pragma config(Sensor, dgtl10, BLQE,           sensorQuadEncoder)
#pragma config(Sensor, dgtl12, mobileGoalShooter, sensorDigitalOut)
#pragma config(Motor,  port2,           MGLeft,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           MGRight,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           FRDrive,       tmotorVex393_MC29, openLoop, reversed, encoderPort, dgtl1)
#pragma config(Motor,  port5,           BRDrive,       tmotorVex393_MC29, openLoop, reversed, encoderPort, dgtl6)
#pragma config(Motor,  port6,           FLDrive,       tmotorVex393_MC29, openLoop, encoderPort, dgtl8)
#pragma config(Motor,  port7,           BLDrive,       tmotorVex393_MC29, openLoop, encoderPort, dgtl10)
#pragma config(Motor,  port8,           armMiddle,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           armTop,        tmotorVex393_MC29, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//
#pragma platform(VEX)
//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(60)
#pragma userControlDuration(60)
#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
#include "NERD_PID.c"
#include "NERD_Gyro.c"

//Auton code and functions

PID pid;
Gyro gyro;

float angleUpdate;
void pre_auton()
{
  bStopTasksBetweenModes = true;
  gyroInit(gyro, in1, 'y');
  pidInit(pid, 1.75, 0, 0, 0, 0);
}

//drives using quadencoder values
void driveQuadencoder(int target){
	resetMotorEncoder(FLDrive);
	resetMotorEncoder(FRDrive);
	resetMotorEncoder(BLDrive);
	resetMotorEncoder(BRDrive);
	int FRValue = getMotorEncoder(FRDrive);
	int FLValue = getMotorEncoder(FLDrive);
	int BRValue = getMotorEncoder(BRDrive);
	int BLValue = getMotorEncoder(BLDrive);
	while (fabs(-FRValue - target) > 10 && fabs(FLValue - target) > 10){
		motor[FLDrive] = pidCalculate(pid, target, FLValue);
		motor[FRDrive] = pidCalculate(pid, target, -FRValue);
		motor[BLDrive] = pidCalculate(pid, target, BLValue);
		motor[BRDrive] = pidCalculate(pid, target, -BRValue);
		FRValue = getMotorEncoder(FRDrive);
		FLValue = getMotorEncoder(FLDrive);
	  BRValue = getMotorEncoder(BRDrive);
		BLValue = getMotorEncoder(BLDrive);
	}
	motor[FLDrive] = 0;
	motor[FRDrive] = 0;
	motor[BLDrive] = 0;
	motor[BRDrive] = 0;
}

//use only for short distances, drive based on time
void driveNoEncoder (int count, char direction) {
	int mp;
	if (direction == 'r') {
			mp = -1;
	} else {
		mp = 1;
	}
	for (int i = 0; i < count; i++) {
		motor[FLDrive] = mp * 60;
		motor[FRDrive] = mp * 60;
		motor[BLDrive] = mp * 60;
		motor[BRDrive] = mp * 60;
		wait1Msec(1);
	}
	motor[FLDrive] = 0;
	motor[FRDrive] = 0;
	motor[BLDrive] = 0;
	motor[BRDrive] = 0;
}

//function to keep track of current robot angle
long pastTime = nPgmTime;
float currentAngle = 0;
float getRobotAngle() {
		long currentTime = nPgmTime;
		float rate = gyroGetRate(gyro);
		float deltaTime = (float) (currentTime - pastTime)/1000.0;
		currentAngle += deltaTime * rate;
		pastTime = currentTime;
		return currentAngle;
}

//turns using an angle
void gyroTurn(float angle){
	int m = 1;
	if (angle < 0){
		m = -1;
	}
	pastTime = nPgmTime;
	currentAngle = 0;
	//angleUpdate is defined in the task
	float currAngle = angleUpdate;
	while(fabs(currAngle) < fabs(angle)){
		currAngle = angleUpdate;
		if(fabs(angle - currAngle) > 20){
			motor[FLDrive] = m * 70;
			motor[FRDrive] = m * -70;
			motor[BLDrive] = m * 70;
			motor[BRDrive] = m * -70;
		}
		else{
			motor[FLDrive] = m * 30;
			motor[FRDrive] = m * -30;
			motor[BLDrive] = m * 30;
			motor[BRDrive] = m * -30;
		}
		writeDebugStreamLine("Gyro angle = %d", currAngle);
	}
	motor[FLDrive] = 0;
	motor[FRDrive] = 0;
	motor[BLDrive] = 0;
	motor[BRDrive] = 0;
}

void shootCone(){
	SensorValue[mobileGoalShooter] = 1;
	wait1Msec(50);
	SensorValue[mobileGoalShooter] = 0;
}

void deployMotorLiftF(){
	while(SensorValue[touchsensor] == 1){
		motor[MGLeft] = -127;
		motor[MGRight] = -127;
		wait1Msec(5);
	}
	motor[MGLeft] = 0;
	motor[MGRight] = 0;
}

void retractMotorLiftF(){
	while(SensorValue[limitswitch] == 1){
		motor[MGLeft] = 127;
		motor[MGRight] = 127;
		wait1Msec(3);
	}
	motor[MGLeft] = 0;
	motor[MGRight] = 0;
}

task deployMotorLift (){
	deployMotorLiftF();
	stopTask(deployMotorLift);
}

task retractMotorLift (){
	retractMotorLiftF();
	stopTask(retractMotorLift);
}

//used to constantly update the current angle for gyro turn to use
task getAngle (){
	while(true) {
		wait1Msec(10);
		angleUpdate = getRobotAngle();
	}
}

task autonomous()
{
	//startTask(liftTest);
	startTask(getAngle);
	SensorValue[gyrosensor] = 0;
	writeDebugStreamLine("Finished init");
	wait1Msec(100);
	startTask(deployMotorLift);
	driveQuadencoder(870);
	wait1Msec(50);
	retractMotorLiftF();
	wait1Msec (50);
	driveQuadencoder(-895);
	wait1Msec (100);
	gyroTurn(150);
	wait1Msec (100);
	driveQuadencoder(40);
	wait1Msec(25);
	gyroTurn(50);
	wait1Msec(50);
	shootCone();
	wait1Msec(25);
	//gyroTurn(-70);
	wait1Msec(25);
	driveQuadencoder(-350);
	wait1Msec(50);
	gyroTurn(70);
	wait1Msec(25);
	driveNoEncoder(800, 'r');
	wait1Msec(100);
	startTask (deployMotorLift);
	wait1Msec(350);
	driveQuadencoder(550);
	wait1Msec(50);
	retractMotorLiftF();
	wait1Msec(50);
	gyroTurn(-90);
	wait1Msec(25);
	driveQuadencoder(700);
	wait1Msec(25);
	shootCone();
	wait1Msec(25);
	//driveQuadencoder(-100, 1);

}


//User control functions
void driveUser(){
	motor[FLDrive] = vexRT[Ch3] + vexRT[Ch1];
	motor[BLDrive] = vexRT[Ch3] + vexRT[Ch1];
	motor[FRDrive] = vexRT[Ch3] - vexRT[Ch1];
	motor[BRDrive] = vexRT[Ch3] - vexRT[Ch1];
}
void mobileGoal(){
	if (vexRT[Btn6U] == 1 && vexRT[Btn6D] == 0 && SensorValue[limitswitch] == 1){
		motor[MGLeft] = 127;
		motor[MGRight] = 127;
	} else if (vexRT[Btn6U] == 0 && vexRT[Btn6D] == 1 && SensorValue[touchsensor] == 1){
		motor[MGLeft] = -127;
		motor[MGRight] = -127;
	} else {
		motor[MGLeft] = 0;
		motor[MGRight] = 0;
	}
}
void pushCone(){
	if (vexRT[Btn8L] == 1 && vexRT[Btn8R] == 0){
		SensorValue[mobileGoalShooter] = 1;
	} else if (vexRT[Btn8L] == 0 && vexRT[Btn8R] == 1){
		SensorValue[mobileGoalShooter] = 0;
	}
}
task usercontrol()
{
	while (true)
	{
		driveUser();
		mobileGoal();
		pushCone();
	}
}
