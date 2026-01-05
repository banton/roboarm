/**
 * Roboarm - 6-axis Robotic Arm Controller
 *
 * Main entry point for ESP-32 firmware.
 * Handles:
 *   - WiFi connection and web server
 *   - Serial command interface
 *   - Motor control loop
 */

#include <Arduino.h>
#include "config.h"
#include "motor_controller.h"
#include "command_parser.h"
#include "web_server.h"

// Serial input buffer
String serialBuffer = "";
bool serialComplete = false;

// Forward declarations
void handleSerialInput();
void handleStatusLED();

// Status LED timing
unsigned long lastStatusBlink = 0;
const unsigned long STATUS_BLINK_INTERVAL = 1000;

void setup() {
    // Initialize serial
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 3000) {
        // Wait up to 3 seconds for serial
    }

    Serial.println();
    Serial.println("=================================");
    Serial.println("  Roboarm Controller v1.0");
    Serial.println("  6-axis Robotic Arm");
    Serial.println("=================================");
    Serial.println();

    // Initialize motor controller
    motors.begin();

    // Initialize WiFi and web server
    Serial.println("Connecting to WiFi...");
    if (webServer.begin(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.print("WiFi connected! IP: ");
        Serial.println(webServer.getIPAddress());
        Serial.print("Web UI: http://");
        Serial.println(webServer.getIPAddress());
    } else {
        Serial.println("WiFi connection failed!");
        Serial.println("Serial-only mode active");
    }

    Serial.println();
    Serial.println("Ready. Type '?' for status or 'M17' to enable motors.");
    Serial.println("Commands: G0, G1, G28, M17, M18, M112, M114, M503");
    Serial.println();
}

void loop() {
    // FastAccelStepper uses hardware timers - no run() needed
    // Motors are controlled automatically via MCPWM/RMT peripherals

    // Handle serial input
    handleSerialInput();

    // Handle web server
    webServer.loop();

    // Status indicator (optional)
    handleStatusLED();
}

/**
 * Handle incoming serial commands
 * Accumulates characters until newline, then executes
 */
void handleSerialInput() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (serialBuffer.length() > 0) {
                // Execute command
                CommandResult result = commandParser.execute(serialBuffer);

                // Print result
                Serial.println(result.message);

                // Clear buffer
                serialBuffer = "";
            }
        } else {
            // Accumulate character
            serialBuffer += c;

            // Prevent buffer overflow
            if (serialBuffer.length() > 256) {
                Serial.println("error: Command too long");
                serialBuffer = "";
            }
        }
    }
}

/**
 * Blink built-in LED to show status
 * Fast blink = moving
 * Slow blink = idle + enabled
 * Off = disabled
 */
void handleStatusLED() {
    #ifdef LED_BUILTIN
    unsigned long now = millis();

    if (!motors.isEnabled()) {
        // Disabled - LED off
        digitalWrite(LED_BUILTIN, LOW);
        return;
    }

    unsigned long interval = motors.isAnyMoving() ? 100 : STATUS_BLINK_INTERVAL;

    if (now - lastStatusBlink >= interval) {
        lastStatusBlink = now;
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    #endif
}
