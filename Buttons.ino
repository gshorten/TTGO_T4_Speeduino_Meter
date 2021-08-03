/*
   Functions to change the display
*/

void handleTopButton() {
  // depending on current mode changes the frequency of data updates.
  // cycles through from 50 to 500 ms in 50 ms increments

  switch (g_Mode) {
    case MODE_MULTI:
      multiFreq = incrementFreq(multiFreq);
      break;
    case MODE_AFR:
      afrFreq = incrementFreq(afrFreq);
      break;
    case MODE_WARMUP:
      warmupFreq = incrementFreq(warmupFreq);
      break;
    case MODE_GAMMA:
      gammaFreq = incrementFreq(gammaFreq);
      break;
    case MODE_MAP:
      mapFreq = incrementFreq(mapFreq);
      break;
    case MODE_ACCEL:
      accelFreq = incrementFreq(accelFreq);
      break;
    case MODE_RPM:
      rpmFreq = incrementFreq(rpmFreq);
      break;
  }
}

int incrementFreq(int freq) {
  int increment = 50;
  freq += 50;
  if (freq > 500) {
    freq = 100;  // roll around
  }
  return freq;
}

void handleBottomButton() {
  // cycle through modes when down button is pressed.  This changes the display shown
  Serial.print("Bottom Button pressed");
  g_Mode ++;       // button push increments mode variable.
  // blank screen, gets rid of old display when switching
  if (g_Mode > NUM_MODES - 1 ) {
    g_Mode = 0;   // wrap around
  }
  tft.fillScreen(TFT_BLACK);
  descText.fillSprite(TFT_BLACK);

  if (g_Mode == MODE_MULTI) {
    // if switching to mult meter then draw the gauge, we only want to do this once
    Serial.println("switching to multimeter");
    drawAnalogMeter();
  }
}

void handleMiddleButton() {
  // toggles test mode on and off
  if (testMode == false) {
    testMode = true;
    SData.testModeOn();
  }
  else if (testMode == true) {
    SData.testModeOff();
    testMode = false;
    if (g_Mode == MODE_MULTI) {
      tft.fillScreen(TFT_BLACK);
      drawAnalogMeter();
    }
  }
}
