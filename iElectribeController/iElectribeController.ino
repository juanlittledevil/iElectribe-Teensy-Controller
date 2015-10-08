/*
 *  iElectribe Controller setup for custom Teensy midi controller.
 *  
 *  Author: Juan Segovia
 *  Contact: www.juanlittledevil.com (juanlittledebil at gmail.com)
 *  Creative Commons: NC (Non-Commercial)
 *  https://wiki.creativecommons.org/wiki/4.0/NonCommercial
 *  
 *  Description: First off about the teensy,
 *  
 *  I'm using a Teensy 2.0++ and it is configured as with the following pinout:
 *  
 *  // PUSH BUTTONS
 *  pins 0 to 15 are connected to momentary switches which are arranged in a matrix of 4 x 4 where
 *  pin 0 is on the lower left and 15 is on the upper right.
 *  
 *  // JOYSTICK
 *  pins 16 - 19 connect to an arcade joystick where 16=Right, 17=Down, 18=Left, 19=Up
 *  
 *  // LED
 *  pins 20 - 35 are connected to the matrix as well and match the push buttons so that 20 is on the
 *  lower left and 35 is on the upper right.
 *  
 *  // KNOBS
 *  38 - 45 are the analogue pins, these connect to 8 potentiometers.
 *  
 *  NOTE: Please read the teensy documentation and take special not to to include pullup resistors
 *  where needed. Depending on which version of the teensy you intend to use you may have to use
 *  resistors for a couple of pins connected to switches as well as those which connect to the LEDs.
 *  
 *  the SmoothAnalogInput.h library was installed separately.
 *  https://github.com/rl337/Arduino/tree/master/libraries/SmoothAnalogInput
 *  
 *  Final words... I'm not a C++ programmer and was not able to get eclipse arduino to work so I was
 *  forced to use the arduino IDE which is kinda lame IMHO. There are some things in here which would have
 *  benefited from making them into objects, instead I had to do some mad multi-dimentional arrays.
 *  The result was satisfactory tho. Have fun with this, I hope you enjoy it as much as I have been.
 *  
 */

#include <Bounce.h>
#include <SmoothAnalogInput.h>


// =======================================================================================
// Variables
// =======================================================================================

bool debug = true;

// We have a 4 x 4 matrix of buttons.
const int channel = 10;
const int matrix_size = 16;
const int bounce_delay = 5; // 10ms
const byte max_knobs = 8;
const int max_stick = 4;
int part_selection = 0;
int effect_type_state = 0;
int play_state = 0;
int rec_state = 4;
int loop_state = 2;

// The pad parts of the iElectribe map to the bottom 8 buttons.
int play_note[] = {36, 38, 40, 41, 42, 46, 49, 39};


// used to store the pin values.
int push_button_pin[matrix_size] = {
  0, 1, 2, 3,
  4, 5, 6, 7,
  8, 9, 10, 11,
  12, 13, 14, 15
};

// Array of button objects with debounce.
Bounce push_button[] = {
  Bounce(0, bounce_delay), Bounce(1, bounce_delay), Bounce(2, bounce_delay), Bounce(3, bounce_delay),
  Bounce(4, bounce_delay), Bounce(5, bounce_delay), Bounce(6, bounce_delay), Bounce(7, bounce_delay),
  Bounce(8, bounce_delay), Bounce(9, bounce_delay), Bounce(10, bounce_delay), Bounce(11, bounce_delay),
  Bounce(12, bounce_delay), Bounce(13, bounce_delay), Bounce(14, bounce_delay), Bounce(15, bounce_delay)
};

// LED pins
int led[matrix_size] = {
  20, 21, 22, 23,
  24, 25, 26, 27,
  28, 29, 30, 31,
  32, 33, 34, 35
};

// holds the current state of the led matrix.
boolean is_lit[matrix_size] = {
  LOW, LOW, LOW, LOW,
  LOW, LOW, LOW, LOW,
  HIGH, LOW, LOW, LOW,
  LOW, LOW, LOW, LOW
};

