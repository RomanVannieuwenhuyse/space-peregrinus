//Hier ergens vragen? Vraag aan Roman
//variabelen voor het meten van de straling (uit voorbeeldcode van een mail van meneer de Schrijver)
int sON = 0;
int nON = 0;
int signCount = 0;
int noiseCount = 0;

void getdata(){
  //dit is gewoon om de metingen van fase 2 uit te voeren als het zo ver is
  if(currentMillis > SECOND_PHASE_START)
  {
    secondPhaseGetData();
    return; //zorgt ervoor dat de rest van de getdata functie niet meer uitgevoerd wordt als we in fase 2 zitten
  }

  //elke seconde meten we het magneetveld en berekenen we hoeveel straling we hebben waargenomen op dat moment
  //daartussen tellen we het aantal keer dat we een deeltje waarnemen ofzo (eerste deel van het if-statement)
  if(currentMillis - previousSSDMillis < 1000){
    
    //dit is gekopieerd uit de voorbeeldcode
    int sign = digitalRead(SIGN_PIN);// Raw data of Radiation Pulse: Not-detected -> High, Detected -> Low
    int noise = digitalRead(NOISE_PIN);// Raw data of Noise Pulse: Not-detected -> Low, Detected -> High
    //Radiation Pulse normally keeps low for about 100[usec]
    if (sign == 0 && sON == 0)
    { //Deactivate Radiation Pulse counting for a while
      sON = 1;
      signCount++;
    } else if (sign == 1 && sON == 1) {
      sON = 0;
    }
    //Noise Pulse normally keeps high for about 100[usec]
    if (noise == 1 && nON == 0)
    { //Deactivate Noise Pulse counting for a while
      nON = 1;
      noiseCount++;
    } else if (noise == 0 && nON == 1) {
      nON = 0;
    }

    
    //hier doen we dus de meting van het magneetveld en berekenen we de straling
  } else {
    imu.readMag();    //accessible after reading at imu.mx, .my, .mz

    measurements.mag_x = imu.calcMag(imu.mx);
    measurements.mag_y = imu.calcMag(imu.my);
    measurements.mag_z = imu.calcMag(imu.mz);
    
    if(noiseCount == 0){
      measurements.cpm = signCount/((currentMillis - startMillis)/60000);
    } else {
      measurements.cpm = 0;
    }

    //we moeten hier de string bouwen die we gaan versturen
    char measurementData[16] = {f_getBits(measurements.cpm).arr[0], f_getBits(measurements.cpm).arr[1], f_getBits(measurements.cpm).arr[2], f_getBits(measurements.cpm).arr[3], f_getBits(measurements.mag_x).arr[0], f_getBits(measurements.mag_x).arr[1], f_getBits(measurements.mag_x).arr[2], f_getBits(measurements.mag_x).arr[3], f_getBits(measurements.mag_y).arr[0], f_getBits(measurements.mag_y).arr[1], f_getBits(measurements.mag_y).arr[2], f_getBits(measurements.mag_y).arr[3], f_getBits(measurements.mag_z).arr[0], f_getBits(measurements.mag_z).arr[1], f_getBits(measurements.mag_z).arr[2], f_getBits(measurements.mag_z).arr[3]};
    Serial.println(F("Measured data 1:"));
    Serial.println(measurements.cpm);
    Serial.println(measurements.mag_x);
    Serial.println(measurements.mag_y);
    Serial.println(measurements.mag_z);    
    memcpy(firstPhaseDataToSendInMessage[firstPhaseDataCounter], measurementData, 16);
    firstPhaseDataCounter++;
    signCount = 0;
    noiseCount = 0;
    previousSSDMillis = currentMillis;
  }
}

void secondPhaseGetData()
{
  if(currentMillis - previousDataMillis > 769){
    imu.readGyro();
    imu.readAccel();
    
    measurements.gyro_x = imu.calcGyro(imu.gx);
    measurements.gyro_y = imu.calcGyro(imu.gy);
    measurements.gyro_z = imu.calcGyro(imu.gz);
    
    measurements.accel_x = imu.calcAccel(imu.ax);
    measurements.accel_y = imu.calcAccel(imu.ay);
    measurements.accel_z = imu.calcAccel(imu.az);

    char measurementData[24] = {f_getBits(measurements.gyro_x).arr[0], f_getBits(measurements.gyro_x).arr[1], f_getBits(measurements.gyro_x).arr[2], f_getBits(measurements.gyro_x).arr[3], f_getBits(measurements.gyro_y).arr[0], f_getBits(measurements.gyro_y).arr[1], f_getBits(measurements.gyro_y).arr[2], f_getBits(measurements.gyro_y).arr[3], f_getBits(measurements.gyro_z).arr[0], f_getBits(measurements.gyro_z).arr[1], f_getBits(measurements.gyro_z).arr[2], f_getBits(measurements.gyro_z).arr[3], f_getBits(measurements.accel_x).arr[0], f_getBits(measurements.accel_x).arr[1], f_getBits(measurements.accel_x).arr[2], f_getBits(measurements.accel_x).arr[3], f_getBits(measurements.accel_y).arr[0], f_getBits(measurements.accel_y).arr[1], f_getBits(measurements.accel_y).arr[2], f_getBits(measurements.accel_y).arr[3], f_getBits(measurements.accel_z).arr[0], f_getBits(measurements.accel_z).arr[1], f_getBits(measurements.accel_z).arr[2], f_getBits(measurements.accel_z).arr[3]};
    if(useSerial){
      Serial.println(F("Measured data 2:"));
      Serial.println(measurements.gyro_x);
      Serial.println(measurements.gyro_y);
      Serial.println(measurements.gyro_z);
      Serial.println(measurements.accel_x);
      Serial.println(measurements.accel_y);
      Serial.println(measurements.accel_z);
    }
    memcpy(secondPhaseDataToSendInMessage[secondPhaseDataCounter], measurementData, 24);
    secondPhaseDataCounter++;
    previousDataMillis = currentMillis;
  }
}
