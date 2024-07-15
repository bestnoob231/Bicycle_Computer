//  ______________________________________________________________________________________
//||_______________________________________________________________________________________||
//||          ___ _            _        ___                     _                          ||
//||         | _ |_)__ _  _ __| |___   / __|___ _ __  _ __ _  _| |_ ___ _ _                ||
//||         | _ \ / _| || / _| / -_) | (__/ _ \ '  \| '_ \ || |  _/ -_) '_|               ||
//||         |___/_\__|\_, \__|_\___|  \___\___/_|_|_| .__/\_,_|\__\___|_|                 ||
//||                   |__/                          |_|                                   ||
//||                              ___          _      _                                    ||
//||                     __ __ __/ /_\  _ _ __| |_  _(_)_ _  ___                           ||
//||                     \ V  V / / _ \| '_/ _` | || | | ' \/ _ \                          ||
//||                      \_/\_/_/_/ \_\_| \__,_|\_,_|_|_||_\___/                          ||
//||                                                                                       ||
//||           _ _   _        _      ___            _                 _    ___ _____       ||
//||      __ _(_) |_| |_ _  _| |__  / / |__  ___ __| |_ _ _  ___  ___| |__|_  )__ / |      ||
//||     / _` | |  _| ' \ || | '_ \/ /| '_ \/ -_|_-<  _| ' \/ _ \/ _ \ '_ \/ / |_ \ |      ||
//||     \__, |_|\__|_||_\_,_|_.__/_/ |_.__/\___/__/\__|_||_\___/\___/_.__/___|___/_|      ||
//||     |___/                                                                             ||
//||_______________________________________________________________________________________||
//||_______________________________________________________________________________________||

//#################   INCLUDES    #################
//#################################################

#include <EEPROM.h>
#include <TFT_eSPI.h>
// #include <SPI.h>

//#################################################
//#################################################

//# # # # # # # # # # # # # # # # # # # # # # # # #

//#################  DEFINITIONS  #################
//#################################################


// TFT screen object
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
TFT_eSPI tftScreen = TFT_eSPI();
unsigned short txtFont = 2;     // Selecting font. For more information search for TFT_eSPI Library
//. . . . . . . . . . . . . . . . . . . . . . . . . . .


// Other pin definition
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

#define HallEffectSensor 2
#define buttonLeft 5
// #define buttonMid 4    // Not used for this version. But will used new versions
#define buttonRight 3
//. . . . . . . . . . . . . . . . . . . . . . . . . . .


// Needs for menu operations
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
enum MenuState {
    MENU_MAIN,
    MENU_AVG
};

MenuState currentState = MENU_MAIN;
//. . . . . . . . . . . . . . . . . . . . . . . . . . .


// Variables to keep track of button presses
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
#define DEBOUNCE_TIME 150
unsigned long currentTime = 0;
unsigned long lastButtonPressTime = 0;
//. . . . . . . . . . . . . . . . . . . . . . . . . . .


//#################################################
//#################################################

//# # # # # # # # # # # # # # # # # # # # # # # # #

//################# INITIALIZIONS #################
//#################################################


// Misc
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
volatile unsigned short counter = 0;  // Counter for storing how many passes did magnet on wheel

const int eeMaxSpeedAddress = 0;  // Speed variable's address for "EEPROM.put()" func.
float fEeMaxSpeed = 0;

const int eeDistAddress = 10;  // Dist. variable's address for "EEPROM.put()" func.
float fEeDist = 0;

const int eeElapsedTotalTimeAddress = 30;  // Elapsed total time variable's address for "EEPROM.put()" func.
float fEeElapsedTotalTime = 0;

float fStartTime;
float fElapsedTime;

float fSpeed = 0.0;
float fMaxSpeed = 0.0;
float fAvgSpeed = 0.0;

float fTakenDistanceCm = 0.0;
float fTotalDistanceKm = 0.0;
float fElapsedTotalTime = 0.0;

//  "W_DIAMETER" is bicycle's wheel diameter (in cm) that magnet attached. It should be;
//  71.12     for     28    inch
//  69.85     for     27.5  inch
//  66.04     for     26    inch
const float F_W_DIAMETER = 66.04;
//. . . . . . . . . . . . . . . . . . . . . . . . . . .


//#################################################
//#################################################


