#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

const int voiceLimit = 610;
const int maxDegreeOfConformation = 3;

#define WIFI_SSID "Paikkattu"
#define WIFI_PASSWORD "youtube357"

void setup()
{
  Serial.begin(9600);
  pinMode(D0, OUTPUT);
  pinMode(A0, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Wifi Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
// value from the sensor
int voiceValue = 0;
// conformation value which increment
int degreeOfConformation = 0;
// degree of asleep
int degreeOfAsleep = 0;

void loop()
{
  // 10 second for loop completion
  // loop identifies the number of times the captured value goes beyond the set value in 10 seconds.
  for (int i = 0; i < 100; i++)
  {
    voiceValue = analogRead(A0);
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
    delay(100); // changeble value decrease to improve accuracy
  }

  if (degreeOfConformation > maxDegreeOfConformation)
  {
    degreeOfAsleep++; // increase the degree of asleep
    Serial.print("Degree of asleep: ");
    Serial.println(degreeOfAsleep);
  }
  else
  {
    digitalWrite(D0, LOW); // turning of the bulb
    degreeOfAsleep--;      // decrease the degree of asleep
    Serial.print("Degree of asleep: ");
    Serial.println(degreeOfAsleep);
  }
  // check if degreeOfConformation is grater than our range
  if (degreeOfAsleep > 0)
  {
    digitalWrite(D0, HIGH); // turning on the bulb
    delay(10000);           // delay 1 sec
  }

  // if degree of asleep goes below zero set it to zero
  if (degreeOfAsleep < 0)
  {
    degreeOfAsleep = 0;
  }

  degreeOfConformation = 0; // after ending loop reset the degree of conformation to zero
  Serial.println("Loop completed");
}
