#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    clawPot2,       sensorPotentiometer)
#pragma config(Sensor, in2,    clawPot1,       sensorPotentiometer)
#pragma config(Sensor, dgtl1,  liftBottom,     sensorTouch)
#pragma config(Sensor, dgtl2,  autonRL,        sensorTouch)
#pragma config(Sensor, I2C_1,  driveR,         sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  driveL,         sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  liftEncoder,    sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           liftL3,        tmotorVex393HighSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           leftDrive,     tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port3,           rightDrive,    tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port4,           intake2,       tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           intake1,       tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port6,           liftL2,        tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           liftL1,        tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_3)
#pragma config(Motor,  port8,           liftR2,        tmotorVex393HighSpeed_MC29, openLoop, driveRight)
#pragma config(Motor,  port9,           liftR1,        tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          liftR3,        tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// This code is for the VEX cortex platform
#pragma platform(VEX2)

// Select Download method as "competition"
#pragma competitionControl(Competition)

//Main competition background code...do not modify!
#include "Vex_Competition_Includes.c"
float desiredClaw1 = 0;
float desiredClaw2 = 0;
float desiredLift = 3340;

void lift(int val){
	motor[liftL1] = motor[liftL2] =motor[liftL3] = motor[liftR1] = motor[liftR2] = motor[liftR3] = val;
}

task driveControl(){
	while(true){
    motor[leftDrive] = abs(vexRT[Ch3])<10?0:vexRT[Ch3];
  	motor[rightDrive] = abs(vexRT[Ch2])<10?0:vexRT[Ch2];
	}
}

int limit(int val, int min = -127, int max = 127){
	if(val>max){
		return max;
	}
	if(val<min){
		return min;
	}
	return val;
}

task liftControl(){
	bool liftMoving = false;
  float kP = 0.7;
	float kI = 0.01;
	float kD = 0.7;
  float error;
  float integral;
  float derivative;
  float lastError;
  float lastSensorValue;
	desiredLift = SensorValue[liftEncoder];

  while(true) {
    if(vexRT[Btn5U]) {
    	liftMoving = true;
			desiredLift -= 30;
		}else if (vexRT[Btn5D]) {
			liftMoving = true;
			if(SensorValue[liftBottom])
				desiredLift = SensorValue[liftEncoder];
			else
				desiredLift += 30;
		} else if(liftMoving) {
			liftMoving = false;
			desiredLift = SensorValue[liftEncoder];
		}


    error = SensorValue[liftEncoder] - desiredLift;

		integral += error;
		if(error == 0 || error>200 || error<-200)
			integral = 0;
		else if(error>1000 || error<-1000)
			error = 1000;
		if(derivative==0 && integral==0)
			desiredLift = SensorValue[liftEncoder];
		if(integral>1500)
			integral = 1500;

		derivative = error - lastError;

		lastError = error;

		lastSensorValue = SensorValue[liftEncoder];

		lift((kP*error + kI*integral + kD*derivative));//*!SensorValue(liftBottom));

		delay(15);
	}
	/**while(true){
		if(vexRT[Btn5U]){
			lift(127);
		}
		else if(vexRT[Btn5D]){
			lift(-127);
		}
		else{
			lift(10);
		}
	}*/
}

task clawDoubleControl(){
  while (true){
    if(!(SensorValue(clawPot1)<=SensorValue(clawPot2)-50 && SensorValue(clawPot1)>=SensorValue(clawPot2)+50)){
      desiredClaw1=desiredClaw2;
    }
    delay(5);
  }
}

typedef struct {
	float kP;
	float kI;
	float kD;
	float target;
	float error;
	float integral;
	float derivative;
	float lastError;
	float threshold;
} Claw;

Claw leftClaw;
Claw rightClaw;

void clawInit (float kP, float kI, float kD) {
	leftClaw.kP = kP;
	rightClaw.kP = kP;

	leftClaw.kI = kI;
	rightClaw.kI = kI;

	leftClaw.kD = kD;
	rightClaw.kD = kD;
}

