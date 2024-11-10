#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>
#include <AccelStepper.h>
#include <math.h>

// Custom mapping function for floats
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Constants for stepper motor movement
const int stepsPerRevolution = 4096;                                     // Steps per revolution for 28BYJ-48
const int irisChangeTime = 500;                                          // Time (ms) to fully open/close the iris (adjusted)
const int baseMotorSpeed = stepsPerRevolution / (irisChangeTime / 1000); // Base speed in steps per second

// Light thresholds for fully open and fully closed iris
const float lightThresholdClose = 1.0;    // Lux value for fully closed iris
const float lightThresholdOpen = 10000.0; // Lux value for fully open iris

// Initialize light sensor and stepper motor
BH1750 lightMeter;                                          // Light sensor at I2C address 0x23
AccelStepper stepper(AccelStepper::FULL4WIRE, 4, 5, 6, 16); // Stepper motor connected to pins 3, 5, 4, 6

// Define iris position limits
const int minIrisPosition = 0;                      // Fully closed position
const int maxIrisPosition = stepsPerRevolution / 4; // Fully open position (quarter revolution)

// Variables for light smoothing and hysteresis
float smoothedLux = 0;           // Smoothed lux value
const float alphaMin = 0.5;      // Minimum smoothing factor
const float alphaMax = 1.0;      // Maximum smoothing factor
float adaptiveHysteresis = 50.0; // Hysteresis value to prevent frequent adjustments

// Timing variables
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 120;

// Variables for plotting
float lux = 0;                  // Current lux reading
float luxChange = 0;            // Change in lux
float adjustedSpeed = 0;        // Adjusted motor speed
float adjustedAcceleration = 0; // Adjusted motor acceleration
int targetPosition = 0;         // Target position for the stepper motor

void setup()
{
  Wire.begin(10, 11);                                                // Initialize I2C communication
  Serial.begin(115200);                                              // Initialize serial communication for Teleplot
  stepper.setMaxSpeed(500.0);                                        // Set initial maximum speed (safe value)
  stepper.setAcceleration(200.0);                                    // Set initial acceleration (safe value)
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2, 0x23, &Wire); // Start light sensor in continuous mode

  smoothedLux = lightMeter.readLightLevel(); // Initialize smoothedLux
  stepper.moveTo(minIrisPosition);           // Move iris to fully closed position
}

void updateSmoothedLux(float newLux)
{
  luxChange = abs(newLux - smoothedLux); // Calculate change in lux

  // Adjust alpha dynamically with a minimum value
  float dynamicAlpha = mapFloat(luxChange, 0.0, lightThresholdOpen, alphaMin, alphaMax);
  dynamicAlpha = constrain(dynamicAlpha, alphaMin, alphaMax);
  smoothedLux = dynamicAlpha * newLux + (1 - dynamicAlpha) * smoothedLux;
}

int getLogarithmicPosition(float luxValue)
{
  float logLux = log10(luxValue);                                                    // Convert lux to logarithmic scale
  logLux = constrain(logLux, log10(lightThresholdClose), log10(lightThresholdOpen)); // Constrain value
  return (int)mapFloat(logLux, log10(lightThresholdClose), log10(lightThresholdOpen), minIrisPosition, maxIrisPosition);
}

void adjustMotorSpeedAndAcceleration()
{
  // Set safe speed and acceleration limits for the 28BYJ-48 motor
  const float minSpeed = 200.0;        // Minimum motor speed (steps per second)
  const float maxSpeed = 500.0;        // Maximum motor speed (steps per second)
  const float minAcceleration = 100.0; // Minimum motor acceleration (steps per second squared)
  const float maxAcceleration = 200.0; // Maximum motor acceleration (steps per second squared)

  // Adjust the speed based on light change
  adjustedSpeed = mapFloat(luxChange, 0.0, lightThresholdOpen, minSpeed, maxSpeed);
  adjustedSpeed = constrain(adjustedSpeed, minSpeed, maxSpeed);
  stepper.setMaxSpeed(adjustedSpeed);

  // Adjust the acceleration
  adjustedAcceleration = mapFloat(luxChange, 0.0, lightThresholdOpen, minAcceleration, maxAcceleration);
  adjustedAcceleration = constrain(adjustedAcceleration, minAcceleration, maxAcceleration);
  stepper.setAcceleration(adjustedAcceleration);
}

void checkAndMoveIris()
{
  lux = lightMeter.readLightLevel(); // Read current light level
  updateSmoothedLux(lux);            // Smooth the lux value
  adjustMotorSpeedAndAcceleration(); // Adjust motor parameters accordingly

  // Adjust hysteresis based on smoothedLux
  adaptiveHysteresis = mapFloat(smoothedLux, lightThresholdClose, lightThresholdOpen, 5.0, 50.0);
  adaptiveHysteresis = constrain(adaptiveHysteresis, 5.0, 50.0);

  if (smoothedLux < lightThresholdClose - adaptiveHysteresis)
  {
    targetPosition = minIrisPosition; // Close the iris completely
  }
  else if (smoothedLux > lightThresholdOpen + adaptiveHysteresis)
  {
    targetPosition = maxIrisPosition; // Open the iris completely
  }
  else
  {
    targetPosition = getLogarithmicPosition(smoothedLux); // Calculate intermediate position
  }
  stepper.moveTo(targetPosition); // Move iris to target position

  // Send data to Teleplot
  Serial.print(">lux:");
  Serial.println(lux); // Current lux reading
  // Serial.print(">smoothedLux:");
  // Serial.println(smoothedLux); // Smoothed lux value
  // Serial.print(">luxChange:");
  // Serial.println(luxChange); // Change in lux
  // Serial.print(">adjustedSpeed:");
  // Serial.println(adjustedSpeed); // Adjusted motor speed
  // Serial.print(">adjustedAcceleration:");
  // Serial.println(adjustedAcceleration); // Adjusted motor acceleration
  // Serial.print(">targetPosition:");
  // Serial.println(targetPosition); // Target position
  // Serial.print(">currentPosition:");
  // Serial.println(stepper.currentPosition()); // Current motor position
}

void loop()
{
  stepper.run(); // Move the motor towards the set position

  if (millis() - lastUpdate > updateInterval) // Check if it's time to read the light sensor
  {
    lastUpdate = millis();
    checkAndMoveIris(); // Read light and adjust iris position
  }
}
