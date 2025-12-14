
// Common pin usage
// Note there are additional pins unique to transmitter or receiver
//

// nRF24L01 uses SPI which is fixed
// on pins 11, 12, and 13 on the Uno
// and on pins 50, 51, and 52 on the Mega

// It also requires two other signals
// (CE = Chip Enable, CSN = Chip Select Not)
// Which can be any pins:

// CHANGEHERE
// For the transmitter
//const int NRF_CE_PIN = A4, NRF_CSN_PIN = A5;

// CHANGEHERE
// for the receiver
const int NRF_CE_PIN = A11, NRF_CSN_PIN = A15;

// nRF 24L01 pin   name
//          1      GND
//          2      3.3V
//          3      CE
//          4      CSN
//          5      SCLK
//          6      MOSI/COPI
//          7      MISO/CIPO

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);  // CE, CSN

//#include <printf.h>  // for debugging



const int CUSTOM_CHANNEL_NUMBER = 60;   // dONT CHANGE
const byte CUSTOM_ADDRESS_BYTE = 0xC6;  // DONT CHANGE

// Do not make changes here
const byte xmtrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0x66 };

const int RF24_POWER_LEVEL = RF24_PA_LOW;

// global variables
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;

struct DataStruct {
  uint8_t stateNumber;
};
DataStruct data;

void setupRF24Common() {

  // RF24 setup
  if (!radio.begin()) {
    Serial.println(F("radio  initialization failed"));
    while (1)
      ;
  } else {
    Serial.println(F("radio successfully initialized"));
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(CUSTOM_CHANNEL_NUMBER);
  radio.setPALevel(RF24_POWER_LEVEL);
}

// Receiver Code
// CHANGEHERE

// Additional libraries for music maker shield
#include <Adafruit_VS1053.h>
#include <SD.h>

// Servo library
#include <Servo.h>

// Additional libraries for NeoPixel Strip
#include <Adafruit_NeoPixel.h>
// Matrix libraries not needed for strip
//#include <Adafruit_GFX.h>
//#include <Adafruit_NeoMatrix.h>
//#ifndef PSTR
//#define PSTR  // Make Arduino Due happy
//#endif

// Additional pin usage for receiver

// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// Connectors for NeoPixels and Servo Motors are labeled
// on the circuit board
// and use pins 16, 17, 18, 19, 20, and 21

// Servo motors
const int SHOULDER_RIGHT_PIN = 20;  // Right shoulder with elbow and attachments
const int SHOULDER_LEFT_PIN = 21;   // Left shoulder (no attachments)
const int HEAD_SERVO_PIN = 17;
const int ELBOW_SERVO_PIN = 18;
//const int ANTENNA_SERVO_PIN = 16;
//const int TAIL_SERVO_PIN = 17;
//const int GRABBER_SERVO_PIN = 18;

// Neopixel Strip
const int NEOPIXELPIN = 19;
const int NUMPIXELS = 64;
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
// Matrix not used - using strip instead
//Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, NEOPIXELPIN,
//                                               NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
//                                               NEO_GRB + NEO_KHZ800);

Servo shoulderRight;  // Right shoulder with elbow and attachments
Servo shoulderLeft;   // Left shoulder (no attachments)
Servo head;           // Head servo
Servo elbowRight;     // Right elbow servo
Servo antenna;
Servo tail;
Servo grabber;
Servo disk;

// change as per your robot
const int SHOULDER_RIGHT_RESET_ANGLE = 30;  // Right shoulder reset position
const int SHOULDER_LEFT_RESET_ANGLE = 30;   // Left shoulder reset position
const int HEAD_RESET_ANGLE = 90;
const int ELBOW_RESET_ANGLE = 0;

// Elbow servo angles for 4 different arm attachment positions
const int ELBOW_ARM_1_ANGLE = 0;    // Pencil 1 angle
const int ELBOW_ARM_2_ANGLE = 90;   // Sword 2 angle
const int ELBOW_ARM_3_ANGLE = 180;  // Toy 3 angle
const int ELBOW_ARM_4_ANGLE = 270;  // Bottle 4 angle

void setup() {
  Serial.begin(9600);
  // printf_begin();

  // Set up all the attached hardware
  setupMusicMakerShield();
  setupServoMotors();
  setupNeoPixels();

  setupRF24();

  // Brief flash to show we're done with setup()
  flashNeoPixels();

  // Print serial monitor instructions
  Serial.println(F("======================================"));
  Serial.println(F("Setup complete!"));
  Serial.println(F("======================================"));
  Serial.println(F("Serial Monitor Test Mode Available:"));
  Serial.println(F("Enter 0-21 to test cases:"));
  Serial.println(F("  0 = RESET to initial position"));
  Serial.println(F("  1-4 = Play track001.mp3 - track004.mp3"));
  Serial.println(F("  5 = Play track021.mp3"));
  Serial.println(F("  6-21 = Play track005.mp3 - track020.mp3"));
  Serial.println(F("======================================"));
}

void setupRF24() {

  // Check whether the correct pins are assigned
  if (NRF_CE_PIN != A11 || NRF_CSN_PIN != A15) {
    Serial.println(F("The wrong NRF_CE_PIN and NRF_CSN_PIN pins are defined for a receiver"));
    while (1)
      ;
  }

  setupRF24Common();

  // Set us as a receiver
  radio.openWritingPipe(rcvrAddress);
  radio.openReadingPipe(1, xmtrAddress);

  // radio.printPrettyDetails();
  Serial.print(F("I am a receiver on channel "));
  Serial.print(CUSTOM_CHANNEL_NUMBER);
  Serial.print(" and at address 0x");
  Serial.print(CUSTOM_ADDRESS_BYTE, HEX);
  Serial.println("");
}

void setupMusicMakerShield() {
  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1)
      ;  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}

void setupServoMotors() {
  head.attach(HEAD_SERVO_PIN);
  head.write(HEAD_RESET_ANGLE);

  shoulderRight.attach(SHOULDER_RIGHT_PIN);
  shoulderRight.write(SHOULDER_RIGHT_RESET_ANGLE);

  shoulderLeft.attach(SHOULDER_LEFT_PIN);
  shoulderLeft.write(180 - SHOULDER_LEFT_RESET_ANGLE);

  elbowRight.attach(ELBOW_SERVO_PIN);
  elbowRight.write(ELBOW_RESET_ANGLE);

  //  antenna.attach(ANTENNA_SERVO_PIN);
  //  tail.attach(TAIL_SERVO_PIN);
  //  grabber.attach(GRABBER_SERVO_PIN);
  //
  //  tail.write(TAIL_HAPPY);
}

void setupNeoPixels() {
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(40);
  pixels.show();
}

void flashNeoPixels() {
  // all on
  for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
  }
  pixels.show();
  delay(500);

  // all off
  pixels.clear();
  pixels.show();
}

// this is just an example - change to suit your needs
int currentHeadAngle = HEAD_RESET_ANGLE;
int headStepSize = 5;
int currentShoulderRightAngle = SHOULDER_RIGHT_RESET_ANGLE;  // Right shoulder current angle
int currentShoulderLeftAngle = SHOULDER_LEFT_RESET_ANGLE;    // Left shoulder current angle
int shoulderStepSize = 5;                                     // smooth step for both shoulders
int currentElbowAngle = ELBOW_RESET_ANGLE;
int elbowStepSize = 5;

// Rainbow gradient animation variables
unsigned long previousRainbowMillis = 0;
const long rainbowInterval = 20;  // Update every 20ms for smooth animation
uint16_t rainbowCycle = 0;        // Current position in rainbow cycle

// Helper function: Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Non-blocking rainbow gradient animation
void updateRainbow() {
  unsigned long currentMillis = millis();

  // Check if it's time to update the rainbow
  if (currentMillis - previousRainbowMillis >= rainbowInterval) {
    previousRainbowMillis = currentMillis;

    // Update all pixels with rainbow gradient
    for (int i = 0; i < NUMPIXELS; i++) {
      // Calculate color for this pixel based on position and cycle
      int pixelHue = ((i * 256 / NUMPIXELS) + rainbowCycle) & 255;
      pixels.setPixelColor(i, Wheel(pixelHue));
    }
    pixels.show();

    // Increment rainbow cycle for next frame
    rainbowCycle++;
    if (rainbowCycle >= 256) {
      rainbowCycle = 0;
    }
  }
}

// ========================================
// SMOOTH NEOPIXEL TRANSITION SYSTEM
// ========================================

