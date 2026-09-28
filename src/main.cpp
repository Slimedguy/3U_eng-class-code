/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Ewan S                                                    */
/*    Created:      9/16/2026, 9:15:31 AM                                     */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include "vex.h"

// Allows you to use vex:: without the vex::
using namespace vex;

// Main instaces of Brain and Controller
brain       Brain;
controller  Master;

/*----------------------------------------------------------------------
  TURN RIGHT unit: degrees, speed:percent units
----------------------------------------------------------------------*/
void TurnRight(int targetDegrees, int speed) {
  Inertial.setRotation(0, degrees);

  right_motors.setVelocity(speed, percent);
  left_motors.setVelocity(speed, percent);

  while (Inertial.rotation() < targetDegrees) {
    
    if (targetDegrees - Inertial.rotation() < 20) {
      right_motors.setVelocity(speed / 2, percent);
      left_motors.setVelocity(speed / 2, percent);
    }

    right_motors.spin(reverse);
    left_motors.spin(forward);
    Brain.Screen.print(Inertial.rotation());

    wait(10, msec);
    Brain.Screen.clearLine();
  }

  left_motors.stop(brake);
  right_motors.stop(brake);
  wait(80, msec);
}

/*----------------------------------------------------------------------
  TURN LEFT unit: degrees, speed:percent units
----------------------------------------------------------------------*/
void TurnLeft(int targetDegrees, int speed) {
  Inertial.setRotation(0, degrees);

  right_motors.setVelocity(speed, percent);
  left_motors.setVelocity(speed, percent);

  while (Inertial.rotation() > -targetDegrees) {
    
    if (targetDegrees + Inertial.rotation() < 20) {
      right_motors.setVelocity(speed / 2, percent);
      left_motors.setVelocity(speed / 2, percent);
    }

    right_motors.spin(forward);
    left_motors.spin(reverse);
    Brain.Screen.print(Inertial.rotation());

    wait(10, msec);
    Brain.Screen.clearLine();
  }

  left_motors.stop(brake);
  right_motors.stop(brake);
  wait(80, msec);
}

/*---------------------------------------
    DRIVE FORWARDS, inch, speed
---------------------------------------*/
void Drive(int distance, int speed) {
    int distance_traveled = 0;
    int error = distance_traveled - distance;
    while (error < 0 - 5 | error > 0 + 5) {
        error = distance_traveled - distance;
        if (Inertial.rotation() > 0) {
            right_motors.setVelocity(speed + Inertial.rotation(), percent);
            left_motors.setVelocity(speed - Inertial.rotation(), percent);
        }
        else if (Inertial.rotation() < 0) {
            right_motors.setVelocity(speed + Inertial.rotation(), percent);
            left_motors.setVelocity(speed - Inertial.rotation(), percent);
        }
        else {
            right_motors.setVelocity(speed, percent);
            left_motors.setVelocity(speed, percent);
        }

    }
}

void autonomous() {
    TurnRight(90, 20);
    wait(4, msec);
}


int main() {
    // calibrate inertial
    Inertial.calibrate();
    while (Inertial.isCalibrating()) {
        wait(2, msec);
    }
    Master.rumble("._.");
    // start autonomous routine
    autonomous();
    // Declare Loop important variables
    int right_drive;
    int left_drive;
    int axis3;
    int axis1;
    int deadzone = 15;
    left_motors.setStopping(brakeType::brake);
    right_motors.setStopping(brakeType::brake);
    Brain.Screen.printAt( 10, 50, "Hello V5" );
   
    while(1) {
        // set controller deadzones
        axis3 = Master.Axis3.position();
        axis1 = Master.Axis1.position();

        if (abs(axis3) < deadzone) {
            axis3 = 0;
        }
        if (abs(axis1) < deadzone) {
            axis1 = 0;
        }
        // Calculate drivetrain half velocity,
        left_drive = axis3 - axis1;
        right_drive = axis3 + axis1;
        // set drivetrain half velocity
        left_motors.setVelocity(-left_drive, percent);
        right_motors.setVelocity(-right_drive, percent);
        // Spin the motors
        if (left_drive > 20) {
            left_motors.spin(fwd);
        }
        if (right_drive > 20) {
            right_motors.spin(fwd);
        }
  

        // Allow other tasks to run
        this_thread::sleep_for(10);
    }
}
