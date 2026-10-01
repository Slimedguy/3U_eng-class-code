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
    DRIVE, inch, speed
---------------------------------------*/
void Drive(int distance, int speed) {
    Inertial.setRotation(0, rotationUnits::deg);
    right_motors.setPosition(0, rotationUnits::rev);
    left_motors.setPosition(0, rotationUnits::rev);
    double distance_traveled = 0;
    double error = distance_traveled - distance;
    timer(drive_timer);
    while (error < 0 - 1 | error > 0 + 1) {
        drive_timer.reset();
        Brain.Screen.clearScreen();
        error = distance_traveled - distance;
        if (Inertial.rotation() > 0) {
            if (distance > 0) {
                right_motors.setVelocity(speed + Inertial.rotation(), percent);
                left_motors.setVelocity(speed - Inertial.rotation(), percent);
            }
            else if (distance < 0) {
                right_motors.setVelocity(speed - Inertial.rotation(), percent);
                left_motors.setVelocity(speed + Inertial.rotation(), percent);
            }
        }
        else if (Inertial.rotation() < 0) {
            if (distance > 0) {
                right_motors.setVelocity(speed + Inertial.rotation(), percent);
                left_motors.setVelocity(speed - Inertial.rotation(), percent);
            }
            else if (distance < 0) {
                right_motors.setVelocity(speed - Inertial.rotation(), percent);
                left_motors.setVelocity(speed + Inertial.rotation(), percent);
            }
        }
        else {
            right_motors.setVelocity(speed, percent);
            left_motors.setVelocity(speed, percent);
        }
        distance_traveled = ((right_motors.position(rev) + left_motors.position(rev)) / 2) * (4 * M_PI);
        if (error < 0) {
            right_motors.spin(fwd);
            left_motors.spin(fwd);
        }
        else if (error > 0) {
            right_motors.spin(reverse);
            left_motors.spin(reverse);
        }
        wait(4, msec);  
    }
    right_motors.stop(brakeType::brake);
    left_motors.stop(brakeType::brake);
}

void autonomous() {
    Drive(12 * 5, 50);
    wait(0.5, sec);
    Drive(-12 * 5, 50);
    wait(0.5, sec);
    Drive(12 * 7, 50);
    wait(0.5, sec);
    Drive(-12 * 7, 50);
    wait(0.5, sec);
    Drive(12 * 10, 50);
    wait(0.5, sec);
    TurnLeft(180, 20);
    Drive(12 * 10, 50);
    wait(0.5, sec);
    wait(4, msec);
}


int main() {
    //testing area
    timer(testing_timer);
    wait(20, msec);
    Brain.Screen.print((testing_timer.time() * pow(10, -3)));
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
