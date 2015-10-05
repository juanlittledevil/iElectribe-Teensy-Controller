#  iElectribe Controller setup for custom Teensy midi controller.

### Author: Juan Segovia
### Contact: www.juanlittledevil.com (juanlittledebil at gmail.com)
### Creative Commons: NC (Non-Commercial)
### https://wiki.creativecommons.org/wiki/4.0/NonCommercial

## Description:

First off about the teensy,

I'm using a Teensy 2.0++ and it is configured as with the following pinout:

// PUSH BUTTONS
pins 0 to 15 are connected to momentary switches which are arranged in a matrix of 4 x 4 where
pin 0 is on the lower left and 15 is on the upper right.

// JOYSTICK
pins 16 - 19 connect to an arcade joystick where 16=Right, 17=Down, 18=Left, 19=Up

// LED
pins 20 - 35 are connected to the matrix as well and match the push buttons so that 20 is on the
lower left and 35 is on the upper right.

// KNOBS
38 - 45 are the analogue pins, these connect to 8 potentiometers.

NOTE: Please read the teensy documentation and take special not to to include pullup resistors
where needed. Depending on which version of the teensy you intend to use you may have to use
resistors for a couple of pins connected to switches as well as those which connect to the LEDs.

the SmoothAnalogInput.h library was installed separately.
https://github.com/rl337/Arduino/tree/master/libraries/SmoothAnalogInput

## MIDI implementation and manual.

The joystick is used to select different modes.

### UP - Part Select mode

iElectribe uses 8 parts (4 synth parts and 4 drum parts). While holding the joystick in the "UP"
position press on the top two rows of buttons. The light will light up indicating the selected part.

### DOWN

TBD

### LEFT

TBD

### RIGHT

TBD

### CENTER

This is the default mode.

## Final words... 

I'm not a C++ programmer and was not able to get eclipse arduino to work so I was
forced to use the arduino IDE which is kinda lame IMHO. There are some things in here which would have
benefited from making them into objects, instead I had to do some mad multi-dimentional arrays.
The result was satisfactory tho. Have fun with this, I hope you enjoy it as much as I have been.
