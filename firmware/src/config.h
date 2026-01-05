#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// WiFi Configuration
// =============================================================================
// TODO: Move to secrets.h or use WiFiManager for production
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_HOSTNAME "roboarm"

// =============================================================================
// Serial Configuration
// =============================================================================
#define SERIAL_BAUD_RATE 115200

// =============================================================================
// Motor Configuration
// =============================================================================
#define MOTOR_COUNT 6

// Motor indices
#define JOINT_1 0  // Base rotation
#define JOINT_2 1  // Shoulder
#define JOINT_3 2  // Elbow
#define JOINT_4 3  // Wrist pitch
#define JOINT_5 4  // Wrist roll
#define JOINT_6 5  // End effector / Gripper

// Motor configuration structure
struct MotorConfig {
    uint8_t stepPin;
    uint8_t dirPin;
    uint8_t enablePin;
    uint16_t stepsPerRev;
    float maxSpeed;        // steps per second
    float acceleration;    // steps per second^2
    bool invertDir;        // Invert direction
    const char* name;      // Joint name for debugging
};

// =============================================================================
// Pin Assignments
// =============================================================================
// Shared enable pin for all drivers (active LOW)
#define MOTORS_ENABLE_PIN 15

// Default motor configurations
// Adjust pins based on your actual wiring!
const MotorConfig MOTOR_CONFIGS[MOTOR_COUNT] = {
    // stepPin, dirPin, enablePin, stepsPerRev, maxSpeed, accel, invertDir, name
    {2,  4,  MOTORS_ENABLE_PIN, 200, 1000.0f, 500.0f, false, "J1-Base"},
    {16, 17, MOTORS_ENABLE_PIN, 200, 1000.0f, 500.0f, false, "J2-Shoulder"},
    {18, 19, MOTORS_ENABLE_PIN, 200, 1000.0f, 500.0f, false, "J3-Elbow"},
    {21, 22, MOTORS_ENABLE_PIN, 200, 1000.0f, 500.0f, false, "J4-WristPitch"},
    {23, 25, MOTORS_ENABLE_PIN, 200, 1000.0f, 500.0f, false, "J5-WristRoll"},
    {26, 27, MOTORS_ENABLE_PIN, 200, 1000.0f, 500.0f, false, "J6-Gripper"},
};

// =============================================================================
// Web Server Configuration
// =============================================================================
#define WEB_SERVER_PORT 80

// =============================================================================
// Safety Limits
// =============================================================================
// Maximum steps from home position (soft limits)
// Set to 0 to disable limit checking
#define MAX_POSITION_STEPS 100000
#define MIN_POSITION_STEPS -100000

// Emergency stop pin (optional, pull LOW to stop)
// #define ESTOP_PIN 34

// =============================================================================
// Debug Configuration
// =============================================================================
#define DEBUG_SERIAL true
#define DEBUG_COMMANDS true
#define DEBUG_MOTORS false

#if DEBUG_SERIAL
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
