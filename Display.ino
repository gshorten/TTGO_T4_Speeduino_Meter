/*
   Display Related Functions
*/

/*!
   @brief Updates the display with Speeduino data based on the mode, which is changed with
   the bottom pushbutton

   @param g_Mode
   Global variable for display mode
*/
void updateDisplay() {
  // update the display based on the current mode
  switch (g_Mode) {
    case MODE_AFR:
      showAFR(afrFreq);
      break;
    case MODE_EGO:
      showEGO(egoFreq);
      break;
    case MODE_LOOPS:
      showLoops(loopsFreq);
      break;
    case MODE_WARMUP:
      showWarmup(warmupFreq);
      break;
    case MODE_GAMMA:
      showGammaE(gammaFreq);
      break;
    case MODE_MAP:
      showMAP(mapFreq);
    default:
      showAFR(afrFreq);
      break;
  }
}

void showWarmup(int freq) {
  // displays warmup enrichment
  dispNum.fillSprite(TFT_BLACK);
  int warmup = SData.getWarmup(freq);     // Get latest Loops reading
  warmup -= 100;              // 0 will be no warmup enrichment, 100 will b 100%
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.drawNumber(warmup, 80, 20);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  descText.setTextColor(TFT_WHITE, TFT_BLACK);
  descText.drawString("Warmup", 10, 20);
  descText.pushSprite(0, 100);
  showFreq(freq);       // display frequency at bottom right
}

void showMAP(int freq){
   // displays Manifold Air Pressure (MAP)
  dispNum.fillSprite(TFT_BLACK);
  int value = SData.getMAP(freq);     // Get latest MAP reading
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.drawNumber(value, 45, 0);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  descText.setTextColor(TFT_WHITE, TFT_BLACK);
  descText.drawString("MAP", 10, 10);
  descText.pushSprite(0, 90);
  showFreq(freq);       // display frequency at bottom right
}

void showAFR(int freq) {
  // displays the current AFR and a triangle on the bottom for AFR variance
  float afrVar;
  float actualAFR = 14.7;
  float targetAFR = 14.0;
  int indPosition = 160;
  static byte indPositionOld = 10;      // initialize old position so that indicator will display ( indPosition must

  // get actual and target afr, use SpeedData library (instance is SData)
  actualAFR = SData.getActualAFR(freq);
  targetAFR = SData.getTargetAFR(freq);

  // Set colours for Actual AFR depending on value.
  if (actualAFR > 16.0 || actualAFR < 12.0) {
    // Dangerous AFR; AFR is outside normal safe range
    dispNum.fillSprite(TFT_RED);
    dispNum.setTextColor(TFT_WHITE);
  }
  else if (actualAFR < 15 && actualAFR > 13.5) {
    // normal range
    dispNum.fillSprite(TFT_BLACK);
    dispNum.setTextColor(TFT_WHITE);
  }
  else {
    // warning; outside normal but not in danger
    dispNum.fillSprite(TFT_BLACK);
    dispNum.setTextColor(TFT_ORANGE);
  }
  dispNum.drawFloat(actualAFR, 1, 80, 20);
  dispNum.pushSprite(0, 0);

  // calculate AFR variance
  afrVar = actualAFR - targetAFR;
  //Serial.print("AFR Variance :"); Serial.println(afrVar);

  // limit variance displayed to +- 1.5
  if ( afrVar < -1.5) {
    afrVar = -1.5;
  }
  else if (afrVar > 1.5) {
    afrVar = 1.5;
  }
  // calulate position of variance indicator
  indPosition = (afrVar * 70) + 105;

  // only draw indicator if it's moved by more than 10 pixels, this stops it from hopping around
  if (abs(indPosition - indPositionOld) > 10) {

    // draw bar at bottom
    afrBar.pushSprite(0, 160);

    // draw indicator
    afrVarInd.pushSprite(indPosition, 140, TFT_TRANSPARENT);
    indPositionOld = indPosition;

    // display update frequency
    //dispFreq.fillSprite(TFT_BLUE);
    //dispFreq.setTextPadding(80);
    dispFreq.setTextColor(TFT_WHITE,TFT_TRANSPARENT);
    dispFreq.drawNumber(afrFreq, 10, 10);
    dispFreq.pushSprite(240, 80);
  }
}

void showEGO(int freq) {
  // displays the EGO Correction value on the display

  long EGO = SData.getEGO(freq);
  //dispNum.fillSprite(TFT_BLACK);

  if (EGO > 115) {
    EGO = 115;
  }
  else if (EGO < 85) {
    EGO = 85;
  }
  // set display colors
  if (EGO > 105) {
    dispNum.fillSprite(TFT_BLUE);
    //dispNum.setTextColor(TFT_WHITE);
  }
  else if (EGO < 95) {
    dispNum.fillSprite(TFT_RED);
    //dispNum.setTextColor(TFT_WHITE);
  }
  else {
    dispNum.fillSprite(TFT_BLACK);
    //dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  // display EGO value
  String dispEGO = String(EGO - 100);
  dispNum.drawString(dispEGO, 50, 0);
  dispNum.pushSprite(0, 0);
  
  // draw label on bottom
  descText.setTextColor(TFT_WHITE, TFT_BLACK);
  descText.drawString("EGO Corr", 10, 10);
  descText.pushSprite(0, 90);
  showFreq(freq);       // display frequency at bottom right
}

void showLoops(int freq) {
  // show the loops per second display
  dispNum.fillSprite(TFT_BLACK);
  int loopsPS = SData.getLoops(freq);     // Get latest Loops reading

  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);

  dispNum.drawNumber(loopsPS, 10, 0);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  descText.setTextColor(TFT_WHITE, TFT_BLACK);
  descText.drawString("Loops", 10, 10 );
  descText.pushSprite(0, 90);
  showFreq(freq);       // display frequency at bottom right
}

void showGammaE(int freq) {
  // show the total fuel enrichment (GammaE);
  dispNum.fillSprite(TFT_BLACK);
  int gammaE = SData.getGammaE(freq);     // Get latest gammaE reading
  gammaE -= 100;
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.drawNumber(gammaE, 40, 0);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  descText.setTextColor(TFT_WHITE, TFT_BLACK);
  descText.drawString("GammaE", 10, 10);
  descText.pushSprite(0, 90);     //X,y,

  showFreq(freq);       // display frequency at bottom right
}

void showFreq(int freq) {
  // display frequency at bottom right
  //dispFreq.fillSprite(TFT_BLACK);
  dispFreq.setTextPadding(60);
  dispFreq.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
  //tft.setTextFont(4);
  dispFreq.drawNumber(freq, 10, 10);
  dispFreq.pushSprite(260, 80);
}
