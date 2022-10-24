#include <Arduino.h>

struct approachables_struct {
  const int tunnel; const int straight; int nothing;
  const int junct_on_right; const int corner;
};
constexpr approachables_struct approachables = {0,1,2,3,4};

struct on_loop_sector_struct {
  String name;
  // -1 indicates no off loop sector option at next junction
  int off_loop_sector_code;
  int approaching;
  int associated_mode;
  float speed_coeff;
};

const int num_of_on_loop_sectors = 13;
on_loop_sector_struct on_loop_sectors[num_of_on_loop_sectors] = {
  {"straight_after_start_junct", -1, approachables.junct_on_right, 0, 1.0},
  {"straight_afer_red_junct", -1, approachables.corner, 0, 1.0},
  {"corner_before_ramp", -1, approachables.straight, 0, 1.0},
  {"ramp_straight", -1, approachables.corner, 0, 1.0},

  {"corner_after_ramp", -1, approachables.straight, 0, 1.0},
  {"block_straight", -1, approachables.corner, 0, 1.0}, // sector 5
  {"corner_before_tunnel", -1, approachables.straight, 0, 1.0},

  {"section_before_tunnel", -1, approachables.tunnel, 0, 0.7},
  {"tunnel", -1, approachables.nothing, modes.traversing_tunnel, 1.0},
  {"section_after_tunnel", -1, approachables.corner, 0, 1.0},
  {"corner_after_tunnel", -1, approachables.straight, 0, 1.0},

  {"straight_before_green_junct", -1, approachables.junct_on_right, 0, 1.0},
  {"straight_after_green_junct", -1, approachables.junct_on_right, 0, 1.0}
};