/*! @brief Air Fuel Ratio (AFR) Gauge
   AFR monitor using TTGO ESP 32 module with 320  x 240 TFT full colour display
   Use  "ESP32 Arduino >> ESP 32 Dev Module " board definition.
   for the TFT_eSPI library (display driver) make sure that User_Setup.h has #define ILI9341_DRIVER uncommented.
*/

#include <TFT_eSPI.h>
#include <SPI.h>
#include <User_Setup.h>          // TFT display configuration file
#include <EasyButton.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SpeedData.h>
#include "arduino_secrets.h"

/*!
   @brief   Define two buttons.
*/
const uint8_t TOP_BUTTON_PIN = 38;       //!< on board button above the USB-C port
const uint8_t BOTTOM_BUTTON_PIN = 39;   //!< on board button below the USB-C port
const uint8_t MIDDLE_BUTTON_PIN = 37;

EasyButton topButton(TOP_BUTTON_PIN);
EasyButton bottomButton(BOTTOM_BUTTON_PIN);
EasyButton middleButton(MIDDLE_BUTTON_PIN);

// SpeedData object to get data from the speeduino.  Using Serial2 (defined in setup)
// use reference operator ("&")!
SpeedData SData(&Serial2);

// wifi
char* ssid = SECRET_SSID;
char* password = SECRET_PWD;

// tft display buffer
uint16_t* tft_buffer = (uint16_t*) malloc( 50000 );
bool      buffer_loaded = false;
const float M_SIZE = 1.33;  // analog meter size

// Mode constants, set what will be displayed on the gauge
byte g_Mode = 0;                // current mode
const int NUM_MODES = 7;        // number of modes

const byte MODE_MULTI = 0;      // multi analog gauge
const byte MODE_AFR = 1;        // display AFR & variance from targeyt
const byte MODE_WARMUP = 2;     // warmup enrichment
const byte MODE_GAMMA = 3;      // total enrichment (GammaE)
const byte MODE_MAP = 4;       // manifold air pressure
const byte MODE_ACCEL = 5;      // acceleration enrichment
const byte MODE_RPM = 6;      // multi display

//update frequencies
int warmupFreq = 200;
int multiFreq = 500;
int egoFreq = 200;
int rpmFreq = 250;
int afrFreq = 250;
int gammaFreq = 250;
int accelFreq = 250;
int mapFreq = 250;

// global to hold status - test mode or not
boolean testMode = true;

// Serial2 pins
#define sTX 21    // Serial2 transmit (out), pin J4 on Speeduino connector (SDA on TTGO_T4)
#define sRX 22    // Serial2 recieve (in), pin K4 on Speeduino connector   (SCL on TTGO_T4)

// define display
TFT_eSPI tft = TFT_eSPI(240, 320);

// make sprites
TFT_eSprite dispNum = TFT_eSprite(&tft);       // the big number displayed at top of screen
TFT_eSprite afrVarInd = TFT_eSprite(&tft);     // the AFR variance indicator
TFT_eSprite descText = TFT_eSprite(&tft);     // text displayed underneath the big number
TFT_eSprite dispFreq = TFT_eSprite(&tft);     // displays the update frequency
TFT_eSprite afrBar = TFT_eSprite(&tft);       // sprite for the AFR bar

// **************************** Function Prototypes *******************
void showWarmup(int freq = 200);
void showAFR(int freq = 250);
void showEGO(int freq = 200);
void showLoops(int freq = 250);
void showGammaE(int freq = 200);
void showMAP(int freq = 200);
void showMulti(int freq = 200);
void plotNeedle(int value, byte ms_delay = 10, float afrVar = 0, float afr = 14.0,
                int bottomLeft = 50, int bottomCenter = 50, int bottomRight = 50, float afrvarmax = 0, float afrvarmin = 0 );
//****************************** Setup *******************************
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, sRX, sTX); //Serial port for connecting to Speeduino
  Serial.println("Start");

  // connnect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed!");
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup OTA
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // initialize display
  tft.init();
  tft.setRotation(1);         // landscape
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);

  // Initialize buttons
  topButton.begin();
  bottomButton.begin();
  middleButton.begin();
  topButton.onPressed(handleTopButton);
  bottomButton.onPressed(handleBottomButton);
  middleButton.onPressed(handleMiddleButton);


  //**************** Initialize Sprites ****************
  // also set defaults for sprites
  // Initialize sprite for numeric display
  dispNum.setTextFont(8);
  dispNum.createSprite(320, 140);
  dispNum.fillSprite(TFT_BLACK);
  dispNum.setTextColor(TFT_WHITE);
  dispNum.setTextDatum(TL_DATUM);

  // Initialize sprite for AFR variance indicator
  afrVarInd.createSprite(40, 100);
  afrVarInd.fillSprite(TFT_TRANSPARENT);
  // greenyellow triangle with black border
  afrVarInd.fillTriangle(20, 0, 0, 100, 40, 100, TFT_BLACK);   // corner 1 x,y,  corner 2 x,y, corner 3 x,y
  afrVarInd.fillTriangle(20, 6, 4, 95, 35, 95, TFT_GREENYELLOW);

  // Initialize sprite for text at bottom of screen (instead of AFR Variance Indicator)
  descText.setTextFont(6);
  descText.createSprite(240, 100);       //width, height
  descText.fillSprite(TFT_BLACK);
  descText.setTextColor(TFT_WHITE, TFT_BLACK);
  descText.setTextDatum(TL_DATUM);

  // sprite to show update frequency at right of screen
  dispFreq.setTextFont(4);
  dispFreq.createSprite(70, 40);
  dispFreq.setTextPadding(70);
  dispFreq.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
  dispFreq.setTextDatum(TL_DATUM);    // text datum is at top left

  // sprite to show AFR bar
  afrBar.createSprite(320, 100);
  afrBar.fillRect(0, 0, 160, 100, TFT_RED);
  afrBar.fillRect(160, 0, 160, 100, TFT_BLUE);

  // start with it in regular (not test) mode
  testMode = false;

  // delay for 20 seconds to wait for speeduino to get going
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Waiting for Speeduino",160,120);
  delay(20000);
  
  //default gauge is the multi meter, we have to draw this once.
  drawAnalogMeter();

}

void loop() {
  ArduinoOTA.handle();
  updateDisplay();
  topButton.read();
  bottomButton.read();
  middleButton.read();
  
}
