#include "motor_controller.h"

// Global instance
MotorController motors;

MotorController::MotorController() : _enabled(false) {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        _steppers[i] = nullptr;
    }
}

void MotorController::begin() {
    DEBUG_PRINTLN("MotorController: Initializing FastAccelStepper engine...");

    // Initialize the FastAccelStepper engine
    _engine.init();

    // Initialize enable pin
    pinMode(MOTORS_ENABLE_PIN, OUTPUT);
    digitalWrite(MOTORS_ENABLE_PIN, HIGH);  // Disable motors initially (active LOW)

    // Create and configure each stepper
    for (int i = 0; i < MOTOR_COUNT; i++) {
        const MotorConfig& cfg = MOTOR_CONFIGS[i];

        // Connect stepper to step pin (FastAccelStepper uses hardware peripherals)
        _steppers[i] = _engine.stepperConnectToPin(cfg.stepPin);

        if (_steppers[i]) {
            // Configure direction pin
            _steppers[i]->setDirectionPin(cfg.dirPin, cfg.invertDir);

            // Configure enable pin with auto-enable
            _steppers[i]->setEnablePin(cfg.enablePin);
            _steppers[i]->setAutoEnable(true);

            // Set motion parameters
            _steppers[i]->setSpeedInHz(cfg.maxSpeedHz);
            _steppers[i]->setAcceleration(cfg.acceleration);

            DEBUG_PRINTF("  %s: Step=%d, Dir=%d, Speed=%lu Hz, Accel=%lu\n",
                         cfg.name, cfg.stepPin, cfg.dirPin,
                         cfg.maxSpeedHz, cfg.acceleration);
        } else {
            DEBUG_PRINTF("  ERROR: Failed to connect %s on pin %d\n",
                         cfg.name, cfg.stepPin);
        }
    }

    _enabled = false;
    DEBUG_PRINTLN("MotorController: Ready (using FastAccelStepper hardware acceleration)");
}

void MotorController::setEnabled(bool enabled) {
    _enabled = enabled;
    // Enable pin is active LOW
    digitalWrite(MOTORS_ENABLE_PIN, enabled ? LOW : HIGH);

    if (enabled) {
        DEBUG_PRINTLN("Motors: ENABLED");
    } else {
        DEBUG_PRINTLN("Motors: DISABLED");
        // Stop all movement when disabling
        stopAll();
    }
}

bool MotorController::moveTo(uint8_t joint, long position) {
    if (!isValidJoint(joint) || !_steppers[joint]) {
        DEBUG_PRINTF("Motors: Invalid joint %d\n", joint);
        return false;
    }

    if (!_enabled) {
        DEBUG_PRINTLN("Motors: Cannot move - motors disabled");
        return false;
    }

    if (!isWithinLimits(joint, position)) {
        DEBUG_PRINTF("Motors: Position %ld out of limits for J%d\n", position, joint + 1);
        return false;
    }

    _steppers[joint]->moveTo(position);

    #if DEBUG_MOTORS
    DEBUG_PRINTF("Motors: %s -> %ld\n", MOTOR_CONFIGS[joint].name, position);
    #endif

    return true;
}

bool MotorController::moveRelative(uint8_t joint, long steps) {
    if (!isValidJoint(joint) || !_steppers[joint]) {
        return false;
    }

    long newPosition = _steppers[joint]->getCurrentPosition() + steps;
    return moveTo(joint, newPosition);
}

bool MotorController::moveToMultiple(const long positions[MOTOR_COUNT]) {
    if (!_enabled) {
        DEBUG_PRINTLN("Motors: Cannot move - motors disabled");
        return false;
    }

    bool allValid = true;

    // Validate all positions first
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (positions[i] != LONG_MIN && !isWithinLimits(i, positions[i])) {
            DEBUG_PRINTF("Motors: J%d position %ld out of limits\n", i + 1, positions[i]);
            allValid = false;
        }
    }

    if (!allValid) {
        return false;
    }

    // Apply all movements (FastAccelStepper starts them near-simultaneously)
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (positions[i] != LONG_MIN && _steppers[i]) {
            _steppers[i]->moveTo(positions[i]);

            #if DEBUG_MOTORS
            DEBUG_PRINTF("Motors: %s -> %ld\n", MOTOR_CONFIGS[i].name, positions[i]);
            #endif
        }
    }

    return true;
}

void MotorController::stop(uint8_t joint) {
    if (isValidJoint(joint) && _steppers[joint]) {
        _steppers[joint]->forceStop();
        DEBUG_PRINTF("Motors: %s STOPPED\n", MOTOR_CONFIGS[joint].name);
    }
}

void MotorController::stopAll() {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (_steppers[i]) {
            _steppers[i]->forceStop();
        }
    }
    DEBUG_PRINTLN("Motors: ALL STOPPED (emergency)");
}

bool MotorController::isMoving(uint8_t joint) const {
    if (!isValidJoint(joint) || !_steppers[joint]) {
        return false;
    }
    return _steppers[joint]->isRunning();
}

bool MotorController::isAnyMoving() const {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (_steppers[i] && _steppers[i]->isRunning()) {
            return true;
        }
    }
    return false;
}

long MotorController::getPosition(uint8_t joint) const {
    if (!isValidJoint(joint) || !_steppers[joint]) {
        return 0;
    }
    return _steppers[joint]->getCurrentPosition();
}

long MotorController::getTargetPosition(uint8_t joint) const {
    if (!isValidJoint(joint) || !_steppers[joint]) {
        return 0;
    }
    return _steppers[joint]->targetPos();
}

long MotorController::getDistanceToGo(uint8_t joint) const {
    if (!isValidJoint(joint) || !_steppers[joint]) {
        return 0;
    }
    return _steppers[joint]->targetPos() - _steppers[joint]->getCurrentPosition();
}

void MotorController::setZero(uint8_t joint) {
    if (isValidJoint(joint) && _steppers[joint]) {
        _steppers[joint]->setCurrentPosition(0);
        DEBUG_PRINTF("Motors: %s zeroed\n", MOTOR_CONFIGS[joint].name);
    }
}

void MotorController::setZeroAll() {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (_steppers[i]) {
            _steppers[i]->setCurrentPosition(0);
        }
    }
    DEBUG_PRINTLN("Motors: All joints zeroed");
}

void MotorController::setMaxSpeed(uint8_t joint, uint32_t speedHz) {
    if (isValidJoint(joint) && _steppers[joint]) {
        // Clamp to safety limit
        if (speedHz > MAX_SPEED_HZ) {
            speedHz = MAX_SPEED_HZ;
        }
        _steppers[joint]->setSpeedInHz(speedHz);
    }
}

void MotorController::setAcceleration(uint8_t joint, uint32_t acceleration) {
    if (isValidJoint(joint) && _steppers[joint]) {
        _steppers[joint]->setAcceleration(acceleration);
    }
}

const MotorConfig& MotorController::getConfig(uint8_t joint) const {
    // Return first config if invalid (shouldn't happen)
    if (!isValidJoint(joint)) {
        return MOTOR_CONFIGS[0];
    }
    return MOTOR_CONFIGS[joint];
}

FastAccelStepper* MotorController::getStepper(uint8_t joint) {
    if (!isValidJoint(joint)) {
        return nullptr;
    }
    return _steppers[joint];
}

bool MotorController::isWithinLimits(uint8_t joint, long position) const {
    if (!isValidJoint(joint)) {
        return false;
    }
    return position >= POSITION_LIMITS_MIN[joint] &&
           position <= POSITION_LIMITS_MAX[joint];
}
