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
    case MODE_MULTI:
      showMulti(multiFreq);
      break;
    case MODE_AFR:
      showAFR(afrFreq);
      break;
    case MODE_WARMUP:
      showWarmup(warmupFreq);
      break;
    case MODE_GAMMA:
      showGammaE(gammaFreq);
      break;
    case MODE_MAP:
      showMAP(mapFreq);
      break;
    case MODE_ACCEL:
      showAccel(accelFreq);
      break;
    case MODE_RPM:
      showRPM(rpmFreq);
      break;
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

void showAccel(int freq) {
  // displays warmup enrichment
  dispNum.fillSprite(TFT_BLACK);
  int accel = SData.getAccelEnrich(freq);     // Get latest Loops reading
  accel -= 100;              // 0 will be no warmup enrichment, 100 will b 100%
  dispNum.setTextFont(8);
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.drawNumber(accel, 80, 20);
  dispNum.pushSprite(0, 0);

  showDescription("Accel Enrich %");   // display description
  showFreq(freq);                // display frequency at bottom right
}

void showMAP(int freq) {
  // displays Manifold Air Pressure (MAP)
  dispNum.fillSprite(TFT_BLACK);
  int mapValue = SData.getMAP(freq);     // Get latest MAP reading
  dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  dispNum.setTextFont(8);
  dispNum.drawNumber(mapValue, 80, 20);
  dispNum.pushSprite(0, 0);

  showDescription("MAP");
  showFreq(freq);       // display frequency at bottom right
}

