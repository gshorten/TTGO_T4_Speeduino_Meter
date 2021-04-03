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
  dispNum.setTextFont(8);
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.drawNumber(warmup, 80, 20);
  dispNum.pushSprite(0, 0);

  showDescription("Warmup %");   // display description
  showFreq(freq);                // display frequency at bottom right
}

void showMAP(int freq) {
  // displays Manifold Air Pressure (MAP)
  dispNum.fillSprite(TFT_BLACK);
  int value = SData.getMAP(freq);     // Get latest MAP reading
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.setTextFont(8);
  dispNum.drawNumber(value, 80, 20);
  dispNum.pushSprite(0, 0);

  showDescription("MAP");
  showFreq(freq);       // display frequency at bottom right
}

void showAFRAnalog(int freq) {
  //analog AFR meter. Based on Bodmer TFT_Meter_linear TFT_eSPI example sketch
  const float M_SIZE = 1.33;
  float ltx = 0;    // Saved x coord of bottom of needle
  uint16_t osx = M_SIZE * 120, osy = M_SIZE * 120; // Saved x & y coords
  uint32_t updateTime = 0;       // time for next update
  int old_analog =  -999; // Value last displayed

  int value[6] = {0, 0, 0, 0, 0, 0};
  int old_value[6] = { -1, -1, -1, -1, -1, -1};
  int d = 0;

}

void showAFR(int freq) {
  // displays the current AFR and a triangle on the bottom for AFR variance

  float actualAFR = 14.7;
  float targetAFR = 14.0;
  int indPosition = 160;
  static byte indPositionOld = 10;      // initialize old position so that indicator will display ( indPosition must
  int backgroundColour;
  int textColour;
  float afrVar;
  const int afrVarReadings = 5;    // number of readings to average the variance over
  static float afrVarAvg;                 // average AFR Variance
  float totalAfrVar = 0;
  int afrVarCounter;                   // afr variance counter
  static float afrVarSamples[afrVarReadings];
  static long lastAfrRead = 0;

  // get actual and target afr, use SpeedData library (instance is SData)
  actualAFR = SData.getActualAFR(freq);
  Serial.print("Actual AFR:");Serial.println(actualAFR);
  targetAFR = SData.getTargetAFR(freq);

  // Set colours for Actual AFR depending on value.
  if (actualAFR > 16.0 || actualAFR < 12.0) {
    // Dangerous AFR; AFR is outside normal safe range
    backgroundColour = TFT_RED;
    textColour = TFT_WHITE;
  }
  else if (actualAFR < 15 && actualAFR > 13.5) {
    // normal range
    backgroundColour = TFT_BLACK;
    textColour = TFT_WHITE;
  }
  else {
    // warning; outside normal but not in danger
    backgroundColour = TFT_BLACK;
    textColour = TFT_ORANGE;
  }
  dispNum.fillSprite(backgroundColour);
  dispNum.setTextColor(textColour);
  dispNum.setTextFont(8);
  dispNum.drawFloat(actualAFR, 1, 80, 20);
  String afr = "AFR";
  dispNum.setTextFont(4);
  dispNum.drawString(afr, 10, 110);
  // show afr update freqency
  dispNum.drawNumber(afrFreq, 270, 110);
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
  // keep last 5 variance readings, save every time we get afrTarget & afrActual
  if (millis() - lastAfrRead > freq) {
    afrVarSamples[afrVarCounter] = afrVar;
    afrVarCounter ++;
    if (afrVarCounter == afrVarReadings) {
      afrVarCounter = 0;
    }
    lastAfrRead = millis();
  }
  // average saved afr Var readings
  for (int i = 0; i < afrVarReadings; i++) {
    totalAfrVar += afrVarSamples[i];
  }
  afrVarAvg = totalAfrVar / afrVarReadings;

  // calulate position of variance indicator
  indPosition = (afrVarAvg * 100) + 140;

  // only draw indicator if it's moved by more than 10 pixels, this stops it from hopping around
  if (abs(indPosition - indPositionOld) > 10) {

    // draw bar at bottom
    //afrBar.pushSprite(0, 160);
    tft.fillRect(0, 140, 160, 100, TFT_RED);
    tft.fillRect(160, 140, 160, 100, TFT_BLUE);

    // draw indicator
    afrVarInd.pushSprite(indPosition, 140, TFT_TRANSPARENT);
    indPositionOld = indPosition;
  }
}

