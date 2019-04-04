/**
 * \file DancingBirds.ino
 * \brief The arduino file for the dancing birds project
 * 
 * \date September-October 2018
 * \author Enrico Miglino <balearicdynamics@gmail.com>
 * \version 1.0
 * 
 */
#include <Servo.h>

#define _DEBUG

#define BIRD1_PIN 0             ///< Bird pin index (servo control)
#define BIRD2_PIN 1             ///< Bird pin index (servo control)
#define BIRD3_PIN 2             ///< Bird pin index (servo control)
#define BIRD4_PIN 3             ///< Bird pin index (servo control)
#define BRID_PLATFORM_PIN 4     ///< Queue motion servo
#define MUSICBUTTON_PIN 5       ///< Music selection button servo 
#define MUSIC_CONTROL_PIN 2     ///< Music power pin

#define NUMSERVOS 6             ///< Number of servos organised in an array

#define MIN_ANGLE 0     ///< Real min angle of the servo. 0-180 Deg is refactored to it
#define MAX_ANGLE 150   ///< Ream max angle of the servo. 0-180 Deg is rfactored to it

//! Array of the servo pins
int servoPins[NUMSERVOS] = { 23, 25, 27, 29, 31, 33 };
//! Array of the servo library instances (one every servo)
Servo servos[NUMSERVOS];

// ===============================================
//  Pad pins
// ===============================================
#define NUM_PADS 4    ///< Pad buttons bits

//! Array of the pad pins
int padPins[NUM_PADS] = { 22, 24, 26, 28 };

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

#define BIRDS_JUMP_UP 90     ///< Birds platform up position (Deg)
#define BIRDS_JUMP_DOWN 135  ///< Birds platform down position (Deg)
#define BIRDS_JUMP_DELAY 500    ///< Duration of the birds platform jump

#define MUSIC_BUTTON_DOWN 60    ///< Music change button press position (Deg)
#define MUSIC_BUTTON_UP 0       ///< Music change button release position (Deg)
#define MUSIC_BUTTON_DELAY 600  ///< Delay of the music change button press to take effect (ms)

#define BIRD_DIRECTION_CW +1    ///< Birds rotation direction
#define BIRD_DIRECTION_CCW -1   ///< Birds rotation direction

//! Structure defining the status of the commands
typedef struct {
    boolean music;                      ///< The status of the music player
    boolean changeMusic;                ///< Request to press the music change button
    boolean dance;                      ///< Current dance status
    int birdsRotation[NUM_BIRDS];       ///< The birds rotation status
    boolean birdsJump;                  ///< Flex the birds
    int rotateDirection[NUM_BIRDS];     ///< Current rotation direction of the birds (+1/-1)
    int currentBird;                    ///< Current bird selected for motion
} cmdStatus;

cmdStatus birdsActivity;

/** 
 * Initialize the commands status structure and set the status 
 * of the servos accordingly
 */
