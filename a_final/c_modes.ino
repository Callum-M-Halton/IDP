#include <Arduino.h>

struct modes_struct {
  const int following_line; // must = 0!!!!!!
  const int finding_line_at_start;
  const int turning_at_symmetric_junction;
	const int making_right_turn;
	const int testing_block;
	const int lowering_grabber;
	const int traversing_tunnel;
	const int approaching_box;
	const int raising_grabber;
	const int doing_a_180;
	const int refinding_line;
  const int refinding_line_after_tunnel;
};
constexpr modes_struct modes = {0,1,2,3,4,5,6,7,8,9,10,11};
String mode_strings[12] = {
"following_line",
"finding_line_at_start",
"turning_at_symmetric_junction",
"making_right_turn",
"testing_block",
"lowering_grabber",
"traversing_tunnel",
"approaching_box",
"raising_grabber",
"doing_a_180",
"refinding_line",
"refinding_line_after_tunnel"
};