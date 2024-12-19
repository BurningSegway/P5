#include "RunningAverage.h"

//define pinout
#define FSR1 A0
#define FSR2 A1

#define PWM_f 3
#define PWM_r 5

#define A 8
#define B 9
#define I 7

//defining variables for determining angular position and speed
int QEM[16] = { 0, -1, 1, 2, 1, 0, 2, -1, -1, 2, 0, 1, 2, 1, -1, 0 };
int Old = 0;
int New = 0;
int Out = 0;
int counter = 0;
int encoderError = 0;
float degree = 0;
float lastDegree = 0;
float speed = 0;
float speedFilt = 0;
float deltaT = 0;
unsigned long timeMillis;
unsigned long lastMillis;

//Variables for storing fsr values
int FSR1_val = 0;
int FSR2_val = 0;

int FSR1_con = 0;
int FSR2_con = 0;

//Direction variables
int direction = 1;  // 1 for forward, -1 for reverse
int lastDirection = 0;

//variables for the PI controller
float velDes = 0;
float velError = 0;
float velIntegral = 0;
float Kp = 8;  //found through trail and error
float Ki = 6;  //same
int controlSig = 0;

//Iniate running average to implement crude low pass filter
RunningAverage speedAvg(15);



void setup() {
  //Set pinmodes
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(I, INPUT_PULLUP);
  pinMode(FSR1, INPUT);
  pinMode(FSR2, INPUT);
  pinMode(PWM_f, OUTPUT);
  pinMode(PWM_r, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(I), ticker, CHANGE);    //Enable interrupts when the state changes on I pin, output from XOR gate

  Serial.begin(115200);

  timeMillis = micros();

  speedAvg.clear();
}


//Interrrupt routine made with inpiration from blog post from SparkFun "How to use a quadrature encoder" https://cdn.sparkfun.com/datasheets/Robotics/How%20to%20use%20a%20quadrature%20encoder.pdf
void ticker() {
  //Serial.println("hej");
  Old = New;
  New = digitalRead(A) * 2 + digitalRead(B);  // Convert binary input to decimal value
  Out = QEM[Old * 4 + New]; //Finding the output by indexing the matrix
  if (Out != 2) {           //If valid to incrmente or decrement
    counter += Out;
  } else {
    encoderError += 1;      //invalid add to error
  }
  //Serial.println(counter);
  //Serial.print("counter:");
  //Serial.print("degree:");
  //Serial.println(degree);
}

