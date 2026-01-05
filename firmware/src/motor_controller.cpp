#include "motor_controller.h"

// Global instance
MotorController motors;

MotorController::MotorController() : _enabled(false) {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        _steppers[i] = nullptr;
    }
}

void MotorController::begin() {
    DEBUG_PRINTLN("MotorController: Initializing...");

    // Initialize enable pin
    pinMode(MOTORS_ENABLE_PIN, OUTPUT);
    digitalWrite(MOTORS_ENABLE_PIN, HIGH);  // Disable motors initially (active LOW)

    // Create and configure each stepper
    for (int i = 0; i < MOTOR_COUNT; i++) {
        const MotorConfig& cfg = MOTOR_CONFIGS[i];

        // Create stepper with DRIVER interface (step + direction pins)
        _steppers[i] = new AccelStepper(AccelStepper::DRIVER, cfg.stepPin, cfg.dirPin);

        // Configure stepper
        _steppers[i]->setMaxSpeed(cfg.maxSpeed);
        _steppers[i]->setAcceleration(cfg.acceleration);
        _steppers[i]->setCurrentPosition(0);

        // Set direction inversion if configured
        _steppers[i]->setPinsInverted(cfg.invertDir, false, false);

        DEBUG_PRINTF("  %s: Step=%d, Dir=%d, Speed=%.0f, Accel=%.0f\n",
                     cfg.name, cfg.stepPin, cfg.dirPin, cfg.maxSpeed, cfg.acceleration);
    }

    _enabled = false;
    DEBUG_PRINTLN("MotorController: Ready");
}

bool MotorController::run() {
    if (!_enabled) {
        return false;
    }

    bool anyMoving = false;
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (_steppers[i]->run()) {
            anyMoving = true;
        }
    }
    return anyMoving;
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
    if (!isValidJoint(joint)) {
        DEBUG_PRINTF("Motors: Invalid joint %d\n", joint);
        return false;
    }

    if (!_enabled) {
        DEBUG_PRINTLN("Motors: Cannot move - motors disabled");
        return false;
    }

    if (!isWithinLimits(position)) {
        DEBUG_PRINTF("Motors: Position %ld out of limits\n", position);
        return false;
    }

    _steppers[joint]->moveTo(position);

    #if DEBUG_MOTORS
    DEBUG_PRINTF("Motors: %s -> %ld\n", MOTOR_CONFIGS[joint].name, position);
    #endif

    return true;
}

bool MotorController::moveRelative(uint8_t joint, long steps) {
    if (!isValidJoint(joint)) {
        return false;
    }

    long newPosition = _steppers[joint]->currentPosition() + steps;
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
        if (positions[i] != -1 && !isWithinLimits(positions[i])) {
            DEBUG_PRINTF("Motors: J%d position %ld out of limits\n", i + 1, positions[i]);
            allValid = false;
        }
    }

    if (!allValid) {
        return false;
    }

    // Apply all movements
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (positions[i] != -1) {
            _steppers[i]->moveTo(positions[i]);

            #if DEBUG_MOTORS
            DEBUG_PRINTF("Motors: %s -> %ld\n", MOTOR_CONFIGS[i].name, positions[i]);
            #endif
        }
    }

    return true;
}

void MotorController::stop(uint8_t joint) {
    if (isValidJoint(joint)) {
        _steppers[joint]->stop();
        DEBUG_PRINTF("Motors: %s STOPPED\n", MOTOR_CONFIGS[joint].name);
    }
}

void MotorController::stopAll() {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        _steppers[i]->stop();
    }
    DEBUG_PRINTLN("Motors: ALL STOPPED");
}

bool MotorController::isMoving(uint8_t joint) const {
    if (!isValidJoint(joint)) {
        return false;
    }
    return _steppers[joint]->distanceToGo() != 0;
}

bool MotorController::isAnyMoving() const {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        if (_steppers[i]->distanceToGo() != 0) {
            return true;
        }
    }
    return false;
}

long MotorController::getPosition(uint8_t joint) const {
    if (!isValidJoint(joint)) {
        return 0;
    }
    return _steppers[joint]->currentPosition();
}

long MotorController::getTargetPosition(uint8_t joint) const {
    if (!isValidJoint(joint)) {
        return 0;
    }
    return _steppers[joint]->targetPosition();
}

long MotorController::getDistanceToGo(uint8_t joint) const {
    if (!isValidJoint(joint)) {
        return 0;
    }
    return _steppers[joint]->distanceToGo();
}

void MotorController::setZero(uint8_t joint) {
    if (isValidJoint(joint)) {
        _steppers[joint]->setCurrentPosition(0);
        DEBUG_PRINTF("Motors: %s zeroed\n", MOTOR_CONFIGS[joint].name);
    }
}

void MotorController::setZeroAll() {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        _steppers[i]->setCurrentPosition(0);
    }
    DEBUG_PRINTLN("Motors: All joints zeroed");
}

void MotorController::setMaxSpeed(uint8_t joint, float speed) {
    if (isValidJoint(joint)) {
        _steppers[joint]->setMaxSpeed(speed);
    }
}

void MotorController::setAcceleration(uint8_t joint, float acceleration) {
    if (isValidJoint(joint)) {
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

AccelStepper* MotorController::getStepper(uint8_t joint) {
    if (!isValidJoint(joint)) {
        return nullptr;
    }
    return _steppers[joint];
}

bool MotorController::isWithinLimits(long position) const {
    #if MAX_POSITION_STEPS != 0
    return position >= MIN_POSITION_STEPS && position <= MAX_POSITION_STEPS;
    #else
    return true;  // No limits configured
    #endif
}
