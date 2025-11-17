/*
   Using the nRF24L01 radio module to communicate
   between two Arduinos with much increased reliability following
   various tutorials, conversations, and studying the nRF24L01 datasheet
   and the library reference.

   Transmitter is
   https://github.com/michaelshiloh/resourcesForClasses/tree/master/kicad/Arduino_Shield_RC_Controller

  Receiver is
  https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRF_servo_Mega

   This file contains code for both transmitter and receiver.
   Transmitter at the top, receiver at the bottom.
   One of them is commented out, so you need to comment in or out
   the correct section. You don't need to make changes to this 
   part of the code, just to comment in or out depending on
   whether you are programming your transmitter or receiver

   You need to set the correct address for your robot.

   Search for the phrase CHANGEHERE to see where to 
   comment or uncomment or make changes.

   These sketches require the RF24 library by TMRh20
   Documentation here: https://nrf24.github.io/RF24/index.html

   change log

   11 Oct 2023 - ms - initial entry based on
                  rf24PerformingRobotsTemplate
   26 Oct 2023 - ms - revised for new board: nRF_Servo_Mega rev 2
   28 Oct 2023 - ms - add demo of NeoMatrix, servo, and Music Maker Shield
	 20 Nov 2023 - as - fixed the bug which allowed counting beyond the limits
   22 Nov 2023 - ms - display radio custom address byte and channel
   12 Nov 2024 - ms - changed names for channel and address allocation for Fall 2024            
   31 Oct 2025 - ms - changed names for channel and address allocation for Fall 2024            
                    - listed pin numbers for servo/NeoPixel connections
                      https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRF_servo_Mega    
                      https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRFControlPanel
   16 Nov 2025 - ms - Check correct pin numbers depending on transmitter or receiver
                    - Example of gradual servo motor movement
*/


// Common code
//

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
const int NRF_CE_PIN = A4, NRF_CSN_PIN = A5;

// CHANGEHERE
// for the receiver
//const int NRF_CE_PIN = A11, NRF_CSN_PIN = A15;

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

// See note in rf24Handshaking about address selection
//

// Channel and address allocation:
// Torico and Sarah: Channel 30, addr = 0x76
// Sudiksha and Aysha: Channel 40, addr = 0x73
// Mariam and Joy:  Channel 50, addr = 0x7C
// Ghadir and Mustafa: Channel 60, addr = 0xC6
// Clara and Jiho:  Channel 70, addr = 0xC3
// Victor and Meera: Channel 80, addr = 0xCC
// Ali and Hari: Channel 90, addr = 0x33

// CHANGEHERE
const int CUSTOM_CHANNEL_NUMBER = 60;   // change as per the above assignment
const byte CUSTOM_ADDRESS_BYTE = 0xC6;  // change as per the above assignment

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

// CHANGEHERE

// Transmitter code

// Transmitter pin usage
const int LCD_RS_PIN = 3, LCD_EN_PIN = 2, LCD_D4_PIN = 4, LCD_D5_PIN = 5, LCD_D6_PIN = 6, LCD_D7_PIN = 7;
const int SW1_PIN = 8, SW2_PIN = 9, SW3_PIN = 10, SW4_PIN = A3, SW5_PIN = A2;

// LCD library code
#include <LiquidCrystal.h>

// initialize the library with the relevant pins
LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);


const int NUM_OF_STATES = 6;
char* theStates[] = { "0 robot start state",
                      "1 robot scene1",
                      "2 robot scene3-1",
                      "3 robot scene3-2",
                      "4 robot scene3-3",
                      "5 robot scene3-4",
                      "6 robot scene3-5" };

void updateLCD() {

  lcd.clear();
  lcd.print(theStates[data.stateNumber]);
  lcd.setCursor(0, 1);  // column, line (from 0)
  lcd.print("not transmitted yet");
}

void countDown() {
  data.stateNumber = (data.stateNumber > 0) ? (data.stateNumber - 1) : 0;
  updateLCD();
}

void countUp() {
  if (++data.stateNumber >= NUM_OF_STATES) {
    data.stateNumber = NUM_OF_STATES - 1;
  }
  updateLCD();
}


