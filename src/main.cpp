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
    Inertial.resetRotation();
    int internalDistance = Distance_sensor.objectDistance(inches);
    double error = input - internalDistance;
    double p = 0.1;
    while (error > 0.1 || error < -0.1) {
        if (Inertial.rotation() > 0) {
          right_motors.setVelocity(speed - Inertial.rotation(), percent);
          left_motors.setVelocity(speed + Inertial.rotation(), percent);
        }
        else if (Inertial.rotation() < 0) {
          right_motors.setVelocity(speed - Inertial.rotation(), percent);
          left_motors.setVelocity(speed + Inertial.rotation(), percent);
         }
         else {
          right_motors.setVelocity(speed, percent);
          left_motors.setVelocity(speed, percent);
        }

    right_motors.spin(reverse);
    left_motors.spin(reverse);
    internalDistance = Distance_sensor.objectDistance(inches);
    error = input - internalDistance;
    wait(10, msec);
  }
  right_motors.stop();
  left_motors.stop();
}

// --- Calibration Constants ---
// You must update these numbers based on your manual Brain testing
const double RED_HUE_TARGET = 18.0;   // Red is usually near 0 or 360
const double YELLOW_HUE_TARGET = 51;   // Red is usually near 0 or 360
const double BLUE_HUE_TARGET = 190.0;   // Red is usually near 0 or 360
const double HUE_TOLERANCE = 10.0;    // Allows for +/- 20 degrees of color shifting
const double MIN_PROXIMITY = 60.0;    // Object must be >60% close to be considered "seen"

/**
 * Detects if a Red object is directly in front of the sensor.
 * Handles the "wraparound" issue of the color wheel where Red is both 0 and 360.
 */