void initActivityStatus() {
    int j;
    
    // Initialize the activity structure
    birdsActivity.music = false;
    birdsActivity.dance = false;
    birdsActivity.changeMusic = false;
    birdsActivity.birdsJump = false;
    birdsActivity.currentBird = 0;

    for(j = 0; j < NUM_BIRDS; j++) {
        birdsActivity.birdsRotation[j] = BIRDS_QUIET;
        birdsActivity.rotateDirection[j] = BIRD_DIRECTION_CW;
    }
    powerMusic();
    initServos();
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
 * Set On/Off the music player. he command needs half a second
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
 * Executes the dance sequence of type "A"
 * The dance steps are exectued starting from the current position
 * of every bird.
 */
void dance() {
    int j, k = 0;

    // Initialize the dance, only if the dance flag is set
    // else the birds position is reset to quiet and manual
    // rotation has no efect
//    if(birdsActivity.dance) {
//      initDance();
//      delay(250);
//    }

    // Dance until the dance stops
    while(birdsActivity.dance) {    
      int b;
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
          delay(50);

          // Check for new commands in queue.
          // Only the dance commands are accepted
          // Note that only music has effect on the global structure
          k = checkMicroBitCommand();
          // Check for jump
          if(k == CMD_JUMP) {
      birdsActivity.birdsJump = true;
            jumpBirds();
            delay(10);
          } // Jump
          // Check for msic change
          if(k == CMD_SOUND) {
            birdsActivity.changeMusic = true;
            changeMusic();
            delay(100);
          } // Change music
          // Check for msic stop, dance ends
          if(k == CMD_MUSIC) {
            birdsActivity.music = false;
            birdsActivity.dance = false;
            powerMusic();
            delay(100);
          } // Music On/Off
        } // Loop the birds
  } // Dance until...
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
void manualRotateBird() {

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
#ifdef _DEBUG
  Serial.print("Brid ");
  Serial.println(birdsActivity.currentBird);
  Serial.print(" Deg ");
  Serial.println(birdsActivity.birdsRotation[birdsActivity.currentBird]);
#endif
}

/**
 * Check if a command byte is available from the micro:bit signals.
 * This function is used by the nomal checkPadStatus() command
 * parser and by the Dance() function to make it interruptable
 * when the music is disabled.
 */
int checkMicroBitCommand() {
  int padByte = 0x00;

  // Calculate the bite value
  padByte = 1 * digitalRead(padPins[0]) +
            2 * digitalRead(padPins[1]) +
            4 * digitalRead(padPins[2]) +
            8 * digitalRead(padPins[3]);
  
  return padByte;
}

/**
 * Check the status of the pad pins and update the 
 * status byte between 0x01 and 0x08
 */
void checkPadStatus() {
  int padByte = 0x00;

  padByte = checkMicroBitCommand();

  switch(padByte) {
    
    case CMD_JUMP:
#ifdef _DEBUG
  Serial.println("CMD_JUMP");
#endif
      // Set the jump status request to true. The jump will be executed
      // anyway from the main loop or the dance function and does no
      // change the other statuses. After the jump is exectued the request
      // is reset.
      birdsActivity.birdsJump = true;
      break;

    case CMD_ROTATE_RIGHT:
#ifdef _DEBUG
  Serial.println("CMD_ROTATE_RIGHT");
#endif
      // Set the rotation direction to clockwise.
      // This command disable the dance mode.
      birdsActivity.rotateDirection[birdsActivity.currentBird] = BIRD_DIRECTION_CW;
      manualRotateBird();
      delay(150); // avoid multiple commands
      break;

    case CMD_NEXT_BIRD:
#ifdef _DEBUG
  Serial.println("CMD_NEXT_BIRD");
#endif
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
#ifdef _DEBUG
  Serial.println("CMD_ROTATE_LEFT");
#endif
      // Set the rotation direction to counterclockwise.
      // This command disable the dance mode.
      birdsActivity.rotateDirection[birdsActivity.currentBird] = BIRD_DIRECTION_CCW;
      manualRotateBird();
      delay(150); // avoid multiple commands
      break;

    case CMD_MUSIC:
#ifdef _DEBUG
  Serial.println("CMD_MUSIC");
#endif
      // Change the music status. The command is processed in 
      // the main loop main or in the dance command/
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
#ifdef _DEBUG
  Serial.println("CMD_SOUND");
#endif
      // Set the change music request.
      // The command is processed in the main loop or at the end of a dance
      // sequece (end of movements loop cycle)
      birdsActivity.changeMusic = true;
      break;

    case CMD_DANCE:
#ifdef _DEBUG
  Serial.println("CMD_DANCE");
#endif
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
#ifdef _DEBUG
  Serial.println("CMD_STOP");
#endif
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
  dance();
 }

//! Initialisation function
void setup() {
    int j;

    // Initialize the music power On/Off
    pinMode(MUSIC_CONTROL_PIN, OUTPUT);
    // Initialize the control pad pins
    for(j = 0; j < NUM_PADS; j++) {
      pinMode(padPins[j], INPUT);
    }
    // Initilize the status structure and the servos
    initActivityStatus();
    // Test the servos on startup
    testSystem();

#ifdef _DEBUG
    Serial.begin(9600);
    Serial.println("\nStarted");
#endif
}

//! Application main loop
//! Constantly check the pad status and launch the command
//! processor.
void loop() {
  checkPadStatus();
  commandProcessor();
  delay(200);
}