void spare1() {}
void spare2() {}

void rf24SendData() {

  radio.stopListening();  // go into transmit mode
  // The write() function will block
  // until the message is successfully acknowledged by the receiver
  // or the timeout/retransmit maxima are reached.
  // Returns 1 if write succeeds
  // Returns 0 if errors occurred (timeout or FAILURE_HANDLING fails)
  int retval = radio.write(&data, sizeof(data));
  
  lcd.clear();
  lcd.setCursor(0, 0);  // column, line (from 0)
  lcd.print("transmitting");
  lcd.setCursor(14, 0);  // column, line (from 0)
  lcd.print(data.stateNumber);

  Serial.print(F(" ... "));
  if (retval) {
    Serial.println(F("success"));
    lcd.setCursor(0, 1);  // column, line (from 0)
    lcd.print("success");
  } else {
    totalTransmitFailures++;
    Serial.print(F("failure, total failures = "));
    Serial.println(totalTransmitFailures);

    lcd.setCursor(0, 1);  // column, line (from 0)
    lcd.print("error, total=");
    lcd.setCursor(13, 1);  // column, line (from 0)
    lcd.print(totalTransmitFailures);
  }
}

class Button {
  int pinNumber;
  bool previousState;
  void (*buttonFunction)();
public:

  // Constructor
  Button(int pn, void* bf) {
    pinNumber = pn;
    buttonFunction = bf;
    previousState = 1;
  }

  // update the button
  void update() {
    bool currentState = digitalRead(pinNumber);
    if (currentState == LOW && previousState == HIGH) {
      Serial.print("button on pin ");
      Serial.print(pinNumber);
      Serial.println();
      buttonFunction();
    }
    previousState = currentState;
  }
};

const int NUMBUTTONS = 5;
Button theButtons[] = {
  Button(SW1_PIN, countDown),
  Button(SW2_PIN, rf24SendData),
  Button(SW3_PIN, countUp),
  Button(SW4_PIN, spare1),
  Button(SW5_PIN, spare2),
};

void setupRF24() {

  // Check whether the correct pins are assigned
  if ( NRF_CE_PIN != A4 || NRF_CSN_PIN != A5)
  {
    Serial.println(F("The wrong NRF_CE_PIN and NRF_CSN_PIN pins are defined for a transmitter"));
    while (1);
  }

  setupRF24Common();

  // Set us as a transmitter
  radio.openWritingPipe(xmtrAddress);
  radio.openReadingPipe(1, rcvrAddress);

  // radio.printPrettyDetails();
  Serial.println(F("I am a transmitter"));

  data.stateNumber = 0;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Setting up LCD"));

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("Radio setup");

  // Display the address in hex
  lcd.setCursor(0, 1);
  lcd.print("addr 0x");
  lcd.setCursor(7, 1);
  char s[5];
  sprintf(s, "%02x", CUSTOM_ADDRESS_BYTE);
  lcd.print(s);

  // Display the channel number
  lcd.setCursor(10, 1);
  lcd.print("ch");
  lcd.setCursor(13, 1);
  lcd.print(CUSTOM_CHANNEL_NUMBER);

  Serial.println(F("Setting up radio"));
  setupRF24();

  // If setupRF24 returned then the radio is set up
  lcd.setCursor(0, 0);
  lcd.print("Radio OK state=");
  lcd.print(theStates[data.stateNumber]);

  // Initialize the switches
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(SW4_PIN, INPUT_PULLUP);
  pinMode(SW5_PIN, INPUT_PULLUP);
}



void loop() {
  for (int i = 0; i < NUMBUTTONS; i++) {
    theButtons[i].update();
  }
  delay(50);  // for testing
}


void clearData() {
  // set all fields to 0
  data.stateNumber = 0;
}

// End of transmitter code
// CHANGEHERE

