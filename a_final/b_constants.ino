#include <Arduino.h>

// pins (not settled)
struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins = {0,1,2};
const byte JUNCT_SENSOR_PIN = 3;
const byte FRONT_TRIG_PIN = 4;
const byte FRONT_ECHO_PIN = 5;
const byte SIDE_TRIG_PIN = 6;
const byte SIDE_ECHO_PIN = 7;
const byte BUTTON_PIN = 8;
const byte SERVO_PIN = 9; //has to go in 9 or 10 (servo specific pins)
const byte GREEN_LED_PIN = 11;
const byte RED_LED_PIN = 12;
const byte AMBER_LED_PIN = 13;
const int HALL_SENSOR_PIN = A0;

// other
const int DROP_GRABBER_VALUE = 10;
const int RAISE_GRABBER_VALUE = 70;
const int MOTOR_CMDS_SIZE = 100;
const bool GO_VIA_RAMP = false;
const int HALL_SENSOR_THRESHOLD = 500; // MAYBE TUNE?
const int HALL_EFFECT_SAMPLE_LENGTH = 10; // MAYBE TUNE?
const int LOST_LINE_TIMER_LENGTH = 500;

struct speeds_struct {
    const int tiny; const int low; const int med; const int high;
};
constexpr speeds_struct speeds = {0, 50, 150, 255};

struct super_timer_lengths_struct {
    const int start_to_straight_before_ramp;
    const int start_to_straight_before_tunnel;
    const int tunnel_end_to_straight_before_block;

    const int tunnel_end_to_straight_before_juncts;
    const int block_aquired_to_straight_before_tunnel;
    const int turned_180_to_straight_before_tunnel;

    const int red_junct_to_just_before_green_junct;
    const int red_junct_to_straight_before_ramp;
    const int just_after_green_junct_to_just_before_home_junct;
};

constexpr super_timer_lengths_struct ST_lengths = {
    9300,
    9300,
    7100,

    4500,
    7000,
    6500,
    
    7000,
    1500,
    1500
};





// const int TIME_FOR_BLOCK_STRAIGHT = 10000; //TUNE
// const int DELTA_DIST_FOR_LOST_BLOCK = 5;
// const int TURN_DISPARITIES_SAMPLE_LENGTH = 100; //TUNE
// const int DISPARITY_SAMPLE_PERIOD = 20;
// const int CURVING_LEFT_THRESHOLD = 0; //TUNE
// const int GOING_STRAIGHT_THRESHOLD = 75; //TUNE
// const int TIME_TO_DRIVE_FORWARD_FOR_AT_START = 2800; //TUNE
// const int TIME_TO_DRIVE_FORWARD_TO_GO_HOME = 1000; //TUNE
// const int TIME_TO_DRIVE_FORWARD_TO_DROP_BLOCK = 500; // TUNE
// unsigned long TIME_BEFORE_HEADING_BACK = 240000; // TUNE (4 minutes)