// Storage for smooth transitions
uint32_t currentColors[64];  // Current color of each pixel
uint32_t targetColors[64];   // Target color for each pixel
bool isTransitioning = false;
float transitionProgress = 0.0;
float transitionSpeed = 0.05; // 0.01 = slow, 0.1 = fast

// Pattern types
enum PatternType {
  PATTERN_RAINBOW,
  PATTERN_SOLID,
  PATTERN_GRADIENT,
  PATTERN_ALTERNATING,
  PATTERN_CHASE
};

// Helper: Extract RGB from 32-bit color
void extractRGB(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (color >> 16) & 0xFF;
  g = (color >> 8) & 0xFF;
  b = color & 0xFF;
}

// Helper: Interpolate between two colors
uint32_t interpolateColor(uint32_t color1, uint32_t color2, float progress) {
  uint8_t r1, g1, b1, r2, g2, b2;
  extractRGB(color1, r1, g1, b1);
  extractRGB(color2, r2, g2, b2);

  uint8_t r = r1 + (r2 - r1) * progress;
  uint8_t g = g1 + (g2 - g1) * progress;
  uint8_t b = b1 + (b2 - b1) * progress;

  return pixels.Color(r, g, b);
}

// Set target pattern: Solid Color
void setTargetSolid(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = pixels.Color(r, g, b);
  for (int i = 0; i < NUMPIXELS; i++) {
    targetColors[i] = color;
  }
}

