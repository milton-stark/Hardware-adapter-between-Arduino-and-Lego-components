#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// ---------- BNO055 (Hardware I2C) ----------
Adafruit_BNO055 bno = Adafruit_BNO055(55);
float refPitch = 0;
float Kp = 1.2, Ki = 0.0, Kd = 0.2;
float integral = 0, previous_error = 0;

// ---------- Motor Pins ----------
#define ENA 10
#define IN1 28
#define IN2 30
#define ENB 11
#define IN3 29
#define IN4 31
bool motorDirection = true;
bool motorEnabled = false;  // Controlled via Serial commands

// ---------- Touch Sensor ----------
const int touchSensorPin = 48;

// ---------- Light Sensor ----------
const int lightSensorPin = A0;
const int ledControlPin = 36;

void setup() {
  Serial.begin(9600);

  // BNO055 initialization
  if (!bno.begin()) {
    Serial.println("BNO055 not detected! Check wiring.");
    while (1);
  }
  bno.setExtCrystalUse(true);
  
  Serial.println("--------------------------------------");
  Serial.println("CALIBRATION MODE STARTED");
  Serial.println("Please keep the robot STATIONARY.");
  Serial.println("Waiting for Gyro Calibration = 3...");
  Serial.println("--------------------------------------");

  uint8_t sys, gyro, accel, mag = 0;
  
  // BLOCKING LOOP: This will not exit until Gyro == 3
  while (gyro != 3) {
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    
    Serial.print("System:"); Serial.print(sys);
    Serial.print(" Gyro:"); Serial.print(gyro);
    Serial.print(" Accel:"); Serial.print(accel);
    Serial.print(" Mag:"); Serial.println(mag);

    if (gyro == 3) {
      Serial.println();
      Serial.println("gyro calibration=3"); // Explicit confirmation requested
      Serial.println("Calibration Complete.");
      
      // Capture the reference pitch immediately after calibration
      sensors_event_t e;
      bno.getEvent(&e);
      refPitch = e.orientation.y;
      
      Serial.print("Reference Pitch Set to: ");
      Serial.println(refPitch);
      Serial.println("Pitch,Speed"); // Header for plotter
      delay(1000); // Brief pause to read message
    } else {
      delay(500); // Check status every 500ms
    }
  }

  // Pin setup
  pinMode(touchSensorPin, INPUT_PULLUP);
  pinMode(ledControlPin, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  Serial.println("System Ready. Type 'start' to begin.");
}

void loop() {
  // Serial Command Handling
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "start") {
      motorEnabled = true;
      Serial.println("Motors ENABLED.");
    } else if (cmd == "stop") {
      motorEnabled = false;
      setMotors(false, 0);
      Serial.println("Motors DISABLED.");
    }
  }

  // Touch sensor reverses direction
  if (digitalRead(touchSensorPin) == LOW) {
    motorDirection = !motorDirection;
    Serial.println("Touch sensor pressed. Reversing direction.");
    delay(200);
  }

  // Read pitch from BNO055
  sensors_event_t event;
  bno.getEvent(&event);
  float pitch = event.orientation.y;
  float error = pitch - refPitch;

  // PID calculation
  integral += error;
  float derivative = error - previous_error;
  float output = Kp * error + Ki * integral + Kd * derivative;
  previous_error = error;

  // PID Speed Calculation
  int pwmSpeed = constrain(150 - abs(output) * 2, 80, 255);
  if (motorEnabled) setMotors(motorDirection, pwmSpeed);

  // Light sensor readings
  digitalWrite(ledControlPin, HIGH); delay(100);
  int reflected = analogRead(lightSensorPin);
  digitalWrite(ledControlPin, LOW); delay(100);
  int ambient = analogRead(lightSensorPin);

  // ---------- Serial Plotter Output ----------
  Serial.print(pitch); 
  Serial.print(",");
  Serial.println(pwmSpeed); 

  delay(300);
}

void setMotors(bool forward, int speed) {
  // Motor A
  digitalWrite(IN1, forward ? HIGH : LOW);
  digitalWrite(IN2, forward ? LOW : HIGH);
  analogWrite(ENA, speed);

  // Motor B
  digitalWrite(IN3, forward ? HIGH : LOW);
  digitalWrite(IN4, forward ? LOW : HIGH);
  analogWrite(ENB, speed);
}