#include <Arduino.h>

// pins (not settled)
const byte JUNCT_SENSOR_PIN = 3;
const byte ERROR_LED_PIN = 4;
const byte GREEN_LED_PIN = 10;
const byte RED_LED_PIN = 11;
const byte AMBER_LED_PIN = 12;
const byte SERVO_PIN = 9; //has to go in 9 or 10 (servo specific pins)
const int HALL_SENSOR_PIN = A1;
struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {0,1,2};

// other
const int MOTOR_CMDS_SIZE = 100;
const int TURN_DISPARITIES_SAMPLE_LENGTH = 100; //TUNE
const int DISPARITY_SAMPLE_PERIOD = 20;
const int CURVING_LEFT_THRESHOLD = 0; //TUNE
const int GOING_STRAIGHT_THRESHOLD = 75; //TUNE
const int TIME_TO_DRIVE_FORWARD_FOR_AT_START = 2800; //TUNE
const int TIME_TO_DRIVE_FORWARD_TO_GO_HOME = 1000; //TUNE
const int TIME_TO_DRIVE_FORWARD_TO_DROP_BLOCK = 500; // TUNE
unsigned long TIME_BEFORE_HEADING_BACK = 240000; // TUNE (4 minutes)
const int HALL_SENSOR_THRESHOLD = 161; // MAYBE TUNE?
const int HALL_EFFECT_SAMPLE_LENGTH = 5; // MAYBE TUNE?
const float BOX_APPROACH_COEFF = 0.3; // MAYBE TUNE?
/*const int TIME_FOR_180_TURN = 1000; // TUNE*/
const int DROP_GRABBER_VALUE = 10; //TUNE
const int RAISE_GRABBER_VALUE = 70; //TUNE
struct speeds_struct {int tiny; int low; int med; int high; int tunnel;};
speeds_struct speeds = {0, 50, 150, 255};
struct stblil_struct {int none; int as_before; int high; int med; int low; };
stblil_struct suggested_timers_by_line_end_likelihoods = { -2, -1, 1000, -7, -7 };
// -7 as irrelevant for current implementation
