

// Check the presence of 9 probes from setup()
void checkSensorPresence(void){
  for (int i=0; i<9; i++){
    if (sensors.isConnected(probeAddr[i])){
      probeAvail[i] = true;
    }
    else probeAvail[i] = false;
    }
}

// Read data block from RPi and verify CRC8
// If the CRC8 fails set crcFAIL to TRUE

void receiveRPiData(void){
  
  int inCount = Serial.available();
  
  if (inCount < 6) {
    return;
    }     //return if less than 6 (plus CR?) bytes are avaialable
  
  if (inCount > 6) { 
    Serial.println("Input Buffer Overflow in function receiveRPiData()");
    // flush the input buffer
    while (Serial.available()) {
      Serial.read();
    }
    return; // if 
  }
  
  if (inCount == 6){    // read and store the buffer contents
      for (int i=0; i < 6; i++){
      RPirecBlock[i]=Serial.read();
      }
  }
      
  if ((RPirecBlock[0] == 72) && (RPirecBlock[2] == 77) && (RPirecBlock[4] == 83)) {
    // IF Data received conforms to format HxMxSx... or 72-"H", 77="M" and 83="S"
    // Is a verification of the received data
    // DATA block received verified. Set Time.
    UTC_hours   = RPirecBlock[1];
    UTC_minutes = RPirecBlock[3];
    UTC_seconds = RPirecBlock[5];      
  }
}



void getTempsF(void){

  sensors.requestTemperatures();            // required before .getTempX()
  for (int i=0; i<5; i++){                  // read all 5 temp sensors. Order of reading is fixed by DallasTemperature so be aware
  float temp = sensors.getTempF(probeAddr[i]);
  greenHouseTemperatures[i] = (int) temp;   // convert from float to int and store to greenHouseTemperature[]
  }

}

void recordMinMax(void){
  int temp;                                 // temperature data

  if ((UTC_hours == 0) && (UTC_minutes == 0)){
    //reset the MinMax temps at midnight
    for (int i=0; i <5; i++){
      greenhouseMaxTemp[i] = 0;
      greenhouseMinTemp[i] = 100;
    }
    return;
  }

  for (int i=0; i<5; i++){
    temp = greenHouseTemperatures[i];    // index to array
    if (temp >= greenhouseMaxTemp[i]){
      greenhouseMaxTemp[i] = temp;
    }
    if (temp <= greenhouseMinTemp[i]){
      greenhouseMinTemp[i] = temp;
    }
    }
}

void controlHouseVent(void){
  int houseTemp   = greenHouseTemperatures[0];    // probe1
  if (houseTemp >= houseVentOnTemp){
    digitalWrite(heatPin1, OFF);
    digitalWrite(heatPin2, OFF);
    digitalWrite(ventPin, ON);
    ventON = true;
  }
  if (houseTemp <= houseVentOffTemp){
    digitalWrite(ventPin, OFF);
    ventON = false;
  }
}

void controlHouseHeater(void){
  int houseTemp   = greenHouseTemperatures[0];
  if (houseTemp <= houseHeatOnTemp){
    digitalWrite(ventPin, OFF);
    digitalWrite(heatPin1, ON);
    digitalWrite(heatPin2, ON);
    heaterON = true;
  }
  if (houseTemp >= houseHeatOffTemp){
    digitalWrite(heatPin1, OFF);
    digitalWrite(heatPin2, OFF);
    heaterON = false;
  }
}


