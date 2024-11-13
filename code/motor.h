#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <Servo.h>

class Motor {
public:
  Motor(int pin, int minUs = 500,
        int maxUs = 2400); // Added min and max microseconds
  void attach();
  void detach();
  void setPosition(int degrees, int speed = 100); // speed in milliseconds
  int getPosition() const;

private:
  int pin;
  Servo servo;
  int currentPosition;
  int minMicroseconds; // Minimum microseconds
  int maxMicroseconds; // Maximum microseconds
  void moveToPosition(int targetPosition, int speed);
  int degreesToMicroseconds(
      int degrees) const; // Convert degrees to microseconds
};

#endif // SERVO_H
