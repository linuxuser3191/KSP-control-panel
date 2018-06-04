/*
 * Arduino sketch for custom KSP controller
 * Interfaces with kRPC mod
 * 
 * Refer to documentation for all pinouts and schematics
 * March 2018 - ...
 * 
 * Thomas Robinson
 * 
 * Special thanks to:
 *  Adam B. for help with code for both Arduino and Python
 *  Joshua F. for help building the enclosure, translation joystick, and troubleshooting wiring
 *  Kevin S. for providing thoughtful input and ideas
 */

#include <ShiftRegister4021BP.h>
#include <ShiftRegister74HC595.h>
#include <LiquidCrystal.h>

const boolean DEBUG = false;                    //DEBUG changes print statements to println for easier reading in serial monitor

//If number of shift registers for srSW change from 2, update initializedebounce() variables and io_panel for loop
ShiftRegister4021BP srSW (2, 14, 15, 3);        //SW shift register attributes(number of registers, datapin, clockpin, latchpin)
ShiftRegister74HC595 srLED (2, 4, 2, 5);        //LED shift register atrributes(number of registers, datapin, clockpin, latchpin)

LiquidCrystal lcd(12,11,10,9,8,7);              //initialize lcd display object(RS, E, DB4, DB5, DB6, DB7)

const int lSTATUS = 13;                         //status LED

void setup() {
  pinMode(lSTATUS, OUTPUT);                     //status LED
  
  lcd.begin(16, 2);                             //setup screen size, 16 characters, 2 lines
  lcd.clear();                                  //clears the lcd screen
  
  Serial.begin(38400);                          //start Serial communication 38400 baud to kRPC with ksp.py

  //switch_initialization();                    //checks position of all switches and buttons and tests all LEDs
  initialize_debounce();                        //prevents multiple commands from single button push; resides in io_panel
  throttle_setup();                             //sets motor pins, then adjusts throttle to ship in game; TODO - resides in throttle panel, needs to move to switch_initialization
}

void loop() {
  throttle_panel();                             //controls throttle
  fcs_panel();                                  //controls fcs joystick
  input();                                      //reads input over serial from kRPC
  output();                                     //reads io_panel, sends output over serial to kRPC, information to displays, and toggles LEDs
}
