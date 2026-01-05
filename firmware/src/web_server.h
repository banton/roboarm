#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "motor_controller.h"
#include "command_parser.h"

/**
 * Async Web Server for robotic arm control
 *
 * REST API Endpoints:
 *   GET  /api/status       - Get current positions and status
 *   POST /api/command      - Execute a G-code command
 *   POST /api/move         - Move joints (JSON body)
 *   POST /api/enable       - Enable/disable motors
 *   GET  /api/config       - Get motor configuration
 *   GET  /                 - Simple web UI (if enabled)
 */

class RoboarmWebServer {
public:
    RoboarmWebServer(uint16_t port = WEB_SERVER_PORT);

    /**
     * Initialize WiFi and start the web server
     * @param ssid WiFi network name
     * @param password WiFi password
     * @return true if connected successfully
     */
    bool begin(const char* ssid, const char* password);

    /**
     * Check if WiFi is connected
     */
    bool isConnected() const;

    /**
     * Get IP address as string
     */
    String getIPAddress() const;

    /**
     * Handle any pending web requests (called in loop)
     * Note: ESPAsyncWebServer handles this automatically,
     * but this method can be used for additional processing
     */
    void loop();

private:
    AsyncWebServer _server;
    bool _connected;

    // Setup route handlers
    void setupRoutes();

    // API handlers
    void handleStatus(AsyncWebServerRequest* request);
    void handleCommand(AsyncWebServerRequest* request, uint8_t* data, size_t len);
    void handleMove(AsyncWebServerRequest* request, uint8_t* data, size_t len);
    void handleEnable(AsyncWebServerRequest* request, uint8_t* data, size_t len);
    void handleConfig(AsyncWebServerRequest* request);

    // Response helpers
    void sendJsonResponse(AsyncWebServerRequest* request, int code, const JsonDocument& doc);
    void sendJsonError(AsyncWebServerRequest* request, int code, const String& message);
    void sendJsonSuccess(AsyncWebServerRequest* request, const String& message);

    // Build status JSON
    void buildStatusJson(JsonDocument& doc);
    void buildConfigJson(JsonDocument& doc);
};

// Global web server instance
extern RoboarmWebServer webServer;

#endif // WEB_SERVER_H