// joystick
int stick_direction = 0;
int stick_pins[max_stick] = {16, 17, 18, 19};
Bounce stick[] = {
  Bounce(16, bounce_delay), Bounce(17, bounce_delay), Bounce(18, bounce_delay), Bounce(19, bounce_delay)
};


// knobs  
int knob_pins[max_knobs] = {38, 39, 40, 41, 42, 43, 44, 45};    // teensy pin values
int knob_state[max_knobs] = {0, 0, 0, 0, 0, 0, 0, 0};           // initialize the knob states
int knob_prev_state[max_knobs] = {0, 0, 0, 0, 0, 0, 0, 0};      // used to compare updates with previous state. Did it change?

SmoothAnalogInput knobs[max_knobs];                             // create array of SmoothAnalogInput objects.

//            [part][cc_number]
int part_midi_map[8][13] = {
  {15, 17, 20, 21, 0, 18, 22, 23, 16, 19, 24, 26, 27},  // part 1
  {28, 30, 34, 35, 0, 31, 36, 37, 29, 33, 38, 40, 41},  // part 2
  {42, 44, 47, 48, 0, 45, 49, 50, 43, 46, 51, 53, 54},  // part 3
  {55, 57, 60, 61, 0, 58, 62, 63, 56, 59, 64, 66, 67},  // part 4
  {68, 70, 73, 74, 0, 71, 75, 76, 69, 72, 77, 79, 80},  // part 5
  {81, 83, 86, 87, 0, 84, 88, 89, 82, 85, 90, 92, 93},  // part 6
  {94, 96, 99, 100, 0, 97, 101, 102, 95, 98, 103, 105, 106},  // part 7
  {107, 109, 112, 113, 0, 110, 114, 115, 108, 111, 116, 118, 119},  // part 8
};

//              [part][cc_value]
int part_midi_state[8][13] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

//        [part][state]
int part_mute[8][2] = {
  {25, 1},
  {39, 1},
  {52, 1},
  {65, 1},
  {78, 1},
  {91, 1},
  {104, 1},
  {117, 1},
};


// =======================================================================================
// Methods
// =======================================================================================
// init as it states gets ran from setup()
void init_hardware() {
  for ( int i=0; i < matrix_size; i++ ) {
    pinMode(push_button_pin[i], INPUT_PULLUP);
    pinMode(led[i], OUTPUT);
  }

  for ( int i=0; i < max_knobs; i++ ) {
    knobs[i].attach(knob_pins[i]);
  }

  for ( int i=0; i < max_stick; i++) {
    pinMode(stick_pins[i], INPUT_PULLUP);
  }
}


// Flashy splash sequence at boot time.
void boot_sequence() {
  for (int i=0; i < matrix_size; i++ ) {
    digitalWrite(led[i], HIGH);
    delay(25);
    digitalWrite(led[i], LOW);
    delay(10);
  }
}


