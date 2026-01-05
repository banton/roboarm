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
    uint8_t microstepping;
    uint32_t maxSpeedHz;     // steps per second (FastAccelStepper uses Hz)
    uint32_t acceleration;   // steps per second^2
    bool invertDir;          // Invert direction
    const char* name;        // Joint name for debugging
};

// =============================================================================
// Pin Assignments
// =============================================================================
// Shared enable pin for all drivers (active LOW)
#define MOTORS_ENABLE_PIN 4

// Motor configurations using validated safe GPIO pins
// Based on ESP32 research: GPIO 6-11 are flash, 34-39 are input-only
// Safe pins: 4, 13, 14, 16-19, 21-23, 25-27, 32-33
const MotorConfig MOTOR_CONFIGS[MOTOR_COUNT] = {
    // stepPin, dirPin, enablePin, stepsPerRev, microstepping, maxSpeedHz, accel, invertDir, name
    {16, 17, MOTORS_ENABLE_PIN, 200, 16, 50000, 10000, false, "J1-Base"},
    {18, 19, MOTORS_ENABLE_PIN, 200, 16, 50000, 10000, false, "J2-Shoulder"},
    {21, 22, MOTORS_ENABLE_PIN, 200, 16, 50000, 10000, false, "J3-Elbow"},
    {23, 25, MOTORS_ENABLE_PIN, 200, 16, 50000, 10000, false, "J4-WristPitch"},
    {26, 27, MOTORS_ENABLE_PIN, 200, 16, 50000, 10000, false, "J5-WristRoll"},
    {32, 33, MOTORS_ENABLE_PIN, 200, 16, 50000, 10000, false, "J6-Gripper"},
};

// Calculated full revolution steps (with microstepping)
inline uint32_t getFullRevolution(uint8_t motor) {
    if (motor >= MOTOR_COUNT) return 3200;
    return MOTOR_CONFIGS[motor].stepsPerRev * MOTOR_CONFIGS[motor].microstepping;
}

// =============================================================================
// Default Motion Parameters
// =============================================================================
#define DEFAULT_SPEED_HZ 1000      // steps/sec
#define DEFAULT_ACCEL 500          // steps/sec²
#define MAX_SPEED_HZ 50000         // safety limit

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

// Per-joint position limits (in steps)
const int32_t POSITION_LIMITS_MIN[MOTOR_COUNT] = {-100000, -50000, -50000, -25000, -25000, -10000};
const int32_t POSITION_LIMITS_MAX[MOTOR_COUNT] = { 100000,  50000,  50000,  25000,  25000,  10000};

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
