#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>
#include <AccelStepper.h>
#include <math.h>

// Wi-Fi Credentials
const char *ssid = "iPhone von Tomi";
const char *password = "12345678";

// Server instance
AsyncWebServer server(80);

// Initialize light sensor and stepper motor
BH1750 lightMeter;
AccelStepper stepper(AccelStepper::FULL4WIRE, 4, 16, 5, 6);

// Define constants and variables (same as before)
const int stepsPerRevolution = 2048;
const int irisChangeTime = 500;
const int baseMotorSpeed = stepsPerRevolution / (irisChangeTime / 1000);
const float lightThresholdClose = 1.0;
const float lightThresholdOpen = 10000.0;
const int minIrisPosition = 0;
const int maxIrisPosition = stepsPerRevolution / 4;
float smoothedLux = 0;
const float alphaMin = 0.5;
const float alphaMax = 1.0;
float adaptiveHysteresis = 50.0;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 120;
float lux = 0;
float luxChange = 0;
float adjustedSpeed = 0;
float adjustedAcceleration = 0;
int targetPosition = 0;

void setup()
{
  // Initialize Serial and I2C communication
  Serial.begin(115200);
  Wire.begin(10, 11);

  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  Serial.println(WiFi.localIP());

  // Initialize devices
  stepper.setMaxSpeed(500.0);
  stepper.setAcceleration(200.0);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2, 0x23, &Wire);

  // Initialize variables
  smoothedLux = lightMeter.readLightLevel();
  stepper.moveTo(minIrisPosition);

  // Define server route for /data to serve JSON
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Create JSON document
    JsonDocument jsonDoc;
    jsonDoc["lux"] = lux;
    jsonDoc["smoothedLux"] = smoothedLux;
    jsonDoc["luxChange"] = luxChange;
    jsonDoc["adjustedSpeed"] = adjustedSpeed;
    jsonDoc["adjustedAcceleration"] = adjustedAcceleration;
    jsonDoc["targetPosition"] = targetPosition;
    jsonDoc["currentPosition"] = stepper.currentPosition();

    String jsonString;
    serializeJson(jsonDoc, jsonString);
    
    // Send JSON response
    request->send(200, "application/json", jsonString); });

  // Start server
  server.begin();
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void updateSmoothedLux(float newLux)
{
  luxChange = abs(newLux - smoothedLux);
  float dynamicAlpha = mapFloat(luxChange, 0.0, lightThresholdOpen, alphaMin, alphaMax);
  dynamicAlpha = constrain(dynamicAlpha, alphaMin, alphaMax);
  smoothedLux = dynamicAlpha * newLux + (1 - dynamicAlpha) * smoothedLux;
}

int getLogarithmicPosition(float luxValue)
{
  float logLux = log10(luxValue);
  logLux = constrain(logLux, log10(lightThresholdClose), log10(lightThresholdOpen));
  return (int)mapFloat(logLux, log10(lightThresholdClose), log10(lightThresholdOpen), minIrisPosition, maxIrisPosition);
}

void adjustMotorSpeedAndAcceleration()
{
  const float minSpeed = 200.0;
  const float maxSpeed = 500.0;
  const float minAcceleration = 100.0;
  const float maxAcceleration = 200.0;

  adjustedSpeed = mapFloat(luxChange, 0.0, lightThresholdOpen, minSpeed, maxSpeed);
  adjustedSpeed = constrain(adjustedSpeed, minSpeed, maxSpeed);
  stepper.setMaxSpeed(adjustedSpeed);

  adjustedAcceleration = mapFloat(luxChange, 0.0, lightThresholdOpen, minAcceleration, maxAcceleration);
  adjustedAcceleration = constrain(adjustedAcceleration, minAcceleration, maxAcceleration);
  stepper.setAcceleration(adjustedAcceleration);
}

void checkAndMoveIris()
{
  lux = lightMeter.readLightLevel();
  updateSmoothedLux(lux);
  adjustMotorSpeedAndAcceleration();

  adaptiveHysteresis = mapFloat(smoothedLux, lightThresholdClose, lightThresholdOpen, 5.0, 50.0);
  adaptiveHysteresis = constrain(adaptiveHysteresis, 5.0, 50.0);

  if (smoothedLux < lightThresholdClose - adaptiveHysteresis)
  {
    targetPosition = minIrisPosition;
  }
  else if (smoothedLux > lightThresholdOpen + adaptiveHysteresis)
  {
    targetPosition = maxIrisPosition;
  }
  else
  {
    targetPosition = getLogarithmicPosition(smoothedLux);
  }
  stepper.moveTo(targetPosition);
}

void loop()
{
  stepper.run();

  if (millis() - lastUpdate > updateInterval)
  {
    lastUpdate = millis();
    checkAndMoveIris();
  }
}
