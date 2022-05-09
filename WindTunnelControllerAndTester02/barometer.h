Adafruit_BMP3XX bmp;
extern float barometer;
void setupBarometer() {

  if (!bmp.begin_I2C()) {   // hardware I2C mode, can pass in address & alt Wire
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    delay(6000);
  }
  else {

    // Set up oversampling and filter initialization
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);
    Serial.println("Found BMP388 pressure sensor - initiatialized");;
    delay(1000);
  }
}

void getBarometer() {
  static unsigned long lastTime;

  if (millis() - lastTime > 1500) {
    lastTime = millis();
    if (! bmp.performReading()) {
      Serial.println("BMP388 reading failed ");
      delay(1000);
      return;
    }

    barometer = bmp.pressure / 100.0;
    Serial.print("barometer "); Serial.println(barometer);
  }
}
