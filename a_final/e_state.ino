#include <Arduino.h>

// ==== OTHER ENUMS ====
struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0,1,2,3};

struct offset_exts_struct {
  const int none; const int little; const int mid; const int far;
};
constexpr offset_exts_struct offset_exts = {0,1,2,3};

struct block_types_struct { int none; int mag; int non_mag; };
block_types_struct block_types = {0,1,2};

// ++++++++++++++++++++

struct motor_cmd_struct {
  int dirs[2];
  int speeds[2];
  unsigned long time_stamp;
  bool is_flag;
};

struct state_struct {
  int motor_speeds[2]; int offset_dir;
  int offset_ext; int approaching;
  unsigned long timer_end; int motor_dirs[2];
  float speed_coeff; int junct_sensor_val;
  unsigned long super_timer_end; int block;
  int blocks_collected; unsigned long start_time;
  int last_side_dist; unsigned long time_at_start_of_block_straight;

  LinkedList<motor_cmd_struct> motor_cmds; unsigned long time_stamp_of_cmd_being_rev_run;
};
state_struct state = {
  {-1, -1}, offset_dirs.none,
  offset_exts.none, approachables.nothing,
  0, {-1, -1},
  1.0, 0,
  0, block_types.none,
  0, 0,
  -1, 0,

  LinkedList<motor_cmd_struct>(), 0
};

// ===== STATE GETTERS ======
bool is_approaching_junct() {
  return state.approaching == approachables.red_junct
    || state.approaching == approachables.green_junct
    || state.approaching == approachables.home_junct;
}

bool is_approaching_EOL() {
  return state.approaching == approachables.tunnel
    // || state.approaching == approachables.red_box
    // || state.approaching == approachables.green_box
    // || state.approaching == approachables.home_box
    || state.approaching == approachables.ramp;
}
// ++++++++++++++++++++++++++

/*
  
  LinkedList<int> disparities_sample; int sector_code;
  int blocks_collected;
  int sector_code_to_turn_off_after;
  unsigned long cycle_num;
  float avg_turns_disparity;

  LinkedList<motor_cmd_struct>(), 0,
  LinkedList<int>(), -1,
  0,
  -1,
  0,
  0,
*/