bool detectRedObject() {
    // STEP 1: Is there actually an object there?
    // We check proximity first so we don't accidentally read the hue of the floor.
    if (Optical_sensor.isNearObject()) {
        
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

    if (Optical_sensor.isNearObject()) {
        
        double currentHue = Optical_sensor.hue();

        if ((currentHue < BLUE_HUE_TARGET + HUE_TOLERANCE) && (currentHue > BLUE_HUE_TARGET - HUE_TOLERANCE)) {
            return true; // It's Blue!
        }
    }
    return false; // Not Blue, or no object present
}

/**
 * Detects if a Yellow object is directly in front of the sensor.
 */
bool detectYellowObject() {

    if (Optical_sensor.isNearObject()) {
        
        double currentHue = Optical_sensor.hue();

        if ((currentHue < YELLOW_HUE_TARGET + HUE_TOLERANCE) && (currentHue > YELLOW_HUE_TARGET - HUE_TOLERANCE)) {
            return true; // It's Yellow!
        }
    }
    return false; // Not Yellow, or no object present
}

void autonomous() {
    Drive(-16.5, 45);
    TurnRight(30, 20);
    TurnLeft(60, 20);
    TurnRight(30, 20);
    wait(50, msec); 
    TurnRight(25, 20);
    Drive(-11, 43);
    wait(100, msec);
    //required movement
    Brain.Screen.clearLine();
    Brain.Screen.setCursor(2, 1);
    Optical_sensor.setLightPower(100);
    Optical_sensor.setLightPower(100);
    Optical_sensor.setLightPower(100);
    Optical_sensor.setLightPower(100);
    Optical_sensor.setLightPower(100);
    Optical_sensor.setLightPower(100);
    wait(200, msec); 
    if (detectRedObject() == true) {
        Brain.Screen.print("Red Object Detected");
        Brain.Screen.newLine();
        Brain.Screen.print(Optical_sensor.hue());
    }
    else if (detectBlueObject() == true) {
        Brain.Screen.print("Blue Object Detected");
        Brain.Screen.newLine();
    }
    else if (detectYellowObject() == true) {
        Brain.Screen.print("Yellow Object Detected");
        Brain.Screen.newLine();
        Brain.Screen.print(Optical_sensor.hue());
        Brain.Screen.print(Optical_sensor.hue());
    }
    else {
        Brain.Screen.print("Nothing Detected");
        Brain.Screen.newLine();
        Brain.Screen.print(Optical_sensor.hue());
    }
    Brain.Screen.setCursor(1, 1);
    Optical_sensor.setLightPower(0);
    wait(2000, msec);
    Drive(6, 47);
    wait(50, msec);
    TurnRight(55, 20);
    wait(50, msec);
    Drive(-48, 45);
    wait(50, msec);
    TurnRight(1084, 20);
    wait(50, msec);
    TurnRight(90, 20);
    wait(50, msec);
    Autodistance(6, 30);
    wait(50, msec);
    TurnRight(93, 20);
    wait(50, msec);
    Drive(-60, 30);
    wait(50, msec);

    wait(5, sec);
}

void comp_autonomous() {
    intake.setVelocity(-100, percent);
    intake.spin(fwd);
    Drive(-32, 72);
    Drive(-22, 22);
    intake.setVelocity(-100, percent);
    intake.spin(fwd);
    TurnLeft(180, 32);
    Drive(-21, 72);
    TurnLeft(90, 32);
    intake.setVelocity(-100, percent);
    intake.spin(fwd);
    Drive(-8, 22);
    Drive(-24, 22);
    Drive(-24, 72);
    TurnRight(135, 32);
    intake.setVelocity(-100, percent);
    intake.spin(fwd);
    Drive(-8, 72);
    Drive(-52, 22);
    TurnRight(45, 32);
    intake.stop();
    Drive(-20, 72);
}

int main() {
    //testing area
    doinker.setPosition(0, deg);
    // calibrate inertial
    Inertial.calibrate();
    while (Inertial.isCalibrating()) {
        wait(2, msec);
    }
    Master.rumble("._.");
    // Declare Loop important variables
    float pCurve = 0.5;        //? curve for fwd/back
    float tCoefficient = 1.1;  //? curve for turn
    float tCurve = 0.66;       //? coefficient for turn
    double power;
    double powerC;
    double turn;
    double turnC;
    bool halfSpeed = false;
    double leftDrv;
    double rightDrv;
    int deadzone = 15;
    doinker.setVelocity(100, percent);
    doinker.setStopping(brakeType::hold);
    arm.setStopping(brakeType::hold);
    claw.setStopping(brakeType::hold);
    left_motors.setStopping(brakeType::brake);
    right_motors.setStopping(brakeType::brake);

    while (!Master.ButtonA.PRESSED) {wait(10, msec);}
    // start autonomous routine
    comp_autonomous();
   
    while(1) {
        if (Master.Axis3.position() > 5 || Master.Axis3.position() < -5) {
            power = Master.Axis3.position();
        }
        else {
            power = 0;
        }
        if (Master.Axis1.position() > 5 || Master.Axis1.position() < -5) {
            turn = Master.Axis1.position();
        }
        else {
            turn = 0;
        }

        // Calculating velocity
        powerC = ((1 - pCurve) * power) + ((pCurve * pow(power, 3)) / 16129); // don't change 16129
        //https://www.desmos.com/calculator/asjs86sdpy

        // Calculating turn curve
        turnC = tCoefficient * ((1 - tCurve) * turn) + ((tCurve * pow(turn, 3)) / 16129); // don't change 16129

        leftDrv = -powerC + turnC;
        rightDrv = -powerC - turnC;
        left_motors.setVelocity(leftDrv, percent);
        right_motors.setVelocity(rightDrv, percent);
        left_motors.spin(fwd);
        right_motors.spin(fwd);
        
        doinker.spin(fwd); // doinkercontrols
        if (Master.ButtonL1.pressing()) {
            doinker.spinToPosition(0, deg);
        }
        else if (Master.ButtonR1.pressing()) {
            doinker.spinToPosition(100, deg);
        }
        else {
            doinker.stop();
        }

        if (Master.ButtonLeft.pressing()) {
            arm.setVelocity(10, percentUnits());
            arm.setPosition(0, deg);
            arm.spin(fwd);
        }
        else if (Master.ButtonRight.pressing()) {
            arm.setVelocity(-10, percentUnits());
            arm.setPosition(0, deg);
            arm.spin(fwd);
        }
        else {
            arm.spinToPosition(0, deg);
        }

        claw.spin(fwd);
        if (Master.ButtonA.pressing()) {
            claw.setVelocity(70, percentUnits());
        }
        else if (Master.ButtonB.pressing()) {
            claw.setVelocity(-70, percentUnits());
        }
        else {
            claw.stop();
        }

        intake.spin(fwd);
        if (Master.ButtonL2.pressing()) {
            intake.setVelocity(100, percentUnits());
        }
        else if (Master.ButtonR2.pressing()) {
            intake.setVelocity(-100, percentUnits());
        }
        else {
            intake.stop();
        }


        //Optical_sensor.setLightPower(100);
        //Brain.Screen.clearLine();
        //Brain.Screen.print(Optical_sensor.hue());
        // Allow other tasks to run
        this_thread::sleep_for(10);
    }
}
