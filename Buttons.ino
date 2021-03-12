/*
   Functions to change the display
*/

void handleTopButton() {
  // depending on current mode changes the frequency of data updates.
  // cycles through from 50 to 500 ms in 50 ms increments

  switch (g_Mode) {
    case MODE_AFR:
      afrFreq = incrementFreq(afrFreq);
      break;
    case MODE_EGO:
      egoFreq = incrementFreq(egoFreq);
      break;
    case MODE_LOOPS:
      loopsFreq = incrementFreq(loopsFreq);
      break;
    case MODE_WARMUP:
      warmupFreq = incrementFreq(warmupFreq);
      break;
    case MODE_GAMMA:
      gammaFreq = incrementFreq(gammaFreq);
      break;
    case MODE_MAP:
      mapFreq = incrementFreq(mapFreq);
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
  // blank screen, gets rid of old display when switching
  tft.fillScreen(TFT_BLACK);
  descText.fillSprite(TFT_BLACK);
  g_Mode ++;       // button push increments mode variable.
  if (g_Mode > NUM_MODES-1) {
    g_Mode = 0;   // wrap around
  }
}
