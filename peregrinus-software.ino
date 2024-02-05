//TE TESTEN: IRIDIUM
/*
                       ~@@@@@@@.
                  /@@@@@@@@@@@@@@@@@.                                 ~@@@@@
               -@@@@@@@@@@@@@@@@@@@@@@@                    ,/@@@@@@@@@@@@
             -@@@@@@@@@@@@@@@@@@@@@@@@@@@       .~@@+,          @@.@@@
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                @@@. +@@,
          ~+@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@          @@@@    @@~
        / @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@~   .@@@@-     /@/
      .+ .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@. /@@@@~        @@
     @+  @@@@@@@@@@@@@@@@@@@@@@@@@@  @@@@@@.          /@
    -@   @@@@@@@@@@@@@@@@@@+  /@@@@@@,              @.
    .@@@ ,@@@@@@/+.  ~@@@@@@@@~                  @-
      ,@@@@@@@@@@@@@@+.                        /
                                            @
                                 _
                                (_)
  _ __   ___ _ __ ___  __ _ _ __ _ _ __  _   _ ___
 | '_ \ / _ \ '__/ _ \/ _` | '__| | '_ \| | | / __|
 | |_) |  __/ | |  __/ (_| | |  | | | | | |_| \__ \
 | .__/ \___|_|  \___|\__, |_|  |_|_| |_|\__,_|___/
 | |                   __/ |
 |_|                  |___/
*/

//Libraries
//Library locations: 
//  - https://github.com/mikalhart/IridiumSBD
//  - https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
//  - https://github.com/chrisfryer78/ArduinoRTClibrary
#include <IridiumSBD.h> //Iridium module
#include <Wire.h> //I2C library
#include <SPI.h> //SPI library
#include <SparkFunLSM9DS1.h> //IMU
#include <SoftwareSerial.h>
#include <virtuabotixRTC.h> //RTC

/*  
    _ _  _ ____ ___ ____ _    _    _ _  _ ____ ____ _  _ 
    | |\ | [__   |  |___ |    |    | |\ | | __ |___ |\ | 
    | | \| ___]  |  |___ |___ |___ | | \| |__] |___ | \| 
*/
//We gaan in twee fases meten:
//fase 1 is meteen na vertrek en voor het grootste deel van de vlucht. Hier verzamelen we data over het magneetveld en de straling
//fase 2 moet zo getimed worden dat die begint op het moment dat de raket terug de atmosfeer in gaat
//we meten dan de versnelling en de oriëntatie
#define SECOND_PHASE_START 48000000 //uitgedrukt in millisecoden

//Dit zijn instellingen voor debugging
//je kan dus apart de serial output, imu en iridium aan en uit zetten om te testen
const bool useSerial PROGMEM = true;
const bool useIMU PROGMEM = true;
const bool useIridium PROGMEM = true;


//Hier definieren we wat naar welke pin gaat
//als later in de code een pin gebruikt wordt, of je wil er zelf een gebruiken, dan schrijf je dus gewoon de naam van de pin
//en zo kunnen we gemakkelijk de pin veranderen
//Opgelet: de pinnummers komen niet overeen met wat er op de atmega staat, maar ze corresponderen met pinnummers op de arduino uno
//check de spreadsheet voor meer info: https://docs.google.com/spreadsheets/d/1WkGOUm99t6b8yQbz2qlvQCFHMPkMHSz41ywEppdT8nU/edit?usp=sharing

#define IRIDIUM_TX 10
#define IRIDIUM_RX 11

//dit zijn de pins van de SSD
#define SIGN_PIN 2
#define NOISE_PIN 8

//Heel belangrijk: het debug ledje
#define DEBUG_LED 3

//I2C communicatieadressen worden hier gespecifieerd
//deze waarden zijn volgens de documentatie van de LSM9DS1 die je hier kan vinden:
//https://learn.sparkfun.com/tutorials/lsm9ds1-breakout-hookup-guide/using-the-arduino-library
//Define imu addresses
#define LSM9DS1_M   0x1E
#define LSM9DS1_AG  0x6B

