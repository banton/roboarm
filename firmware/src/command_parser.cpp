#include "command_parser.h"

// Global instance
CommandParser commandParser;

CommandParser::CommandParser() {
}

CommandResult CommandParser::execute(const String& command) {
    String cmd = command;
    cmd.trim();

    if (cmd.length() == 0) {
        return CommandResult::ok();
    }

    #if DEBUG_COMMANDS
    DEBUG_PRINTF("CMD: %s\n", cmd.c_str());
    #endif

    // Quick status
    if (cmd == "?") {
        return CommandResult::ok(getQuickStatus());
    }

    // Get command code
    char cmdType = cmd.charAt(0);
    int cmdNum = -1;

    // Parse command number (e.g., "G0" -> 0, "M114" -> 114)
    int numStart = 1;
    int numEnd = 1;
    while (numEnd < cmd.length() && isDigit(cmd.charAt(numEnd))) {
        numEnd++;
    }

    if (numEnd > numStart) {
        cmdNum = cmd.substring(numStart, numEnd).toInt();
    }

    // Get arguments (everything after command code)
    String args = "";
    if (numEnd < cmd.length()) {
        args = cmd.substring(numEnd);
        args.trim();
    }

    // Dispatch command
    switch (cmdType) {
        case 'G':
        case 'g':
            switch (cmdNum) {
                case 0:  return handleG0(args);
                case 1:  return handleG1(args);
                case 28: return handleG28(args);
                default:
                    return CommandResult::error("Unknown G-code: G" + String(cmdNum));
            }
            break;

        case 'M':
        case 'm':
            switch (cmdNum) {
                case 17:  return handleM17();
                case 18:  return handleM18();
                case 112: return handleM112();
                case 114: return handleM114();
                case 503: return handleM503();
                default:
                    return CommandResult::error("Unknown M-code: M" + String(cmdNum));
            }
            break;

        default:
            return CommandResult::error("Unknown command: " + cmd);
    }
}

CommandResult CommandParser::handleG0(const String& args) {
    long positions[MOTOR_COUNT];
    for (int i = 0; i < MOTOR_COUNT; i++) {
        positions[i] = LONG_MIN;  // LONG_MIN means don't move
    }

    int parsed = parseJointPositions(args, positions);
    if (parsed < 0) {
        return CommandResult::error("Invalid joint format. Use: G0 J1:1000 J2:500");
    }

    if (parsed == 0) {
        return CommandResult::error("No joints specified");
    }

    if (!motors.moveToMultiple(positions)) {
        return CommandResult::error("Move failed - check limits or enable motors");
    }

    return CommandResult::ok();
}

CommandResult CommandParser::handleG1(const String& args) {
    long positions[MOTOR_COUNT];
    for (int i = 0; i < MOTOR_COUNT; i++) {
        positions[i] = LONG_MIN;
    }

    int parsed = parseJointPositions(args, positions);
    if (parsed < 0) {
        return CommandResult::error("Invalid joint format");
    }

    if (parsed == 0) {
        return CommandResult::error("No joints specified");
    }

    // Convert to absolute positions for relative move
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (positions[i] != LONG_MIN) {
            positions[i] = motors.getPosition(i) + positions[i];
        }
    }

    if (!motors.moveToMultiple(positions)) {
        return CommandResult::error("Move failed - check limits or enable motors");
    }

    return CommandResult::ok();
}

CommandResult CommandParser::handleG28(const String& args) {
    // For now, just set current position as zero
    // TODO: Implement actual homing with endstops
    motors.setZeroAll();
    return CommandResult::ok("All joints homed (zeroed)");
}

CommandResult CommandParser::handleM17() {
    motors.setEnabled(true);
    return CommandResult::ok("Motors enabled");
}

CommandResult CommandParser::handleM18() {
    motors.setEnabled(false);
    return CommandResult::ok("Motors disabled");
}

CommandResult CommandParser::handleM112() {
    motors.stopAll();
    motors.setEnabled(false);
    return CommandResult::ok("EMERGENCY STOP - Motors disabled");
}

CommandResult CommandParser::handleM114() {
    return CommandResult::ok(getPositionReport());
}

CommandResult CommandParser::handleM503() {
    return CommandResult::ok(getSettingsReport());
}

String CommandParser::getPositionReport() const {
    String report = "Position:";
    for (int i = 0; i < MOTOR_COUNT; i++) {
        report += " J" + String(i + 1) + ":" + String(motors.getPosition(i));
    }

    report += "\nTarget:";
    for (int i = 0; i < MOTOR_COUNT; i++) {
        report += " J" + String(i + 1) + ":" + String(motors.getTargetPosition(i));
    }

    report += "\nMoving: ";
    report += motors.isAnyMoving() ? "yes" : "no";

    report += "\nEnabled: ";
    report += motors.isEnabled() ? "yes" : "no";

    return report;
}

String CommandParser::getSettingsReport() const {
    String report = "Settings (FastAccelStepper):\n";

    for (int i = 0; i < MOTOR_COUNT; i++) {
        const MotorConfig& cfg = motors.getConfig(i);
        report += String(cfg.name) +
                  " Step:" + String(cfg.stepPin) +
                  " Dir:" + String(cfg.dirPin) +
                  " SPR:" + String(cfg.stepsPerRev) +
                  " uStep:" + String(cfg.microstepping) +
                  " MaxHz:" + String(cfg.maxSpeedHz) +
                  " Accel:" + String(cfg.acceleration) + "\n";
    }

    return report;
}

String CommandParser::getQuickStatus() const {
    String status = motors.isEnabled() ? "E" : "D";  // Enabled/Disabled
    status += motors.isAnyMoving() ? "M" : "I";      // Moving/Idle

    status += " P:";
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (i > 0) status += ",";
        status += String(motors.getPosition(i));
    }

    return status;
}

int CommandParser::parseJointPositions(const String& args, long positions[MOTOR_COUNT]) {
    int parsed = 0;
    String remaining = args;
    remaining.toUpperCase();

    while (remaining.length() > 0) {
        remaining.trim();

        // Find joint specifier (J1, J2, etc.)
        int jIdx = remaining.indexOf('J');
        if (jIdx < 0) break;

        // Get joint number
        int jointNum = 0;
        int colonIdx = remaining.indexOf(':', jIdx);
        if (colonIdx < 0) {
            return -1;  // Invalid format
        }

        jointNum = remaining.substring(jIdx + 1, colonIdx).toInt();
        if (jointNum < 1 || jointNum > MOTOR_COUNT) {
            return -1;  // Invalid joint number
        }

        // Find end of value (next space or end)
        int valueStart = colonIdx + 1;
        int valueEnd = remaining.indexOf(' ', valueStart);
        if (valueEnd < 0) {
            valueEnd = remaining.length();
        }

        // Parse value
        long value;
        if (!parseInt(remaining.substring(valueStart, valueEnd), value)) {
            return -1;
        }

        positions[jointNum - 1] = value;  // Convert to 0-indexed
        parsed++;

        // Move past this joint specifier
        remaining = remaining.substring(valueEnd);
    }

    return parsed;
}

bool CommandParser::parseInt(const String& str, long& value) {
    String s = str;
    s.trim();

    if (s.length() == 0) {
        return false;
    }

    // Check for valid integer format
    int start = 0;
    if (s.charAt(0) == '-' || s.charAt(0) == '+') {
        start = 1;
    }

    for (int i = start; i < s.length(); i++) {
        if (!isDigit(s.charAt(i))) {
            return false;
        }
    }

    value = s.toInt();
    return true;
}
