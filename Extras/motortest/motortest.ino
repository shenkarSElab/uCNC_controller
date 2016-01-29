//
//Stepper myStepperX(3,4,2);
//Stepper myStepperY(12, 11,10);

// open serial
// to change the stepps number change in the file.
int steps = 1000;
/* 0 - start x
  1 - start y
  2 - change x direction
  3 - change y direction
*/
#define stepPin1 4
#define dirPin1 3
#define enPin1 2

#define stepPin2 11
#define dirPin2 12
#define enPin2 10

#include <Servo.h>

Servo myservo;
int pos = 0;
int inByte = 0;
/////////////////////////////////////

boolean dir1 = 1;
boolean dir2 = 1;
///////////////////////////////////////
void setup() {
  Serial.begin(9600);

  myservo.attach(9);
  myservo.write(0);
  delay(20);
  pinMode(dirPin1, OUTPUT);//dir
  pinMode(stepPin1, OUTPUT);//step
  pinMode(enPin1, OUTPUT);//enable
  digitalWrite(enPin1, LOW);
  digitalWrite(dirPin1, dir1);

  pinMode(dirPin2, OUTPUT);//dir
  pinMode(stepPin2, OUTPUT);//step
  pinMode(enPin2, OUTPUT);//enable
  digitalWrite(enPin2, LOW);
  digitalWrite(dirPin2, dir2);

  /*
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  */
}


void loop() {
  if (Serial.available() > 0) {
    inByte = Serial.read() - '0';

    switch (inByte) {
      case 0:
        Serial.print("Y dir "); Serial.println(dir2);
        for (int i = 0; i < steps; i++) {
          digitalWrite(stepPin1, HIGH);
          delay(2);
          digitalWrite(stepPin1, LOW);
          delay(2);
        }
        break;

      case 1:
        Serial.print("X dir "); Serial.println(dir1);
        for (int i = 0; i < steps; i++) {
          digitalWrite(stepPin2, HIGH);
          delay(1);
          digitalWrite(stepPin2, LOW);
          delay(1);
        }
        break;

      case 2:
        dir1 = !dir1;
        Serial.print("dir1:") ; Serial.println(dir1);
        digitalWrite(dirPin1, dir1);
        break;

      case 3:
        dir2 = !dir2;
        digitalWrite(dirPin2, dir2);
        Serial.print("dir2: "); Serial.println(dir2);
        break;


      case 5:
        for (pos = 0; pos <= 10; pos += 1) { // goes from 0 degrees to 180 degrees
          // in steps of 1 degree
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          delay(20);                       // waits 15ms for the servo to reach the position
        }
        for (pos = 10; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          delay(20);                       // waits 15ms for the servo to reach the position
        }
        break;
    }

  }
}
