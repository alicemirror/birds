/**
 * \file DancingBirdsMega.ino
 * \brief The arduino file for the dancing birds project, 
 * Arduino Mega only for Art-a-Tronic exhibition
 * 
 * \date April 2019 = June 2019
 * \author Enrico Miglino <balearicdynamics@gmail.com>
 * \version 2.1
 * 
 */
#undef _DEBUG

#include <Servo.h>

#ifdef _DEBUG
#include <Streaming.h>
#endif

#include "constants.h"

//! Array of the servo pins
int servoPins[NUMSERVOS] = { 23, 25, 27, 29, 31, 33 };
//! Array of the servo library instances (one every servo)
Servo servos[NUMSERVOS];

cmdStatus birdsActivity;


//! Initialisation function
void setup() {
    int j;
#ifdef _DEBUG
    Serial.begin(9600);
#endif
    // Initialize the music power On/Off
    pinMode(MUSIC_CONTROL_PIN, OUTPUT);
    // Initilize the status structure and the servos
    initActivityStatus();
    // Test the servos on startup
    testSystem();
}

//! Application main loop
void loop() {
    int j;

    // Music on
#ifdef _DEBUG
    Serial << "Loop() musicOn" << endl;
#endif
    setupCommand(CMD_MUSIC);
    commandProcessor();
    delay(150);

    // Action cycle
    for(j = 0; j < ACTION_LOOP_CYCLES; j++) {
#ifdef _DEBUG
    Serial << "Loop() Action loop cycle " << j << endl;
#endif
        setupCommand(CMD_JUMP);
        commandProcessor();
        delay(150);
        setupCommand(CMD_DANCE);
        commandProcessor();
        setupCommand(CMD_SOUND);
        commandProcessor();
        delay(150);
    }

    // Music off
#ifdef _DEBUG
    Serial << "Loop() musicOff" << endl;
#endif
    setupCommand(CMD_MUSIC);
    commandProcessor();
    delay(150);

    // Pause
    delay(ACTION_LOOP_PAUSE);
}
// ============================================================
//                      FUNCTIONS & COMMANDS
// ============================================================

/** 
 * Initialize the commands status structure and set the status 
 * of the servos accordingly
 */
void initActivityStatus() {
    initBirds();
    powerMusic();
    initServos();
}

//
/**
 * Initialize the birds platform and preset the comands structure
 * and servos status
 */
void initBirds() {
    // Initialize the activity structure
    birdsActivity.music = false;
    birdsActivity.dance = false;
    birdsActivity.changeMusic = false;
    birdsActivity.birdsJump = false;
    birdsActivity.currentBird = 0;

    int j;
    
    for(j = 0; j < NUM_BIRDS; j++) {
        birdsActivity.birdsRotation[j] = BIRDS_QUIET;
        birdsActivity.rotateDirection[j] = BIRD_DIRECTION_CW;
    }
}

/**
 * Change the currently selected bird. 
 * If the last is reached, start with the firs
 */
 void changeBird() {
  
  birdsActivity.currentBird++;

  // If last bird is reached, select the first
  if(birdsActivity.currentBird == NUM_BIRDS) {
    birdsActivity.currentBird = 0;
  }
 }

/** 
 * Initialise the servos.
 * This method is also called by the initActivityStatus() to setup the
 * servos to the initial position.
 */
void initServos() {
    int j;

    // Attach the servos to the corresponding pins
    for(j = 0; j < NUMSERVOS; j++) {
        servos[j].attach(servoPins[j]);
    }

    // Initialize the birds servos to the current position
    for(j = 0; j < NUM_BIRDS; j++) {
        setServo(j, birdsActivity.birdsRotation[j]);
    }

    // Initialize the position of the birds platform to the down position
    setServo(BRID_PLATFORM_PIN, BIRDS_JUMP_DOWN);

    // Initialize the position of the birds platform to the down position
    setServo(MUSICBUTTON_PIN, MUSIC_BUTTON_UP);

    // Force the music off
    powerMusic();
}

/** 
 * Initialise the system for dancing.
 */