void loop() {

  lastMillis = timeMillis;    //Note the last loop time
  timeMillis = micros();      //note the current time actual in microseconds and not milliseconds as the var name suggests
  lastDegree = degree;      //note the last loop angularpostiiton
  degree = ((float)counter * 0.0265) / 4;       //Calculate angular position based on encoder count
  deltaT = ((float)(timeMillis - lastMillis)) / 1000000;    //Calculate the dT based on the last loop time and the current, convert tosconds
  speed = (degree - lastDegree) / deltaT;     //calculate speed as difference en position over difference in time
  speedAvg.addValue(speed);         //smoothing out the speed signal, crude lowpass filder
  speedFilt = speedAvg.getAverage();

  //read FSR values
  FSR1_val = analogRead(FSR1);
  FSR2_val = analogRead(FSR2);

  //constrain the values, the maximum value can be tuned to the specific user
  FSR1_con = constrain(FSR1_val, 70, 400);
  FSR2_con = constrain(FSR2_val, 70, 250);

  /*Serial.print("Count:");
  Serial.print(counter);
  Serial.print(",");
  Serial.print("Theta:");
  Serial.print(degree);
  Serial.print(",");
  Serial.print("Speed:");
  Serial.print(speed);
  Serial.print(",");
  Serial.print("SpeedAVG:");
  Serial.print(speedAvg.getAverage());
  Serial.print(",");

  Serial.print("FSR1:");
  Serial.print(FSR1_val);
  Serial.print(",");
  Serial.print("FSR2:");
  Serial.print(FSR2_val);
  Serial.print(",");

  Serial.print("PWM1:");
  Serial.print(PWM1);
  Serial.print(",");
  Serial.print("PWM2:");
  Serial.println(PWM2);*/
  Serial.print("THETA:");
  Serial.print(-degree);
  Serial.print(",");
  Serial.print("Speed:");
  Serial.print(speedFilt);
  Serial.print(",");
  Serial.print("ControlSig:");
  Serial.print(controlSig);
  Serial.print(",");
  Serial.print("Goal:");
  Serial.println(velDes);
/*
  Serial.print(",");
  Serial.print("Error:");
  Serial.print(velError);

  Serial.print(",");
  Serial.print("F1:");
  Serial.print(FSR1_con);

  Serial.print(",");
  Serial.print("F2:");
  Serial.println(FSR2_con);
*/

//change direction to forwards if fsr1 is higher
if ((FSR1_con) > (FSR2_con)) {
  velDes = map(FSR1_con, 70, 400, 0, 35); //map the force input to a velocity
  direction = 1;  // Forward
  if (lastDirection != direction) {
    lastDirection = direction;
    velIntegral = 0;  // Reset velocity integral when direction changes
  }
  //or change the direction to reverse if fsr2 is larger
} else if ((FSR2_con) > (FSR1_con)) {
  velDes = map(FSR2_con, 70, 250, 0, -35);
  direction = -1;  // Reverse
  if (lastDirection != direction) {
    lastDirection = direction;
    velIntegral = 0;  // Reset velocity integral when direction changes
  }
} else {
  velDes = 0;
}

// PI controller 
velError = abs(velDes) - abs(speedFilt);    //calculate error
velIntegral = velIntegral + velError * deltaT;    //updating the error integral
controlSig = Kp * velError + Ki * velIntegral;    //calculate the contorlsignal with controller gains
controlSig = constrain(controlSig, 0, 255);   //constrain the controlsignal to valid values for the motor driver

// Boundary control logic (automatic direction reversal with FSR check)
if (degree >= 53) {
  // At upper boundary, allow reverse direction only
  if (direction != -1) {
    direction = -1;                  // Force reverse direction
    lastDirection = direction;       // Update lastDirection
    velIntegral = 0;                 // Reset velocity integral when direction changes
  }

  // Allow reverse motion only if FSR2 is pressed (reverse direction)
  if ((FSR2_con) > (FSR1_con)) {
    analogWrite(PWM_r, controlSig);  // Enable reverse motion
    analogWrite(PWM_f, 0);           // Disable forward motion
  } else {
    analogWrite(PWM_r, 0);           // Stop reverse if no reverse FSR input
    analogWrite(PWM_f, 0);           // Stop motor if no reverse FSR input
  }
} else if (degree <= -53) {
  // At lower boundary, allow forward direction only
  if (direction != 1) {
    direction = 1;                   // Force forward direction
    lastDirection = direction;       // Update lastDirection
    velIntegral = 0;                 // Reset velocity integral when direction changes
  }

  // Allow forward motion only if FSR1 is pressed (forward direction)
  if ((FSR1_con) > (FSR2_con)) {
    analogWrite(PWM_f, controlSig);  // Enable forward motion
    analogWrite(PWM_r, 0);           // Disable reverse motion
  } else {
    analogWrite(PWM_f, 0);           // Stop forward if no forward FSR input
    analogWrite(PWM_r, 0);           // Stop motor if no forward FSR input
  }
} else {
  // Regular motor control when within bounds (i.e., -50 < degree < 50)
  if (direction == 1) {
    analogWrite(PWM_r, 0);
    analogWrite(PWM_f, controlSig);  // Move forward
  } else if (direction == -1) {
    analogWrite(PWM_f, 0);
    analogWrite(PWM_r, controlSig);  // Move reverse
  } else {
    // Stop motor if no FSR input
    analogWrite(PWM_f, 0);
    analogWrite(PWM_r, 0);
  }
}
  delay(1);   //effectively gives samplerate of 1 kHz
}