void showEGO(int freq) {
  // displays the EGO Correction value on the display

  long EGO = SData.getEGO(freq);
  //dispNum.fillSprite(TFT_BLACK);

  EGO = constrain(EGO, 85,115);
  
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
  dispNum.setTextFont(8);
  dispNum.drawString(dispEGO, 80, 20);
  dispNum.pushSprite(0, 0);

  // draw label on bottom
  showDescription("EGO Correction");
  showFreq(freq);       // display frequency at bottom right
}

void showLoops(int freq) {
  // show the loops per second display
  dispNum.fillSprite(TFT_BLACK);
  int loopsPS = SData.getLoops(freq);     // Get latest Loops reading

  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.setTextFont(8);
  dispNum.drawNumber(loopsPS, 80, 20);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  showDescription("Loops per second");
  showFreq(freq);       // display frequency at bottom right
}

void showGammaE(int freq) {
  // show the total fuel enrichment (GammaE);
  dispNum.fillSprite(TFT_BLACK);
  int gammaE = SData.getGammaE(freq);     // Get latest gammaE reading
  gammaE -= 100;
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.setTextFont(8);
  dispNum.drawNumber(gammaE, 80, 20);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  showDescription("Total GammaE");
  showFreq(freq);       // display frequency at bottom right
}

void showFreq(int freq) {
  // display frequency at bottom right
  //dispFreq.fillSprite(TFT_BLACK);
  //dispFreq.setTextPadding(60);
  dispFreq.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
  tft.setTextFont(4);
  dispFreq.drawNumber(freq, 20, 10);
  dispFreq.pushSprite(250, 200);
}

void showDescription(String desc) {
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(desc, 20, 220);
}

