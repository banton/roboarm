#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "config.h"

/**
 * Motor Controller for 6-axis robotic arm
 *
 * Uses FastAccelStepper library for hardware-accelerated pulse generation.
 * FastAccelStepper uses ESP32's MCPWM/RMT peripherals for up to 200k steps/sec.
 * All movements are non-blocking - the hardware handles pulse timing.
 */
class MotorController {
public:
    MotorController();

    /**
     * Initialize all motors with their configurations
     * Must be called in setup()
     */
    void begin();

    /**
     * Enable/disable all stepper drivers
     * @param enabled true to enable motors (allow movement)
     */
    void setEnabled(bool enabled);

    /**
     * Check if motors are enabled
     */
    bool isEnabled() const { return _enabled; }

    /**
     * Move a single joint to an absolute position
     * @param joint Joint index (0-5)
     * @param position Target position in steps
     * @return true if command accepted
     */
    bool moveTo(uint8_t joint, long position);

    /**
     * Move a single joint relative to current position
     * @param joint Joint index (0-5)
     * @param steps Number of steps to move (positive or negative)
     * @return true if command accepted
     */
    bool moveRelative(uint8_t joint, long steps);

    /**
     * Move multiple joints simultaneously
     * @param positions Array of 6 target positions (LONG_MIN to skip)
     * @return true if command accepted
     */
    bool moveToMultiple(const long positions[MOTOR_COUNT]);

    /**
     * Stop a single joint immediately
     */
    void stop(uint8_t joint);

    /**
     * Stop all joints immediately (emergency stop)
     */
    void stopAll();

    /**
     * Check if a specific joint is moving
     */
    bool isMoving(uint8_t joint) const;

    /**
     * Check if any joint is moving
     */
    bool isAnyMoving() const;

    /**
     * Get current position of a joint
     */
    long getPosition(uint8_t joint) const;

    /**
     * Get target position of a joint
     */
    long getTargetPosition(uint8_t joint) const;

    /**
     * Get distance to target for a joint
     */
    long getDistanceToGo(uint8_t joint) const;

    /**
     * Set current position as zero for a joint (does not move)
     */
    void setZero(uint8_t joint);

    /**
     * Set current position as zero for all joints
     */
    void setZeroAll();

    /**
     * Set maximum speed for a joint
     * @param joint Joint index
     * @param speedHz Speed in steps/second
     */
    void setMaxSpeed(uint8_t joint, uint32_t speedHz);

    /**
     * Set acceleration for a joint
     * @param joint Joint index
     * @param acceleration Acceleration in steps/second^2
     */
    void setAcceleration(uint8_t joint, uint32_t acceleration);

    /**
     * Get motor configuration for a joint
     */
    const MotorConfig& getConfig(uint8_t joint) const;

    /**
     * Access underlying FastAccelStepper instance (for advanced use)
     */
    FastAccelStepper* getStepper(uint8_t joint);

private:
    FastAccelStepperEngine _engine;
    FastAccelStepper* _steppers[MOTOR_COUNT];
    bool _enabled;

    bool isValidJoint(uint8_t joint) const { return joint < MOTOR_COUNT; }
    bool isWithinLimits(uint8_t joint, long position) const;
};

// Global motor controller instance
extern MotorController motors;

#endif // MOTOR_CONTROLLER_H
