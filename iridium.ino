
int err;
int signalQuality = -1;
//err = modem.getSignalQuality(signalQuality);

void startIridium(int baud)
{
  if(useSerial) iridiumSerial.begin(baud);
  err = modem.begin();
  if (err != ISBD_SUCCESS){
    if (useSerial) {
      Serial.println(F("Begin failed!"));
          if (err == ISBD_NO_MODEM_DETECTED) Serial.println(F("No modem detected: check wiring."));
    } 
   
    return;
  } else { 
    if (useSerial) Serial.println(F("iridium ok!"));
    modem.adjustATTimeout(60);
    int quality = 100;
    Serial.println("q:"); 
    Serial.println(modem.getSignalQuality(quality));
    Serial.println(quality);
  }
}


void sendPhaseOneData(){
  char message[337] = {'\0'};

  RTC.updateTime();
  
  measurements.hour = RTC.hours;
  measurements.mins = RTC.minutes;
  measurements.sec = RTC.seconds;
  measurements.ms = millis();
  
  //10 bytes for timestamp
  char timestamp[10] = {i_getBits(measurements.hour).arr[0], i_getBits(measurements.hour).arr[1], i_getBits(measurements.mins).arr[0], i_getBits(measurements.mins).arr[1], i_getBits(measurements.sec).arr[0], i_getBits(measurements.sec).arr[1], l_u_getBits(measurements.ms).arr[0], l_u_getBits(measurements.ms).arr[1], l_u_getBits(measurements.ms).arr[2], l_u_getBits(measurements.ms).arr[3]};
  Serial.println("timestamp:");
  Serial.print(measurements.hour);
  Serial.print(":");
  Serial.print(measurements.mins);
  Serial.print(":");
  Serial.print(measurements.sec);
  Serial.print("   ");
  Serial.println(measurements.ms);
  //18*16 bytes for data to send in message
  char * memcpyMessagePtr = message;
  memcpy(memcpyMessagePtr, timestamp, 10);
  memcpyMessagePtr += 10;
  for(int i = 0; i < 18; i++){
    memcpy(memcpyMessagePtr, firstPhaseDataToSendInMessage[i], 16);
    memcpyMessagePtr += 16;
  }
  for(int i = 0; i < sizeof(message) - 1; i++){
    if(message[i] == '\0'){
      message[i] = 0b11111111;
      if(i < sizeof(message) - 38){
        int byteNumber = (int)i/8;
        int bitNumber = i%38;
        message[298+byteNumber] = (0b10000000 >> bitNumber) | message[298+byteNumber];
      }       
    }
  }
  iridiumSendMessage(message);
}

void sendPhaseTwoData()
{
  char message[337] = {'\0'};

  RTC.updateTime();
  
  measurements.hour = RTC.hours;
  measurements.mins = RTC.minutes;
  measurements.sec = RTC.seconds;
  measurements.ms = millis();
  
  //10 bytes for timestamp
  char timestamp[10] = {i_getBits(measurements.hour).arr[0], i_getBits(measurements.hour).arr[1], i_getBits(measurements.mins).arr[0], i_getBits(measurements.mins).arr[1], i_getBits(measurements.sec).arr[0], i_getBits(measurements.sec).arr[1], l_u_getBits(measurements.ms).arr[0], l_u_getBits(measurements.ms).arr[1], l_u_getBits(measurements.ms).arr[2], l_u_getBits(measurements.ms).arr[3]};
  Serial.println("timestamp:");
  Serial.print(measurements.hour);
  Serial.print(":");
  Serial.print(measurements.mins);
  Serial.print(":");
  Serial.print(measurements.sec);
  Serial.print("   ");
  Serial.println(measurements.ms);
  char * memcpyMessagePtr = message;
  //8*36 bytes for data + 10
  memcpy(message, timestamp, 10);
  memcpyMessagePtr += 10;
  for(int i = 0; i < 8; i++){
    memcpy(memcpyMessagePtr, secondPhaseDataToSendInMessage[i], 36);
    memcpyMessagePtr += 36;
  }
  for(int i = 0; i < sizeof(message) - 1; i++){
    if(message[i] == '\0'){
      message[i] = 0b11111111;
      if(i < sizeof(message) - 38){
        int byteNumber = (int)i/8;
        int bitNumber = i%38;
        message[298+byteNumber] = (0b10000000 >> bitNumber) | message[298+byteNumber];
      }      
    }
  }
  iridiumSendMessage(message);
}

void iridiumSendMessage(char message[]){
  Serial.print("{");
  for(int i = 0; i < 337; i++){
    Serial.print((unsigned char)message[i]);
    Serial.print(", ");
    if(message[i] == '\0'){
      Serial.println("}");
    }
  }

  
  // err = modem.sendSBDText(message);
  
  // if (err != ISBD_SUCCESS)
  // {
  //  if(useSerial){
  //    Serial.print(F("sendSBDText failed: error "));
  //    Serial.println(err);
  //    if (err == ISBD_SENDRECEIVE_TIMEOUT){
  //      Serial.println(F("Try again with a better view of the sky."));
  //    }
  //    else { 
  //      Serial.println(F("Message transmitted!"));
  //    }
  //  }
  // }
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
