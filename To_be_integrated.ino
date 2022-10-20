//leave the box by going straight for a certain amount of time

state.line_offset = [offset_dirs.none, offset_exts.none];
set_motor_speed(true, speeds.high); //right motor
set_motor_speed(false, speeds.high); //left motor

int period = 1000;// needs to be calibrated
unsigned long time_now = 0;

void leave_start() {
    state.line_offset = [offset_dirs.none, offset_exts.none];
    set_motor_speed(true, speeds.high); //right motor
    set_motor_speed(false, speeds.high); //left motor
    delay(period)

    //reverse until the side sensor detects the line
    L_motor->run(BACKWARD);
    R_motor->run(BACKWARD);

    side_sensor = digitalRead(side_sensor_pin)
    while (side_sensor != 1){
        set_motor_speed(true, speeds.med); //right motor
        set_motor_speed(false, speeds.med); //left motor
    }
    
    //turns right until line detected
    //set direction to turn right
    L_motor->run(RELEASE);
    R_motor->run(RELEASE);
    L_motor->run(FORWARD);
    R_motor->run(BACKWARD);
        
    //while no front sensors are firing
    while (!(sensors[0]) && !(sensors[1]) && !(sensors[2])){
        set_motor_speed(true, speeds.high); //right motor
        set_motor_speed(false, speeds.high); //left motor
    }

    //start normal mode and update current mode
    L_motor->run(FORWARD);
    R_motor->run(FORWARD);

    set_mode(modes.following_line);

}

// to turn 90 degrees in one way for either leaving box or when object is located
void turning_on_spot(direction){
    if (direction == left){
        //set direction to turn left
        L_motor->run(BACKWARD);
        R_motor->run(FORWARD);
    }
    else if (direction == right){
        //set direction to turn right
        L_motor->run(FORWARD);
        R_motor->run(BACKWARD);
    }
    
    int turn_period = 50 // need to calibrate
    set_motor_speed(true, speeds.high); //right motor
    set_motor_speed(false, speeds.high); //left motor
    delay(turn_period) // not gonna be doing anything apart form turning if turning on spot anyway
}




//copy to top of file and test val2 as needed
/*
========================================================================
*/
#include <Servo.h>
#define echoPin 13

Servo myservo;
float duration, distance;
int val1 = 0;
int val2 = 0;
/*
========================================================================
*/











//attach servo in set up
/*
========================================================================
*/
myservo.attach(9);
/*
========================================================================
*/










//add drop_grabber to enum list and copy into main body
/*
========================================================================
*/
void drop_grabber() {
  myservo.write(val1);
  delay(3000);
  myservo.write(val2);
  delay(3000);
}
/*
========================================================================
*/


//copy to top of file and change names as needed
/*
========================================================================
*/

#define echoPin 3 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 2 //attach pin D3 Arduino to pin Trig of HC-SR04

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

/*
========================================================================
*/

//attach ultrasonic in set up
/*
========================================================================
*/
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
}



//add ultrasonic_distance to enum list and copy into main body
/*
========================================================================
*/

void ultrasonic_distance() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance in cm: ");
  Serial.print(distance);
  return distance
}