void clawUpdate (Claw *claw, tSensors sensor, tMotor clawMotor) {
	claw->error = claw->target - SensorValue[sensor];

	claw->integral = claw->integral + claw->error;

	if(claw->error == 0)
		claw->integral = 0;
	if(abs(claw->error)<25)
		claw->integral = 0;

	claw->derivative = claw->error - claw->lastError;

	claw->lastError = claw->error;

	motor[clawMotor] = claw->kP*claw->error + claw->kI*claw->integral + claw->kD*claw->derivative;
}



void clawSetDegrees (int deg) {
	leftClaw.target = -0.0352*pow(deg,2) + 23.536*deg + 194.32;
	rightClaw.target = -0.0302*pow(deg,2) + 21.176*deg + 268.19;
}

float clawValuesToDegrees () {
	return 334.318-0.227273*sqrt(2270720-550*leftClaw.target);
}

float degrees;
task clawControl () {
	clawInit(0.1, 0.0003, 0.3);

	bool movingIn = false;
	bool moving = false;

	degrees = 90;
	
	leftClaw.target = SensorValue[clawPot1];
	rightClaw.target = SensorValue[clawPot2];

	while(true) {
		if(vexRT[Btn6U]) {
			degrees = degrees>240?240:degrees+2;
			moving = true;
			movingIn = true;
		} else if(vexRT[Btn6D]) {
			degrees-=2;
			moving = true;
			movingIn = false;
		} else if(movingIn) {
			degrees = clawValuesToDegrees();
			degrees = degrees>240?240:degrees;
			movingIn = false;
			moving = false;
		} else if(moving) {
			degrees = clawValuesToDegrees();
			leftClaw.integral = 0;
			rightClaw.integral = 0;
			moving = false;
		}
		
		clawSetDegrees(degrees);
		
		clawUpdate(leftClaw, clawPot1, intake1);
		clawUpdate(rightClaw, clawPot2, intake2);

		delay(10);
	}
}

//bool clawMovingOut1 = false;
//bool clawInMotion1 = false;
//task clawControl1(){
//  float Kp = 0.15;
//	float Ki = 0.008;
//	float Kd = 1.2;
//  float error;
//  float integral;
//  float derivative;
//  float previous_error;
//	desiredClaw1 = SensorValue[clawPot1];

//  while(true) {
//    if(vexRT[Btn6U]) {
//			if(clawMovingOut1)
//				desiredClaw1 = SensorValue[clawPot1];
//			desiredClaw1 -= 70;
//			clawMovingOut1 = false;
//			clawInMotion1 = true;
//		} else if (vexRT[Btn6D]) {
//			if(!clawMovingOut1)
//				desiredClaw1 = SensorValue[clawPot1];
//			desiredClaw1 += 70;
//			clawMovingOut1 = true;
//			clawInMotion1 = true;
//		} else if (clawInMotion1) {
//			clawInMotion1 = false;
//			desiredClaw1 = SensorValue[clawPot1]-((!clawMovingOut1)*70);
//		}

//    error = (desiredClaw1) - (SensorValue[clawPot1]);
//    integral = integral + error;
//    if (error == 0) {
//      integral = 0;
//    }
//    if ( abs(error) > 40) {
//      integral = 0;
//    }
//    derivative = error - previous_error;
//    previous_error = error;
//    motor[intake1] = Kp*error + Ki*integral + Kd*derivative;
//  }
//}

//bool clawMovingOut2 = false;
//bool clawInMotion2 = false;
//task clawControl2(){
//  float Kp = 0.15;
//	float Ki = 0.008;
//	float Kd = 1.2;
//  float error;
//  float integral;
//  float derivative;
//  float previous_error;
//	desiredClaw2 = SensorValue[clawPot2];

//  while(true) {
//    if(vexRT[Btn6U]) {
//			if(clawMovingOut2)
//				desiredClaw2 = SensorValue[clawPot2];
//			desiredClaw2 -= 70;
//			clawMovingOut2 = false;
//			clawInMotion2 = true;
//		} else if (vexRT[Btn6D]) {
//			if(!clawMovingOut2)
//				desiredClaw2 = SensorValue[clawPot2];
//			desiredClaw2 += 70;
//			clawMovingOut2 = true;
//			clawInMotion2 = true;
//		} else if (clawInMotion2) {
//			clawInMotion2 = false;
//			desiredClaw2 = SensorValue[clawPot2]-((!clawMovingOut2)*70);
//		}