/*
// Receiver Code
// CHANGEHERE

// Additional libraries for music maker shield
#include <Adafruit_VS1053.h>
#include <SD.h>

// Servo library
#include <Servo.h>

// Additional libraries for graphics on the Neo Pixel Matrix
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#ifndef PSTR
#define PSTR  // Make Arduino Due happy
#endif

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
const int SHOULDER_RIGHT_PIN = 20;  
const int HEAD_SERVO_PIN = 17;
const int ELBOW_SERVO_PIN = 18;
//const int ANTENNA_SERVO_PIN = 16;
//const int TAIL_SERVO_PIN = 17;
//const int GRABBER_SERVO_PIN = 18;

// Neopixel
const int NEOPIXELPIN = 19;
const int NUMPIXELS = 64;
//#define NEOPIXELPIN 18
//#define NUMPIXELS 64  // change to fit
//Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, NEOPIXELPIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

Servo shoulderRight;  // renamed for clarity
Servo head;  // change names to describe what's moving
Servo elbowRight;
Servo antenna;
Servo tail;
Servo grabber;
Servo disk;

// change as per your robot
const int SHOULDER_RESET_ANGLE = 30;
const int HEAD_RESET_ANGLE = 90;
const int ELBOW_RESET_ANGLE = 0;
const int NOSE_TWEAK = 90;
const int TAIL_ANGRY = 0;
const int TAIL_HAPPY = 180;
const int GRABBER_RELAX = 0;
const int GRABBER_GRAB = 180;

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
head.write(HEAD_RESET_ANGLE);;

shoulderRight.attach(SHOULDER_RIGHT_PIN);
shoulderRight.write(SHOULDER_RESET_ANGLE);

elbowRight.attach(ELBOW_SERVO_PIN);
elbowRight.write(ELBOW_RESET_ANGLE);

  //  antenna.attach(ANTENNA_SERVO_PIN);
  //  tail.attach(TAIL_SERVO_PIN);
  //  grabber.attach(GRABBER_SERVO_PIN);
  //
  //  tail.write(TAIL_HAPPY);
}

void setupNeoPixels() {
  //  pixels.begin();
  //  pixels.clear();
  //  pixels.show();
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(matrix.Color(200, 30, 40));
}

void flashNeoPixels() {

  // Using the Matrix library
  matrix.fillScreen(matrix.Color(0, 255, 0));
  matrix.show();
  delay(500);
  matrix.fillScreen(0);
  matrix.show();

  //  // all on
  //  for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...
  //    pixels.setPixelColor(i, pixels.Color(0, 100, 0));
  //  }
  //  pixels.show();
  //  delay(500);
  //
  //  // all off
  //  pixels.clear();
  //  pixels.show();
}

// this is just an example - change to suit your needs
int currentHeadAngle = HEAD_RESET_ANGLE;
int headStepSize = 5;
int currentShoulderAngle = SHOULDER_RESET_ANGLE;   // start from neutral position
int shoulderStepSize = 5;        // smooth step
int currentElbowAngle = ELBOW_RESET_ANGLE;
int elbowStepSize = 5;

void resetRobotPose() {
  Serial.println(F("Resetting robot to initial position"));

  //
  // --- RESET SHOULDER ---
  //
  int shoulderTarget = SHOULDER_RESET_ANGLE;

  if (currentShoulderAngle > shoulderTarget) {
    while (currentShoulderAngle > shoulderTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderTarget)
          currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      delay(15);
    }
  } else {
    while (currentShoulderAngle < shoulderTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderTarget)
          currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
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

void resetShoulderAndHead() {
  Serial.println(F("Resetting ONLY shoulder and head to initial position"));

  //
  // --- RESET SHOULDER ---
  //
  int shoulderTarget = SHOULDER_RESET_ANGLE;

  if (currentShoulderAngle > shoulderTarget) {
    while (currentShoulderAngle > shoulderTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderTarget)
          currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      delay(15);
    }
  } else {
    while (currentShoulderAngle < shoulderTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderTarget)
          currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
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

void loop() {
  // If there is data, read it,
  // and do the needfull
  // Become a receiver
  radio.startListening();
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    Serial.print(F("message received Data = "));
    Serial.print(data.stateNumber);
    Serial.println();

    switch (data.stateNumber) {
 
case 0: {  
  Serial.println(F("Case 0: RESET to initial robot position"));
  resetRobotPose();
  break;
}

case 1: {  
  Serial.println(F("Case 1: Shoulder up, then elbow, then head right, then audio, then reset + elbow to 180"));

  //
  // --- 1) MOVE SHOULDER TO 90° ---
  //
  int shoulderTarget = 90;  // final shoulder angle

  if (currentShoulderAngle < shoulderTarget) {
    while (currentShoulderAngle < shoulderTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderTarget) currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      delay(15);
    }
  } else {
    while (currentShoulderAngle > shoulderTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderTarget) currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      delay(15);
    }
  }

  delay(1200);

  //
  // --- 2) MOVE ELBOW TO 100° ---
  //
  int elbowTarget = 100;

  if (currentElbowAngle < elbowTarget) {
    while (currentElbowAngle < elbowTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowTarget) currentElbowAngle = elbowTarget;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > elbowTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowTarget) currentElbowAngle = elbowTarget;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } 

  delay(1800);  // pause before head

  //
  // --- 3) MOVE HEAD TO THE RIGHT (60°) ---
  //
  int headTarget = 60;  // 30° to the right from 90°

  if (currentHeadAngle < headTarget) {
    while (currentHeadAngle < headTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headTarget) currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  } else {
    while (currentHeadAngle > headTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headTarget) currentHeadAngle = headTarget;
      head.write(currentHeadAngle);
      delay(15);
    }
  }

  delay(1500);   // wait a bit before playing the audio

  // --- 4) PLAY AUDIO AFTER HEAD FINISHES ---
  Serial.println(F("Playing 1.mp3"));
  musicPlayer.startPlayingFile("/1.mp3");   // ensure SD has 1.mp3

  // Wait until the audio is done
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- 5) RESET SHOULDER & HEAD, MOVE ELBOW TO 180° (ALL TOGETHER) ---
  //
  int shoulderResetTarget = SHOULDER_RESET_ANGLE;  // 30
  int headResetTarget     = HEAD_RESET_ANGLE;      // 90
  int elbowFinalTarget    = 180;                   // new elbow pose

  bool allDone = false;
  while (!allDone) {
    allDone = true;  // assume done, then check each joint

    // Shoulder toward reset
    if (currentShoulderAngle < shoulderResetTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderResetTarget) currentShoulderAngle = shoulderResetTarget;
      shoulderRight.write(currentShoulderAngle);
      allDone = false;
    } else if (currentShoulderAngle > shoulderResetTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderResetTarget) currentShoulderAngle = shoulderResetTarget;
      shoulderRight.write(currentShoulderAngle);
      allDone = false;
    }

    // Head toward reset
    if (currentHeadAngle < headResetTarget) {
      currentHeadAngle += headStepSize;
      if (currentHeadAngle > headResetTarget) currentHeadAngle = headResetTarget;
      head.write(currentHeadAngle);
      allDone = false;
    } else if (currentHeadAngle > headResetTarget) {
      currentHeadAngle -= headStepSize;
      if (currentHeadAngle < headResetTarget) currentHeadAngle = headResetTarget;
      head.write(currentHeadAngle);
      allDone = false;
    }

    // Elbow toward 180°
    if (currentElbowAngle < elbowFinalTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowFinalTarget) currentElbowAngle = elbowFinalTarget;
      elbowRight.write(currentElbowAngle);
      allDone = false;
    } else if (currentElbowAngle > elbowFinalTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowFinalTarget) currentElbowAngle = elbowFinalTarget;
      elbowRight.write(currentElbowAngle);
      allDone = false;
    }

    delay(20);  // smooth motion
  }

  matrix.drawRect(2, 2, 5, 5, matrix.Color(200, 90, 30));
  matrix.show();

  break;
}

case 2: {
  Serial.println(F("Case 2: Elbow to 95°, audio 2, shoulder→90 + elbow mirror, audio 3 wobble, then return elbow to 95 & shoulder down"));

  //
  // --- 1) MOVE ELBOW TO 95° ---
  //
  int elbowTarget = 95;

  if (currentElbowAngle < elbowTarget) {
    while (currentElbowAngle < elbowTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowTarget) currentElbowAngle = elbowTarget;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  } else {
    while (currentElbowAngle > elbowTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowTarget) currentElbowAngle = elbowTarget;
      elbowRight.write(currentElbowAngle);
      delay(15);
    }
  }

  // Small green blink on matrix
  matrix.fillScreen(matrix.Color(0, 200, 30));
  matrix.show();
  delay(300);
  matrix.fillScreen(0);
  matrix.show();

  delay(1000);  // dramatic pause before audio 2

  //
  // --- 2) PLAY AUDIO 2 ---
  //
  Serial.println(F("Playing 2.mp3"));
  musicPlayer.startPlayingFile("/2.mp3");

  // Wait until audio 2 finishes completely
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- 3) MOVE SHOULDER TO 90° AND ELBOW MIRROR THE MOVEMENT ---
  //
  int shoulderTarget = 90;

  // how far the shoulder will move
  int shoulderStart    = currentShoulderAngle;
  int shoulderDistance = shoulderTarget - shoulderStart;

  // elbow mirrors in opposite direction (1.5× exaggeration)
  int elbowFinalTarget = currentElbowAngle - (shoulderDistance * 1.5);

  // clamp safely
  if (elbowFinalTarget < 0)   elbowFinalTarget = 0;
  if (elbowFinalTarget > 180) elbowFinalTarget = 180;

  bool finished = false;

  while (!finished) {
    finished = true;

    // --- SHOULDER MOVEMENT ---
    if (currentShoulderAngle < shoulderTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderTarget) currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      finished = false;
    }
    else if (currentShoulderAngle > shoulderTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderTarget) currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      finished = false;
    }

    // --- ELBOW MIRRORED MOVEMENT ---
    if (currentElbowAngle < elbowFinalTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowFinalTarget) currentElbowAngle = elbowFinalTarget;
      elbowRight.write(currentElbowAngle);
      finished = false;
    }
    else if (currentElbowAngle > elbowFinalTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowFinalTarget) currentElbowAngle = elbowFinalTarget;
      elbowRight.write(currentElbowAngle);
      finished = false;
    }

    delay(20);
  }

  // tiny pause after hitting the dramatic pose
  delay(1800);

  //
  // --- 4) PLAY AUDIO 3 + BIG ELBOW WOBBLE ---
  //
  Serial.println(F("Playing 3.mp3 with BIG elbow wobble"));
  musicPlayer.startPlayingFile("/3.mp3");   // Make sure 3.mp3 exists on SD

  int baseElbow       = currentElbowAngle;   // wherever the elbow ended (likely near 180)
  int wobbleAmplitude = 20;                  // how far it swings

  // Compute safe wobble limits so we NEVER go below 0° or above 180°
  int lowLimit  = baseElbow - wobbleAmplitude;
  if (lowLimit < 0) lowLimit = 0;

  int highLimit = baseElbow + wobbleAmplitude;
  if (highLimit > 180) highLimit = 180;

  // Start from the base angle
  currentElbowAngle = baseElbow;
  bool goingUp = true;   // first move upward (toward highLimit)

  while (musicPlayer.playingMusic) {
    if (goingUp) {
      currentElbowAngle += 4;          // bigger step for stronger movement
      if (currentElbowAngle >= highLimit) {
        currentElbowAngle = highLimit;
        goingUp = false;
      }
    } else {
      currentElbowAngle -= 4;
      if (currentElbowAngle <= lowLimit) {
        currentElbowAngle = lowLimit;
        goingUp = true;
      }
    }

    elbowRight.write(currentElbowAngle);
    delay(70);   // wobble speed
  }

  //
  // --- 5) AFTER AUDIO 3: SHOULDER DOWN + ELBOW BACK TO 95° ---
  //
  Serial.println(F("Audio 3 done – returning shoulder down and elbow back to 95°"));

  int shoulderDownTarget = SHOULDER_RESET_ANGLE; // usually 30
  int elbowReturnTarget  = 95;                   // previous elbow pose in this case

  bool returnDone = false;
  while (!returnDone) {
    returnDone = true;

    // Shoulder down to reset
    if (currentShoulderAngle < shoulderDownTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderDownTarget) currentShoulderAngle = shoulderDownTarget;
      shoulderRight.write(currentShoulderAngle);
      returnDone = false;
    } else if (currentShoulderAngle > shoulderDownTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderDownTarget) currentShoulderAngle = shoulderDownTarget;
      shoulderRight.write(currentShoulderAngle);
      returnDone = false;
    }

    // Elbow back to 95°
    if (currentElbowAngle < elbowReturnTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowReturnTarget) currentElbowAngle = elbowReturnTarget;
      elbowRight.write(currentElbowAngle);
      returnDone = false;
    } else if (currentElbowAngle > elbowReturnTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowReturnTarget) currentElbowAngle = elbowReturnTarget;
      elbowRight.write(currentElbowAngle);
      returnDone = false;
    }

    delay(20);  // smooth motion on the way back
  }

  break;
}

case 3: {
  Serial.println(F("Case 3: Play audio 4.mp3 only"));

  // Start playing the file
  Serial.println(F("Playing 4.mp3"));
  musicPlayer.startPlayingFile("/4.mp3");

  // Wait until audio finishes
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  break;
}

case 4: {
  Serial.println(F("Case 4: audio 5 → shoulder+elbow move (like case 2) → audio 6 + wobble"));

  //
  // --- 1) PLAY AUDIO 5 ---
  //
  Serial.println(F("Playing 5.mp3"));
  musicPlayer.startPlayingFile("/5.mp3");

  // Wait until audio 5 finishes
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  //
  // --- 2) SHOULDER→90 AND ELBOW MIRROR MOVE (LIKE CASE 2) ---
  //
  int shoulderTarget = 90;

  // how far the shoulder will move
  int shoulderStart    = currentShoulderAngle;
  int shoulderDistance = shoulderTarget - shoulderStart;

  // elbow mirrors in the opposite direction, a bit exaggerated
  int elbowFinalTarget = currentElbowAngle - (shoulderDistance * 1.5);

  // safety clamp
  if (elbowFinalTarget < 0)   elbowFinalTarget = 0;
  if (elbowFinalTarget > 180) elbowFinalTarget = 180;

  bool finished = false;
  while (!finished) {
    finished = true;

    // --- SHOULDER MOVEMENT ---
    if (currentShoulderAngle < shoulderTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderTarget) currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      finished = false;
    }
    else if (currentShoulderAngle > shoulderTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderTarget) currentShoulderAngle = shoulderTarget;
      shoulderRight.write(currentShoulderAngle);
      finished = false;
    }

    // --- ELBOW MIRRORED MOVEMENT ---
    if (currentElbowAngle < elbowFinalTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowFinalTarget) currentElbowAngle = elbowFinalTarget;
      elbowRight.write(currentElbowAngle);
      finished = false;
    }
    else if (currentElbowAngle > elbowFinalTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowFinalTarget) currentElbowAngle = elbowFinalTarget;
      elbowRight.write(currentElbowAngle);
      finished = false;
    }

    delay(20);
  }

  // short pause after the pose
  delay(1800);

  //
  // --- 3) PLAY AUDIO 6 WITH SAME WOBBLE AS CASE 2 ---
  //
  Serial.println(F("Playing 6.mp3 with BIG elbow wobble"));
  musicPlayer.startPlayingFile("/6.mp3");   // like 3.mp3 in case 2

  int baseElbow       = currentElbowAngle;   // wobble around the final pose
  int wobbleAmplitude = 20;

  // SAME LIMIT LOGIC AS CASE 2
  int lowLimit  = baseElbow - wobbleAmplitude;
  if (lowLimit < 0) lowLimit = 0;

  int highLimit = baseElbow + wobbleAmplitude;
  if (highLimit > 180) highLimit = 180;

  currentElbowAngle = baseElbow;
  bool goingUp = true;

  while (musicPlayer.playingMusic) {
    if (goingUp) {
      currentElbowAngle += 4;
      if (currentElbowAngle >= highLimit) {
        currentElbowAngle = highLimit;
        goingUp = false;
      }
    } else {
      currentElbowAngle -= 4;
      if (currentElbowAngle <= lowLimit) {
        currentElbowAngle = lowLimit;
        goingUp = true;
      }
    }

    elbowRight.write(currentElbowAngle);
    delay(80);   // wobble speed (same feel as case 2)
  }

  // After audio 6, elbow stays at last wobble position
  break;
}

case 5: {
  Serial.println(F("Case 5: audio 7 + wobble, then shoulder down + elbow back to 95°"));

// --- 1) PLAY AUDIO 7 WITH SAME WOBBLE AS CASE 2 ---
Serial.println(F("Playing 7.mp3 with elbow wobble (same as case 2)"));
musicPlayer.startPlayingFile("/7.mp3");   // audio 7

int baseElbow       = currentElbowAngle;   // start wobble from EXACT current pose
int wobbleAmplitude = 20;                  // same as case 2
bool goingUp        = false;               // start by going slightly down

while (musicPlayer.playingMusic) {
  if (goingUp) {
    currentElbowAngle += 4;
    if (currentElbowAngle >= baseElbow) {
      currentElbowAngle = baseElbow;
      goingUp = false;
    }
  } else {
    currentElbowAngle -= 4;
    if (currentElbowAngle <= baseElbow - wobbleAmplitude) {
      currentElbowAngle = baseElbow - wobbleAmplitude;
      goingUp = true;
    }
  }

  // safety clamp
  if (currentElbowAngle < 0)   currentElbowAngle = 0;
  if (currentElbowAngle > 180) currentElbowAngle = 180;

  elbowRight.write(currentElbowAngle);
  delay(80);   // same wobble speed as case 2
}

  //
  // --- 2) AFTER AUDIO 7: SHOULDER DOWN + ELBOW BACK TO 95° ---
  //
  Serial.println(F("Audio 7 done – returning shoulder down and elbow back to 95°"));

  int shoulderDownTarget = SHOULDER_RESET_ANGLE; // usually 30
  int elbowReturnTarget  = 95;                   // same as case 2

  bool returnDone = false;
  while (!returnDone) {
    returnDone = true;

    // Shoulder down to reset
    if (currentShoulderAngle < shoulderDownTarget) {
      currentShoulderAngle += shoulderStepSize;
      if (currentShoulderAngle > shoulderDownTarget) currentShoulderAngle = shoulderDownTarget;
      shoulderRight.write(currentShoulderAngle);
      returnDone = false;
    } else if (currentShoulderAngle > shoulderDownTarget) {
      currentShoulderAngle -= shoulderStepSize;
      if (currentShoulderAngle < shoulderDownTarget) currentShoulderAngle = shoulderDownTarget;
      shoulderRight.write(currentShoulderAngle);
      returnDone = false;
    }

    // Elbow back to 95°
    if (currentElbowAngle < elbowReturnTarget) {
      currentElbowAngle += elbowStepSize;
      if (currentElbowAngle > elbowReturnTarget) currentElbowAngle = elbowReturnTarget;
      elbowRight.write(currentElbowAngle);
      returnDone = false;
    } else if (currentElbowAngle > elbowReturnTarget) {
      currentElbowAngle -= elbowStepSize;
      if (currentElbowAngle < elbowReturnTarget) currentElbowAngle = elbowReturnTarget;
      elbowRight.write(currentElbowAngle);
      returnDone = false;
    }

    delay(20);  // smooth motion
  }

  break;
}

case 6: {
  Serial.println(F("Case 6: Play audio 8.mp3 only"));

  Serial.println(F("Playing 8.mp3"));
  musicPlayer.startPlayingFile("/8.mp3");

  // Wait until audio finishes
  while (musicPlayer.playingMusic) {
    delay(100);
  }

  break;
}



    }
  }
}  // end of loop()
// end of receiver code
// CHANGEHERE
*/
