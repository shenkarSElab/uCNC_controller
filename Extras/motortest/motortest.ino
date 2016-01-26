//
//Stepper myStepperX(3,4,2);
//Stepper myStepperY(12, 11,10);
#define stepPin1 4
#define dirPin1 3
#define enPin1 2

#define stepPin2 11
#define dirPin2 12
#define enPin2 10

#include <Servo.h>

Servo myservo;
int pos = 0;

void setup() {
  myservo.attach(9);
  pinMode(dirPin1, OUTPUT);//dir
  pinMode(stepPin1, OUTPUT);//step
  pinMode(enPin1, OUTPUT);//enable
  digitalWrite(enPin1, LOW);
  digitalWrite(dirPin1, HIGH);

  pinMode(dirPin2, OUTPUT);//dir
  pinMode(stepPin2, OUTPUT);//step
  pinMode(enPin2, OUTPUT);//enable
  digitalWrite(enPin2, LOW);
  digitalWrite(dirPin2, HIGH);


  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}


void loop() {
  digitalWrite(stepPin1, HIGH);
  digitalWrite(stepPin2, HIGH);

  delay(2);
  digitalWrite(stepPin1, LOW);
  digitalWrite(stepPin2, LOW);

  delay(2);
}