void setup() {

  // Serial.begin(9600);                                // Set Serial connection for debug
  // Serial.println("Serial Connection Initialized!");  // Write "TFT Screen Initialized!" if

  pinMode(HallEffectSensor, INPUT_PULLUP);  // Set pin mode for hall effect sensor
  pinMode(buttonRight, INPUT_PULLUP);            // Set pin mode for right button
  pinMode(buttonLeft, INPUT_PULLUP);            // Set pin mode for left button
  // pinMode(buttonMid, INPUT_PULLUP);            // Set pin mode for middle button

  delay(100);

  tftScreen.init();
  tftScreen.fillScreen(TFT_BLACK);  // Ekranı temizle
  tftScreen.setRotation(2);
  displayMenuStatic(currentState);
  displayMenuDynamic(currentState);


  // Read the stored values from EEPROM
  //-----------------------------------------------------
  float fEeData;  // Declare float variable for EEPROM.get func.

  // If any max speed value exist on EEPROM, read and assign to "fMaxSpeed"
  fEeData = EEPROM.get(eeMaxSpeedAddress, fEeMaxSpeed);
  if (fEeData != 0 && !isnan(fEeData)) { fMaxSpeed = fEeData; }

  // If any total dist. value exist on EEPROM, read and assign to "fTotalDistanceKm"
  fEeData = EEPROM.get(eeDistAddress, fEeDist);
  if (fEeData != 0 && !isnan(fEeData)) { fTotalDistanceKm = fEeData; }

  // If any elapsed total time value exist on EEPROM, read and assign to "fElapsedTotalTime"
  fEeData = EEPROM.get(eeElapsedTotalTimeAddress, fEeElapsedTotalTime);
  if (fEeData != 0 && !isnan(fEeData)) { fElapsedTotalTime = fEeData; }
  //-----------------------------------------------------


  //-----------------------------------------------------
  
  
  // PCICR Settings. To understand these, search for Arduino PCICR
  //-----------------------------------------------------
  PCICR |= 0b00000100;   // enable port D for PCI
  PCMSK2 |= 0b00001000;  // enable INT19 attached to pin d3
  PCMSK2 |= 0b00010000;  // enable INT20 attached to pin d4
  PCMSK2 |= 0b00100000;  // enable INT21 attached to pin d5
  //-----------------------------------------------------
  //-----------------------------------------------------


}

void loop() {

  counter = 0;    // Resetting counter to zero for each loop
  fStartTime = millis();


  attachInterrupt(digitalPinToInterrupt(HallEffectSensor), revolution, FALLING);
  
  delay(1600);
  detachInterrupt(HallEffectSensor);

  calculate();

  eepromWrite();
  
  displayMenuDynamic(currentState);
  
}

void revolution() {
  counter++;
}

ISR(PCINT2_vect) {

  currentTime = millis();
  
  if ((currentTime - lastButtonPressTime) > DEBOUNCE_TIME) {

    if (digitalRead(buttonLeft) == LOW) {
      previousMenu();
      displayMenuStatic(currentState);
    } else if (digitalRead(buttonRight) == LOW) {
      nextMenu();
      displayMenuStatic(currentState); }
    // else if (digitalRead(buttonMid) == LOW) {
    //   resetAll();
    // }

    lastButtonPressTime = currentTime;
  }
}

void nextMenu() {
    switch (currentState) {
        case MENU_MAIN:
            currentState = MENU_AVG;
            break;
        case MENU_AVG:
            currentState = MENU_MAIN;
            break;
        // case MENU_AVG:
        //     currentState = MENU_SETTINGS;
        //     break;
        // case MENU_SETTINGS:
        //     currentState = MENU_MAIN;
        //     break;
    }
}

void previousMenu() {
    switch (currentState) {
        case MENU_MAIN:
            currentState = MENU_AVG;
            break;
        // case MENU_MAIN:
        //     currentState = MENU_SETTINGS;
        //     break;
        case MENU_AVG:
            currentState = MENU_MAIN;
            break;
        // case MENU_SETTINGS:
        //     currentState = MENU_AVG;
        //     break;
    }
}

void displayMenuStatic(MenuState state) {
  tftScreen.fillScreen(TFT_BLACK);  // Clear the screen

  

  switch (state) {
      case MENU_MAIN:
        mainMenuStatics(5, 0, 70, 0, 50, 50, 5, 115, 70, 115);
        break;
      case MENU_AVG:
        mainMenuStatics(5, 0, 70, 0, 70, 115, 5, 115, 53, 50);
        break;
      // case MENU_SETTINGS:
      //   break;
  }
}

void displayMenuDynamic(MenuState state) {
  switch (currentState) {
      case MENU_MAIN:
          mainMenuDynamic(5, 13, 126, 13, 10, 70, 5, 130, 125, 130);
          break;
      case MENU_AVG:
          mainMenuDynamic(5, 13, 126, 13, 70, 140, 5, 130, 86, 70);
          break;
      // case MENU_SETTINGS:
      //     break;
  }
}

void resetAll() {
  fMaxSpeed = 0;
  fElapsedTotalTime = 0;
  fAvgSpeed = 0;
  fTotalDistanceKm = 0;
}