//    error = (desiredClaw2) - (SensorValue[clawPot2]);
//    integral = integral + error;
//    if (error == 0) {
//      integral = 0;
//    }
//    if ( abs(error) > 40) {
//      integral = 0;
//    }
//    derivative = error - previous_error;
//    previous_error = error;
//    motor[intake2] = Kp*error + Ki*integral + Kd*derivative;
//  }
//}

void resetEncoders() {
	nMotorEncoder[rightDrive]=0;
	nMotorEncoder[leftDrive]=0;
	nMotorEncoder[liftEncoder]=0;
}

void drive(int power){
	motor[leftDrive]=motor[rightDrive]=power;
}

void autonR(){
	/*startTask(clawControl);
	startTask(liftControl);
	desiredClaw1=1900;
  desiredClaw2=1900;
	desiredLift=50;
	while(nMotorEncoder(rightDrive)<150){
		drive(127);
		delay(3);
	}
	drive(0);
  desiredClaw1=1900;
  desiredClaw2=1900;
	desiredLift=0;
	while(nMotorEncoder(rightDrive)<180){
		drive(127);
		delay(3);
	}
	drive(0);
  desiredClaw1=1900;
  desiredClaw2=1900;
	desiredLift=75;
	while(nMotorEncoder(rightDrive)<225){
		drive(127);
		delay(3);
	}
	drive(0);
	while(nMotorEncoder(leftDrive)>160){
		motor[leftDrive]=-127;
		delay(3);
	}
	while(nMotorEncoder(rightDrive)<275){
		drive(127);
		delay(3);
	}
	drive(0);
	motor[leftDrive]=0;
	desiredLift=500;
	wait1Msec(250);
	desiredClaw=500;
	wait1Msec(75);
	desiredLift=0;
  desiredClaw1=1900;
  desiredClaw2=1900;
	while(nMotorEncoder(rightDrive)<325){
		motor[rightDrive]=127;
		delay(3);
	}
	motor[rightDrive]=0;
	while(nMotorEncoder(rightDrive)<450){
		drive(127);
		delay(3);
	}
	while(nMotorEncoder(leftDrive)<325){
		motor[leftDrive]=127;
		delay(3);
	}
	motor[leftDrive]=0;
	while(nMotorEncoder(rightDrive)<550){
		drive(127);
		delay(3);
	}
	drive(0);
  desiredClaw1=1900;
  desiredClaw2=1900;
	wait1Msec(150);
	desiredLift=50;
	wait1Msec(100);
	while(nMotorEncoder(leftDrive)<450){
		motor[leftDrive]=127;
		delay(3);
	}
	motor[leftDrive]=0;
	while(nMotorEncoder(rightDrive)<700){
		drive(127);
		delay(3);
	}
	drive(0);
  desiredClaw1=1900;
  desiredClaw2=1900;
	wait1Msec(250);
  desiredClaw1=1900;
  desiredClaw2=1900;
	wait1Msec(75);
	desiredLift=0;
  desiredClaw1=1900;
  desiredClaw2=1900;*/
}

void autonL(){
	//add this by changing sides
  wait1Msec(100);
}

void pre_auton() {
	resetEncoders();
}

task autonomous {
	/**
	lift(127);
	wait1Msec(200);
	lift(0);
	drive(-127);
	wait1Msec(100);
	drive(0);
	motor[intake1]=127;
	motor[intake2]=127;
	wait1Msec(500);
	motor[intake1]=0;
	motor[intake2]=0;
	lift(127);
	wait1Msec(50);
	lift(0);
	drive(-127);
	wait1Msec(1100);
	drive(10);
	lift(127);
	wait1Msec(700);
	motor[intake1]=127;
	motor[intake2]=127;
	wait1Msec(300);
	drive(0);
	motor[intake1]=0;
	motor[intake2]=0;
	*/
	autonR();
}

task usercontrol(){
	startTask(clawControl);
	startTask(clawDoubleControl);
	startTask(liftControl);
	startTask(driveControl);
}