// Button states - this method gets ran from loop()
// This is is how we get the state of the buttons and do stuff.
void update_button_states() {
  for ( int i=0; i < matrix_size; i++ ) {
    if (push_button[i].update()) {
      // LOW is means the button has been pressed.
      if (push_button[i].read() == LOW) {
        
        //   -- Center --
        if ( stick_direction == 0 && i >= 8 ) {
          process_part_buttons(i - 8);
        }
        
        if ( stick_direction == 0 && i < 8 ) {
          // Send note on --
          play_notes(play_note[i], HIGH);
        } 
        
        //   -- Right --
        // Update the last step value cc 6 value.
        if ( stick_direction == 1 ) {
          usbMIDI.sendControlChange(5, i + 1, channel);
          if (debug == true) {
            Serial.print("update_button_states(cc, value)");
            print_debug(5, i + 1);
          }
        }
        
        //   --  UP --
        if ( stick_direction == 2 && i < 8 ) {
          part_mutes(i, HIGH); 
        }
        if (stick_direction == 2 && i >= 8) {
          part_selection = i - 8;
          if (debug == true) {
            Serial.print("part_selection(button, part_number)");
            print_debug(i,i - 8);
          }
        }
        
        //   --  Left --
        // Update the loop length with the 4 bottom buttons.
        if ( stick_direction == 3 && i < 4 ) {
          usbMIDI.sendControlChange(6, i + 1, channel);
          if (debug == true) {
            Serial.print("update_button_states(cc, value)");
            print_debug(6, i + 1);
          }
        }
        
        //   -- Down --
        // Mute the parts
        if ( stick_direction == 4 ) {
          if ( i == 0 ) {
            if (rec_state == 4) {
               rec_state = 5;
            } else {
               rec_state = 4;
            }
//            usbMIDI.sendControlChange(7, rec_state, channel);
          }
          
//           if ( i == 1 ) {
//             if (play_state == 0) {
//                play_state = 1;
//             } else {
//                play_state = 0;
//             }
//             usbMIDI.sendControlChange(7, play_state, channel);
//           }
//
//           if ( i == 3 ) {
//             if (loop_state == 2) {
//                loop_state = 3;
//             } else {
//                loop_state = 2;
//             }
//             usbMIDI.sendControlChange(7, loop_state, channel);
//           }
        }

        
        // Light when the button is pressed
        is_lit[i] = HIGH;
        
      // HIGH (else) means the button has been released.
      } else {
        // Notes only played on bottom 2 rows
        if ( i < 8 ) {
          // Send note off --
          play_notes(play_note[i], LOW);
        }
        is_lit[i] = LOW;
      }
    }
  }
}


// Send a button pressed control change.
void send_cc_when_pressed(int button_number, int max_value) {
/* input: button_number which translates to the key which holds the current cc value.
 * input: max_value the button will send a cc message 1 value higher each time it is pressed.
 *        when the max_value is reached the cc message will be set to 0 and the cycle will repeat.
 *
 */
  if ( part_midi_state[part_selection][button_number] == max_value ) {
    part_midi_state[part_selection][button_number] = 0;
  } else {
    part_midi_state[part_selection][button_number]++;
  }
  usbMIDI.sendControlChange(part_midi_map[part_selection][button_number], part_midi_state[part_selection][button_number], channel);

  if (debug == true) {
    Serial.print("send_cc_when_pressed(cc, value)");
    print_debug(part_midi_map[part_selection][button_number], part_midi_state[part_selection][button_number]);
  }
}


// These are the top two rows of buttons. Each part as a bank of 8.
void process_part_buttons(int i) {
  if ( i == 0 ) {
    // Motion
    send_cc_when_pressed(10, 2);

  } else if ( i == 1 ) {
    // Accent
    send_cc_when_pressed(12, 1);
      
  } else if ( i == 2 ) {
    // FX Edit1
    part_midi_map[part_selection][4] = 13;
    if (debug == true) {
      Serial.print("process_part_buttons(cc, value)");
      print_debug(part_midi_map[part_selection][4], part_midi_state[part_selection][4]);
    }


  } else if ( i == 3 ) {
    // FX Edit1
    part_midi_map[part_selection][4] = 14;
    if (debug == true) {
      Serial.print("process_part_buttons(cc, value)");
      print_debug(part_midi_map[part_selection][4], part_midi_state[part_selection][4]);
    }

  } else if ( i == 4 ) {
    // Waveform
    send_cc_when_pressed(8, 3);

  } else if ( i == 5 ) {
    // Mod Type
    send_cc_when_pressed(9, 5);

  } else if ( i == 6 ) {
    // Effect Type
    if ( effect_type_state == 7 ) {
      effect_type_state = 0;
    } else {
      effect_type_state++;
    }
    usbMIDI.sendControlChange(12, effect_type_state, channel);
    if (debug == true) {
      Serial.print("process_part_buttons(cc, value)");
      print_debug(12, effect_type_state);
    }

  } else if ( i == 7 ) {
    // Effect
    send_cc_when_pressed(11, 1);
  }
}


