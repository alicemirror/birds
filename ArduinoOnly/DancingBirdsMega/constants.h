/**
 * \file constants.h
 * \brief constants for the dancing birds project, 
 * Arduino Mega only for Art-a-Tronic exhibition
 * 
 * \date April 2019 = June 2019
 * \author Enrico Miglino <balearicdynamics@gmail.com>
 * \version 2.1
 * 
 */
#ifndef _CONSTANTS
#define _CONSTANTS

//#define PIR_PIN 8               ///< Pir sensor pin NOT USED
//#define END_SEQ_DELAY 30000     ///< End sequence delay before checking the PIR again (ms)

#define BIRD1_PIN 0             ///< Bird pin index (servo control)
#define BIRD2_PIN 1             ///< Bird pin index (servo control)
#define BIRD3_PIN 2             ///< Bird pin index (servo control)
#define BIRD4_PIN 3             ///< Bird pin index (servo control)
#define BRID_PLATFORM_PIN 4     ///< Queue motion servo
#define MUSICBUTTON_PIN 5       ///< Music selection button servo 
#define MUSIC_CONTROL_PIN 2     ///< Music power pin

#define NUMSERVOS 6             ///< Number of servos organised in an array

#define MIN_ANGLE 0     ///< Real min angle of the servo. 0-180 Deg is refactored to it
#define MAX_ANGLE 150   ///< Real max angle of the servo. 0-180 Deg is r5factored to it
// ===============================================
//  Birds Commands
// ===============================================

#define CMD_JUMP 0x01               ///< Birds platform jump once
#define CMD_ROTATE_RIGHT 0x02       ///< Rotate current bird right (no action if reach limit)
#define CMD_NEXT_BIRD 0x03          ///< Move control to next bird (1-4 cyclic)
#define CMD_ROTATE_LEFT 0x04        ///< Rotate current bird left (no action if reach limit)
#define CMD_MUSIC 0x05              ///< Change the status of the music player (On/Off)
#define CMD_SOUND 0x06              ///< Change the current playing music
#define CMD_DANCE 0X07              ///< Set birds dancing On/Off
#define CMD_STOP 0X08               ///< Power off the game

//! Commands constants
#define BIRDS_ROT_STEP 10    ///< Manual rotation step of the birds (Deg)
#define BIRDS_QUIET 90      ///< 
#define BIRDS_MIN_ROT 0     ///< Minimum rotation angle of the birds (Deg)
#define BIRDS_MAX_ROT 180   ///< Minimum rotation angle of the birds (Deg)
#define BIRDS_RESTING 90    ///< Birds resting oint angle (Deg)
#define NUM_BIRDS 4         ///< Number of birds

#define BIRDS_JUMP_UP 90        ///< Birds platform up position (Deg)
#define BIRDS_JUMP_DOWN 135     ///< Birds platform down position (Deg)
#define BIRDS_JUMP_DELAY 500    ///< Duration of the birds platform jump

#define MUSIC_BUTTON_DOWN 60    ///< Music change button press position (Deg)
#define MUSIC_BUTTON_UP 0       ///< Music change button release position (Deg)
#define MUSIC_BUTTON_DELAY 600  ///< Delay of the music change button press to take effect (ms)

#define BIRD_DIRECTION_CW +1    ///< Birds rotation direction
#define BIRD_DIRECTION_CCW -1   ///< Birds rotation direction

#define ACTION_LOOP_CYCLES 5    ///< Number of times the action loop is exectued before a pause
#define ACTION_LOOP_PAUSE 300000    ///< Delay (ms) after every dancing cycle, 5 min
#define DANCE_CYCLE 6          ///< Dance cycle duration (steps)

//! Structure defining the status of the birds platform
typedef struct {
    boolean music;                      ///< The status of the music player
    boolean changeMusic;                ///< Request to press the music change button
    boolean dance;                      ///< Current dance status
    int birdsRotation[NUM_BIRDS];       ///< The birds rotation status
    boolean birdsJump;                  ///< Flex the birds
    int rotateDirection[NUM_BIRDS];     ///< Current rotation direction of the birds (+1/-1)
    int currentBird;                    ///< Current bird selected for motion
} cmdStatus;

#endif