void calculate() {

  if (counter > 0 && counter <= 20) {

    fElapsedTime = millis() - fStartTime;
    fElapsedTotalTime += fElapsedTime;
    fTakenDistanceCm = 2.0 * PI * (F_W_DIAMETER / 2.0) * float(counter);
    fTotalDistanceKm += (fTakenDistanceCm / 100000.0);
    fSpeed = (fTakenDistanceCm / float(fElapsedTime)) * 36;
    fMaxSpeed = max(fMaxSpeed, fSpeed);
    fElapsedTotalTime += fElapsedTime;
    fAvgSpeed = fTotalDistanceKm / (fElapsedTotalTime / 3600000.0);

    // For debug purposes
    //. . . . . . . . . . . . . . . . . . . . . . . . . . .

    // Serial.println("------------------------");
    // Serial.print("Counter :");
    // Serial.println(counter);
    // Serial.print("Taken distance :");
    // Serial.println(fTakenDistanceCm);
    // Serial.print("Total distance (Km):");
    // Serial.println(fTotalDistanceKm);
    // Serial.print("Elapsed Time (mlsec.) :");
    // Serial.println(fElapsedTime);
    // Serial.print("Speed (km/h) :");
    // Serial.println(fSpeed);
    // Serial.print("MaxSpeed: ");
    // Serial.println(fMaxSpeed);
    // Serial.print("Average Speed: ");
    // Serial.println(fAvgSpeed);

    //. . . . . . . . . . . . . . . . . . . . . . . . . . .

  } else {
    fSpeed = 0.0;  // Set speed value zero if there is no sensor input
  }
}

void eepromWrite() {
  float data = 0;   // Temp variable for reading from eeprom

  // If data on eeprom is different than write new data to eeprom
  //. . . . . . . . . . . . . . . . . . . . . . . . . . .
  data = EEPROM.get(eeMaxSpeedAddress, fEeMaxSpeed);
  if (data != fMaxSpeed) { EEPROM.put(eeMaxSpeedAddress, fMaxSpeed); }

  data = EEPROM.get(eeDistAddress, fEeDist);
  if (data != fTotalDistanceKm) { EEPROM.put(eeDistAddress, fTotalDistanceKm); }

  data = EEPROM.get(eeElapsedTotalTimeAddress, fEeElapsedTotalTime);
  if ( data != fElapsedTotalTime) {EEPROM.put(eeElapsedTotalTimeAddress, fElapsedTotalTime); }
  //. . . . . . . . . . . . . . . . . . . . . . . . . . .


}

void mainMenuDynamic(int spd_x, int spd_y, int max_x, int max_y, int time_x, int time_y, int dist_x, int dist_y, int avg_x, int avg_y){

  tftScreen.setTextSize(2);

  // Variables for calculating and tracking time
  //. . . . . . . . . . . . . . . . . . . . . . . . . . .
  unsigned long seconds = 0;
  unsigned long minutes = 0;
  unsigned long hours = 0;
  //. . . . . . . . . . . . . . . . . . . . . . . . . . .

  // Write the texts
  //-----------------------------------------------------

  seconds = fElapsedTotalTime / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;

  seconds = seconds % 60;
  minutes = minutes % 60;
  
  char time[9];     // "hh:mm:ss" + null
  sprintf(time, "%02lu:%02lu:%02lu", hours, minutes, seconds);

  int padding = tftScreen.textWidth("99.9", txtFont);
  tftScreen.setTextPadding(padding);
  tftScreen.setTextColor(TFT_WHITE, TFT_BLACK);
  tftScreen.drawFloat(fSpeed, 1, spd_x, spd_y, txtFont);

  tftScreen.drawFloat(fTotalDistanceKm, 1, dist_x, dist_y, txtFont);  

  tftScreen.setTextDatum(TR_DATUM);
  tftScreen.drawFloat(fMaxSpeed, 1, max_x, max_y, txtFont);

  tftScreen.drawFloat(fAvgSpeed, 1, avg_x, avg_y, txtFont);
  tftScreen.setTextDatum(TL_DATUM);

  if (currentState == MENU_AVG) {
    tftScreen.setTextSize(1);
  }

  padding = tftScreen.textWidth("99.99.99", txtFont);
  tftScreen.setTextPadding(padding);
  tftScreen.drawString(time, time_x,  time_y, txtFont);


  //-----------------------------------------------------
  //-----------------------------------------------------
}

void mainMenuStatics(int spd_x, int spd_y, int max_x, int max_y, int time_x, int time_y, int dist_x, int dist_y, int avg_x, int avg_y){
  
  // Write static text to screen
  //-----------------------------------------------------

  tftScreen.fillScreen(TFT_BLACK);          // Set background of screen fully TFT_BLACK
  tftScreen.setTextSize(1);

  tftScreen.drawFastHLine(0, 45, 128, TFT_ORANGE);
  tftScreen.drawFastVLine(64, 0, 45, TFT_ORANGE);
  tftScreen.drawFastHLine(0, 115, 128, TFT_ORANGE);
  tftScreen.drawFastVLine(64, 115, 45, TFT_ORANGE);


  tftScreen.setTextColor(TFT_MAGENTA);

  tftScreen.drawString("TIME", time_x, time_y, txtFont);
  tftScreen.drawString("SPD", spd_x, spd_y, txtFont);
  tftScreen.drawString("DIST", dist_x, dist_y, txtFont);
  tftScreen.drawString("MAX", max_x, max_y, txtFont);
  tftScreen.drawString("AVG", avg_x, avg_y, txtFont);

  //-----------------------------------------------------
  //-----------------------------------------------------
}
