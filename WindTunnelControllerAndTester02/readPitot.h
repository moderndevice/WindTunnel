#define PITOTPIN A15
#define PITOTREADS 50
#define PITOTREADDELAY 200 // delay between readings
#define PITOTSMOOTHFACTOR 0.95
float pitotOut, smoothPitot, lastSmoothPitot;
int pitotDeltaVal, validData;
unsigned long lastPitotSmoothReset;
extern unsigned int tach;
extern volatile unsigned long  fanTach;

void reportPitot() {  // for debugging - comment this out
  Serial.print("pitot ");        Serial.print(pitotOut);
  Serial.print("  tach ");       Serial.print(tach);
  float ratioTach = pitotOut / (float)tach;
  Serial.print(" ratio ");       Serial.print(ratioTach);
  Serial.print("  delta ");      Serial.print(pitotDeltaVal);
  Serial.print("  smthPitot ");  Serial.print(smoothPitot);
  Serial.print("  validData ");    Serial.println(validData);
}

void readPitot() {
  static unsigned long lastTime, lastTimeTach;
  static int longWaitDelay = 1;
  static long pitotTotal;
  static int pitotReads = 0,  pitotMaxVal = 0, pitotMinVal = 32000;


  if (longWaitDelay) {  // reading is reset - wait longer
    if (millis() - lastTime > PITOTREADDELAY) {
      lastTime = millis();
      longWaitDelay = 0;
    }
  }
  else { // space readings out 5 mS - read Pitot PITOTREADS times
    if (millis() - lastTime > 3) {
      lastTime = millis();
      int pitotTempADC = analogRead(PITOTPIN);
      // Serial.println(pitotTempADC);
      pitotTotal += pitotTempADC;
      pitotReads++;
      if (pitotTempADC > pitotMaxVal) pitotMaxVal = pitotTempADC;
      if (pitotTempADC < pitotMinVal) pitotMinVal = pitotTempADC;

      if (pitotReads > PITOTREADS - 2) { // reset and report

        /* read the tach in synch with the pitot read ??
          but we are reading the tach elsewhere so this is a problem */
        //long tempTach = fanTach;
        //fanTach = 0;

        float period_S = (float)(millis() - lastTimeTach) / 1000.0; // convert to Secs
        lastTimeTach = millis();
        pitotOut = (float)pitotTotal / (float)pitotReads;

        pitotDeltaVal = pitotMaxVal - pitotMinVal;
        smoothPitot = smooth(pitotOut, PITOTSMOOTHFACTOR, smoothPitot);
       
        /* compare data and test how long it has been stable */
        if (abs(smoothPitot - lastSmoothPitot) > 1.0) {
          lastPitotSmoothReset = millis();
          lastSmoothPitot = smoothPitot;
        }
        if (millis() - lastPitotSmoothReset > 5000) {
          validData = 1;
        } else validData = 0;



       // reportPitot(); // debug  ????? comment out
        pitotReads = 0;
        pitotTotal = 0;
        pitotMaxVal = 0;
        pitotMinVal = 32000;
        longWaitDelay = 1;
      }
    }
  }
}
