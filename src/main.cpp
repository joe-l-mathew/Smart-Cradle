#include <Arduino.h>

const int voiceLimit = 590;
const int noOfConformation = 3;

void setup()
{
  pinMode(D0, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);
}
// value from the sensor
int voiceValue = 0;
// conformation value which increment
int conformation = 0;
// degree of asleep
int degreeOfAsleep = 0;

void loop()
{
  // 10 second for loop completion
  // loop identifies the number of times the captured value goes beyond the set value in 5 seconds.
  for (int i = 0; i < 100; i++)
  {
    voiceValue = analogRead(A0);
    // if voice value> voice limit conformation gets incremented by 1
    if (voiceValue > voiceLimit)
    {
      conformation++;
      Serial.print("Voice breached times : ");
      Serial.println(conformation);
    }
    if (conformation > noOfConformation)
    {
      // if the conformation is grater than fos then brake out of the loop
      break;
    }
    delay(100); // changeble value decrease to improve accuracy
  }

  if (conformation > noOfConformation)
  {
    // increase the degree of asleep
    degreeOfAsleep++;
  }
  else
  {
    // turning of the bulb
    digitalWrite(D0, LOW);
    Serial.println(degreeOfAsleep);
    degreeOfAsleep--;
  }
  // check if conformation is grater than our range
  if (degreeOfAsleep > 0)
  {

    // turning on the bulb
    digitalWrite(D0, HIGH);
    // delay 1 sec
    delay(10000);
  }

  // if degree of asleep goes below zero set it to zero
  if (degreeOfAsleep < 0)
  {
    degreeOfAsleep = 0;
  }
  Serial.print("Degree of asleep: ");
  Serial.println(degreeOfAsleep);
  conformation = 0; // after ending loop reset the conformation to 0
  Serial.println("Loop completed");
}