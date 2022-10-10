//install AFMOTOR library 
//go to sketch, include library, add .ZIP library, select afmotor zip file

//including the libraries
#include <AFMotor.h>

//defining pins and variables
#define left A0
#define right A1

//defining motors
AF_DCMotor motor1(1, MOTOR12_1KHZ); 
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);



void setup() {
  //declaring pin types
  pinMode(left,INPUT);
  pinMode(right,INPUT);
  //begin serial communication
  Serial.begin(9600);
  
}

void loop(){
    motor1.run(FORWARD);
    motor1.setSpeed(200);
    motor2.run(FORWARD);
    motor2.setSpeed(200);
    motor3.run(FORWARD);
    motor3.setSpeed(200);
    motor4.run(FORWARD);
    motor4.setSpeed(200);


    //motor1.run(BACKWARD);
    //motor1.setSpeed(200);
    //motor2.run(BACKWARD);
    //motor2.setSpeed(200);
    //motor3.run(BACKWARD);
    //motor3.setSpeed(200);
    //motor4.run(BACKWARD);
    //motor4.setSpeed(200);
}
