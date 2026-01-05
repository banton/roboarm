#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>
#include "config.h"
#include "motor_controller.h"

/**
 * G-code style command parser for robotic arm control
 *
 * Supported commands:
 *   G0 J1:1000 J2:500    - Move joints to absolute positions
 *   G1 J1:100            - Move joints relative to current position
 *   G28                  - Home all axes (sets current position as zero)
 *   M17                  - Enable steppers
 *   M18                  - Disable steppers
 *   M112                 - Emergency stop
 *   M114                 - Report current positions
 *   M503                 - Report settings
 *   ?                    - Quick status
 */

// Command result structure
struct CommandResult {
    bool success;
    String message;

    static CommandResult ok(const String& msg = "ok") {
        return {true, msg};
    }

    static CommandResult error(const String& msg) {
        return {false, "error: " + msg};
    }
};

class CommandParser {
public:
    CommandParser();

    /**
     * Parse and execute a command string
     * @param command The command to parse (e.g., "G0 J1:1000")
     * @return Result with success/error and message
     */
    CommandResult execute(const String& command);

    /**
     * Get status as a string (for M114)
     */
    String getPositionReport() const;

    /**
     * Get settings as a string (for M503)
     */
    String getSettingsReport() const;

    /**
     * Get quick status (for ?)
     */
    String getQuickStatus() const;

private:
    // Command handlers
    CommandResult handleG0(const String& args);   // Move absolute
    CommandResult handleG1(const String& args);   // Move relative
    CommandResult handleG28(const String& args);  // Home
    CommandResult handleM17();                    // Enable
    CommandResult handleM18();                    // Disable
    CommandResult handleM112();                   // Emergency stop
    CommandResult handleM114();                   // Position report
    CommandResult handleM503();                   // Settings report

    // Parse joint positions from args string
    // Format: "J1:1000 J2:500" -> fills positions array
    // Returns number of joints parsed, -1 on error
    int parseJointPositions(const String& args, long positions[MOTOR_COUNT]);

    // Parse a single integer value from a string
    bool parseInt(const String& str, long& value);
};

// Global command parser instance
extern CommandParser commandParser;

#endif // COMMAND_PARSER_H
