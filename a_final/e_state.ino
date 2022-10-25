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
  bool is_right; int speed; unsigned long time_stamp;
};

struct state_struct {
  /*StackArray<int> prev_modes;*/
  int motor_speeds[2]; int offset_dir;
  int offset_ext; int mode;
  bool timer_set; float avg_turns_disparity;
  // may revert this to prev_mode if stack not needed
  int approaching; unsigned long timer_end;
  LinkedList<motor_cmd_struct> motor_cmds; unsigned long time_stamp_of_cmd_being_rev_run;
  LinkedList<int> disparities_sample; int sector_code;
  //
  float speed_coeff; int blocks_collected;
};
state_struct state = {
  /*{},*/
  {0, 0}, offset_dirs.none,
  offset_exts.none, -1, 
  false, 0,
  //
  approachables.nothing, 0,
  LinkedList<motor_cmd_struct>(), 0,
  LinkedList<int>(), -1,
  //
  1.0, 1 /////////// 1 for testing, 0 in production!!!!!
};