#include "vex.h"

using namespace vex;

// Configure the devices here
// Motor - Red = 36_1 - Green = 18_1 - Blue = 6_1
motor left_motor_front = motor(PORT18, ratio18_1, true);
motor left_motor_back = motor(PORT13, ratio18_1, true);
motor_group left_motors = motor_group(left_motor_front, left_motor_back);

motor right_motor_front = motor(PORT17, ratio18_1, false);
motor right_motor_back = motor(PORT16, ratio18_1, false);
motor_group right_motors = motor_group(right_motor_front, right_motor_back);

inertial Inertial = inertial(PORT20);

distance Distance_sensor = distance(PORT12);
optical Optical_sensor = optical(PORT11);