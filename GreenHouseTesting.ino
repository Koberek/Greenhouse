
//************************************************************************************************************
// This TESTING ground is for program developement... Copying the most recent fully functional code and adding to it here
// instead of breaking the perfectly good code already saved.

//************************************************************************************************************
// PINS>
// 3,4    Heater (parallel circuit)
// 6      Vent
// A0-A4  Water pots 1-5
// A5     RUN LED


//*************************************************************************************************************
// CURRENT work>>


// NTP request data packet looks like this
// E3 00 06 EC 00 00 00 00 00 00 00 00 31 4E 31 34 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

// NTP reply packet. 
// 1C 01 0D E3 00 00 00 10 00 00 00 20 4E 49 53 54 E2 20 A8 E4 00 00 00 00 00 00 00 00 00 00 00 00 E2 20 A9 40 CE 24 FD 69 E2 20 A9 40 CE 25 11 83 



//************************************************************************************************************
// DESIRED function

//  Each Pot to have its own cycle
//  waterPots() should adjust watering based on tent and external temperatures. Hotter = more water ??
//  need to add an override watering switch to manually water/soak

//************************************************************************************************************
// IMPORTANT NOTES

// SAMD5x can sink/sourse 8mA
// Temp probes
//    probe1-5 for the pots
//    probe6 for purge water temp
//    probe7 for houseTemp
//    probe8 for outsideTemp
//    probe9 for testing or additional probe

// THESE pins can't be used... 1,2,5,8,10,11 and 13. They are used somewhere else in the included libraries
// Pin 13 can be used for indication that the program is running. CANNOT be use to control devices.
// Only have 6 digital pins available ... 3,4,6,7,9 and 12
// Using analog pins A0-A4 for "waterPot1-Pot5"

// 10 second interval to get NTP
// NOTE... NTP response is NOT 100%

//************************************************************************************************************


#include <OneWire.h>
#include <DallasTemperature.h>
//#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>


#define ONE_WIRE_BUS 2                 // OneWire connected to pin2 on Uno
#define TEMPERATURE_PRECISION 12       //Set precision of the 18B20's

#define pot1pin   A0                   // analog pin A0
#define pot2pin   A1
#define pot3pin   A2
#define pot4pin   A3
#define pot5pin   A4
#define runPin    A5
#define ventPin   6
#define heatPin1  3
#define heatPin2  4
#define LEDpin    13

#define OFF HIGH                          // Active LOW inputs on the external relay board
#define ON  LOW                           // Active LOW inputs on the external relay board

// for the timer and time
#define NTP   0x00
unsigned long NTP_int   = 10000;             // 10 sec. DO NOT call this function at > 1 call/3 sec or faster. Will get banned from site
#define PROBE 0x01
unsigned long PROBE_int = 60000;            // 1 minute read temp interval
#define PRINT 0x02
unsigned long PRINT_int = 2000;            // 2 sec print interval
#define WATER 0x03
unsigned long WATER_int = 120000;           // 2 minute watering timer. Water ON for 2 minutes
#define RUNNING   0x04
unsigned long RUNNING_int   = 250;              // LED toggle only indicates prgram running

// init the timers                       
unsigned long NTP_lastRead_millis;        // These are used in timer_lapsed() function
unsigned long PROBE_lastRead_millis;
unsigned long PRINT_lastRead_millis;
unsigned long WATER_lastRead_millis;
unsigned long RUNNING_lastRead_millis;

// Variables to hold current time from decodeTime()
int UTC_hours   = 25;                    // init to 25 to prevent actions until a valid NTP time is received. 25 is invalid time.
int UTC_minutes = 65;                    // same as above
int UTC_seconds = 0;                     //


int waterSchedule[]   {6, 14};           // 24 hour clock. 6am and 2pm. Minutes are always 0 in waterPots()
int firstWatering   = 0;                 // index to waterSchedule[]
int secondWatering  = 1;
bool wateringON     = false;             // true if watering is active... i.e. water valves are open
bool waterON        = false;             // true if it is time to water

bool  crcFAIL = false;                  // CRC used in serial communication with RPi.  NOT USED currently


const int houseHeatOnTemp   = 40;
const int houseHeatOffTemp  = 45;
const int houseVentOffTemp  = 80;
const int houseVentOnTemp   = 85;
const int houseWARNTemp     = 90;

// memory for communication from RPi
uint8_t   RPirecBlock[4];                               // Data block received from RPi
uint8_t   testBlock[4] {0xAA, 0x00, 0x01, 0xC0};        // for testing only. Remove all references when finished
uint8_t   RPirecCRC;                                    // CRC included with received data block (from RPi)
uint8_t   calcCRC;                                      // calculated CRC8 of the data received from RPi

// Temperature sensors
OneWire oneWire(ONE_WIRE_BUS);            // create OneWire instance on pin2
DallasTemperature sensors(&oneWire);      // pass onewire instance to Dallas

