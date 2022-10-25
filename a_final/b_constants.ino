#include <Arduino.h>

// pins (not settled)
const int JUNCT_SENSOR_PIN = 3;
struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {0,1,2};
const int ERROR_LED_PIN = 4;
//const int GREEN_LED_PIN = 10;
//const int RED_LED_PIN = 11;
const int AMBER_LED_PIN = 12;
const int SERVO_PIN = 5;

US_pins_struct front_US_pins = {6, 7}; // for HC-SR04
US_pins_struct side_US_pins = {8, 9}; // for HC-SR04
// other
const int MOTOR_CMDS_SIZE = 100;
const int TURN_DISPARITIES_SAMPLE_LENGTH = 100; //TUNE
const int CURVING_LEFT_THRESHOLD = 10000; //TUNE
const int GOING_STRAIGHT_THRESHOLD = 50; //TUNE
const int TIME_TO_DRIVE_FORWARD_FOR_AT_START = 3000; //TUNE
const int TIME_TO_DRIVE_FORWARD_TO_GO_HOME = 1000; //TUNE
const int TIME_TO_DRIVE_FORWARD_TO_DROP_BLOCK = 500; // TUNE
unsigned long TIME_BEFORE_HEADING_BACK = 240000 // TUNE (4 minutes)
/*const int TIME_FOR_180_TURN = 1000; // TUNE*/
const int DROP_GRABBER_VALUE = 90; //TUNE
const int RAISE_GRABBER_VALUE = 0; //TUNE
struct speeds_struct {int tiny; int low; int med; int high; int tunnel;};
speeds_struct speeds = {0, 50, 150, 255, 100};
struct stblil_struct {int none; int as_before; int high; int med; int low; };
stblil_struct suggested_timers_by_line_end_likelihoods = { -2, -1, 1000, 10000, 10000 };