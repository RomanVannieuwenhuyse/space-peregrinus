/*  IRIDIUM library documentatie:
    https://github.com/mikalhart/IridiumSBD/blob/master/extras/IridiumSBD%20Arduino%20Library%20Documentation.pdf
*/


int err;
int signalQuality = -1;
//err = modem.getSignalQuality(signalQuality);

void startIridium(int baud)
{
  if(useSerial) iridiumSerial.begin(baud);
  //Start de modem op
  err = modem.begin();
  if (err != ISBD_SUCCESS){
    //Het opstarten heeft gefaald, we communiceren door in de serial output iets te sturen
    if (useSerial) {
      Serial.println(F("Begin failed!"));
          if (err == ISBD_NO_MODEM_DETECTED) Serial.println(F("No modem detected: check wiring."));
    }
  } else {
    modem.adjustATTimeout(60);
    //Het opstarten van Iridium was succesvol
    if (useSerial){
      Serial.println(F("iridium ok!"));
      int quality = 100;
      Serial.println("q:"); 
      modem.getSignalQuality(quality);
      //0 means signal is nonexistant, 5 means high quality signal
      Serial.println(quality);
    } 
  }
}


void sendPhaseOneData(){
  /*De data die we willen versturen is:
     -1x timestamp = 10
     -10x magnetometer data = 10x3x4 = 120 (want sizeof(float) = 4)
     -10x counts per minute = 10x4 = 40
  */
  uint8_t message[171] = {'\0'};

  RTC.updateTime();
  
  measurements.hour = RTC.hours;
  measurements.mins = RTC.minutes;
  measurements.sec = RTC.seconds;
  measurements.ms = millis();
  
  //We voegen de timestamp vooraan in het bericht toe
  //10 bytes for timestamp
  char timestamp[10] = {i_getBits(measurements.hour).arr[0], i_getBits(measurements.hour).arr[1], i_getBits(measurements.mins).arr[0], i_getBits(measurements.mins).arr[1], i_getBits(measurements.sec).arr[0], i_getBits(measurements.sec).arr[1], l_u_getBits(measurements.ms).arr[0], l_u_getBits(measurements.ms).arr[1], l_u_getBits(measurements.ms).arr[2], l_u_getBits(measurements.ms).arr[3]};
  if(useSerial){
    Serial.println("timestamp:");
    Serial.print(measurements.hour);
    Serial.print(":");
    Serial.print(measurements.mins);
    Serial.print(":");
    Serial.print(measurements.sec);
    Serial.print("   ");
    Serial.println(measurements.ms);
  }
  //18*16 bytes for data to send in message
  uint8_t * memcpyMessagePtr = message;
  memcpy(memcpyMessagePtr, timestamp, 10);
  memcpyMessagePtr += 10;
  
  //We voegen de data die gemeten is toe aan het bericht
  for(int i = 0; i < 10; i++){
    memcpy(memcpyMessagePtr, firstPhaseDataToSendInMessage[i], 16);
    memcpyMessagePtr += 16;
  }
  iridiumSendMessage(message, 171);
}

void sendPhaseTwoData()
{
  uint8_t message[323] = {'\0'};

  RTC.updateTime();
  
  measurements.hour = RTC.hours;
  measurements.mins = RTC.minutes;
  measurements.sec = RTC.seconds;
  measurements.ms = millis();
  
  //10 bytes for timestamp
  char timestamp[10] = {i_getBits(measurements.hour).arr[0], i_getBits(measurements.hour).arr[1], i_getBits(measurements.mins).arr[0], i_getBits(measurements.mins).arr[1], i_getBits(measurements.sec).arr[0], i_getBits(measurements.sec).arr[1], l_u_getBits(measurements.ms).arr[0], l_u_getBits(measurements.ms).arr[1], l_u_getBits(measurements.ms).arr[2], l_u_getBits(measurements.ms).arr[3]};
  if(useSerial){
    Serial.println("timestamp:");
    Serial.print(measurements.hour);
    Serial.print(":");
    Serial.print(measurements.mins);
    Serial.print(":");
    Serial.print(measurements.sec);
    Serial.print("   ");
    Serial.println(measurements.ms);
  }
  uint8_t * memcpyMessagePtr = message;
  //8*36 bytes for data + 10
  memcpy(message, timestamp, 10);
  memcpyMessagePtr += 10;
  for(int i = 0; i < 13; i++){
    memcpy(memcpyMessagePtr, secondPhaseDataToSendInMessage[i], 24);
    memcpyMessagePtr += 24;
  }
  iridiumSendMessage(message, 323);
}

void iridiumSendMessage(uint8_t message[], int size){
  if(useSerial){
    Serial.print("{");
    for(int i = 0; i < 337; i++){
      Serial.print((unsigned char)message[i]);
      Serial.print(", ");
      if(message[i] == '\0'){
        Serial.println("}");
      }
    }
  }
  
  err = modem.sendSBDBinary(message, size);
  
  if (err != ISBD_SUCCESS)
  {
    if(useSerial){
      Serial.print(F("sendSBDText failed: error "));
      Serial.println(err);
      if (err == ISBD_SENDRECEIVE_TIMEOUT){
        Serial.println(F("Try again with a better view of the sky."));
      }
      else { 
        Serial.println(F("Message transmitted!"));
      }
    }
   }
}

#define DIAGNOSTICS false
#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  Serial.write(c);
}
#endif
