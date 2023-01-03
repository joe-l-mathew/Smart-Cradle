/*
Add WIFI_SSID
ADD WIFI_PASSWORD
MIC Analog - A0
MOTOR +VE - D0
change voiceValue, increasing decreases accuracy always keep arount 600
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Paikkattu"
#define WIFI_PASSWORD "youtube357"
#define API_KEY "AIzaSyCYBqDGk7KnavARvPk_2JpCCSUvL8bTRKE"
#define DATABASE_URL "smart-cradle-d7ce9-default-rtdb.firebaseio.com/"

FirebaseData fbdo;     // firebase data object
FirebaseConfig config; // firebase config object
FirebaseAuth auth;     // firebase auth object

const int voiceLimit = 610;            // analog out from mic limit
const int maxDegreeOfConformation = 3; // conformation limit
unsigned long sendDataPrevMillis = 0;
bool isCraddleOscilating = false; // craddle status
bool isBabyAwake = false;         // baby sleep status
bool signupOK = false;            // Signup Status
bool manualCradleStatus = false;  // user turning on and off

void detectBabyCry();            // detect baby cry
void babyAwake();                // if baby is awake
void babyAsleep();               // if baby is asleep
void turnOnOffCradle(bool isOn); // to turn on and off cradle movement
void connectWifi();              // connect wifi
bool firebaseStatus();           // firebase connection status
void manualCradleStatusFun();    // get manuel on of data

void setup()
{
  Serial.begin(9600); // serial begin for print
  //-- Setting Pinmode --//
  pinMode(D0, OUTPUT);
  pinMode(A0, INPUT);
  connectWifi(); // connect wifi
  // Declaring dbUrl and Api Key
  config.database_url = DATABASE_URL;
  config.api_key = API_KEY;
  // firebase signup
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    signupOK = true; // signup okey
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth); // Firebase config
  Firebase.reconnectWiFi(true);
}
int voiceValue = 0;           // value from the sensor
int degreeOfConformation = 0; // conformation value which increment
int degreeOfAsleep = 0;       // degree of asleep

void loop()
{
  detectBabyCry(); // increase degreeOfConformation for each cry detection

  if (degreeOfConformation > maxDegreeOfConformation)
  {
    babyAwake();
  }
  else
  {
    babyAsleep();
  }
  // check if degreeOfConformation is grater than our range
  if (degreeOfAsleep > 0 || manualCradleStatus)
  {
    turnOnOffCradle(true); // turn on the cradle
  }
  // if degreeOfAsleep <= 0  and if cradle is osccilating
  else if (isCraddleOscilating && degreeOfAsleep <= 0)
  {
    turnOnOffCradle(false); // turn of the cradle
  }

  // if degree of asleep goes below zero set it to zero
  if (degreeOfAsleep < 0)
  {
    degreeOfAsleep = 0; // resetting degree of asleep
  }

  degreeOfConformation = 0; // after ending loop reset the degree of conformation to zero
  Serial.println("Loop completed");
}

// used to detect is baby crying
void detectBabyCry()
{
  // 10 second for loop completion
  // loop identifies the number of times the captured value goes beyond the set value in 10 seconds.
  for (int i = 0; i < 100; i++)
  {
    voiceValue = analogRead(A0); // input from mic
    // if voice value > voice limit degreeOfConformation gets incremented by 1
    if (voiceValue > voiceLimit)
    {
      degreeOfConformation++;
      Serial.print("Voice breached times : ");
      Serial.println(degreeOfConformation);
    }
    if (degreeOfConformation > maxDegreeOfConformation)
    {
      // if the degreeOfConformation is grater than fos then brake out of the loop
      break;
    }
    if (i % 50 == 0)
    {
      manualCradleStatusFun();
      // turn on cradle if set on and cradle is currently off
      if (manualCradleStatus && !isCraddleOscilating)
      {
        turnOnOffCradle((true));
      }
      else if (!manualCradleStatus && isCraddleOscilating && degreeOfAsleep <= 0)
      {
        turnOnOffCradle(false);
      }
    }
    else
    {
      delay(100); // changeble value decrease to improve accuracy
    }
  }
}

void connectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Connecting Wifi
  Serial.println("Wifi Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("."); // waiting for wifi to connect
    delay(300);
  }
}

// if baby is awake
void babyAwake()
{
  degreeOfAsleep++;     // increase the degree of asleep
  isBabyAwake = true;   // baby is awake
  if (firebaseStatus()) // is firebase accesable
  {
    Firebase.RTDB.setInt(&fbdo, "smartCradle/degreeOfAsleep", degreeOfAsleep);
    Firebase.RTDB.setBool(&fbdo, "smartCradle/isBabyAwake", isBabyAwake);
  }
  Serial.print("Degree of asleep: ");
  Serial.println(degreeOfAsleep);
}

// if baby is asleep
void babyAsleep()
{

  degreeOfAsleep--;     // decrease the degree of asleep
  isBabyAwake = false;  // baby starts sleeping
  if (firebaseStatus()) // is firebase accesable
  {
    if (degreeOfAsleep >= 0)
    {
      Firebase.RTDB.setInt(&fbdo, "smartCradle/degreeOfAsleep", degreeOfAsleep);
    }
    else
    {
      Firebase.RTDB.setInt(&fbdo, "smartCradle/degreeOfAsleep", 0); // IF -1 set that to 0
    }
    Firebase.RTDB.setBool(&fbdo, "smartCradle/isBabyAwake", isBabyAwake);
  }
  Serial.print("Degree of asleep: ");
  Serial.println(degreeOfAsleep);
}

// cradle on or off
void turnOnOffCradle(bool On)
{
  if (On)
  {

    digitalWrite(D0, HIGH);     // turning on the bulb
    isCraddleOscilating = true; // updating status of cradle
    if (firebaseStatus())
    {
      Firebase.RTDB.setBool(&fbdo, "smartCradle/isCradleOsccilating", isCraddleOscilating);
    }

    delay(10000); // delay 10 sec
  }
  else
  {
    digitalWrite(D0, LOW);       // turning of the bulb
    isCraddleOscilating = false; // updating cradle status
    if (firebaseStatus())
    {
      Firebase.RTDB.setBool(&fbdo, "smartCradle/isCradleOsccilating", isCraddleOscilating);
    }
  }
}
// to get update from firebase
bool firebaseStatus()
{
  if (Firebase.ready() && signupOK)
  {
    Serial.print("Firebase status: ");
    Serial.println("True");
    return true;
  }
  else
  {
    Serial.print("Firebase status: ");
    Serial.println("False");
    return false;
  }
}

// to get RTDB update
void manualCradleStatusFun()
{

  bool bVal;
  Firebase.RTDB.getBool(&fbdo, F("/smartCradle/onOff"), &bVal);
  manualCradleStatus = bVal;
}