void showMulti(int freq) {
  // analog multi gauge, this is the main display.  has AFR, MAP, GammaE, Advance
  // AFR variance is on the top analog gauge, others are on bottom.
  //  the AFR variance is the peak high or low of the last x (ie, 5)  readings.  If there is a negative and positive variance peak
  //  then we show whichever one is biggest.

  // variables to hold afr values
  float minAFR = 11.0;
  float maxAFR = 19.0;
  static float actualAFR = 14.7;
  static float targetAFR = 14.0;
  static float afrVar = 1.0;
  static int afrNeedlePos;
  const int noSamples = 10;  // number of AFR variance samples to average
  static float afrVarSamples[noSamples];
  static int afrCounter = 0;
  static float afrVarAvg;
  static float afrVarTotal;
  static float afrVarMin = 0;
  static float afrVarMax = 0;
  // draw afr variance gauge, we only want to do this once
  //drawAnalogMeter();

  // get actual and target afr
  actualAFR = SData.getActualAFR(100);
  targetAFR = SData.getTargetAFR(300);
  afrVar = targetAFR - actualAFR;
  afrVar = constrain(afrVar, -1.5, 1.5);
  actualAFR = constrain(actualAFR, minAFR, maxAFR);
  int displayAFR = actualAFR * 10;     // convert to integer for map function next
  afrNeedlePos = map(displayAFR, minAFR * 10, maxAFR * 10, 0, 100); //convert afr to 0 - 100 range for gauge

  // calculate average, max, min AFR variances
  // get the maximum and minimum of the last 5 readings, and the average
  // kind of like a meter that holds the peak values
  //  if (afrCounter < noSamples) {
  //    // add to the array, we don't have enough samples yet
  //    afrVarSamples[afrCounter] = afrVar;
  //    afrCounter ++;
  //  }
  //  if (afrCounter == noSamples) {
  //    //now we have enough samples, get avg, max, min and reset counter
  //    afrVarMin = 0;
  //    afrVarMax = 0;
  //    afrVarTotal = 0;
  //    for (int i = 0; i < noSamples; i ++) {
  //      //get min
  //      if (afrVarSamples[i] < afrVarMin) {
  //        afrVarMin = afrVarSamples[i];
  //      }
  //      // get max
  //      if (afrVarSamples[i] > afrVarMax) {
  //        afrVarMax  = afrVarSamples[i];
  //      }
  //      // add to total for average
  //      afrVarTotal += afrVarSamples[i];
  //    }
  //    afrVarAvg = afrVarTotal / noSamples;
  //    afrCounter = 0;
  //    Serial.print("AFR Avg, Max, Min:"); Serial.print(afrVarAvg); Serial.print(" / "); Serial.print(afrVarMax);
  //    Serial.print(" / "); Serial.println(afrVarMin);
  //  }

  int mapValue = SData.getMAP(200);     // Get latest MAP reading
  //  Serial.print("Actual AFR: "); Serial.println(actualAFR);
  //  Serial.print("Target AFR: "); Serial.println(targetAFR);
  //  Serial.print("MAP : "); Serial.println(mapValue);
  //delay(25);
  int gamma = SData.getGammaE(200);
  // Serial.print("unadjusted Gamma"); Serial.println(gamma);
  gamma = map(gamma, 75, 150, -25, 50);
  //delay(25);
  //  Serial.print("gamma:"); Serial.println(gamma);
  //  Serial.println();

  //get ego
  //  int EGO = SData.getEGO(freq);
  //  Serial.print("EGO: "); Serial.println(EGO);
  //  EGO = constrain(EGO, 75, 150);
  //  EGO = map(EGO, 75, 150, -25, 50);

  // get TPS
  int tps = SData.getTPS();
  //delay(25);
  // get water temperature
  int water = SData.getWaterTemp();

  plotNeedle(afrNeedlePos, 20, afrVar, actualAFR, mapValue, tps, water, afrVarMax, afrVarMin);
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
  const int afrVarReadings = 3;    // number of readings to average the variance over
  static float afrVarAvg;                 // average AFR Variance
  float totalAfrVar = 0;
  int afrVarCounter;                   // afr variance counter
  static float afrVarSamples[afrVarReadings];
  static long lastAfrRead = 0;

  // get actual and target afr, use SpeedData library (instance is SData)
  actualAFR = SData.getActualAFR(freq - 50);
  Serial.print("Actual AFR:"); Serial.println(actualAFR);
  targetAFR = SData.getTargetAFR(freq + 50);

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

  // limit variance displayed to +- 1
  afrVar = constrain(afrVar, -1, 1);

  // keep last n variance readings, save every time we get afrTarget & afrActual
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

  // calulate position of variance indicator, convert -1 to 0, 1 to 320
  //indPosition = (afrVarAvg * 160) + 160;
  afrVarAvg *= 150;    // have to multipy afrVar by 150 for map function (it only works with integers), also adjust so typical variances use full gauge range
  indPosition = map(afrVarAvg, -100, 100, 0, 320);

  // only draw indicator if it's moved by more than 5 pixels, this stops it from hopping around
  if (abs(indPosition - indPositionOld) > 5) {

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

  EGO = constrain(EGO, 85, 115);

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

void showRPM(int freq) {
  // show the loops per second display
  dispNum.fillSprite(TFT_BLACK);
  int rpm = SData.getRPM(freq);     // Get latest Loops reading
  if (rpm <= 4300) {
    dispNum.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  else if (rpm > 4300 && rpm <= 6500) {
    dispNum.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  }
  else if (rpm > 6500) {
    dispNum.setTextColor(TFT_ORANGE, TFT_BLACK);
  }

  dispNum.setTextFont(8);
  dispNum.drawNumber(rpm, 80, 20);
  dispNum.pushSprite(0, 0);
  // draw label on bottom
  showDescription("     RPM");
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

void drawAnalogMeter() {
  // draws the analog meter
  // Meter outline
  Serial.println("Drawing gauge");

  tft.fillRect(0, 0, M_SIZE * 239, M_SIZE * 126, TFT_WHITE);
  tft.fillRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);  // Text colour

  // Loop through the degrees of the meter and draw elements of the meter.
  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -80; i < 81; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
    uint16_t y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
    uint16_t x1 = sx * M_SIZE * 100 + M_SIZE * 120;
    uint16_t y1 = sy * M_SIZE * 100 + M_SIZE * 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (M_SIZE * 100 + tl) + M_SIZE * 120;
    int y2 = sy2 * (M_SIZE * 100 + tl) + M_SIZE * 140;
    int x3 = sx2 * M_SIZE * 100 + M_SIZE * 120;
    int y3 = sy2 * M_SIZE * 100 + M_SIZE * 140;

    // Rich (Red) zone limits
    if (i >= -90 && i < -40) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_RED);
    }

    // Green zone limits
    if (i >= -40 && i < 40) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    //
    // Blue (lean) zone limits
    if (i >= 40 && i < 80) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_BLUE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_BLUE);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
    y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
    x1 = sx * M_SIZE * 100 + M_SIZE * 120;
    y1 = sy * M_SIZE * 100 + M_SIZE * 140;

    // Draw ticks
    if (i >= -90 && i < -40 || i >= 40 && i < 80) {
      tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
    }
    else {
      tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
    }
    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      tft.setTextDatum(TC_DATUM);
      tft.setTextPadding(30);
      // Calculate label positions
      x0 = sx * (M_SIZE * 100 + tl + 10) + M_SIZE * 120;
      y0 = sy * (M_SIZE * 100 + tl + 10) + M_SIZE * 140;
      switch (i / 25) {
        case -3: tft.drawString("11", x0 + 10, y0 - 10, 4); break;
        case -2: tft.drawString("12", x0, y0 - 20, 4); break;
        case -1: tft.drawString("13", x0, y0 - 20, 4); break;
        case 0: tft.drawString("14", x0, y0 - 20, 4); break;
        case 1: tft.drawString("15", x0, y0 - 20, 4); break;
        case 2: tft.drawString("16", x0, y0 - 20, 4); break;
        case 3: tft.drawString("17", x0 - 20 , y0 - 10 , 4); break;
      }
    }
    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * M_SIZE * 100 + M_SIZE * 120;
    y0 = sy * M_SIZE * 100 + M_SIZE * 140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }
  // add stoichiometric indicator
  tft.fillTriangle(205, 20, 215, 23, 200, 50, TFT_GREENYELLOW);
  // Draw bezel line
  tft.drawRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_BLACK);


}
// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(int value, byte ms_delay, float afrVar, float afr, int bottomLeft, int bottomCenter,
                int bottomRight, float afrvarmax, float afrvarmin) {
  static int oldValue =  0; // Value last displayed
  static long timeSinceUpdate = millis();
  static float ltx = 0;    // Saved x coord of bottom of needle
  static int osx = M_SIZE * 120, osy = M_SIZE * 120; // Saved x & y coords

  // check for time since last needle update,
  if ( millis() - timeSinceUpdate > ms_delay) {
    //update
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // convert float to char array
    char buf[8];
    dtostrf(value, 4, 0, buf);
    //    tft.drawRightString(buf, M_SIZE*40, M_SIZE*(119 - 20), 6);

    // Move the needle until new value reached
    while (!(value == oldValue)) {
      if (oldValue < value) {
        oldValue++;
      }
      else oldValue --;

      //if (ms_delay == 0) old_analog = value; // Update immediately if delay is 0

      //float sdeg = map(oldValue, -10, 110, -150, -30); // Map value to angle
      float sdeg = map(oldValue, 0, 100, -165, -15); // Map value to angle
      // Calcualte tip of needle coords
      float sx = cos(sdeg * 0.0174532925);
      float sy = sin(sdeg * 0.0174532925);

      // Calculate x delta of needle start (does not start at pivot point)
      float tx = tan((sdeg + 90) * 0.0174532925);

      // Erase old needle image
      tft.drawLine(M_SIZE * (120 + 20 * ltx - 3), M_SIZE * (140 - 20), osx - 3, osy, TFT_BLACK);
      tft.drawLine(M_SIZE * (120 + 20 * ltx - 2), M_SIZE * (140 - 20), osx - 2, osy, TFT_BLACK);
      tft.drawLine(M_SIZE * (120 + 20 * ltx - 1), M_SIZE * (140 - 20), osx - 1, osy, TFT_BLACK);
      tft.drawLine(M_SIZE * (120 + 20 * ltx), M_SIZE * (140 - 20), osx, osy, TFT_BLACK);
      tft.drawLine(M_SIZE * (120 + 20 * ltx + 1), M_SIZE * (140 - 20), osx + 1, osy, TFT_BLACK);
      tft.drawLine(M_SIZE * (120 + 20 * ltx + 2), M_SIZE * (140 - 20), osx + 2, osy, TFT_BLACK);
      tft.drawLine(M_SIZE * (120 + 20 * ltx + 3), M_SIZE * (140 - 20), osx + 3, osy, TFT_BLACK);

      // Re-plot text under needle
      if (afrVar <= .5 && afrVar >= -.5) {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      else if (afrVar < -.5) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
      }
      else {
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
      }
      tft.setTextPadding(130);
      tft.setTextDatum(TC_DATUM);
      tft.drawFloat(afrVar, 1, M_SIZE * 110, M_SIZE * 65, 7);

      // Store new needle end coords for next erase
      ltx = tx;
      osx = M_SIZE * (sx * 98 + 120);
      osy = M_SIZE * (sy * 98 + 140);

      // Draw the needle in the new postion, magenta makes needle a bit bolder
      // draws 3 lines to thicken needle
      tft.drawLine(M_SIZE * (120 + 20 * ltx - 3), M_SIZE * (140 - 20), osx - 3, osy, TFT_WHITE);
      tft.drawLine(M_SIZE * (120 + 20 * ltx - 2), M_SIZE * (140 - 20), osx - 2, osy, TFT_WHITE);
      tft.drawLine(M_SIZE * (120 + 20 * ltx - 1), M_SIZE * (140 - 20), osx - 1, osy, TFT_WHITE);
      tft.drawLine(M_SIZE * (120 + 20 * ltx), M_SIZE * (140 - 20), osx, osy, TFT_WHITE);
      tft.drawLine(M_SIZE * (120 + 20 * ltx + 1), M_SIZE * (140 - 20), osx + 1, osy, TFT_WHITE);
      tft.drawLine(M_SIZE * (120 + 20 * ltx + 2), M_SIZE * (140 - 20), osx + 2, osy, TFT_WHITE);
      tft.drawLine(M_SIZE * (120 + 20 * ltx + 3), M_SIZE * (140 - 20), osx + 3, osy, TFT_WHITE);
      // Slow needle down slightly as it approaches new postion
      if (abs(oldValue - value) < 10) ms_delay += ms_delay / 5;

      // Show MAP at bottom left
      tft.setTextPadding(100);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextDatum(BL_DATUM);  // set text datum to bottom left, easier to place text at bottom
      tft.drawNumber(bottomLeft, 0, 240, 7);

      // Show GammaE in middle
      tft.setTextPadding(125);
      tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
      tft.setTextDatum(BC_DATUM);
      tft.drawNumber(bottomCenter, 160, 240, 7);

      // show water temperature at right
      tft.setTextPadding(100);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextDatum(BR_DATUM);  // set text datum to bottom left, easier to place text at bottom
      tft.drawNumber(bottomRight, 320, 240, 7);

      // bottom labels
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextPadding(100);
      tft.setTextDatum(BL_DATUM);
      tft.drawString("MAP", 30, 190, 2);
      tft.setTextDatum(BC_DATUM);
      tft.drawString("Throttle", 160, 190, 2);
      tft.setTextDatum(BR_DATUM);
      tft.drawString("Water", 290, 190, 2);


      if (testMode == true) {
        //tft.setTextPadding(140);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("TEST MODE", 0, 0, 4);
      }
    }
  }
}
