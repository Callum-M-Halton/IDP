#include <Arduino.h>

// checks if any of the front 3 sensors are returning 1
bool any_front_line_sensors_firing() {
  return digitalRead(front_sensor_pins.left) 
    || digitalRead(front_sensor_pins.mid)
    || digitalRead(front_sensor_pins.right);
}

// outputs front sensor values for testing purposes
void print_sensor_vals() {
  Serial.print("\nFront Line Sensor Values: ");
  Serial.print(digitalRead(front_sensor_pins.left));
  Serial.print(digitalRead(front_sensor_pins.mid));
  Serial.println(digitalRead(front_sensor_pins.right));
}

// storing the state of the motors: speed, directions and time stamps
void add_motor_cmd() {
  motor_cmd_struct motor_cmd = {
    {state.motor_dirs[0], state.motor_dirs[1]},
    {state.motor_speeds[0], state.motor_speeds[1]},
    millis()
  };
  state.motor_cmds.add(motor_cmd);
  if (state.motor_cmds.size() > MOTOR_CMDS_SIZE) {
    state.motor_cmds.shift();
  }
}

// This function changes the direction of motor and then also records the state of the motor so that it can be reversed if needed
void set_motor_dir(bool is_right, int dir) {
  if (state.motor_dirs[int(is_right)] != dir) {
    state.motor_dirs[int(is_right)] = dir;
    if (is_right) {
      R_motor->run(dir);
    } else {
      L_motor->run(dir);
    }
    if (state.recording) { add_motor_cmd(); }
  }
}

// sets direction for BOTH motors at once for e.g. reversing
void set_motor_dirs(int dir) {
  set_motor_dir(false, dir);
  set_motor_dir(true, dir);
}

// controls the speed of ONE motor, accounting for power discrepencies, then stores the state of the motors and flashes the amber led as needed
void set_motor_speed(bool is_right, int speed, bool raw=false) {

  // if not raw speeds wanted underdrives right motor to match left motor
  if (!raw) {
    if (state.motor_dirs[0] == BACKWARD && state.motor_dirs[1] == BACKWARD) {
      if (is_right) { speed *= 0.95; } // CHECK
    } else if (is_right) { speed *= 0.9; }
  }

	if (state.motor_speeds[int(is_right)] != speed) {
    state.motor_speeds[int(is_right)] = speed;
    if (is_right) {
      R_motor -> setSpeed(speed);
    } else {
      L_motor -> setSpeed(speed);
    }
    if (state.recording) { add_motor_cmd(); }
    flash_amber();
	}
}

// controlling the motor speeds for BOTH motors
void set_motor_speeds(int speed) {
  set_motor_speed(false, speed);
  set_motor_speed(true, speed);
}

// rotates without traversing -> sets the motors to rotate in opposite directions (to_right depedent) at full speed to turn
void turn_on_spot(bool to_right) {
    if (to_right) {
        // set direction to turn right
        set_motor_dir(false, FORWARD);
        set_motor_dir(true, BACKWARD);
    } else {
        // set direction to turn left
        set_motor_dir(true, FORWARD);
        set_motor_dir(false, BACKWARD);
    }
    
    set_motor_speed(true, 255); //right motor
    set_motor_speed(false, 255); //left motor
}

// performs the magentic test and then outputs the result as required through LEDs in the spec
bool test_if_magnetic() {
  Serial.println("Testing block with hall sensor");
  int tot = 0;
  
  // get's an average of reading to account for anomolous results
  for(int i = 0; i < HALL_EFFECT_SAMPLE_LENGTH; i++) {
      tot += analogRead(HALL_SENSOR_PIN);
  } 
  int avg = tot / HALL_EFFECT_SAMPLE_LENGTH;
  if (millis() % 2 == 0 /*avg > HALL_SENSOR_THRESHOLD*/) {
    //turn on red light for 5 sec if magnetic
    digitalWrite(RED_LED_PIN, HIGH);
    my_delay(5000);
    digitalWrite(RED_LED_PIN, LOW);
    // box is magnetic
    return true;
  } else {
    //turn on green light for 5 sec if not magnetic
    digitalWrite(GREEN_LED_PIN, HIGH);
    my_delay(5000);
    digitalWrite(GREEN_LED_PIN, LOW);
    //box is not magnetic
    return false;
  }
}

// rotates the grabbing mechanism down to capture block
void lower_grabber() {
    myservo.write(DROP_GRABBER_VALUE);
    my_delay(1000);
}

// raises the grabbing mechanics back above the ultrasonic sensor
void raise_grabber() {
    myservo.write(RAISE_GRABBER_VALUE);
    my_delay(1000);
}

// flashes amber light when moving
void flash_amber() {
  // if stationary the amber LED is off
  if (state.motor_speeds[0] == 0 && state.motor_speeds[1] == 0) {
    digitalWrite(AMBER_LED_PIN, LOW);
  } else {
    // blinks at a rate of 2Hz (once every 0.5 seconds) by blinking in the first half of each half second increment
    if (millis() % 500 < 250) {
      digitalWrite(AMBER_LED_PIN, HIGH);
    } else {
      digitalWrite(AMBER_LED_PIN, LOW);
    }
  }
}

// facilitates flashing during a millisecond delay
void my_milli_delay() {
  flash_amber();
  delay(1);
}

// A delay that enables flash_amber() to be called while it happens 
void my_delay(int delay) {
  unsigned long timer_end = millis() + delay;
  while (millis() < timer_end) {
    my_milli_delay();
  }
}

// Gets the distance in front of the specified US sensor in cm
int get_ultrasonic_distance(bool is_front) {
  int trig_pin;
  int echo_pin;
  if (is_front) {
    trig_pin = FRONT_TRIG_PIN;
    echo_pin = FRONT_ECHO_PIN;
  } else {
    trig_pin = SIDE_TRIG_PIN;
    echo_pin = SIDE_ECHO_PIN;
  }
    // Clears the trig_pin
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  // Sets the trig_pin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  // Reads the ECHO_PIN, returns the sound wave travel time in microseconds
  long duration = pulseIn(echo_pin, HIGH);
  // Calculating the distance
  int distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.println("Distance Read: " + String(distance));
  return distance;
}