// Set target pattern: Two-color Gradient
void setTargetGradient(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {
  for (int i = 0; i < NUMPIXELS; i++) {
    float ratio = (float)i / (NUMPIXELS - 1);
    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;
    targetColors[i] = pixels.Color(r, g, b);
  }
}

// Set target pattern: Rainbow (static snapshot)
void setTargetRainbow() {
  for (int i = 0; i < NUMPIXELS; i++) {
    int pixelHue = (i * 256 / NUMPIXELS) & 255;
    targetColors[i] = Wheel(pixelHue);
  }
}

// Set target pattern: Alternating two colors
void setTargetAlternating(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {
  uint32_t color1 = pixels.Color(r1, g1, b1);
  uint32_t color2 = pixels.Color(r2, g2, b2);
  for (int i = 0; i < NUMPIXELS; i++) {
    targetColors[i] = (i % 2 == 0) ? color1 : color2;
  }
}

// Set target pattern: Chase pattern
void setTargetChase(uint8_t r, uint8_t g, uint8_t b, int chaseLength, int offset) {
  uint32_t color = pixels.Color(r, g, b);
  for (int i = 0; i < NUMPIXELS; i++) {
    int pos = (i + offset) % NUMPIXELS;
    if ((pos % (chaseLength * 2)) < chaseLength) {
      targetColors[i] = color;
    } else {
      targetColors[i] = pixels.Color(0, 0, 0);
    }
  }
}

// Start a smooth transition to the target pattern
void startTransition(float speed = 0.05) {
  // Capture current pixel colors
  for (int i = 0; i < NUMPIXELS; i++) {
    currentColors[i] = pixels.getPixelColor(i);
  }

  isTransitioning = true;
  transitionProgress = 0.0;
  transitionSpeed = speed;
}

// Update transition (call this in loop or during blocking operations)
void updateTransition() {
  if (!isTransitioning) return;

  transitionProgress += transitionSpeed;

  if (transitionProgress >= 1.0) {
    // Transition complete
    transitionProgress = 1.0;
    isTransitioning = false;
  }

  // Interpolate all pixels
  for (int i = 0; i < NUMPIXELS; i++) {
    uint32_t color = interpolateColor(currentColors[i], targetColors[i], transitionProgress);
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

// Blocking transition - completes the full transition before returning
void transitionToPattern(PatternType pattern, uint32_t param1 = 0, uint32_t param2 = 0, float speed = 0.05) {
  // Set target based on pattern type
  switch (pattern) {
    case PATTERN_SOLID:
      setTargetSolid((param1 >> 16) & 0xFF, (param1 >> 8) & 0xFF, param1 & 0xFF);
      break;
    case PATTERN_GRADIENT:
      setTargetGradient(
        (param1 >> 16) & 0xFF, (param1 >> 8) & 0xFF, param1 & 0xFF,
        (param2 >> 16) & 0xFF, (param2 >> 8) & 0xFF, param2 & 0xFF
      );
      break;
    case PATTERN_RAINBOW:
      setTargetRainbow();
      break;
    case PATTERN_ALTERNATING:
      setTargetAlternating(
        (param1 >> 16) & 0xFF, (param1 >> 8) & 0xFF, param1 & 0xFF,
        (param2 >> 16) & 0xFF, (param2 >> 8) & 0xFF, param2 & 0xFF
      );
      break;
    case PATTERN_CHASE:
      setTargetChase((param1 >> 16) & 0xFF, (param1 >> 8) & 0xFF, param1 & 0xFF, 4, 0);
      break;
  }

  // Start and complete transition
  startTransition(speed);
  while (isTransitioning) {
    updateTransition();
    delay(20);
  }
}

// Easy-to-use helper functions for common patterns

void transitionToSolid(uint8_t r, uint8_t g, uint8_t b, float speed = 0.05) {
  uint32_t color = (r << 16) | (g << 8) | b;
  transitionToPattern(PATTERN_SOLID, color, 0, speed);
}

void transitionToGradient(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float speed = 0.05) {
  uint32_t color1 = (r1 << 16) | (g1 << 8) | b1;
  uint32_t color2 = (r2 << 16) | (g2 << 8) | b2;
  transitionToPattern(PATTERN_GRADIENT, color1, color2, speed);
}

void transitionToRainbow(float speed = 0.05) {
  transitionToPattern(PATTERN_RAINBOW, 0, 0, speed);
}

void transitionToAlternating(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float speed = 0.05) {
  uint32_t color1 = (r1 << 16) | (g1 << 8) | b1;
  uint32_t color2 = (r2 << 16) | (g2 << 8) | b2;
  transitionToPattern(PATTERN_ALTERNATING, color1, color2, speed);
}

void transitionToOff(float speed = 0.05) {
  transitionToSolid(0, 0, 0, speed);
}

// ========================================
// END NEOPIXEL TRANSITION SYSTEM
// ========================================

void resetRobotPose() {
  Serial.println(F("Resetting robot to initial position"));

  //
  // --- RESET RIGHT SHOULDER ---
  //
  int shoulderRightTarget = SHOULDER_RIGHT_RESET_ANGLE;

  if (currentShoulderRightAngle > shoulderRightTarget) {
    while (currentShoulderRightAngle > shoulderRightTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < shoulderRightTarget)
          currentShoulderRightAngle = shoulderRightTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle < shoulderRightTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > shoulderRightTarget)
          currentShoulderRightAngle = shoulderRightTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  //
  // --- RESET LEFT SHOULDER ---
  //
  int shoulderLeftTarget = SHOULDER_LEFT_RESET_ANGLE;

  if (currentShoulderLeftAngle < shoulderLeftTarget) {
    while (currentShoulderLeftAngle < shoulderLeftTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > shoulderLeftTarget)
          currentShoulderLeftAngle = shoulderLeftTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle > shoulderLeftTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < shoulderLeftTarget)
          currentShoulderLeftAngle = shoulderLeftTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  delay(1200);

  //
  // --- RESET ELBOW ---
  //
  int elbowTarget = ELBOW_RESET_ANGLE;

  if (currentElbowAngle > elbowTarget) {
    while (currentElbowAngle > elbowTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowTarget)
          currentElbowAngle = elbowTarget;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle < elbowTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowTarget)
          currentElbowAngle = elbowTarget;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(1800);

  //
  // --- RESET HEAD ---
  //
  int headTarget = HEAD_RESET_ANGLE;

  if (currentHeadAngle > headTarget) {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
          currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
          currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }
}

void quickResetRobotPose() {
  Serial.println(F("Quick reset to initial position (2 seconds)"));

  // Very fast reset with larger steps and minimal delays
  int fastStepSize = 15;  // Larger steps for faster movement
  int fastDelay = 5;      // Minimal delay

  //
  // --- RESET ALL SERVOS SIMULTANEOUSLY ---
  //
  bool allDone = false;
  while (!allDone) {
    allDone = true;

    // Reset right shoulder
    if (currentShoulderRightAngle < SHOULDER_RIGHT_RESET_ANGLE) {
      currentShoulderRightAngle += fastStepSize;
      if (currentShoulderRightAngle > SHOULDER_RIGHT_RESET_ANGLE)
        currentShoulderRightAngle = SHOULDER_RIGHT_RESET_ANGLE;
      shoulderRight.write(currentShoulderRightAngle);
      allDone = false;
    } else if (currentShoulderRightAngle > SHOULDER_RIGHT_RESET_ANGLE) {
      currentShoulderRightAngle -= fastStepSize;
      if (currentShoulderRightAngle < SHOULDER_RIGHT_RESET_ANGLE)
        currentShoulderRightAngle = SHOULDER_RIGHT_RESET_ANGLE;
      shoulderRight.write(currentShoulderRightAngle);
      allDone = false;
    }

    // Reset left shoulder
    if (currentShoulderLeftAngle > SHOULDER_LEFT_RESET_ANGLE) {
      currentShoulderLeftAngle -= fastStepSize;
      if (currentShoulderLeftAngle < SHOULDER_LEFT_RESET_ANGLE)
        currentShoulderLeftAngle = SHOULDER_LEFT_RESET_ANGLE;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      allDone = false;
    } else if (currentShoulderLeftAngle < SHOULDER_LEFT_RESET_ANGLE) {
      currentShoulderLeftAngle += fastStepSize;
      if (currentShoulderLeftAngle > SHOULDER_LEFT_RESET_ANGLE)
        currentShoulderLeftAngle = SHOULDER_LEFT_RESET_ANGLE;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      allDone = false;
    }

    // Reset elbow
    if (currentElbowAngle < ELBOW_RESET_ANGLE) {
      currentElbowAngle += fastStepSize;
      if (currentElbowAngle > ELBOW_RESET_ANGLE)
        currentElbowAngle = ELBOW_RESET_ANGLE;
      elbowRight.write(currentElbowAngle);
      allDone = false;
    } else if (currentElbowAngle > ELBOW_RESET_ANGLE) {
      currentElbowAngle -= fastStepSize;
      if (currentElbowAngle < ELBOW_RESET_ANGLE)
        currentElbowAngle = ELBOW_RESET_ANGLE;
      elbowRight.write(currentElbowAngle);
      allDone = false;
    }

    // Reset head
    if (currentHeadAngle < HEAD_RESET_ANGLE) {
      currentHeadAngle += fastStepSize;
      if (currentHeadAngle > HEAD_RESET_ANGLE)
        currentHeadAngle = HEAD_RESET_ANGLE;
      head.write(currentHeadAngle);
      allDone = false;
    } else if (currentHeadAngle > HEAD_RESET_ANGLE) {
      currentHeadAngle -= fastStepSize;
      if (currentHeadAngle < HEAD_RESET_ANGLE)
        currentHeadAngle = HEAD_RESET_ANGLE;
      head.write(currentHeadAngle);
      allDone = false;
    }

    delay(fastDelay);
  }
}

void resetShoulderAndHead() {
  Serial.println(F("Resetting ONLY shoulders and head to initial position"));

  //
  // --- RESET RIGHT SHOULDER ---
  //
  int shoulderRightTarget = SHOULDER_RIGHT_RESET_ANGLE;

  if (currentShoulderRightAngle > shoulderRightTarget) {
    while (currentShoulderRightAngle > shoulderRightTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < shoulderRightTarget)
          currentShoulderRightAngle = shoulderRightTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle < shoulderRightTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > shoulderRightTarget)
          currentShoulderRightAngle = shoulderRightTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  //
  // --- RESET LEFT SHOULDER ---
  //
  int shoulderLeftTarget = SHOULDER_LEFT_RESET_ANGLE;

  if (currentShoulderLeftAngle < shoulderLeftTarget) {
    while (currentShoulderLeftAngle < shoulderLeftTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > shoulderLeftTarget)
          currentShoulderLeftAngle = shoulderLeftTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle > shoulderLeftTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < shoulderLeftTarget)
          currentShoulderLeftAngle = shoulderLeftTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  delay(1200);

  //
  // --- RESET HEAD ---
  //
  int headTarget = HEAD_RESET_ANGLE;

  if (currentHeadAngle > headTarget) {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
          currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
          currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }
}

void processCommand(uint8_t commandNumber) {
  Serial.print(F("Processing command: "));
  Serial.println(commandNumber);

  switch (commandNumber) {
 
case 0: {
  Serial.println(F("Case 0: ROBOT DANCE PARTY!"));

  //
  // --- PHASE 1: WAVE MOTION WITH RAINBOW CYCLING ---
  //
  Serial.println(F("Dance Phase 1: Wave Motion"));

  unsigned long phase1Start = millis();
  unsigned long phase1Duration = 3000;  // 3 seconds
  int waveCount = 0;

  while (millis() - phase1Start < phase1Duration) {
    // Rainbow color cycling
    int hue = (millis() / 10) % 256;
    for (int i = 0; i < NUMPIXELS; i++) {
      int pixelHue = (hue + (i * 256 / NUMPIXELS)) % 256;
      uint32_t color = pixels.ColorHSV(pixelHue * 256);
      pixels.setPixelColor(i, color);
    }
    pixels.show();

    // Alternating shoulder waves
    if (waveCount % 2 == 0) {
      // Right shoulder up, left down
      currentShoulderRightAngle = 80;
      currentShoulderLeftAngle = 30;
    } else {
      // Left shoulder up, right down
      currentShoulderRightAngle = 30;
      currentShoulderLeftAngle = 80;
    }
    shoulderRight.write(currentShoulderRightAngle);
    shoulderLeft.write(180 - currentShoulderLeftAngle);

    delay(300);
    waveCount++;
  }

  //
  // --- PHASE 2: HEAD NODDING WITH PULSING COLORS ---
  //
  Serial.println(F("Dance Phase 2: Head Nodding"));

  unsigned long phase2Start = millis();
  unsigned long phase2Duration = 3000;  // 3 seconds
  bool nodUp = true;

  while (millis() - phase2Start < phase2Duration) {
    // Pulsing color effect
    int brightness = 128 + 127 * sin((millis() / 200.0));
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(brightness, 0, 255 - brightness));
    }
    pixels.show();

    // Head nodding
    if (nodUp) {
      currentHeadAngle += 5;
      if (currentHeadAngle >= 120) nodUp = false;
    } else {
      currentHeadAngle -= 5;
      if (currentHeadAngle <= 60) nodUp = true;
    }
    head.write(currentHeadAngle);

    // Both shoulders at 60 degrees
    currentShoulderRightAngle = 60;
    currentShoulderLeftAngle = 60;
    shoulderRight.write(currentShoulderRightAngle);
    shoulderLeft.write(180 - currentShoulderLeftAngle);

    delay(50);
  }

  //
  // --- PHASE 3: ATTACHMENT SHOWCASE ---
  //
  Serial.println(F("Dance Phase 3: Attachment Showcase"));

  int attachmentAngles[4] = {ELBOW_ARM_1_ANGLE, ELBOW_ARM_2_ANGLE, ELBOW_ARM_3_ANGLE, ELBOW_ARM_4_ANGLE};
  uint32_t attachmentColors[4] = {
    pixels.Color(0, 255, 255),    // Cyan for Pen
    pixels.Color(255, 0, 0),      // Red for Sword
    pixels.Color(255, 105, 180),  // Pink for Toy
    pixels.Color(255, 165, 0)     // Orange for Bottle
  };

  for (int i = 0; i < 4; i++) {
    // Move to attachment angle
    int targetAngle = attachmentAngles[i];
    while (currentElbowAngle != targetAngle) {
      if (currentElbowAngle < targetAngle) {
        currentElbowAngle += 15;
        if (currentElbowAngle > targetAngle) currentElbowAngle = targetAngle;
      } else {
        currentElbowAngle -= 15;
        if (currentElbowAngle < targetAngle) currentElbowAngle = targetAngle;
      }
      elbowRight.write(currentElbowAngle);
      delay(20);
    }

    // Light up with attachment color
    for (int j = 0; j < NUMPIXELS; j++) {
      pixels.setPixelColor(j, attachmentColors[i]);
    }
    pixels.show();

    // Quick shoulder raise
    currentShoulderRightAngle = 90;
    shoulderRight.write(currentShoulderRightAngle);
    delay(500);

    currentShoulderRightAngle = 30;
    shoulderRight.write(currentShoulderRightAngle);
    delay(200);
  }

  //
  // --- PHASE 4: GRAND FINALE ---
  //
  Serial.println(F("Dance Phase 4: Grand Finale"));

  unsigned long phase4Start = millis();
  unsigned long phase4Duration = 3000;  // 3 seconds
  int colorIndex = 0;

  uint32_t finaleColors[6] = {
    pixels.Color(255, 0, 0),      // Red
    pixels.Color(255, 165, 0),    // Orange
    pixels.Color(255, 255, 0),    // Yellow
    pixels.Color(0, 255, 0),      // Green
    pixels.Color(0, 0, 255),      // Blue
    pixels.Color(128, 0, 128)     // Purple
  };

  unsigned long lastColorChange = millis();

  while (millis() - phase4Start < phase4Duration) {
    // Fast color changes
    if (millis() - lastColorChange >= 200) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, finaleColors[colorIndex]);
      }
      pixels.show();
      colorIndex = (colorIndex + 1) % 6;
      lastColorChange = millis();
    }

    // All joints moving
    currentHeadAngle = 90 + 20 * sin(millis() / 200.0);
    head.write(currentHeadAngle);

    currentShoulderRightAngle = 60 + 30 * sin(millis() / 250.0);
    shoulderRight.write(currentShoulderRightAngle);

    currentShoulderLeftAngle = 60 + 30 * cos(millis() / 250.0);
    shoulderLeft.write(180 - currentShoulderLeftAngle);

    currentElbowAngle = 90 + 50 * sin(millis() / 300.0);
    elbowRight.write(currentElbowAngle);

    delay(20);
  }

  //
  // --- TRANSITION BACK TO RAINBOW ---
  //
  Serial.println(F("Dance complete! Returning to default"));
  transitionToRainbow(0.05);

  //
  // --- RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 1: {
  Serial.println(F("Case 1: Both shoulders up, Toy arm swing, Blue gradient"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track001.mp3"));
  musicPlayer.startPlayingFile("/track001.mp3");

  // Transition NeoPixels to blue gradient (dark blue to bright blue)
  transitionToGradient(0, 0, 100, 0, 100, 255, 0.05);

  //
  // --- MOVE LEFT SHOULDER TO 90° ---
  //
  int leftShoulderTarget = 90;

  if (currentShoulderLeftAngle > leftShoulderTarget) {
    while (currentShoulderLeftAngle > leftShoulderTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  //
  // --- MOVE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- MOVE ELBOW TO TOY ANGLE (180°) ---
  //
  int toyAngle = ELBOW_ARM_3_ANGLE;  // Toy attachment angle

  if (currentElbowAngle < toyAngle) {
    while (currentElbowAngle < toyAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > toyAngle)
        currentElbowAngle = toyAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > toyAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < toyAngle)
        currentElbowAngle = toyAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SWING TOY ARM +30/-30 FOR 5 SECONDS ---
  //
  Serial.println(F("Swinging toy arm"));
  unsigned long swingStartTime = millis();
  unsigned long swingDuration = 5000;  // 5 seconds
  int swingAmplitude = 30;  // Full 30 degree swing
  int baseAngle = toyAngle;
  bool swingUp = true;

  while (millis() - swingStartTime < swingDuration) {
    if (swingUp) {
      // Swing up (+30)
      int targetAngle = baseAngle + swingAmplitude;
      while (currentElbowAngle < targetAngle) {
        currentElbowAngle += 3;  // 3-degree steps
        if (currentElbowAngle > targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);  // Give servo time to physically move
      }
      swingUp = false;
    } else {
      // Swing down (-30)
      int targetAngle = baseAngle - swingAmplitude;
      while (currentElbowAngle > targetAngle) {
        currentElbowAngle -= 3;  // 3-degree steps
        if (currentElbowAngle < targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);  // Give servo time to physically move
      }
      swingUp = true;
    }
  }

  // Return to base toy angle
  while (currentElbowAngle < baseAngle) {
    currentElbowAngle += elbowStepSize;
    if (currentElbowAngle > baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }
  while (currentElbowAngle > baseAngle) {
    currentElbowAngle -= elbowStepSize;
    if (currentElbowAngle < baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }

  // Wait for audio to finish
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 2: {
  Serial.println(F("Case 2: Party lights, Pen swing, Head movement"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track002.mp3"));
  musicPlayer.startPlayingFile("/track002.mp3");

  //
  // --- MOVE ELBOW TO PEN ANGLE (0°) ---
  //
  int penAngle = ELBOW_ARM_1_ANGLE;  // Pen attachment angle

  if (currentElbowAngle < penAngle) {
    while (currentElbowAngle < penAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > penAngle)
        currentElbowAngle = penAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > penAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < penAngle)
        currentElbowAngle = penAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- MOVE HEAD TO 60° (Turn right) ---
  //
  int headTarget = 60;

  if (currentHeadAngle < headTarget) {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SWING PEN + FAST PARTY LIGHTS FOR 5 SECONDS ---
  //
  Serial.println(F("Party mode with pen swing!"));
  unsigned long partyStartTime = millis();
  unsigned long partyDuration = 5000;  // 5 seconds
  int swingAmplitude = 30;
  int baseAngle = penAngle;
  bool swingUp = true;

  // Party light colors
  uint8_t colorIndex = 0;
  uint8_t partyColors[6][3] = {
    {255, 0, 0},    // Red
    {0, 255, 0},    // Green
    {0, 0, 255},    // Blue
    {255, 255, 0},  // Yellow
    {255, 0, 255},  // Magenta
    {0, 255, 255}   // Cyan
  };

  while (millis() - partyStartTime < partyDuration) {
    // Fast party lights - change every cycle
    for (int i = 0; i < NUMPIXELS; i++) {
      // Alternate colors across the strip
      int colorIdx = (i + colorIndex) % 6;
      pixels.setPixelColor(i, pixels.Color(
        partyColors[colorIdx][0],
        partyColors[colorIdx][1],
        partyColors[colorIdx][2]
      ));
    }
    pixels.show();
    colorIndex = (colorIndex + 1) % 6;  // Cycle through colors

    // Swing motion
    if (swingUp) {
      // Swing up (+30)
      int targetAngle = baseAngle + swingAmplitude;
      while (currentElbowAngle < targetAngle && (millis() - partyStartTime < partyDuration)) {
        currentElbowAngle += 3;
        if (currentElbowAngle > targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);

        // Update party lights during swing
        for (int i = 0; i < NUMPIXELS; i++) {
          int colorIdx = (i + colorIndex) % 6;
          pixels.setPixelColor(i, pixels.Color(
            partyColors[colorIdx][0],
            partyColors[colorIdx][1],
            partyColors[colorIdx][2]
          ));
        }
        pixels.show();
        colorIndex = (colorIndex + 1) % 6;

        delay(30);
      }
      swingUp = false;
    } else {
      // Swing down (-30)
      int targetAngle = baseAngle - swingAmplitude;
      while (currentElbowAngle > targetAngle && (millis() - partyStartTime < partyDuration)) {
        currentElbowAngle -= 3;
        if (currentElbowAngle < targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);

        // Update party lights during swing
        for (int i = 0; i < NUMPIXELS; i++) {
          int colorIdx = (i + colorIndex) % 6;
          pixels.setPixelColor(i, pixels.Color(
            partyColors[colorIdx][0],
            partyColors[colorIdx][1],
            partyColors[colorIdx][2]
          ));
        }
        pixels.show();
        colorIndex = (colorIndex + 1) % 6;

        delay(30);
      }
      swingUp = true;
    }
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  // Return pen to base angle
  while (currentElbowAngle < baseAngle) {
    currentElbowAngle += elbowStepSize;
    if (currentElbowAngle > baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }
  while (currentElbowAngle > baseAngle) {
    currentElbowAngle -= elbowStepSize;
    if (currentElbowAngle < baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }

  //
  // --- MOVE HEAD BACK TO 90° ---
  //
  headTarget = 90;

  if (currentHeadAngle < headTarget) {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  // Wait for audio to finish
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 3: {
  Serial.println(F("Case 3: Aggressive mode - Red lights, Sword swing, Head swing"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track003.mp3"));
  musicPlayer.startPlayingFile("/track003.mp3");

  // Transition NeoPixels to aggressive bright red gradient
  transitionToGradient(255, 0, 0, 150, 0, 0, 0.05);  // Bright red to darker red

  //
  // --- MOVE ELBOW TO SWORD ANGLE (90°) ---
  //
  int swordAngle = ELBOW_ARM_2_ANGLE;  // Sword attachment angle

  if (currentElbowAngle < swordAngle) {
    while (currentElbowAngle < swordAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > swordAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  //
  // --- RAISE LEFT SHOULDER ALL THE WAY (180°) ---
  //
  int leftShoulderTarget = 180;

  if (currentShoulderLeftAngle > leftShoulderTarget) {
    while (currentShoulderLeftAngle > leftShoulderTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SWING SWORD + HEAD SIMULTANEOUSLY FOR 5 SECONDS ---
  //
  Serial.println(F("Aggressive sword and head swinging!"));
  unsigned long swingStartTime = millis();
  unsigned long swingDuration = 5000;  // 5 seconds
  int swordSwingAmplitude = 30;
  int swordBaseAngle = swordAngle;
  bool swordSwingUp = true;

  // Head swing parameters
  int headBaseAngle = HEAD_RESET_ANGLE;  // 90 degrees
  int headSwingRange = 90;  // Swing 180 degrees total (90° each side)
  int currentHeadDirection = 1;  // 1 for right, -1 for left
  int headSwingTarget = headBaseAngle + headSwingRange;

  while (millis() - swingStartTime < swingDuration) {
    // SWORD SWING
    if (swordSwingUp) {
      int targetAngle = swordBaseAngle + swordSwingAmplitude;
      while (currentElbowAngle < targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentElbowAngle += 3;
        if (currentElbowAngle > targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);
      }
      swordSwingUp = false;
    } else {
      int targetAngle = swordBaseAngle - swordSwingAmplitude;
      while (currentElbowAngle > targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentElbowAngle -= 3;
        if (currentElbowAngle < targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);
      }
      swordSwingUp = true;
    }

    // HEAD SWING - Move head during sword swing
    if (currentHeadDirection == 1) {
      // Moving right (from 90° to 180°)
      headSwingTarget = headBaseAngle + headSwingRange;
      while (currentHeadAngle < headSwingTarget && (millis() - swingStartTime < swingDuration)) {
        currentHeadAngle += headStepSize;
        if (currentHeadAngle > headSwingTarget)
          currentHeadAngle = headSwingTarget;
        head.write(currentHeadAngle);
        delay(30);
      }
      currentHeadDirection = -1;
    } else {
      // Moving left (from 180° to 0°)
      headSwingTarget = headBaseAngle - headSwingRange;
      while (currentHeadAngle > headSwingTarget && (millis() - swingStartTime < swingDuration)) {
        currentHeadAngle -= headStepSize;
        if (currentHeadAngle < headSwingTarget)
          currentHeadAngle = headSwingTarget;
        head.write(currentHeadAngle);
        delay(30);
      }
      currentHeadDirection = 1;
    }
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  // Return sword to base angle
  while (currentElbowAngle < swordBaseAngle) {
    currentElbowAngle += elbowStepSize;
    if (currentElbowAngle > swordBaseAngle)
      currentElbowAngle = swordBaseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }
  while (currentElbowAngle > swordBaseAngle) {
    currentElbowAngle -= elbowStepSize;
    if (currentElbowAngle < swordBaseAngle)
      currentElbowAngle = swordBaseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }

  // Return head to center
  int headTarget = HEAD_RESET_ANGLE;
  while (currentHeadAngle < headTarget) {
    currentHeadAngle += headStepSize;
    if (currentHeadAngle > headTarget)
      currentHeadAngle = headTarget;
    head.write(currentHeadAngle);
    delay(15);
  }
  while (currentHeadAngle > headTarget) {
    currentHeadAngle -= headStepSize;
    if (currentHeadAngle < headTarget)
      currentHeadAngle = headTarget;
    head.write(currentHeadAngle);
    delay(15);
  }

  // Wait for audio to finish
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 4: {
  Serial.println(F("Case 4: Pink waves, Toy swing, Shoulder raise"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track004.mp3"));
  musicPlayer.startPlayingFile("/track004.mp3");

  // Transition NeoPixels to pink gradient waves (light pink to hot pink)
  transitionToGradient(255, 182, 193, 255, 105, 180, 0.05);

  //
  // --- MOVE ELBOW TO TOY ANGLE (180°) ---
  //
  int toyAngle = ELBOW_ARM_3_ANGLE;  // Toy attachment angle

  if (currentElbowAngle < toyAngle) {
    while (currentElbowAngle < toyAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > toyAngle)
        currentElbowAngle = toyAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > toyAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < toyAngle)
        currentElbowAngle = toyAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SWING TOY FOR 5 SECONDS ---
  //
  Serial.println(F("Swinging toy"));
  unsigned long swingStartTime = millis();
  unsigned long swingDuration = 5000;  // 5 seconds
  int swingAmplitude = 30;
  int baseAngle = toyAngle;
  bool swingUp = true;

  while (millis() - swingStartTime < swingDuration) {
    if (swingUp) {
      // Swing up (+30)
      int targetAngle = baseAngle + swingAmplitude;
      while (currentElbowAngle < targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentElbowAngle += 3;
        if (currentElbowAngle > targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);
      }
      swingUp = false;
    } else {
      // Swing down (-30)
      int targetAngle = baseAngle - swingAmplitude;
      while (currentElbowAngle > targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentElbowAngle -= 3;
        if (currentElbowAngle < targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);
      }
      swingUp = true;
    }
  }

  // Return toy to base angle
  while (currentElbowAngle < baseAngle) {
    currentElbowAngle += elbowStepSize;
    if (currentElbowAngle > baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }
  while (currentElbowAngle > baseAngle) {
    currentElbowAngle -= elbowStepSize;
    if (currentElbowAngle < baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 5: {
  Serial.println(F("Case 5: Pink lights, Left shoulder wave"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track021.mp3"));
  musicPlayer.startPlayingFile("/track021.mp3");

  // Transition NeoPixels to pink gradient
  transitionToGradient(255, 182, 193, 255, 105, 180, 0.05);

  //
  // --- RAISE LEFT SHOULDER TO 90° ---
  //
  int leftShoulderTarget = 90;

  if (currentShoulderLeftAngle > leftShoulderTarget) {
    while (currentShoulderLeftAngle > leftShoulderTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- WAVE LEFT SHOULDER FOR 5 SECONDS ---
  //
  Serial.println(F("Waving left shoulder"));
  unsigned long waveStartTime = millis();
  unsigned long waveDuration = 5000;  // 5 seconds
  int waveAmplitude = 30;
  int baseAngle = leftShoulderTarget;
  bool waveUp = true;

  while (millis() - waveStartTime < waveDuration) {
    if (waveUp) {
      // Wave up (+30)
      int targetAngle = baseAngle + waveAmplitude;
      while (currentShoulderLeftAngle > targetAngle && (millis() - waveStartTime < waveDuration)) {
        currentShoulderLeftAngle -= 3;
        if (currentShoulderLeftAngle < targetAngle)
          currentShoulderLeftAngle = targetAngle;
        shoulderLeft.write(180 - currentShoulderLeftAngle);
        delay(30);
      }
      waveUp = false;
    } else {
      // Wave down (-30)
      int targetAngle = baseAngle - waveAmplitude;
      while (currentShoulderLeftAngle < targetAngle && (millis() - waveStartTime < waveDuration)) {
        currentShoulderLeftAngle += 3;
        if (currentShoulderLeftAngle > targetAngle)
          currentShoulderLeftAngle = targetAngle;
        shoulderLeft.write(180 - currentShoulderLeftAngle);
        delay(30);
      }
      waveUp = true;
    }
  }

  // Return shoulder to base angle
  while (currentShoulderLeftAngle > baseAngle) {
    currentShoulderLeftAngle -= shoulderStepSize;
    if (currentShoulderLeftAngle < baseAngle)
      currentShoulderLeftAngle = baseAngle;
    shoulderLeft.write(180 - currentShoulderLeftAngle);
    delay(15);
  }
  while (currentShoulderLeftAngle < baseAngle) {
    currentShoulderLeftAngle += shoulderStepSize;
    if (currentShoulderLeftAngle > baseAngle)
      currentShoulderLeftAngle = baseAngle;
    shoulderLeft.write(180 - currentShoulderLeftAngle);
    delay(15);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 6: {
  Serial.println(F("Case 6: Gold lights, Head swing"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track005.mp3"));
  musicPlayer.startPlayingFile("/track005.mp3");

  // Transition NeoPixels to gold gradient (bright gold to darker gold)
  transitionToGradient(255, 215, 0, 218, 165, 32, 0.05);

  //
  // --- SWING HEAD FOR 5 SECONDS ---
  //
  Serial.println(F("Swinging head"));
  unsigned long swingStartTime = millis();
  unsigned long swingDuration = 5000;  // 5 seconds
  int swingAmplitude = 30;
  int baseAngle = HEAD_RESET_ANGLE;  // Starting from 90 degrees
  bool swingUp = true;

  while (millis() - swingStartTime < swingDuration) {
    if (swingUp) {
      // Swing up (+30)
      int targetAngle = baseAngle + swingAmplitude;
      while (currentHeadAngle < targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentHeadAngle += 3;
        if (currentHeadAngle > targetAngle)
          currentHeadAngle = targetAngle;
        head.write(currentHeadAngle);
        delay(30);
      }
      swingUp = false;
    } else {
      // Swing down (-30)
      int targetAngle = baseAngle - swingAmplitude;
      while (currentHeadAngle > targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentHeadAngle -= 3;
        if (currentHeadAngle < targetAngle)
          currentHeadAngle = targetAngle;
        head.write(currentHeadAngle);
        delay(30);
      }
      swingUp = true;
    }
  }

  // Return head to base angle
  while (currentHeadAngle < baseAngle) {
    currentHeadAngle += headStepSize;
    if (currentHeadAngle > baseAngle)
      currentHeadAngle = baseAngle;
    head.write(currentHeadAngle);
    delay(15);
  }
  while (currentHeadAngle > baseAngle) {
    currentHeadAngle -= headStepSize;
    if (currentHeadAngle < baseAngle)
      currentHeadAngle = baseAngle;
    head.write(currentHeadAngle);
    delay(15);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 7: {
  Serial.println(F("Case 7: Red blinking lights, Head disapproval swing"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track006.mp3"));
  musicPlayer.startPlayingFile("/track006.mp3");

  //
  // --- HEAD SWING IN DISAPPROVAL WITH RED BLINKING LIGHTS ---
  //
  Serial.println(F("Head disapproval swing with red blinking"));

  int baseAngle = HEAD_RESET_ANGLE;  // 90 degrees center
  int swingAmplitude = 30;
  bool swingRight = true;
  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 500;  // Blink every half second

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Check if it's time to toggle lights
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn lights red
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }

    // Swing head in disapproval (left-right-left-right)
    if (swingRight) {
      // Swing right (+30 from center)
      int targetAngle = baseAngle + swingAmplitude;
      if (currentHeadAngle < targetAngle) {
        currentHeadAngle += 3;
        if (currentHeadAngle > targetAngle)
          currentHeadAngle = targetAngle;
        head.write(currentHeadAngle);
        delay(30);
      } else {
        swingRight = false;  // Switch direction
      }
    } else {
      // Swing left (-30 from center)
      int targetAngle = baseAngle - swingAmplitude;
      if (currentHeadAngle > targetAngle) {
        currentHeadAngle -= 3;
        if (currentHeadAngle < targetAngle)
          currentHeadAngle = targetAngle;
        head.write(currentHeadAngle);
        delay(30);
      } else {
        swingRight = true;  // Switch direction
      }
    }
  }

  // Return head to base angle
  while (currentHeadAngle < baseAngle) {
    currentHeadAngle += headStepSize;
    if (currentHeadAngle > baseAngle)
      currentHeadAngle = baseAngle;
    head.write(currentHeadAngle);
    delay(15);
  }
  while (currentHeadAngle > baseAngle) {
    currentHeadAngle -= headStepSize;
    if (currentHeadAngle < baseAngle)
      currentHeadAngle = baseAngle;
    head.write(currentHeadAngle);
    delay(15);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 8: {
  Serial.println(F("Case 8: Orange lights, Right shoulder raise, Sword swing"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track007.mp3"));
  musicPlayer.startPlayingFile("/track007.mp3");

  // Transition NeoPixels to orange gradient (bright orange to darker orange)
  transitionToGradient(255, 165, 0, 255, 140, 0, 0.05);

  //
  // --- MOVE ELBOW TO SWORD ANGLE (90°) ---
  //
  int swordAngle = ELBOW_ARM_2_ANGLE;  // Sword attachment angle

  if (currentElbowAngle < swordAngle) {
    while (currentElbowAngle < swordAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > swordAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SWING SWORD FOR 5 SECONDS ---
  //
  Serial.println(F("Swinging sword"));
  unsigned long swingStartTime = millis();
  unsigned long swingDuration = 5000;  // 5 seconds
  int swingAmplitude = 30;
  int baseAngle = swordAngle;
  bool swingUp = true;

  while (millis() - swingStartTime < swingDuration) {
    if (swingUp) {
      // Swing up (+30)
      int targetAngle = baseAngle + swingAmplitude;
      while (currentElbowAngle < targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentElbowAngle += 3;
        if (currentElbowAngle > targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);
      }
      swingUp = false;
    } else {
      // Swing down (-30)
      int targetAngle = baseAngle - swingAmplitude;
      while (currentElbowAngle > targetAngle && (millis() - swingStartTime < swingDuration)) {
        currentElbowAngle -= 3;
        if (currentElbowAngle < targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
        delay(30);
      }
      swingUp = true;
    }
  }

  // Return sword to base angle
  while (currentElbowAngle < baseAngle) {
    currentElbowAngle += elbowStepSize;
    if (currentElbowAngle > baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }
  while (currentElbowAngle > baseAngle) {
    currentElbowAngle -= elbowStepSize;
    if (currentElbowAngle < baseAngle)
      currentElbowAngle = baseAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 9: {
  Serial.println(F("Case 9: Green blinking lights, Right shoulder raise with movement, Pen swing"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track008.mp3"));
  musicPlayer.startPlayingFile("/track008.mp3");

  //
  // --- MOVE ELBOW TO PEN ANGLE (0°) ---
  //
  int penAngle = ELBOW_ARM_1_ANGLE;  // Pen attachment angle

  if (currentElbowAngle < penAngle) {
    while (currentElbowAngle < penAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > penAngle)
        currentElbowAngle = penAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > penAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < penAngle)
        currentElbowAngle = penAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderBase = 90;

  if (currentShoulderRightAngle < rightShoulderBase) {
    while (currentShoulderRightAngle < rightShoulderBase) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderBase)
        currentShoulderRightAngle = rightShoulderBase;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderBase) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderBase)
        currentShoulderRightAngle = rightShoulderBase;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- PEN SWING WITH SHOULDER MOVEMENT AND GREEN BLINKING ---
  //
  Serial.println(F("Pen swinging with shoulder movement and green blinking"));

  int penSwingAmplitude = 30;
  int shoulderSwingAmplitude = 15;  // Smaller shoulder movements
  bool penSwingUp = true;
  bool shoulderSwingUp = true;
  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 500;  // Blink every half second

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Check if it's time to toggle lights
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn lights green
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 255, 0));
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }

    // Swing pen
    if (penSwingUp) {
      int targetAngle = penAngle + penSwingAmplitude;
      if (currentElbowAngle < targetAngle) {
        currentElbowAngle += 3;
        if (currentElbowAngle > targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
      } else {
        penSwingUp = false;
      }
    } else {
      int targetAngle = penAngle - penSwingAmplitude;
      if (currentElbowAngle > targetAngle) {
        currentElbowAngle -= 3;
        if (currentElbowAngle < targetAngle)
          currentElbowAngle = targetAngle;
        elbowRight.write(currentElbowAngle);
      } else {
        penSwingUp = true;
      }
    }

    // Small shoulder movements
    if (shoulderSwingUp) {
      int targetAngle = rightShoulderBase + shoulderSwingAmplitude;
      if (currentShoulderRightAngle < targetAngle) {
        currentShoulderRightAngle += 2;
        if (currentShoulderRightAngle > targetAngle)
          currentShoulderRightAngle = targetAngle;
        shoulderRight.write(currentShoulderRightAngle);
      } else {
        shoulderSwingUp = false;
      }
    } else {
      int targetAngle = rightShoulderBase - shoulderSwingAmplitude;
      if (currentShoulderRightAngle > targetAngle) {
        currentShoulderRightAngle -= 2;
        if (currentShoulderRightAngle < targetAngle)
          currentShoulderRightAngle = targetAngle;
        shoulderRight.write(currentShoulderRightAngle);
      } else {
        shoulderSwingUp = true;
      }
    }

    delay(30);
  }

  // Return pen to base angle
  while (currentElbowAngle < penAngle) {
    currentElbowAngle += elbowStepSize;
    if (currentElbowAngle > penAngle)
      currentElbowAngle = penAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }
  while (currentElbowAngle > penAngle) {
    currentElbowAngle -= elbowStepSize;
    if (currentElbowAngle < penAngle)
      currentElbowAngle = penAngle;
    elbowRight.write(currentElbowAngle);
    delay(15);
  }

  // Return shoulder to base position
  while (currentShoulderRightAngle < rightShoulderBase) {
    currentShoulderRightAngle += shoulderStepSize;
    if (currentShoulderRightAngle > rightShoulderBase)
      currentShoulderRightAngle = rightShoulderBase;
    shoulderRight.write(currentShoulderRightAngle);
    delay(15);
  }
  while (currentShoulderRightAngle > rightShoulderBase) {
    currentShoulderRightAngle -= shoulderStepSize;
    if (currentShoulderRightAngle < rightShoulderBase)
      currentShoulderRightAngle = rightShoulderBase;
    shoulderRight.write(currentShoulderRightAngle);
    delay(15);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 10: {
  Serial.println(F("Case 10: Half red half green blinking lights"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track009.mp3"));
  musicPlayer.startPlayingFile("/track009.mp3");

  //
  // --- BLINK HALF RED HALF GREEN UNTIL AUDIO ENDS ---
  //
  Serial.println(F("Blinking half red half green"));

  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 500;  // Blink every half second
  int halfPixels = NUMPIXELS / 2;  // 32 pixels each half

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Check if it's time to toggle lights
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn on half red, half green
        for (int i = 0; i < halfPixels; i++) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));  // First half red
        }
        for (int i = halfPixels; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 255, 0));  // Second half green
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }
    delay(10);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 11: {
  Serial.println(F("Case 11: Head moves 45 degrees"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track010.mp3"));
  musicPlayer.startPlayingFile("/track010.mp3");

  //
  // --- MOVE HEAD 45 DEGREES ---
  //
  int headTarget = HEAD_RESET_ANGLE + 45;  // Move to 135 degrees (90 + 45)

  if (currentHeadAngle < headTarget) {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 12: {
  Serial.println(F("Case 12: Fast party lights, Exciting attachment cycling"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track011.mp3"));
  musicPlayer.startPlayingFile("/track011.mp3");

  //
  // --- CYCLE THROUGH ATTACHMENTS WITH PARTY LIGHTS ---
  //
  Serial.println(F("Cycling through attachments with party lights"));

  // Party light colors (6 colors)
  uint32_t partyColors[6] = {
    pixels.Color(255, 0, 0),     // Red
    pixels.Color(0, 255, 0),     // Green
    pixels.Color(0, 0, 255),     // Blue
    pixels.Color(255, 255, 0),   // Yellow
    pixels.Color(255, 0, 255),   // Magenta
    pixels.Color(0, 255, 255)    // Cyan
  };

  // Attachment angles array
  int attachmentAngles[4] = {
    ELBOW_ARM_1_ANGLE,  // Pen (0°)
    ELBOW_ARM_2_ANGLE,  // Sword (90°)
    ELBOW_ARM_3_ANGLE,  // Toy (180°)
    ELBOW_ARM_4_ANGLE   // Bottle (270°)
  };

  int currentAttachment = 0;
  int currentColorIndex = 0;
  unsigned long lastColorChange = millis();
  unsigned long lastAttachmentChange = millis();
  unsigned long colorChangeInterval = 150;      // Change color every 150ms (fast!)
  unsigned long attachmentChangeInterval = 400; // Change attachment every 400ms

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Fast party light cycling
    if (millis() - lastColorChange >= colorChangeInterval) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, partyColors[currentColorIndex]);
      }
      pixels.show();
      currentColorIndex = (currentColorIndex + 1) % 6;
      lastColorChange = millis();
    }

    // Exciting attachment cycling
    if (millis() - lastAttachmentChange >= attachmentChangeInterval) {
      int targetAngle = attachmentAngles[currentAttachment];

      // Quick movement to next attachment
      while (currentElbowAngle != targetAngle) {
        if (currentElbowAngle < targetAngle) {
          currentElbowAngle += 10;  // Faster steps for excitement
          if (currentElbowAngle > targetAngle)
            currentElbowAngle = targetAngle;
        } else {
          currentElbowAngle -= 10;
          if (currentElbowAngle < targetAngle)
            currentElbowAngle = targetAngle;
        }
        elbowRight.write(currentElbowAngle);
        delay(10);
      }

      currentAttachment = (currentAttachment + 1) % 4;  // Cycle through all 4
      lastAttachmentChange = millis();
    }

    delay(10);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 13: {
  Serial.println(F("Case 13: Red blinking lights, Both shoulders raised, Sword raised"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track012.mp3"));
  musicPlayer.startPlayingFile("/track012.mp3");

  //
  // --- MOVE ELBOW TO SWORD ANGLE (90°) ---
  //
  int swordAngle = ELBOW_ARM_2_ANGLE;  // Sword attachment angle

  if (currentElbowAngle < swordAngle) {
    while (currentElbowAngle < swordAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > swordAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE LEFT SHOULDER TO 90° ---
  //
  int leftShoulderTarget = 90;

  if (currentShoulderLeftAngle > leftShoulderTarget) {
    while (currentShoulderLeftAngle > leftShoulderTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- BLINK RED LIGHTS UNTIL AUDIO ENDS ---
  //
  Serial.println(F("Blinking red lights"));

  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 500;  // Blink every half second

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Check if it's time to toggle lights
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn lights red
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }
    delay(10);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 14: {
  Serial.println(F("Case 14: Head and shoulders 45 degrees, immediate return"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track013.mp3"));
  musicPlayer.startPlayingFile("/track013.mp3");

  //
  // --- MOVE HEAD 45 DEGREES ---
  //
  int headTarget = HEAD_RESET_ANGLE + 45;  // Move to 135 degrees (90 + 45)

  if (currentHeadAngle < headTarget) {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  //
  // --- RAISE RIGHT SHOULDER 45 DEGREES ---
  //
  int rightShoulderTarget = SHOULDER_RIGHT_RESET_ANGLE + 45;  // 30 + 45 = 75 degrees

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  //
  // --- RAISE LEFT SHOULDER 45 DEGREES ---
  //
  int leftShoulderTarget = SHOULDER_LEFT_RESET_ANGLE + 45;  // 30 + 45 = 75 degrees

  if (currentShoulderLeftAngle > leftShoulderTarget) {
    while (currentShoulderLeftAngle > leftShoulderTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  //
  // --- IMMEDIATELY RETURN TO DEFAULT (NO DELAY) ---
  //

  // Return head to reset angle
  int headReset = HEAD_RESET_ANGLE;
  if (currentHeadAngle < headReset) {
    while (currentHeadAngle < headReset) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headReset)
        currentHeadAngle = headReset;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headReset) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headReset)
        currentHeadAngle = headReset;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  // Return right shoulder to reset angle
  int rightShoulderReset = SHOULDER_RIGHT_RESET_ANGLE;
  if (currentShoulderRightAngle < rightShoulderReset) {
    while (currentShoulderRightAngle < rightShoulderReset) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderReset)
        currentShoulderRightAngle = rightShoulderReset;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderReset) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderReset)
        currentShoulderRightAngle = rightShoulderReset;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  // Return left shoulder to reset angle
  int leftShoulderReset = SHOULDER_LEFT_RESET_ANGLE;
  if (currentShoulderLeftAngle > leftShoulderReset) {
    while (currentShoulderLeftAngle > leftShoulderReset) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderReset)
        currentShoulderLeftAngle = leftShoulderReset;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderReset) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderReset)
        currentShoulderLeftAngle = leftShoulderReset;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 15: {
  Serial.println(F("Case 15: Red patterns, Left shoulder raise"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track014.mp3"));
  musicPlayer.startPlayingFile("/track014.mp3");

  // Transition NeoPixels to red gradient
  transitionToGradient(255, 0, 0, 150, 0, 0, 0.05);

  //
  // --- RAISE LEFT SHOULDER TO 90° ---
  //
  int leftShoulderTarget = 90;

  if (currentShoulderLeftAngle > leftShoulderTarget) {
    while (currentShoulderLeftAngle > leftShoulderTarget) {
      currentShoulderLeftAngle -= shoulderStepSize;
      if (currentShoulderLeftAngle < leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  } else {
    while (currentShoulderLeftAngle < leftShoulderTarget) {
      currentShoulderLeftAngle += shoulderStepSize;
      if (currentShoulderLeftAngle > leftShoulderTarget)
        currentShoulderLeftAngle = leftShoulderTarget;
      shoulderLeft.write(180 - currentShoulderLeftAngle);
      delay(15);
    }
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- WAIT FOR AUDIO TO FINISH ---
  //
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 16: {
  Serial.println(F("Case 16: Sharp RGB cycling, Sword raise"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track015.mp3"));
  musicPlayer.startPlayingFile("/track015.mp3");

  //
  // --- MOVE ELBOW TO SWORD ANGLE (90°) ---
  //
  int swordAngle = ELBOW_ARM_2_ANGLE;  // Sword attachment angle

  if (currentElbowAngle < swordAngle) {
    while (currentElbowAngle < swordAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > swordAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- RAISE RIGHT SHOULDER TO 90° ---
  //
  int rightShoulderTarget = 90;

  if (currentShoulderRightAngle < rightShoulderTarget) {
    while (currentShoulderRightAngle < rightShoulderTarget) {
      currentShoulderRightAngle += shoulderStepSize;
      if (currentShoulderRightAngle > rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  } else {
    while (currentShoulderRightAngle > rightShoulderTarget) {
      currentShoulderRightAngle -= shoulderStepSize;
      if (currentShoulderRightAngle < rightShoulderTarget)
        currentShoulderRightAngle = rightShoulderTarget;
      shoulderRight.write(currentShoulderRightAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SHARP RGB COLOR CYCLING UNTIL AUDIO ENDS ---
  //
  Serial.println(F("Sharp RGB color cycling"));

  // RGB colors (no fading, sharp transitions)
  uint32_t rgbColors[3] = {
    pixels.Color(255, 0, 0),     // Red
    pixels.Color(0, 255, 0),     // Green
    pixels.Color(0, 0, 255)      // Blue
  };

  int currentColorIndex = 0;
  unsigned long lastColorChange = millis();
  unsigned long colorChangeInterval = 500;  // Change color every half second

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Sharp color cycling (no fading)
    if (millis() - lastColorChange >= colorChangeInterval) {
      // Instantly switch to next color
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, rgbColors[currentColorIndex]);
      }
      pixels.show();
      currentColorIndex = (currentColorIndex + 1) % 3;  // Cycle through R->G->B
      lastColorChange = millis();
    }
    delay(10);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 17: {
  Serial.println(F("Case 17: Very fast green blinking, Exciting head shake"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track016.mp3"));
  musicPlayer.startPlayingFile("/track016.mp3");

  //
  // --- MOVE HEAD TO 60 DEGREES ---
  //
  int headTarget = 60;  // Turn right (same as case 2)

  if (currentHeadAngle < headTarget) {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget)
        currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  //
  // --- VERY FAST GREEN BLINKING UNTIL AUDIO ENDS ---
  //
  Serial.println(F("Very fast green blinking"));

  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 100;  // Very fast blinking (100ms)

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Check if it's time to toggle lights
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn lights green
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 255, 0));
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }
    delay(10);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

case 18: {
  Serial.println(F("Case 18: Slow red blinking, Sword"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track017.mp3"));
  musicPlayer.startPlayingFile("/track017.mp3");

  //
  // --- MOVE ELBOW TO SWORD ANGLE (90°) ---
  //
  int swordAngle = ELBOW_ARM_2_ANGLE;  // Sword attachment angle

  if (currentElbowAngle < swordAngle) {
    while (currentElbowAngle < swordAngle) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > swordAngle) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < swordAngle)
        currentElbowAngle = swordAngle;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  delay(500);

  //
  // --- SLOW RED BLINKING UNTIL AUDIO ENDS ---
  //
  Serial.println(F("Slow red blinking"));

  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 1000;  // Slow blinking (1 second)

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Check if it's time to toggle lights
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn lights red
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }
    delay(10);
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  break;
}

case 19: {
  Serial.println(F("Case 19: Playing track018.mp3"));
  musicPlayer.startPlayingFile("/track018.mp3");
  while (musicPlayer.playingMusic) {
    delay(100);
  }
  quickResetRobotPose();
  break;
}

case 20: {
  Serial.println(F("Case 20: Playing track019.mp3"));
  musicPlayer.startPlayingFile("/track019.mp3");
  while (musicPlayer.playingMusic) {
    delay(100);
  }
  quickResetRobotPose();
  break;
}

case 21: {
  Serial.println(F("Case 21: Slow red blinking, All joints going crazy"));

  //
  // --- PLAY AUDIO FIRST ---
  //
  Serial.println(F("Playing track020.mp3"));
  musicPlayer.startPlayingFile("/track020.mp3");

  //
  // --- ALL JOINTS GO CRAZY WITH SLOW RED BLINKING ---
  //
  Serial.println(F("All joints going crazy with slow red blinking"));

  bool lightsOn = true;
  unsigned long lastBlinkTime = millis();
  unsigned long blinkInterval = 1000;  // Slow blinking (1 second)

  // Random movement targets
  bool headMovingUp = true;
  bool rightShoulderMovingUp = true;
  bool leftShoulderMovingUp = true;
  bool elbowMovingUp = true;

  // Continue while audio is playing
  while (musicPlayer.playingMusic) {
    // Slow red blinking
    if (millis() - lastBlinkTime >= blinkInterval) {
      if (lightsOn) {
        // Turn lights red
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        pixels.show();
        lightsOn = false;
      } else {
        // Turn lights off
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        lightsOn = true;
      }
      lastBlinkTime = millis();
    }

    // Crazy head movement (random between 60 and 120)
    if (headMovingUp) {
      currentHeadAngle += random(5, 15);
      if (currentHeadAngle >= 120) {
        currentHeadAngle = 120;
        headMovingUp = false;
      }
    } else {
      currentHeadAngle -= random(5, 15);
      if (currentHeadAngle <= 60) {
        currentHeadAngle = 60;
        headMovingUp = true;
      }
    }
    head.write(currentHeadAngle);

    // Crazy right shoulder movement (random between 20 and 100)
    if (rightShoulderMovingUp) {
      currentShoulderRightAngle += random(5, 15);
      if (currentShoulderRightAngle >= 100) {
        currentShoulderRightAngle = 100;
        rightShoulderMovingUp = false;
      }
    } else {
      currentShoulderRightAngle -= random(5, 15);
      if (currentShoulderRightAngle <= 20) {
        currentShoulderRightAngle = 20;
        rightShoulderMovingUp = true;
      }
    }
    shoulderRight.write(currentShoulderRightAngle);

    // Crazy left shoulder movement (random between 20 and 100)
    if (leftShoulderMovingUp) {
      currentShoulderLeftAngle -= random(5, 15);
      if (currentShoulderLeftAngle <= 20) {
        currentShoulderLeftAngle = 20;
        leftShoulderMovingUp = false;
      }
    } else {
      currentShoulderLeftAngle += random(5, 15);
      if (currentShoulderLeftAngle >= 100) {
        currentShoulderLeftAngle = 100;
        leftShoulderMovingUp = true;
      }
    }
    shoulderLeft.write(180 - currentShoulderLeftAngle);

    // Crazy elbow movement (random between all attachment angles)
    if (elbowMovingUp) {
      currentElbowAngle += random(10, 30);
      if (currentElbowAngle >= 270) {
        currentElbowAngle = 270;
        elbowMovingUp = false;
      }
    } else {
      currentElbowAngle -= random(10, 30);
      if (currentElbowAngle <= 0) {
        currentElbowAngle = 0;
        elbowMovingUp = true;
      }
    }
    elbowRight.write(currentElbowAngle);

    delay(50);  // Small delay for crazy movements
  }

  //
  // --- TRANSITION LIGHTS BACK TO RAINBOW ---
  //
  Serial.println(F("Transitioning lights back to rainbow"));
  transitionToRainbow(0.05);

  //
  // --- QUICK RESET TO DEFAULT POSITION ---
  //
  quickResetRobotPose();

  break;
}

  }
}

void loop() {
  // Continuously update rainbow gradient (non-blocking)
  updateRainbow();

  // Check for serial input for testing
  if (Serial.available() > 0) {
    int commandNum = Serial.parseInt();

    // Clear any remaining characters in the buffer
    while (Serial.available() > 0) {
      Serial.read();
    }

    if (commandNum >= 0 && commandNum <= 21) {
      Serial.print(F("Serial input received: "));
      Serial.println(commandNum);
      processCommand(commandNum);
    } else {
      Serial.println(F("Invalid input. Enter 0-21 to test cases."));
    }
  }

  // Check for radio input
  radio.startListening();
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    Serial.print(F("Radio message received Data = "));
    Serial.print(data.stateNumber);
    Serial.println();
    processCommand(data.stateNumber);
  }
}  // end of loop()
// end of receiver code
// CHANGEHERE
