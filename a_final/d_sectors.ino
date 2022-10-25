#include <Arduino.h>

struct approachables_struct {
  const int tunnel; const int straight; int nothing;
  const int junct_on_right; const int corner; const int box;
};
constexpr approachables_struct approachables = {0,1,2,3,4,5};

struct sector_struct {
  String name;
  int approaching;
};

const int num_of_sectors = 13;
sector_struct sectors[num_of_sectors] = {
  /* 0  */ {"straight_after_start_junct", approachables.junct_on_right},
  /* 1  */ {"straight_afer_red_junct", approachables.corner},
  /* 2  */ {"corner_before_ramp", approachables.straight},
  /* 3  */ {"ramp_straight", approachables.corner},

  /* 4  */ {"corner_after_ramp", approachables.straight},
  /* 5  */ {"block_straight", approachables.corner},
  /* 6  */ {"corner_before_tunnel", approachables.straight},
  /* 7  */ {"section_before_tunnel", approachables.tunnel},

  /* 8  */ {"tunnel", approachables.nothing},
  /* 9  */ {"section_after_tunnel", approachables.corner},
  /* 10 */ {"corner_after_tunnel", approachables.straight},
  /* 11 */ {"straight_before_green_junct", approachables.junct_on_right},
  /* 12 */ {"straight_before_start_junct", approachables.junct_on_right}
};