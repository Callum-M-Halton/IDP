#include <Arduino.h>

// ==== OTHER ENUMS ====
struct offset_dirs_struct {
  const int none; const int left; const int right; const int unknown; // Relative direction to line
};
constexpr offset_dirs_struct offset_dirs = {0,1,2,3}; // numeric indexes for offset direction

struct offset_exts_struct {
  const int none; const int moderate; const int large; // extent of offset from line
};
constexpr offset_exts_struct offset_exts = {0,1,2}; // numeric index for offset extent

struct block_types_struct {const int none; const int mag; const int non_mag;}; // whether the block is magnetic or not
constexpr block_types_struct block_types = {0,1,2};
// ++++++++++++++++++++

// to track the movement of the rover so it can be reversed 
// - records motor speed, direction and duration (via command's timestamp)
struct motor_cmd_struct {
  int dirs[2];
  int speeds[2];
  unsigned long time_stamp;
};


struct state_struct {
  int motor_speeds[2]; int offset_dir;
  int offset_ext; int approaching;
  unsigned long timer_end; int motor_dirs[2];
  int junct_sensor_val; unsigned long super_timer_end;
  int block_type; LinkedList<motor_cmd_struct> motor_cmds;
  unsigned long time_stamp_of_cmd_being_rev_run; bool recording;
};

// State is used to store all the robot's global variables - its state
state_struct state = {
  {-1, -1}, offset_dirs.none,
  offset_exts.none, approachables.nothing,
  0, {-1, -1},
  0, 0,
  block_types.none, LinkedList<motor_cmd_struct>(),
  0, false
};

// ===== STATE GETTERS ======
bool is_approaching_junct() {
  // returns true if any junction of the 3 is being approached
  return state.approaching == approachables.red_junct
    || state.approaching == approachables.green_junct
    || state.approaching == approachables.home_junct;
}
// ++++++++++++++++++++++++++

// ====== STATE SETTERS ======
void start_super_timer(int ST_length) {
  // +++++++++
  // super timer to time accounting for starting time of board powering on to be used to toggle which inputs inform decision making
  state.super_timer_end = millis() + ST_length;
}
// +++++++++++++++++++++++++++

// RAMP NOT USED so has been commented out
/*
bool is_approaching_EOL() {
  return state.approaching == approachables.tunnel
    || state.approaching == approachables.ramp;
}
*/