//void analogMeter() {
//  // generic analog meter
//  // Meter outline
//  tft.fillRect(0, 0, M_SIZE * 239, M_SIZE * 126, TFT_WHITE);
//  tft.fillRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_BLACK);
//
//  tft.setTextColor(TFT_WHITE);  // Text colour
//
//  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
//  for (int i = -50; i < 51; i += 5) {
//    // Long scale tick length
//    int tl = 15;
//
//    // Coodinates of tick to draw
//    float sx = cos((i - 90) * 0.0174532925);
//    float sy = sin((i - 90) * 0.0174532925);
//    uint16_t x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
//    uint16_t y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
//    uint16_t x1 = sx * M_SIZE * 100 + M_SIZE * 120;
//    uint16_t y1 = sy * M_SIZE * 100 + M_SIZE * 140;
//
//    // Coordinates of next tick for zone fill
//    float sx2 = cos((i + 5 - 90) * 0.0174532925);
//    float sy2 = sin((i + 5 - 90) * 0.0174532925);
//    int x2 = sx2 * (M_SIZE * 100 + tl) + M_SIZE * 120;
//    int y2 = sy2 * (M_SIZE * 100 + tl) + M_SIZE * 140;
//    int x3 = sx2 * M_SIZE * 100 + M_SIZE * 120;
//    int y3 = sy2 * M_SIZE * 100 + M_SIZE * 140;
//
//    // Yellow zone limits
//    //if (i >= -50 && i < 0) {
//    //  tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
//    //  tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
//    //}
//
//    // Green zone limits
//    if (i >= 0 && i < 25) {
//      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
//      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
//    }
//
//    // Orange zone limits
//    if (i >= 25 && i < 50) {
//      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
//      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
//    }
//
//    // Short scale tick length
//    if (i % 25 != 0) tl = 8;
//
//    // Recalculate coords incase tick lenght changed
//    x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
//    y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
//    x1 = sx * M_SIZE * 100 + M_SIZE * 120;
//    y1 = sy * M_SIZE * 100 + M_SIZE * 140;
//
//    // Draw tick
//    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
//
//    // Check if labels should be drawn, with position tweaks
//    if (i % 25 == 0) {
//      // Calculate label positions
//      x0 = sx * (M_SIZE * 100 + tl + 10) + M_SIZE * 120;
//      y0 = sy * (M_SIZE * 100 + tl + 10) + M_SIZE * 140;
//      switch (i / 25) {
//        case -2: tft.drawCentreString("12.0", x0, y0 - 12, 4); break;
//        case -1: tft.drawCentreString("13.0", x0, y0 - 9, 4); break;
//        case 0: tft.drawCentreString("14.0", x0, y0 - 7, 4); break;
//        case 1: tft.drawCentreString("15.0", x0, y0 - 9, 4); break;
//        case 2: tft.drawCentreString("16.0", x0, y0 - 12, 4); break;
//      }
//    }
//
//    // Now draw the arc of the scale
//    sx = cos((i + 5 - 90) * 0.0174532925);
//    sy = sin((i + 5 - 90) * 0.0174532925);
//    x0 = sx * M_SIZE * 100 + M_SIZE * 120;
//    y0 = sy * M_SIZE * 100 + M_SIZE * 140;
//    // Draw scale arc, don't draw the last part
//    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
//  }
//
//  tft.drawString("AFR", M_SIZE * (5 + 230 - 40), M_SIZE * (119 - 20), 2); // Units at bottom right
//  tft.drawCentreString("AFR", M_SIZE * 120, M_SIZE * 70, 6); // Comment out to avoid font 4
//  tft.drawRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_BLACK); // Draw bezel line
//
//  plotNeedle(0, 0); // Put meter needle at 0
//}
//
//// #########################################################################
//// Update needle position
//// This function is blocking while needle moves, time depends on ms_delay
//// 10ms minimises needle flicker if text is drawn within needle sweep area
//// Smaller values OK if text not in sweep area, zero for instant movement but
//// does not look realistic... (note: 100 increments for full scale deflection)
//// #########################################################################
//void plotNeedle(int value, byte ms_delay)
//{
//  tft.setTextColor(TFT_WHITE, TFT_BLACK);
//  char buf[8]; dtostrf(value, 4, 0, buf);
//  //tft.drawRightString(buf, M_SIZE*40, M_SIZE*(119 - 20), 6);
//
//  if (value < -10) value = -10; // Limit value to emulate needle end stops
//  if (value > 110) value = 110;
//
//  // Move the needle until new value reached
//  while (!(value == old_analog)) {
//    if (old_analog < value) old_analog++;
//    else old_analog--;
//
//    if (ms_delay == 0) old_analog = value; // Update immediately if delay is 0
//
//    float sdeg = map(old_analog, -10, 110, -150, -30); // Map value to angle
//    // Calcualte tip of needle coords
//    float sx = cos(sdeg * 0.0174532925);
//    float sy = sin(sdeg * 0.0174532925);
//
//    // Calculate x delta of needle start (does not start at pivot point)
//    float tx = tan((sdeg + 90) * 0.0174532925);
//
//    // Erase old needle image
//    tft.drawLine(M_SIZE * (120 + 20 * ltx - 2), M_SIZE * (140 - 20), osx - 2, osy, TFT_BLACK);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx - 1), M_SIZE * (140 - 20), osx - 1, osy, TFT_BLACK);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx), M_SIZE * (140 - 20), osx, osy, TFT_BLACK);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx + 1), M_SIZE * (140 - 20), osx + 1, osy, TFT_BLACK);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx + 2), M_SIZE * (140 - 20), osx + 2, osy, TFT_BLACK);
//
//    // Re-plot text under needle
//    tft.setTextColor(TFT_WHITE, TFT_BLACK);
//    tft.setTextPadding(80);
//    tft.drawCentreString(buf, M_SIZE * 120, M_SIZE * 70, 6); // // Comment out to avoid font 4
//
//    // Store new needle end coords for next erase
//    ltx = tx;
//    osx = M_SIZE * (sx * 98 + 120);
//    osy = M_SIZE * (sy * 98 + 140);
//
//    // Draw the needle in the new postion, magenta makes needle a bit bolder
//    // draws 3 lines to thicken needle
//    tft.drawLine(M_SIZE * (120 + 20 * ltx - 2), M_SIZE * (140 - 20), osx - 2, osy, TFT_YELLOW);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx - 1), M_SIZE * (140 - 20), osx - 1, osy, TFT_YELLOW);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx), M_SIZE * (140 - 20), osx, osy, TFT_MAGENTA);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx + 1), M_SIZE * (140 - 20), osx + 1, osy, TFT_YELLOW);
//    tft.drawLine(M_SIZE * (120 + 20 * ltx + 2), M_SIZE * (140 - 20), osx + 2, osy, TFT_YELLOW);
//
//    // Slow needle down slightly as it approaches new postion
//    if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;
//
//    // Wait before next update
//    delay(ms_delay);
//  }
//}