void waterPots(void){
  bool endwatering, endinhibit;                                     // endwatering true when watering time/duration has expired
                                   
    // ZONE 1 watering control
    if ((waterZone1ON == false) && (waterZone1Inhibit == false)){   // waterON = false if it's not time to water
      if (((UTC_hours == waterScheduleZone1[0]) && (UTC_minutes == waterScheduleZone1[1])) || 
         ((UTC_hours == waterScheduleZone1[2]) && (UTC_minutes == waterScheduleZone1[3]))) {
        waterZone1ON = true;                                        // waterON = time to water. Doesn't mean that watering is active
        waterZone1Inhibit = true;
        INHIBIT_Zone1_lastRead_millis = millis();                   // start the inhibit timer. Prevent early restart of watering time
      }
    }
    if ((waterZone1ON == true) && (wateringZone1ON == false)){      //waterON if inside watering window and wateringON is not active
        wateringZone1ON = true;                                     // wateringON is true when  watering is active
          digitalWrite(zone1WaterPin, ON);                          // turn Zone1 watering valve ON
        WATER_Zone1_lastRead_millis = millis();                     // init the wateringZone1 timer start value
    }
    // Check if Zone1 watering timer has elapsed
    if ((waterZone1ON == true) && (wateringZone1ON == true)){       // if Zone1 is already watering (valves ON)
      endwatering = timer_lapsed(WATER_Zone1);                      // endwatering true when watering time has expired
      if (endwatering == true){
        waterZone1ON = false;                                       // reset the waterON to false
        wateringZone1ON = false;                                    // turn off Zone1 water valve
          digitalWrite(zone1WaterPin, OFF);
      }
    }
    // Check if Zone1 watering inhibit timer has elapsed
    if (waterZone1Inhibit == true){          
      endinhibit = timer_lapsed(INHIBIT_Zone1);                     // endwatering true when watering time has expired
      if (endinhibit == true){
        waterZone1Inhibit = false;                                  // reset the waterON to false
      }
    }

    // ZONE 2 watering control
    if ((waterZone2ON == false) && (waterZone2Inhibit == false)){   // waterON = false if it's not time to water
      if ((UTC_hours == waterScheduleZone2[0]) && (UTC_minutes == waterScheduleZone2[1])){
        waterZone2ON = true;                                        // waterON = time to water. Doesn't mean that watering is active
        waterZone2Inhibit = true;
        INHIBIT_Zone2_lastRead_millis = millis();                   // start the inhibit timer. Prevent early restart of watering time
      }
    }
    if ((waterZone2ON == true) && (wateringZone2ON == false)){      //waterON if inside watering window and wateringON is not active
        wateringZone2ON = true;                                     // wateringON is true when  watering is active
          digitalWrite(zone2WaterPin, ON);                          // turn Zone1 watering valve ON
        WATER_Zone2_lastRead_millis = millis();                     // init the wateringZone1 timer start value
    }
    // Check if Zone2 watering timer has elapsed
    if ((waterZone2ON == true) && (wateringZone2ON == true)){       // if Zone1 is already watering (valves ON)
      endwatering = timer_lapsed(WATER_Zone2);                      // endwatering true when watering time has expired
      if (endwatering == true){
        waterZone2ON = false;                                       // reset the waterON to false
        wateringZone2ON = false;                                    // turn off Zone1 water valve
          digitalWrite(zone2WaterPin, OFF);
      }
    }
    // Check if Zone2 watering inhibit timer has elapsed
    if (waterZone2Inhibit == true){          
      endinhibit = timer_lapsed(INHIBIT_Zone2);                     // endwatering true when watering time has expired
      if (endinhibit == true){
        waterZone2Inhibit = false;                                  // reset the waterON to false
      }
    }

    // ZONE 3 watering control
    if ((waterZone3ON == false) && (waterZone3Inhibit == false)){   // waterON = false if it's not time to water
      if ((UTC_hours == waterScheduleZone3[0]) && (UTC_minutes == waterScheduleZone3[1])){
        waterZone3ON = true;                                        // waterON = time to water. Doesn't mean that watering is active
        waterZone3Inhibit = true;
        INHIBIT_Zone3_lastRead_millis = millis();                   // start the inhibit timer. Prevent early restart of watering time
      }
    }
    if ((waterZone3ON == true) && (wateringZone3ON == false)){      //waterON if inside watering window and wateringON is not active
        wateringZone3ON = true;                                     // wateringON is true when  watering is active
          digitalWrite(zone3WaterPin, ON);                          // turn Zone1 watering valve ON
        WATER_Zone3_lastRead_millis = millis();                     // init the wateringZone1 timer start value
    }
    // Check if Zone3 watering timer has elapsed
    if ((waterZone3ON == true) && (wateringZone3ON == true)){       // if Zone1 is already watering (valves ON)
      endwatering = timer_lapsed(WATER_Zone3);                      // endwatering true when watering time has expired
      if (endwatering == true){
        waterZone3ON = false;                                       // reset the waterON to false
        wateringZone3ON = false;                                    // turn off Zone1 water valve
          digitalWrite(zone3WaterPin, OFF);
      }
    }
    // Check if Zone3 watering inhibit timer has elapsed
    if (waterZone3Inhibit == true){          
      endinhibit = timer_lapsed(INHIBIT_Zone3);                     // endwatering true when watering time has expired
      if (endinhibit == true){
        waterZone3Inhibit = false;                                  // reset the waterON to false
      }
    }

    // ZONE 4 watering control
    if ((waterZone4ON == false) && (waterZone4Inhibit == false)){   // waterON = false if it's not time to water
      if ((UTC_hours == waterScheduleZone4[0]) && (UTC_minutes == waterScheduleZone4[1])){
        waterZone4ON = true;                                        // waterON = time to water. Doesn't mean that watering is active
        waterZone4Inhibit = true;
        INHIBIT_Zone4_lastRead_millis = millis();                   // start the inhibit timer. Prevent early restart of watering time
      }
    }
    if ((waterZone4ON == true) && (wateringZone4ON == false)){      //waterON if inside watering window and wateringON is not active
        wateringZone4ON = true;                                     // wateringON is true when  watering is active
          digitalWrite(zone4WaterPin, ON);                          // turn Zone1 watering valve ON
        WATER_Zone4_lastRead_millis = millis();                     // init the wateringZone1 timer start value
    }
    // Check if Zone4 watering timer has elapsed
    if ((waterZone4ON == true) && (wateringZone4ON == true)){       // if Zone1 is already watering (valves ON)
      endwatering = timer_lapsed(WATER_Zone4);                      // endwatering true when watering time has expired
      if (endwatering == true){
        waterZone4ON = false;                                       // reset the waterON to false
        wateringZone4ON = false;                                    // turn off Zone1 water valve
          digitalWrite(zone4WaterPin, OFF);
      }
    }
    // Check if Zone4 watering inhibit timer has elapsed
    if (waterZone4Inhibit == true){          
      endinhibit = timer_lapsed(INHIBIT_Zone4);                     // endwatering true when watering time has expired
      if (endinhibit == true){
        waterZone4Inhibit = false;                                  // reset the waterON to false
      }
    }

    // ZONE 5 watering control
    if ((waterZone5ON == false) && (waterZone5Inhibit == false)){   // waterON = false if it's not time to water
      if ((UTC_hours == waterScheduleZone5[0]) && (UTC_minutes == waterScheduleZone5[1])){
        waterZone5ON = true;                                        // waterON = time to water. Doesn't mean that watering is active
        waterZone5Inhibit = true;
        INHIBIT_Zone5_lastRead_millis = millis();                   // start the inhibit timer. Prevent early restart of watering time
      }
    }
    if ((waterZone5ON == true) && (wateringZone5ON == false)){      //waterON if inside watering window and wateringON is not active
        wateringZone5ON = true;                                     // wateringON is true when  watering is active
          digitalWrite(zone5WaterPin, ON);                          // turn Zone1 watering valve ON
        WATER_Zone5_lastRead_millis = millis();                     // init the wateringZone1 timer start value
    }
    // Check if Zone5 watering timer has elapsed
    if ((waterZone5ON == true) && (wateringZone5ON == true)){       // if Zone1 is already watering (valves ON)
      endwatering = timer_lapsed(WATER_Zone5);                      // endwatering true when watering time has expired
      if (endwatering == true){
        waterZone5ON = false;                                       // reset the waterON to false
        wateringZone5ON = false;                                    // turn off Zone1 water valve
          digitalWrite(zone5WaterPin, OFF);
      }
    }
    // Check if Zone5 watering inhibit timer has elapsed
    if (waterZone5Inhibit == true){          
      endinhibit = timer_lapsed(INHIBIT_Zone5);                     // endwatering true when watering time has expired
      if (endinhibit == true){
        waterZone5Inhibit = false;                                  // reset the waterON to false
      }
    }
}

