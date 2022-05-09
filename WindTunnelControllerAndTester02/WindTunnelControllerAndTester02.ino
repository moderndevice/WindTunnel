/* WindTunnel Controller and Tester
    Functions:
    Read Wind Sensor P or C need to switch wiring)
    Read Pitot tube
    Read Temperature Sensors two places
    Control Fan (PWM)
    Perform Test Routines - Cycle Fan and Temperature and Report Data via serial to spreadsheet
    Report Wind Tunnel Conditions on LCD or OLED display
    Test Sensors manually - switch for P & C  OR
    Run temp profiles automatically with serial output
    Git test
*/

int CPflag; // 0 = rev C

#define REVCFLAG 0
#define REVPFLAG 1

// print directives
//#define PIDprint  1          // PID for heater            git test - delete this!
 #define SensorTestPrint  // test sensor setup

#include "Wire.h"
//#include <LibTempTMP421.h>
#include "Adafruit_MCP9808.h"
#include <PID_v1.h>
#include "modeFunctions.h"
#include "interfaceBd.h"
#include "utility.h"
#include "tempSensors_heaterPID.h"
#include "readPitot.h"
#include "fan.h"
#include "display.h"




/* stupid function prototypes */
void readTempC();

int mode = 0, lastMode = 20, newMode = 1; // state variable

void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial.println(" sketch begin");
  initInterfaceBd();
  initHeaterPID();
  initLCD2();
  initFan();
  initLCD_display();
  newMode = 1; // triggers more printing
  printTempDiplay();
  newMode = 1; // triggers more printing
  printTempDiplay();
}

void loop() {

  readTempC();
  readInterfaceBd();
  doHeaterLoop();    // heater control
  doTestChamberLoop(); // offset control for temp offset
  doSwitchDecode();
  doLCD2();
  doFan();
  readPitot();
  readWindC();
  printTempDiplay();

    switch (mode) {
      case 0:  // Rev C
    //      testSensors();
    //      CPflag = REVCFLAG;
        break;

      case 1:  // Rev P
        testSensors();
        CPflag = REVPFLAG;
        break;

      case 2:  // C run curves
        runCcurves();
        break;

      case 3:  // P run curves
        runPcurves();
        break;
    } 
}


void doSwitchDecode() {
  static unsigned long lastStateRead;
  if (millis() - lastStateRead > 60) {
    lastStateRead = millis();

    if (momSw != 0) {  // button pressed
      switch (momSw) {

        case 4: // Rev C
          mode = 0;
          CPflag = REVCFLAG;
          break;

        case 2: // Rev P
          mode = 1;
          CPflag = REVPFLAG;
          break;

        case 1: // run C curves
          mode = 2;
          break;

        case 8: // run P curves
          mode = 3;
          break;
      }

      if (mode != lastMode) {
        lastMode = mode;
        newMode = 1;
      }
    }
  }
}