// These are the ID's of the 5-probe temperature probe assembly (pre-made assembly  fixed addresses) and 4 extra probes
DeviceAddress probe1 = { 0x28, 0xFF, 0xC3, 0x0D, 0x81, 0x14, 0x02, 0x1B };
DeviceAddress probe2 = { 0x28, 0xFF, 0x7D, 0x65, 0x81, 0x14, 0x02, 0x7A };
DeviceAddress probe3 = { 0x28, 0xFF, 0xD2, 0x5C, 0x30, 0x17, 0x04, 0x62 };
DeviceAddress probe4 = { 0x28, 0xFF, 0xDB, 0x57, 0x81, 0x14, 0x02, 0x9A };
DeviceAddress probe5 = { 0x28, 0xFF, 0x7B, 0xF6, 0x80, 0x14, 0x02, 0x24 };
// 4 additional temperature probes
DeviceAddress probe6 = { 0x28, 0xAA, 0x4B, 0x76, 0x53, 0x14, 0x01, 0xA3 };    // purge water temp
DeviceAddress probe7 = { 0x28, 0xAA, 0x66, 0x74, 0x53, 0x14, 0x01, 0xB0 };    // greenhouse temp
DeviceAddress probe8 = { 0x28, 0xAA, 0xEA, 0x79, 0x53, 0x14, 0x01, 0xC7 };    // outside air temp
DeviceAddress probe9 = { 0x28, 0xAA, 0x28, 0x7B, 0x53, 0x14, 0x01, 0x61 };    // extra probe

uint8_t*    probeAddr[]  {probe1, probe2, probe3, probe4, probe5, probe6, probe7, probe8, probe9};     // indexes to each of the 8 temp probes
bool        probeAvail[] {probe1, probe2, probe3, probe4, probe5, probe6, probe7, probe8, probe9};     // available YES NO
int         greenHouseTemperatures [9];  // values from each of the 9 indexed probes (probe1...etc)


// WiFi initialization
int status = WL_IDLE_STATUS;
#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // 2390 local port to listen for UDP packets

IPAddress testServer(192, 168, 1, 2); // Send NTP req packet to PC for testing. Changed from time.nist.gov NTP server (129, 6, 15, 28)
IPAddress timeServer(129, 6, 15, 28);   // NTP timer server address

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  pinMode(pot1pin, OUTPUT);                   // pin A0 as GPIO OUTPUT
  digitalWrite(pot1pin, OFF);
  pinMode(pot2pin, OUTPUT);                   // pin A1
  digitalWrite(pot2pin, OFF);
  pinMode(pot3pin, OUTPUT);                   // pin A2
  digitalWrite(pot3pin, OFF);
  pinMode(pot4pin, OUTPUT);                   // pin A3
  digitalWrite(pot4pin, OFF);
  pinMode(pot5pin, OUTPUT);                   // pin A4
  digitalWrite(pot5pin, OFF);
  pinMode(runPin, OUTPUT);                    // Pin to indicate that the prgram is running
  digitalWrite(runPin, LOW);
  pinMode(ventPin, OUTPUT);                   // pin 6
  digitalWrite(ventPin, OFF);
  pinMode(heatPin1, OUTPUT);                 // pin 3
  digitalWrite(heatPin1, OFF);
  pinMode(heatPin2, OUTPUT);                 // pin 4
  digitalWrite(heatPin2, OFF);
  pinMode(LEDpin, OUTPUT);                    // pin 13
  digitalWrite(LEDpin, LOW);

  sensors.begin();    // Start Dallas 18B20 on oneWire

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // HALT don't continue
    digitalWrite(LEDpin, ON);                 // Solid LED13 means FAIL
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);

  // init the timers. used to schedule function calls at interval
  // should be a better way to init the timers
  NTP_lastRead_millis         = millis();
  PROBE_lastRead_millis       = millis();
  PRINT_lastRead_millis       = millis();
  WATER_lastRead_millis       = millis();
  RUNNING_lastRead_millis     = millis();

  // get first NTP packet before loop(). 
  getNTPtime();

  // Start first temperature conversion
  getTempsF();        // first call to get temperatures. Using this instead of delay(1000) since the getTempsF() takes ~ 1 sec.
}

void loop() {

  // TOGGLE the runn state LED
  if (timer_lapsed(RUNNING) == true){
    digitalWrite(LEDpin, !digitalRead(LEDpin));
  }
  
  // get NTP time every 10 seconds
  if (timer_lapsed(NTP) == true) {    // get NTP time every NTP_int. Make sure to NOT send NTP requests too fast
    //digitalWrite(runPin, HIGH);
    decodeTime();                       // check for NTP packet. IF  received then decode time
    getNTPtime();                       // send request to get new NTP packet
    //digitalWrite(runPin, LOW);
}
  
  waterPots();                        // checks time decoded by decodeTime() and waters pot if.......

  // get probe temps every 1 secconds
  if (timer_lapsed(PROBE) == true) {  // read temps every PROBE_int
    getTempsF();                      // This function take a LOT of time.. 830ms !
  }
  
  controlHouseVent();                 //  Vent if house too hot
  controlHouseHeater();               //  Heat if too cold
 
  if (timer_lapsed(PRINT) == true) {  // print Time and Temp data to Serial
    printData();                    // This function hangs if
  }

}
