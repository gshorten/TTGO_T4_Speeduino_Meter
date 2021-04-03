/*! @brief Air Fuel Ratio (AFR) Gauge
   AFR monitor using TTGO ESP 32 module with 240 x 135 TFT full colour display
   Use ESP 32 Dev Module board definition.
   Uses OTA updates.
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

EasyButton topButton(TOP_BUTTON_PIN);
EasyButton bottomButton(BOTTOM_BUTTON_PIN);

// SpeedData object to get data from the speeduino.  Using Serial2 (defined in setup)
// use reference operator ("&")!
SpeedData SData(&Serial2);

// wifi
char* ssid = SECRET_SSID;
char* password = SECRET_PWD;

// tft display buffer
uint16_t* tft_buffer = (uint16_t*) malloc( 34000 );
bool      buffer_loaded = false;

// Mode constants, set what will be displayed on the gauge
byte g_Mode = 0;
const byte MODE_AFR = 0;        // display AFR & variance from targeyt
const byte MODE_EGO = 1;        // display EGO correction
const byte MODE_LOOPS = 2;      // loops per second
const byte MODE_WARMUP = 3;     // warmup enrichment
const byte MODE_GAMMA = 4;      // total enrichment (GammaE)
const byte MODE_MAP = 5;      // acceleration enrichment
const byte MODE_ACCEL = 6;        // manifold air pressure

const int NUM_MODES = 5;

//update frequencies
int warmupFreq = 200;
int egoFreq = 200;
int loopsFreq = 250;
int afrFreq = 250;
int gammaFreq = 250;
int accelFreq = 250;
int mapFreq = 250;

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

  // turn test mode on to  use simulated data instead of actual
  SData.testModeOn();
  //SData.testModeOff();
  SData.setFakeAFR(125,175);
  
  // initialize display
  tft.init();
  tft.setRotation(1);         // landscape
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);

  // Initialize buttons
  topButton.begin();
  bottomButton.begin();
  topButton.onPressed(handleTopButton);
  bottomButton.onPressed(handleBottomButton);


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
  dispFreq.createSprite(70,40);
  dispFreq.setTextPadding(70);
  dispFreq.setTextColor(TFT_WHITE,TFT_TRANSPARENT);
  dispFreq.setTextDatum(TL_DATUM);    // text datum is at top left 

  // sprite to show AFR bar
  afrBar.createSprite(320,100);
  afrBar.fillRect(0, 0, 160, 100, TFT_RED);
  afrBar.fillRect(160, 0, 160, 100, TFT_BLUE);

}

void loop() {
  ArduinoOTA.handle();
  updateDisplay();
  topButton.read();
  bottomButton.read();
}
