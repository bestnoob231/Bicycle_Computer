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
//||	 																					                                           ||
//||_______________________________________________________________________________________||
//||_______________________________________________________________________________________||

#include <Adafruit_ST7735.h>
#include <SPI.h>


//#################  DEFINITIONS  #################
//#################################################
// Color definitions
//-------------------------------------------------

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//-------------------------------------------------
//-------------------------------------------------


// TFT pins definitions
//-------------------------------------------------

#define TFT_SCLK      13
#define TFT_MOSI      11
#define TFT_CS        10
#define TFT_DC        9
#define TFT_RST       8 // Or set to -1 and connect to Arduino RESET pin

Adafruit_ST7735 tftScreen = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//-------------------------------------------------
//-------------------------------------------------


// Sensor pin definition
//-------------------------------------------------

#define HallEffectSensor      2

//-------------------------------------------------
//-------------------------------------------------
//#################################################
//#################################################

//# # # # # # # # # # # # # # # # # # # # # # # # #

//################# INITIALIZIONS #################
//#################################################

// Initialized variables
//-------------------------------------------------

String speedStr;
String maxSpeedStr;
String avgSpeedStr;
String totalDistanceStr;

volatile unsigned short counter = 0;

float startTime;
float fElapsedTime;

float fSpeed = 0.0;
float fMaxSpeed = 0.0;
float fAvgSpeed = 0.0;

float fTakenDistanceCm = 0.0;
float fTotalDistanceKm = 0.0;
float fElapsedTotalTime = 0.0;

//  W_DIAMETER is bicycle's wheel diameter (in cm)
//  that magnet attached. It should be;
//  71.12     for     28    inch
//  69.85     for     27.5  inch
//  66.04     for     26    inch
const float W_DIAMETER = 66.04;

//-------------------------------------------------
//-------------------------------------------------
//#################################################
//#################################################


void setup() {

  Serial.begin(9600);                                   // Set serial connection for debug
  Serial.println("Serial Connection Initialized!");     // Write "TFT Screen Initialized!" if

  pinMode(HallEffectSensor, INPUT);                     // Set pin mode for hall effect sensor
  

  tftScreen.initR(INITR_GREENTAB); 
  Serial.println("TFT Screen Initialized!");            // Write "TFT Screen Initialized!" if 


  // Write static text to screen
  //-----------------------------------------------------

  tftScreen.fillScreen(BLACK);
  tftScreen.setRotation(3);
  tftScreen.setTextColor(MAGENTA);
  tftScreen.setTextSize(2);
  tftScreen.setCursor(6, 5);
  tftScreen.print("SPD");
  tftScreen.setCursor(6, 60);
  tftScreen.print("Dist");
  tftScreen.setCursor(105, 5);
  tftScreen.print("Max");
  tftScreen.setCursor(105, 60);
  tftScreen.print("AVG");

  //-----------------------------------------------------
  //-----------------------------------------------------

}


void loop() {
  
  counter = 0;
  startTime = millis();

  attachInterrupt(digitalPinToInterrupt(HallEffectSensor), revs, RISING);
  delay(1600);
  detachInterrupt(HallEffectSensor);
  
  calculate();

  // Erase the texts that written
  //-----------------------------------------------------

  tftScreen.setTextColor(BLACK);
  tftScreen.setCursor(10, 25);
  tftScreen.print(speedStr);


  tftScreen.setTextColor(BLACK);
  tftScreen.setCursor(100, 25);
  tftScreen.print(maxSpeedStr);
  
  
  tftScreen.setTextColor(BLACK);
  tftScreen.setCursor(10, 80);
  tftScreen.print(totalDistanceStr);


  tftScreen.setTextColor(BLACK);
  tftScreen.setCursor(100, 80);
  tftScreen.print(avgSpeedStr);

  //-----------------------------------------------------
  //-----------------------------------------------------

 
  totalDistanceStr = String(fTotalDistanceKm);
  speedStr = String(fSpeed, 1);
  maxSpeedStr = String(fMaxSpeed, 1);
  avgSpeedStr = String(fAvgSpeed , 1);


  // Write the texts
  //-----------------------------------------------------

  tftScreen.setTextColor(WHITE);
  tftScreen.setCursor(10, 25);
  tftScreen.print(speedStr);


  tftScreen.setTextColor(WHITE);
  tftScreen.setCursor(100, 25);
  tftScreen.print(maxSpeedStr);


  tftScreen.setTextColor(WHITE);
  tftScreen.setCursor(10, 80);
  tftScreen.print(totalDistanceStr);

  
  tftScreen.setTextColor(WHITE);
  tftScreen.setCursor(100, 80);
  tftScreen.print(avgSpeedStr);

  //-----------------------------------------------------
  //-----------------------------------------------------

}


void revs() 
{  

  counter++; 

}


void calculate()
{

  if(counter > 0)
  {

    fElapsedTime = millis() - startTime;
    fTakenDistanceCm = 2.0 * PI * (W_DIAMETER / 2.0) * float(counter);
    fTotalDistanceKm += (fTakenDistanceCm / 100000.0);
    fSpeed = (fTakenDistanceCm / float(fElapsedTime)) * 36;
    fMaxSpeed = max(fMaxSpeed, fSpeed);
    fElapsedTotalTime += fElapsedTime;
    fAvgSpeed = fTotalDistanceKm / (fElapsedTotalTime / 3600000.0);


    // For debug purposes
    //------------------------------------------------------------------

    Serial.println("------------------------");
    Serial.print("Counter :");
    Serial.println(counter);
    Serial.print("Taken distance :");
    Serial.println(fTakenDistanceCm);
    Serial.print("Total distance (Km):");
    Serial.println(fTotalDistanceKm);
    Serial.print("Elapsed Time (mlsec.) :");
    Serial.println(fElapsedTime);
    Serial.print("Speed (km/h) :");
    Serial.println(fSpeed);
    Serial.print("MaxSpeed: ");
    Serial.println(fMaxSpeed);
    Serial.print("Average Speed: ");
    Serial.println(fAvgSpeed);

    //-------------------------------------------------------------------

  }else
  {

    fSpeed = 0.0; // Set speed value zero if there is no sensor input

  }

}
