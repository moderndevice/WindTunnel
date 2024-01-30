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

// define Serial1 as LCD to make coding more clear
int CPflag; // 0 = rev C

#define REVCFLAG 0
#define REVPFLAG 1

// print directives
//#define PIDprint  1          // PID for heater    git test - delete this!
//#define SensorTestPrint  // test sensor setup
//#define TempSensorTestprint
//#define curvePdebug
#define sendDataToProcessing

#include "fan.h"
#include "Wire.h"
#include "switchDecode_controlModes.h"
//#include <LibTempTMP421.h>
#include "Adafruit_MCP9808.h"
#include "Adafruit_BMP3XX.h"
#include <PID_v1.h>
#include "modeFunctions.h"
#include "interfaceBd.h"
#include "utility.h"
#include "tempSensors_heaterPID.h"
#include "readPitot.h"
#include "display.h"
#include "barometer.h"
#include "writeSerialData.h"
#include "runCurves.h"

float barometer;

/* stupid function prototypes */
void readTempC();
void doFan(int fanPWM);
void manualTest(int cpFlag);

//int controlMode = 0, lastMode = 20, newMode = 1; // state variable

void setup() {
  initLCD_display();
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
  setupBarometer();
  // for the LED panel at top of controls
  pinMode(LED_WHITE_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(startSwitchPin, INPUT_PULLUP);
  pinMode(resetSwitchPin, INPUT_PULLUP);
}

void loop() {
  readTempC();  // errant print not here
  readInterfaceBd();   // errant print not here
  doHeaterLoop();      // errant print not here
  doTestChamberLoop(); // offset control for temp offset // errant print not here
  doSwitchDecode();  // errant print not here
  //doLCD2();
  doFan(fanPWM); // errant print not here
  readPitot(); // errant print not here
 // printTempDiplay(); // errant print not here
  getBarometer();  // errant print not here
}
