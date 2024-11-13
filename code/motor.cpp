#include "motor.h"

Motor::Motor(int pin, int minUs, int maxUs)
    : pin(pin), currentPosition(0), minMicroseconds(minUs),
      maxMicroseconds(maxUs) {}

void Motor::attach() { servo.attach(pin, minMicroseconds, maxMicroseconds); }

void Motor::detach() { servo.detach(); }

void Motor::setPosition(int degrees, int step_delay) {
  moveToPosition(degrees, step_delay);
}

int Motor::getPosition() const { return currentPosition; }

void Motor::moveToPosition(int targetPosition, int step_delay) {
  if (step_delay <= 0) {
    servo.writeMicroseconds(degreesToMicroseconds(targetPosition));
    currentPosition = targetPosition;
    return;
  }

  if (targetPosition == currentPosition) {
    return; // No movement needed
  }

  // int stepDelay = speed / abs(targetPosition - currentPosition);
  int step = (targetPosition > currentPosition) ? 1 : -1;

  for (int pos = currentPosition; pos != targetPosition; pos += step) {
    servo.writeMicroseconds(degreesToMicroseconds(pos));
    delay(step_delay);
  }

  currentPosition = targetPosition;
}

int Motor::degreesToMicroseconds(int degrees) const {
  // Map degrees to microseconds based on the provided min and max values
  int us = map(degrees, 0, 180, minMicroseconds, maxMicroseconds);
  return constrain(us, minMicroseconds,
                   maxMicroseconds); // Ensure within bounds
}
