#include <Arduino.h>

// Assigning numeric indexes to each approachables
struct approachables_struct {
  const int tunnel; int nothing;
  const int box; const int block;
  const int red_junct; const int green_junct;
  const int home_junct; const int straight_before_block;
  const int straight_before_juncts; const int straight_before_tunnel;
  const int straight_before_ramp; const int ramp; /*const int block_to_left;
  const int block_straight;*/ // VIA_RAMP based approachables not used
  const int just_before_green_junct;
  const int just_before_home_junct;
};
constexpr approachables_struct approachables = {0,1,2,3,4,5,6,7,8,9,10,11,12,13};


