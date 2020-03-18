

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
  if (inCount < 4) { return;}     //return if less than 4 bytes are avaialable
  if (inCount > 4) { Serial.write(" Input Buffer Overflow in function receiveData()"); Serial.println("");
    // must deal with buffer overrun
  }
  if (inCount == 4){    // read and store the buffr contents
      for (int i=0; i < 4; i++){
      RPirecBlock[i]=Serial.read();
      calcCRC = crc8(RPirecBlock, 3);
      }
 
  if ((RPirecBlock[3]) != calcCRC){     // check CRC8
    Serial.write("CRC FAILED in function receiveData()");
    Serial.println("");
    crcFAIL = true;
    // request data to be resent
    }
  }
}

void decodeRPiData(){
  // create decode strategy
  }


void getTempsF(void){
  sensors.requestTemperatures();            // required before .getTempX()
  for (int i=0; i<9; i++){                  // read all 8 temp sensors. Order of reading is fixed by DallasTemperature so be aware
  float temp = sensors.getTempF(probeAddr[i]);
  greenHouseTemperatures[i] = (int) temp;   // convert from float to int and store to greenHouseTemperature[]
  }
}

void controlHouseVent(void){
  int houseTemp   = greenHouseTemperatures[6];
  if (houseTemp >= houseVentOnTemp){
    digitalWrite(heaterPin, OFF);
    digitalWrite(ventPin, ON);
  }
  if (houseTemp <= houseVentOffTemp){
    digitalWrite(ventPin, OFF);
  }
}

void controlHouseHeater(void){
  int houseTemp   = greenHouseTemperatures[6];
  if (houseTemp <= houseHeatOnTemp){
    digitalWrite(ventPin, OFF);
    digitalWrite(heaterPin, ON);
  }
  if (houseTemp >= houseHeatOffTemp){
    digitalWrite(heaterPin, OFF);
  }
}


void waterPots(void){

        
        // Check if NTP returned a valid time
    if (UTC_hours >= 24) {return;}  // 24 is an invalid UTC value. UTC_hours was init to 25 so nothing is done until a valid time received
                                    // so no action until after the first successful getNTPtime()
                                   
        // check if UTC time matches first preset watering time
    if (waterON == false){          // check schedule if the waterON = false. If true then moving on..........
      if ((UTC_hours == waterSchedule[firstWatering]) && (UTC_minutes == 0)){
        waterON = true;
        WATER_int = 120000;             // first watering 6am is for 2 minutes
      }
    }
        // check if UTC time matches second preset watering time
    if (waterON == false){
      if ((UTC_hours == waterSchedule[secondWatering]) && (UTC_minutes == 0)){
        waterON = true;
        WATER_int = 60000;              // second watering 2pm is for 1 minute
      }
    }
    
    if ((waterON == true) && (wateringON == false)){        //waterON if inside watering window 
        wateringON = true;                                  // wateringON is true is watering is active
          digitalWrite(pot1pin, ON);
          digitalWrite(pot2pin, ON);
          digitalWrite(pot3pin, ON);
          digitalWrite(pot4pin, ON);
          digitalWrite(pot5pin, ON);
          
        WATER_lastRead_millis = millis();                   // init the wateringON timer start value
      }

    if ((waterON == true) && (wateringON == true)){
      
      bool endwatering = timer_lapsed(WATER);               // endwatering true is the watering time has expired
      if (endwatering == true){
        waterON = false;                                    // reset the waterON to false
        wateringON = false;                                 // 
          digitalWrite(pot1pin, OFF);
          digitalWrite(pot2pin, OFF);
          digitalWrite(pot3pin, OFF);
          digitalWrite(pot4pin, OFF);
          digitalWrite(pot5pin, OFF);
      }
    }
}

void printData(void){
  Serial.print("UTC time is:  ");
    if ((UTC_hours >= 24) || (UTC_minutes >= 60)){
      Serial.println("  INVALID TIME STAMP:");
    }
  Serial.print(UTC_hours);
  Serial.print(':');
  if (UTC_minutes < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
  Serial.println(UTC_minutes);

  if (wateringON == true){
    Serial.println("Watering is ACTIVE");
  }
    else Serial.println("Watering in INACTIVE");
  
    
  // print the temperatures
  for (int i=0; i<9; i++){
    Serial.print(greenHouseTemperatures[i]);
    Serial.print("  ");
  }
    Serial.println();
    Serial.println();
}

// These are the system timers. Using millis() function. Each timer has a NAME_int to define the length of time in ms.
// if current time - start time >= whatever_int then {}
// 
bool timer_lapsed(uint8_t PID){                             // timer. used for short interval scheduling 1sec- a few minutes
  if (PID == NTP){
    if ((millis() - NTP_lastRead_millis) >= NTP_int){        // set to 5 sec
      NTP_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }
  if (PID == PROBE){
    if ((millis() - PROBE_lastRead_millis) >= PROBE_int){    // set to 1 sec
      PROBE_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }
  if (PID == WATER){
    if ((millis() - WATER_lastRead_millis) >= WATER_int){   // set to 2minutes
      WATER_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }
  if (PID == PRINT){
    if ((millis() - PRINT_lastRead_millis) >= PRINT_int){    // set to 5 sec
      PRINT_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }

  if (PID == LED){
    if ((millis() - LED_lastRead_millis) >= LED_int){        // set to .5 sec
      LED_lastRead_millis = millis();
      return true;
    }
    else {return false;}
    }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}



void getNTPtime(void){
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  //sendNTPpacket(testServer);  // Desktop
  //Serial.println("NTP packet sent...");
}

void decodeTime(void ){
  // wait to see if a reply is available
  //delay(1000);
  if (Udp.parsePacket()) {
    //Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = ");
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    //Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    //Serial.println(epoch);


    // print the hour, minute and second:
    //Serial.println();
    //Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    UTC_hours = ((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
        // convert UTC_hours to local time (24h)
        // This is for DST (UTC - 5) and Central Time
    if (UTC_hours >= 5){
      UTC_hours = (UTC_hours - 5);
    }
    else UTC_hours = (UTC_hours + 19);
    
    //Serial.print(UTC_hours);
    //Serial.print(':');
    //if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      //Serial.print('0');
    //}
    UTC_minutes = ((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    //Serial.println(UTC_minutes);
    //Serial.println("");         // Add another CR
    // Cut off the rest of this function since I don't need seconds
    //Serial.print(':');
    //if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      //Serial.print('0');
    //}
    //Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  //delay(10000);  
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
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