void initDance() {
    int j;

    for(j = 0; j < NUM_BIRDS; j++) {
        birdsActivity.birdsRotation[j] = BIRDS_QUIET;
        birdsActivity.rotateDirection[j] = BIRD_DIRECTION_CW;
        setServo(j, birdsActivity.birdsRotation[j]);
    }

    // Initialize the position of the birds platform to the down position
    setServo(BRID_PLATFORM_PIN, BIRDS_JUMP_DOWN);
    // Initialize the position of the birds platform to the down position
    setServo(MUSICBUTTON_PIN, MUSIC_BUTTON_UP);
}

/**
 * Test the system after startup
 */
 void testSystem() {

    birdsActivity.music = true;
    powerMusic();
    delay(1000);
    changeMusic();
    delay(1000);
    birdsActivity.music = false;
    powerMusic();
    birdsActivity.changeMusic = false;

    birdsActivity.birdsJump = true;
    jumpBirds();
    birdsActivity.birdsJump = false;
 }

/**
 * Executes a birds platform jump. The operation is conditioned to the
 * flex status.
 * This allows the function to be called programmatically every loop cycle
 * but the jump has effect depending on the flag status.
 * After the jump is executed, the jump status is reset
 */
void jumpBirds() {
    if(birdsActivity.birdsJump) {
        setServo(BRID_PLATFORM_PIN, BIRDS_JUMP_UP);
        delay(BIRDS_JUMP_DELAY);
        setServo(BRID_PLATFORM_PIN, BIRDS_JUMP_DOWN);
        birdsActivity.birdsJump = false;
    }
}

//! Position a servo at the desired angle
void setServo(int num, int deg) {
  deg = constrain(deg, MIN_ANGLE, MAX_ANGLE);
  servos[num].write(deg);
}

/**
 * Set On/Off the music player. The command needs half a second
 * of delay to be completed.
 */
void powerMusic() {
  if(birdsActivity.music) {
      digitalWrite(MUSIC_CONTROL_PIN, HIGH);
  }
  else {
      digitalWrite(MUSIC_CONTROL_PIN, LOW);
  }
  delay(500);
}

/**
 * Press the music change button, if there is a queued request.
 * Ater the command has been executed, the flag status is reset
 */
void changeMusic() {
  if(birdsActivity.changeMusic) {
    setServo(MUSICBUTTON_PIN, MUSIC_BUTTON_DOWN);
    delay(MUSIC_BUTTON_DELAY);
    setServo(MUSICBUTTON_PIN, MUSIC_BUTTON_UP);
    birdsActivity.changeMusic = false;
  }
}

/**
 * Executes the dance sequence for the desired period
 * The dance steps are exectued starting from the current position
 * of every bird.
 * 
 * @param duration The period to dance (ms)
 */
void dance(int duration) {
    int j, k = 0, danceSteps;

#ifdef _DEBUG
    Serial << "dance() steps " << duration << endl;
#endif

    // Initialize the dance, only if the dance flag is set
    if(birdsActivity.dance) {
//      initDance();
//      delay(250);

        // Dance until the end of world (cit.)
        for(danceSteps = 0; danceSteps < duration; danceSteps++) {  
          int b;

#ifdef _DEBUG
    Serial << "dance() dancing " << danceSteps << endl;
#endif

          // Loop the birds        
          for(b = 0; b < NUM_BIRDS; b++) {
              // -----------------------------------
              // Check for the direction clockwise
              // -----------------------------------
              if(birdsActivity.rotateDirection[b] == BIRD_DIRECTION_CW) {
                  // Check if the position is at the end and invert the direction
                  if(birdsActivity.birdsRotation[b] >= BIRDS_MAX_ROT) {
                      // Invert the direction
                      birdsActivity.rotateDirection[b] = BIRD_DIRECTION_CCW;
                  } // Invert direction
                  else {
                      // Increment the step
                      birdsActivity.birdsRotation[b] += BIRDS_ROT_STEP;
                  } // Directiion clockwise
              } else {
                  // -----------------------------------
                  // Direction counterclockwise
                  // -----------------------------------
                  // Check if the position is at the end and invert the direction
                  if(birdsActivity.birdsRotation[b] <= BIRDS_MIN_ROT) {
                      // Invert the direction
                      birdsActivity.rotateDirection[b] = BIRD_DIRECTION_CW;
                  } // Invert direction
                  else {
                      // Decrement the step
                      birdsActivity.birdsRotation[b] -= BIRDS_ROT_STEP;
                  } // Decrement the position
              } // Direction counterclockwise
              // Position the bird servo
              birdsActivity.currentBird = b;
              rotateBird();
              delay(150);
            } // Loop the birds
        } // Dance until...
    }
}