void printData(void){
  Serial.print("TIME:  ");
  Serial.print(UTC_hours);
  Serial.print(':');
  if (UTC_minutes < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
  Serial.println(UTC_minutes);

  if (wateringZone1ON == true){
    Serial.println("Watering is ON");
  }
  else Serial.println("Watering is OFF");

  if (ventON == true){
    Serial.println("Venting  is ON");  
  }
  else Serial.println("Venting  is OFF");

  if (heaterON == true) {
    Serial.println("Heater   is ON");
    }
  else Serial.println("Heater   is OFF");

  Serial.print("Temperature   ");  
  Serial.println("Probe1 Probe2 Probe3 Probe4 Probe5 ");

    
  // print the current probe temperatures
  if (greenHouseTemperatures[0] > 99){
    Serial.print("             ");              // keeps characters aligned in case temps go to 3 digits
  }
  else {
    Serial.print("              ");
  }
  for (int i=0; i<5; i++){
    Serial.print(greenHouseTemperatures[i]);
    Serial.print("     ");
  }
  
  Serial.println();
  // print the current MAX temps
  Serial.print("MAX temp=");
  if (greenhouseMaxTemp[0] > 99){
    Serial.print("    ");                         // keeps characters aligned in case temps go to 3 digits
  }
  else {
    Serial.print("     ");
  }
  for (int i=0; i<5; i++){
    Serial.print(greenhouseMaxTemp[i]);
    Serial.print("     ");
  }

  Serial.println();
  // print the current MIN temps
  Serial.print("MIN temp=");
  Serial.print("     ");
  for (int i=0; i<5; i++){
    Serial.print(greenhouseMinTemp[i]);
    Serial.print("     ");
  }

  Serial.println();
  Serial.println();
}


// These are the system timers. Using millis() function. Each timer has a NAME_int to define the length of time in ms.
// if current time - start time >= whatever_int then {}
// 
bool timer_lapsed(uint8_t PID){                             // timer. used for short interval scheduling 1sec- a few minutes

  if (PID == PROBE){
    if ((millis() - PROBE_lastRead_millis) >= PROBE_int){    // set to 10 sec
      PROBE_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }

  if (PID == WATER_Zone1){
    if ((millis() - WATER_Zone1_lastRead_millis) >= WATER_Zone1_int){   // set to 2minutes
      //WATER_Zone1_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }
    
  if (PID == WATER_Zone2){
    if ((millis() - WATER_Zone2_lastRead_millis) >= WATER_Zone2_int){   // set to 2minutes
      //WATER_Zone2_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == WATER_Zone3){
    if ((millis() - WATER_Zone3_lastRead_millis) >= WATER_Zone3_int){   // set to 2minutes
      //WATER_Zone3_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == WATER_Zone4){
    if ((millis() - WATER_Zone4_lastRead_millis) >= WATER_Zone4_int){   // set to 2minutes
      //WATER_Zone4_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == WATER_Zone5){
    if ((millis() - WATER_Zone5_lastRead_millis) >= WATER_Zone5_int){   // set to 2minutes
      //WATER_Zone5_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == INHIBIT_Zone1){
    if ((millis() - INHIBIT_Zone1_lastRead_millis) >= INHIBIT_Zone1_int){    // set to 60sec
      //INHIBIT_Zone1_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == INHIBIT_Zone2){
    if ((millis() - INHIBIT_Zone2_lastRead_millis) >= INHIBIT_Zone2_int){    // set to 60sec
      //INHIBIT_Zone1_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == INHIBIT_Zone3){
    if ((millis() - INHIBIT_Zone3_lastRead_millis) >= INHIBIT_Zone3_int){    // set to 60sec
      //INHIBIT_Zone1_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == INHIBIT_Zone4){
    if ((millis() - INHIBIT_Zone4_lastRead_millis) >= INHIBIT_Zone4_int){    // set to 60sec
      //INHIBIT_Zone1_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == INHIBIT_Zone5){
    if ((millis() - INHIBIT_Zone5_lastRead_millis) >= INHIBIT_Zone5_int){    // set to 60sec
      //INHIBIT_Zone1_lastRead_millis = millis();
      return true;
    }
    else {return false;}
  }

  if (PID == FLUSH_water){
    if ((millis() - FLUSH_lastRead_millis) >= FLUSH_water_int){    // set to 60sec
      return true;
    }
    else {return false;}
  }

  if (PID == INHIBIT_flush){
    if ((millis() - INHIBIT_flush_lastRead_millis) >= INHIBIT_flush_int){    // 2 hours
      return true;
    }
    else {return false;}
  }


  if (PID == PRINT){
    if ((millis() - PRINT_lastRead_millis) >= PRINT_int){    // set to 10 sec
      PRINT_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }

  if (PID == RUNNING){
    if ((millis() - RUNNING_lastRead_millis) >= RUNNING_int){        // set to .5 sec
      RUNNING_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }
}

// CRC8 function for Arduino (c++)
uint8_t crc8( uint8_t *addr, uint8_t len) {
      uint8_t crc=0;
      for (uint8_t i=0; i<len;i++) {
         uint8_t inbyte = addr[i];
         for (uint8_t j=0;j<8;j++) {
             uint8_t mix = (crc ^ inbyte) & 0x01;
             crc >>= 1;
             if (mix) 
                crc ^= 0x8C;
         inbyte >>= 1;
      }
    }
   return crc;
}
