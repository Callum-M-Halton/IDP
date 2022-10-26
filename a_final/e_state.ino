#include <Arduino.h>

// ==== OTHER ENUMS ====
struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0,1,2,3};

struct offset_exts_struct {
  const int none; const int little; const int mid; const int far;
};
constexpr offset_exts_struct offset_exts = {0,1,2,3};
// ++++++++++++++++++++

struct motor_cmd_struct {
  int dirs[2];
  int speeds[2];
  unsigned long time_stamp;
  bool is_flag;
};

struct state_struct {
  int motor_speeds[2]; int offset_dir;
  int offset_ext; float avg_turns_disparity;
  // may revert this to prev_mode if stack not needed
  int approaching; unsigned long timer_end;
  LinkedList<motor_cmd_struct> motor_cmds; unsigned long time_stamp_of_cmd_being_rev_run;
  LinkedList<int> disparities_sample; int sector_code;
  //
  int blocks_collected; int motor_dirs[2];
  int sector_code_to_turn_off_after; float speed_coeff;
  int junct_sensor_val;
};
state_struct state = {
  {-1, -1}, offset_dirs.none,
  offset_exts.none, 0,
  //
  approachables.nothing, 0,
  LinkedList<motor_cmd_struct>(), 0,
  LinkedList<int>(), -1,
  //
  0, {-1, -1},
  -1, 1.0,
  0
};