/**
 * Rotate the currentbird consideing the direction
 */
void rotateBird() {
  setServo(birdsActivity.currentBird, 
      birdsActivity.birdsRotation[birdsActivity.currentBird]);
}

/**
 * Mnually rotate the current selected bird of a single step
 * when the right or let limit is reached the rotation stops
 */
void stepRotateBird() {

  if(birdsActivity.rotateDirection[birdsActivity.currentBird] == BIRD_DIRECTION_CW) {
      // Check if the position is at the end and invert the direction
      if(birdsActivity.birdsRotation[birdsActivity.currentBird] != BIRDS_MAX_ROT) {
        // Increment the step
        birdsActivity.birdsRotation[birdsActivity.currentBird] += BIRDS_ROT_STEP;
      } // Directiion clockwise
  } else {
      // -----------------------------------
      // Direction counterclockwise
      // -----------------------------------
      // Check if the position is at the end and invert the direction
      if(birdsActivity.birdsRotation[birdsActivity.currentBird] != BIRDS_MIN_ROT) {
        // Decrement the step
        birdsActivity.birdsRotation[birdsActivity.currentBird] -= BIRDS_ROT_STEP;
      } // Decrement the position
  } // Direction counterclockwise
  // Position the bird servo
  rotateBird();
}

/**
 * Execute the padByte command
 */
void setupCommand(int padByte) {

  switch(padByte) {
    
    case CMD_JUMP:
      // Set the jump status request to true. The jump will be executed
      // anyway from the main loop or the dance function and does no
      // change the other statuses. After the jump is exectued the request
      // is reset.
      birdsActivity.birdsJump = true;
      break;

    case CMD_ROTATE_RIGHT:
      // Set the rotation direction to clockwise.
      // This command disable the dance mode.
      birdsActivity.rotateDirection[birdsActivity.currentBird] = BIRD_DIRECTION_CW;
      stepRotateBird();
      delay(150); // avoid multiple commands
      break;

    case CMD_NEXT_BIRD:
      // Change the current controlled bird to the next one. If
      // the max limit is reached, the next bird is the first one
      // This command disable the dance mode.
      birdsActivity.currentBird++;
      if(birdsActivity.currentBird == NUM_BIRDS) {
        birdsActivity.currentBird = 0;
      }
      delay(500); // avoid multiple commands
      break;

    case CMD_ROTATE_LEFT:
      // Set the rotation direction to counterclockwise.
      // This command disable the dance mode.
      birdsActivity.rotateDirection[birdsActivity.currentBird] = BIRD_DIRECTION_CCW;
      stepRotateBird();
      delay(150); // avoid multiple commands
      break;

    case CMD_MUSIC:
      // Change the music status. 
      // Disabling the music dance if running is stoped
      if(birdsActivity.music) {
        birdsActivity.music = false;
        birdsActivity.dance = false;
      }
      else {
        birdsActivity.music = true;
      }
      powerMusic();
      break;

    case CMD_SOUND:
      // Set the change music request.
      // The command is processed in the main loop or at the end of a dance
      // sequece (end of movements loop cycle)
      birdsActivity.changeMusic = true;
      break;

    case CMD_DANCE:
      // Change the dance status. The command is processed in 
      // the main loop or in the dance command.
      // If the music is off the command has no effect (dance will be off)
      // Stopping dance does not stop the music
      if(birdsActivity.music) {
        birdsActivity.dance = true;
      }
      delay(500);
      break;
    
    case CMD_STOP:
      initActivityStatus(); // Reinitialize the system
      break;
  }
}

/**
 * Process the commands in the right order and eventually
 * pass the control to the dance process
 * The commands not managed by the commandProcessor() are
 * executed as they are detected vy the checkPadStaus() 
 * command parser
 */
 void commandProcessor() {
  jumpBirds();
  changeMusic();
  dance(DANCE_CYCLE);
 }