//Variables voor timing
unsigned long previousSSDMillis = 0;
unsigned long previousDataMillis = 0;
unsigned long currentMillis = 0;
unsigned long startMillis;

//Objects
//ik kan niet echt uitleggen wat een object is
//je kan een beetje denken dat het een machine is waar je bepaalde variabelen aan kan veranderen
//en die je bepaalde functies kan doen uitvoeren (dat is echt slecht uitgelegd, maar kijk vooral naar de code om een idee te krijgen van hoe ze werken)
SoftwareSerial iridiumSerial(IRIDIUM_RX, IRIDIUM_TX); //virtual serial
IridiumSBD modem(iridiumSerial); //iridium
virtuabotixRTC RTC(13, 7, 6); //clock definition
LSM9DS1 imu; //imu

//deze struct is een structuur om als het ware meerdere variabelen onder een grote noemer te brengen
//op zichzelf is data een beetje zoals het keyword "int", "bool" of "float"
//je kan er een variabele mee maken waarvoor je dan alle subvariabelen, zoals hier bv hours kan invullen (measurements.hours = RTC.hours;)
//die variabele heet hier measurements, en we slaan daarin al onze metingen op
//daarna vult iridium met die metingen een langere reeks data, die hij dan verstuurt
struct data {
  int hour, mins, sec; //12, 35, 22
  long int ms; //15960001

  float cpm; //1001.34443

  float mag_x, mag_y, mag_z; //44.335, 0.0001, 1355.400

  float gyro_x, gyro_y, gyro_z; //5.540, 654.154, 0
  float accel_x, accel_y, accel_z; //1321.000, 654.444, 325.999
};
data measurements;
//hier maken we ruimte vrij voor de data die we in een bericht gaan versturen
char firstPhaseDataToSendInMessage[10][16];
char secondPhaseDataToSendInMessage[13][24];
//tellen hoeveel keer we de data al gemeten hebben
int firstPhaseDataCounter = 0;
int secondPhaseDataCounter = 0;

void setup()
{ 
  //alle verwijzingen naar serial moeten voor de lancering uit de code verwijderd worden, gebruik hiervoor ctrl+f in alle documenten
  //en onze enige vorm van communicatie met de programmeur zal dus via het LED-lampje zijn
  if (useSerial) Serial.begin(19200); //Used for debugging; REMOVE ON LAUNCH 
  //opstarten van de IMU
  //Hier gebeurt niet zo veel, het belangrijkste is dat als er iets niet werkt met de imu, de code hier met opzet zal vastlopen
  //als er iets niet werkt ofzo, altijd documentatie checken
  if (useIMU)
  {
    Wire.begin();
    
    //IMU settings
    imu.settings.device.commInterface = IMU_MODE_I2C; // Set mode to I2C
    imu.settings.device.mAddress = LSM9DS1_M;
    imu.settings.device.agAddress = LSM9DS1_AG;
    
    if (!imu.begin())
    {
      if (useSerial) Serial.print(F("IMU not started"));
      while(1);
    };
  };

  //opstarten van iridium
  //als iridium niet werkt, vragen aan Yves of deze site met documentatie: http://arduiniana.org/libraries/iridiumsbd/
  if (useIridium) {
   startIridium(19200);
  }

  //als led aan gaat, is de setup succesvol verlopen
  pinMode(DEBUG_LED, OUTPUT);
  digitalWrite(DEBUG_LED, HIGH);
  startMillis = millis();
}

void loop()
{
  currentMillis = millis(); //Update de huidige milliseconden
  
  //implementatie van deze functie is in het document getdata.ino te vinden
  //kijk daar voor meer informatie over deze functie
  getdata();

  //om de tien seconden versturen we een bericht
  //de senddata functie staat in het document iridium.ino
  //kijk daar voor meer informatie over deze functie
  if (firstPhaseDataCounter == 10) {
    firstPhaseDataCounter = 0;
    if (useSerial) Serial.println(F("Sending message 1!"));
    sendPhaseOneData();
  }
  if(secondPhaseDataCounter == 13) {
    secondPhaseDataCounter = 0;
    Serial.println(F("Sending message 2!"));
    sendPhaseTwoData();
  }
} 