// This is what happens when we push the buttons.
void play_notes(int note, boolean on) {
  // == PART NOTES ==
  if (on == HIGH) {
    usbMIDI.sendNoteOn(note, 99, channel);
    if (debug == true) {
      Serial.print("play_notes(cc, value)");
      print_debug(note, on);
    }
  } else if (on == LOW) {
    usbMIDI.sendNoteOff(note, 0, channel);
  }
}


void part_mutes(int i, boolean on) {  
  // == PART MUTES ==
  if (on == HIGH) {
    //Serial.print("ON");
    if ( part_mute[i][1] == 0 ) {
       part_mute[i][1] = 1;
       is_lit[i] = HIGH;
    } else {
      part_mute[i][1] = 0;
      is_lit[i] = LOW;
    }
    usbMIDI.sendControlChange(part_mute[i][0], part_mute[i][1], channel);
    if (debug == true) {
      Serial.print("part_mutes(cc, value)");
      print_debug(part_mute[i][0], part_mute[i][1]);
  }
  }
}


// Update Stick - Ran from loop().
void update_stick_states() {
  for ( int i=0; i < max_stick; i++ ) {
    if (stick[i].update()) {
      if (stick[i].read() == LOW) {
        detect_direction(i, HIGH);
      } else {
        detect_direction(i, LOW);
      }
    }
  }
}


// Which direction is the Joystick pressed?
void detect_direction(int i, boolean on) {
  if (on == HIGH) {
  
    if (i == 0) {
      stick_direction = 1;
      part_midi_map[part_selection][4] = 10;
    } else if (i == 1) {
      // Down
      stick_direction = 2;
      part_midi_map[part_selection][4] = 7;
    } else if (i == 2) {
      // Left
      stick_direction = 3;
      part_midi_map[part_selection][4] = 11;
    } else if (i == 3) {
      // Up
      stick_direction = 4;
    }
  } else {
    // Center
    stick_direction = 0;
  }
  if (debug == true) {
    Serial.print("detect_direction(pin, direction)");
    print_debug(stick_pins[i], stick_direction);
  }
}


// Turn on leds, or off depending on our state matrix.
void update_leds() {
  for (int i=0; i < matrix_size; i++ ) {
    if ((i - 8) == part_selection) {
      digitalWrite(led[i], HIGH);
      delay(1);
      digitalWrite(led[i], LOW);
      delay(1);
    }
    digitalWrite(led[i], is_lit[i]);
  }
}


// Knob states...
void update_knob_states() {
  for (int i=0; i < max_knobs; i++ ) {
    knob_state[i] = map(knobs[i].read(), 0, 1024, 0, 128);
    if ( knob_state[i] != knob_prev_state[i] ) {
      part_midi_state[part_selection][i] = knob_state[i];
      usbMIDI.sendControlChange(part_midi_map[part_selection][i], part_midi_state[part_selection][i], channel);

      if (debug == true) {
        Serial.print("update_knob_states(cc, value)");
        print_debug(part_midi_map[part_selection][i], part_midi_state[part_selection][i]);
      }
    }
    knob_prev_state[i] = knob_state[i];
  }
}


// Tool for debugging output into serial console.
void print_debug(int arg1, int arg2) {
  Serial.print(" [");
  Serial.print(arg1);
  Serial.print("]: ");
  Serial.print(arg2);
  Serial.println("");
}


// =======================================================================================
// Setup
// =======================================================================================
void setup() {
  Serial.begin(9600);
  init_hardware();
  boot_sequence();
}


// =======================================================================================
// The loop()
// =======================================================================================
void loop() {
  update_button_states();
  update_stick_states();
  update_knob_states();
  update_leds();

  while (usbMIDI.read()) {
    // ignore incoming messages
  }
}



