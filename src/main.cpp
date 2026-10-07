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

/*----------------------------------------------------------------------
  distance sensor, input: distance inches
----------------------------------------------------------------------*/
void Autodistance(int input, int speed) {
  int internalDistance = Distance_sensor.objectDistance(inches);
  double error = input - internalDistance;
  double p = 0.1;
  while (error > 0.1 || error < -0.1) {
    if (Inertial.rotation() > 0) {
      right_motors.setVelocity(speed * (error * p) + Inertial.rotation(), percent);
      left_motors.setVelocity(speed * (error * p) - Inertial.rotation(), percent);
    }
    else if (Inertial.rotation() < 0) {
      right_motors.setVelocity(speed * (error * p) + Inertial.rotation(), percent);
      left_motors.setVelocity(speed * (error * p) - Inertial.rotation(), percent);
     }
     else {
      right_motors.setVelocity(speed * (error * p), percent);
      left_motors.setVelocity(speed * (error * p), percent);
    }

    right_motors.spin(fwd);
    left_motors.spin(fwd);
    internalDistance = Distance_sensor.objectDistance(inches);
    error = input - internalDistance;
    wait(10, msec);
  }
  right_motors.stop();
  left_motors.stop();
}

// --- Calibration Constants ---
// You must update these numbers based on your manual Brain testing
const double RED_HUE_TARGET = 10.0;   // Red is usually near 0 or 360
const double YELLOW_HUE_TARGET = 55.0;   // Red is usually near 0 or 360
const double BLUE_HUE_TARGET = 210.0;   // Red is usually near 0 or 360
const double HUE_TOLERANCE = 20.0;    // Allows for +/- 20 degrees of color shifting
const double MIN_PROXIMITY = 60.0;    // Object must be >60% close to be considered "seen"

/**
 * Detects if a Red object is directly in front of the sensor.
 * Handles the "wraparound" issue of the color wheel where Red is both 0 and 360.
 */
bool detectRedObject() {
    // STEP 1: Is there actually an object there?
    // We check proximity first so we don't accidentally read the hue of the floor.
    if (Optical_sensor.isNearObject() && Optical_sensor.objectDetectThreshold() > MIN_PROXIMITY) {
        
        // STEP 2: What color is it?
        double currentHue = Optical_sensor.hue();
        
        // STEP 3: Math check. 
        // Because Red sits at the top of the color wheel, a red object might 
        // read as Hue 5, or it might read as Hue 355. We must check both sides!
        if ((currentHue > 360 - HUE_TOLERANCE) || (currentHue < RED_HUE_TARGET + HUE_TOLERANCE)) {
            return true; // It's Red!
        }
    }
    return false; // Not Red, or no object present
}

/**
 * Detects if a Blue object is directly in front of the sensor.
 */
bool detectBlueObject() {

    if (Optical_sensor.isNearObject() && Optical_sensor.objectDetectThreshold() > MIN_PROXIMITY) {
        
        double currentHue = Optical_sensor.hue();

        if ((currentHue < BLUE_HUE_TARGET + HUE_TOLERANCE) || (currentHue < BLUE_HUE_TARGET - HUE_TOLERANCE)) {
            return true; // It's Blue!
        }
    }
    return false; // Not Blue, or no object present
}

/**
 * Detects if a Yellow object is directly in front of the sensor.
 */
bool detectYellowObject() {

    if (Optical_sensor.isNearObject() && Optical_sensor.objectDetectThreshold() > MIN_PROXIMITY) {
        
        double currentHue = Optical_sensor.hue();

        if ((currentHue < YELLOW_HUE_TARGET + HUE_TOLERANCE) || (currentHue < YELLOW_HUE_TARGET - HUE_TOLERANCE)) {
            return true; // It's Blue!
        }
    }
    return false; // Not Blue, or no object present
}

void autonomous() {
    Drive(-48, 45);
    TurnRight(30, 20);
    TurnLeft(60, 20);
    TurnRight(30, 20);
    wait(100, msec);
    //required movement
    Brain.Screen.clearLine();
    Optical_sensor.setLightPower(100);
    if (detectRedObject) {
        Brain.Screen.print("Red Object Detected");
    }
    else if (detectBlueObject) {
        Brain.Screen.print("Blue Object Detected");
    }
    else if (detectYellowObject) {
        Brain.Screen.print("Yellow Object Detected");
    }
    else {
        Brain.Screen.print("Nothing Detected");
    }
    Optical_sensor.setLightPower(0);
    TurnRight(40, 30);
    Drive(-10, 45);
    TurnRight(80, 30);
    Drive(-70, 45);
    Autodistance(18, 40);
    TurnRight(1080, 30);
    
}


int main() {
    //testing area

    // calibrate inertial
    Inertial.calibrate();
    while (Inertial.isCalibrating()) {
        wait(2, msec);
    }
    Master.rumble("._.");
    // Declare Loop important variables
    int right_drive;
    int left_drive;
    int axis3;
    int axis1;
    int deadzone = 15;
    left_motors.setStopping(brakeType::brake);
    right_motors.setStopping(brakeType::brake);
    Brain.Screen.printAt( 10, 50, "Hello V5" );
    // start autonomous routine
    autonomous();
   
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
        
        Brain.Screen.clearLine();
        Brain.Screen.print(Optical_sensor.hue());

        // Allow other tasks to run
        this_thread::sleep_for(10);
    }
}
