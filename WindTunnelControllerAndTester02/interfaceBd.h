/* Control Board (pots) code, for reading 8 pots and 4 momentary switches
   on an interface board attached to the Arduino mega
*/
#include <Adafruit_MCP3008.h>
Adafruit_MCP3008 adc;

int pots[8];
int momSw; //variable for momentary switches

void initInterfaceBd() {  // this is the control board with the pots
  Serial.begin(57600);
  while (!Serial);

  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  // (sck, mosi, miso, cs);
  adc.begin(52, 51, 50, 53); //Mega SPI pins
  Serial.println( "int interface");
 
}

void readInterfaceBd() {
  /** reads teh interface board for pots and switches  **/

  static unsigned long lastTime;
    
  if (millis() - lastTime > 200 ) { // 5 times a sec
    lastTime = millis();

    for (int chan = 0; chan < 8; chan++) {
      pots[chan] = adc.readADC(chan);
    }

    momSw = 0;
    for (int pin = 9; pin < 13; pin++) {  // read 4 switches and encode binary
      momSw = (momSw * 2) + !digitalRead(pin); // switches active low state
    }

/*

    for (int chan = 0; chan < 8; chan++) {
      Serial.print(pots[chan]);
      Serial.print("\t");
    }


    for (int chan = 0; chan < 4; chan++) {
      Serial.print(momSw);
      Serial.print("\t");
    }
    Serial.println("");
*/

  }
}
