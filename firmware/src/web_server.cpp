#include "web_server.h"

// Global instance
RoboarmWebServer webServer;

RoboarmWebServer::RoboarmWebServer(uint16_t port)
    : _server(port), _connected(false) {
}

bool RoboarmWebServer::begin(const char* ssid, const char* password) {
    DEBUG_PRINTLN("WebServer: Connecting to WiFi...");
    DEBUG_PRINTF("  SSID: %s\n", ssid);

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(WIFI_HOSTNAME);
    WiFi.begin(ssid, password);

    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        DEBUG_PRINT(".");
        attempts++;
    }
    DEBUG_PRINTLN();

    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WebServer: WiFi connection FAILED");
        _connected = false;
        return false;
    }

    _connected = true;
    DEBUG_PRINTLN("WebServer: WiFi connected!");
    DEBUG_PRINTF("  IP Address: %s\n", WiFi.localIP().toString().c_str());
    DEBUG_PRINTF("  Hostname: %s\n", WIFI_HOSTNAME);

    // Setup routes and start server
    setupRoutes();
    _server.begin();
    DEBUG_PRINTLN("WebServer: HTTP server started");

    return true;
}

bool RoboarmWebServer::isConnected() const {
    return _connected && WiFi.status() == WL_CONNECTED;
}

String RoboarmWebServer::getIPAddress() const {
    if (!_connected) {
        return "Not connected";
    }
    return WiFi.localIP().toString();
}

void RoboarmWebServer::loop() {
    // ESPAsyncWebServer handles requests automatically
    // This method can be used for periodic tasks if needed

    // Reconnect WiFi if disconnected
    if (_connected && WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WebServer: WiFi disconnected, attempting reconnect...");
        WiFi.reconnect();
    }
}

void RoboarmWebServer::setupRoutes() {
    // CORS headers for all responses
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // Handle OPTIONS preflight requests
    _server.onNotFound([](AsyncWebServerRequest* request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404, "application/json", "{\"error\":\"Not found\"}");
        }
    });

    // GET /api/status - Get current status
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });

    // POST /api/command - Execute G-code command
    _server.on("/api/command", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            handleCommand(request, data, len);
        }
    );

    // POST /api/move - Move joints (JSON)
    _server.on("/api/move", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            handleMove(request, data, len);
        }
    );

    // POST /api/enable - Enable/disable motors
    _server.on("/api/enable", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            handleEnable(request, data, len);
        }
    );

    // GET /api/config - Get configuration
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleConfig(request);
    });

    // GET / - Simple status page
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        String html = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
    <title>Roboarm</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: monospace; padding: 20px; background: #1a1a1a; color: #0f0; }
        h1 { color: #0ff; }
        pre { background: #000; padding: 10px; border: 1px solid #0f0; }
        .btn { background: #0f0; color: #000; border: none; padding: 10px 20px; margin: 5px; cursor: pointer; }
        .btn:hover { background: #0ff; }
        input { background: #000; color: #0f0; border: 1px solid #0f0; padding: 5px; }
    </style>
</head>
<body>
    <h1>Roboarm Controller</h1>
    <div>
        <button class="btn" onclick="sendCmd('M17')">Enable</button>
        <button class="btn" onclick="sendCmd('M18')">Disable</button>
        <button class="btn" onclick="sendCmd('M112')">E-STOP</button>
        <button class="btn" onclick="getStatus()">Status</button>
    </div>
    <div style="margin-top: 20px;">
        <input type="text" id="cmd" placeholder="G0 J1:1000" style="width: 200px;">
        <button class="btn" onclick="sendInput()">Send</button>
    </div>
    <pre id="output">Ready...</pre>
    <script>
        async function sendCmd(cmd) {
            const res = await fetch("/api/command", {
                method: "POST",
                headers: {"Content-Type": "application/json"},
                body: JSON.stringify({command: cmd})
            });
            const data = await res.json();
            document.getElementById("output").textContent = JSON.stringify(data, null, 2);
        }
        async function getStatus() {
            const res = await fetch("/api/status");
            const data = await res.json();
            document.getElementById("output").textContent = JSON.stringify(data, null, 2);
        }
        function sendInput() {
            const cmd = document.getElementById("cmd").value;
            if (cmd) sendCmd(cmd);
        }
        document.getElementById("cmd").addEventListener("keypress", (e) => {
            if (e.key === "Enter") sendInput();
        });
    </script>
</body>
</html>
)rawhtml";
        request->send(200, "text/html", html);
    });
}

void RoboarmWebServer::handleStatus(AsyncWebServerRequest* request) {
    JsonDocument doc;
    buildStatusJson(doc);
    sendJsonResponse(request, 200, doc);
}

void RoboarmWebServer::handleCommand(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
    }

    const char* command = doc["command"];
    if (!command) {
        sendJsonError(request, 400, "Missing 'command' field");
        return;
    }

    CommandResult result = commandParser.execute(String(command));

    JsonDocument response;
    response["success"] = result.success;
    response["message"] = result.message;

    sendJsonResponse(request, result.success ? 200 : 400, response);
}

void RoboarmWebServer::handleMove(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
    }

    // Build G0 command from JSON
    String command = "G0";
    bool hasJoint = false;

    for (int i = 1; i <= MOTOR_COUNT; i++) {
        String key = "j" + String(i);
        if (doc[key].is<long>()) {
            command += " J" + String(i) + ":" + String(doc[key].as<long>());
            hasJoint = true;
        }
    }

    if (!hasJoint) {
        sendJsonError(request, 400, "No joint positions specified. Use j1, j2, ..., j6");
        return;
    }

    CommandResult result = commandParser.execute(command);

    JsonDocument response;
    response["success"] = result.success;
    response["message"] = result.message;
    response["command"] = command;

    sendJsonResponse(request, result.success ? 200 : 400, response);
}

void RoboarmWebServer::handleEnable(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
    }

    bool enabled = doc["enabled"] | false;
    motors.setEnabled(enabled);

    JsonDocument response;
    response["success"] = true;
    response["enabled"] = motors.isEnabled();

    sendJsonResponse(request, 200, response);
}

void RoboarmWebServer::handleConfig(AsyncWebServerRequest* request) {
    JsonDocument doc;
    buildConfigJson(doc);
    sendJsonResponse(request, 200, doc);
}

void RoboarmWebServer::sendJsonResponse(AsyncWebServerRequest* request, int code, const JsonDocument& doc) {
    String output;
    serializeJson(doc, output);
    request->send(code, "application/json", output);
}

void RoboarmWebServer::sendJsonError(AsyncWebServerRequest* request, int code, const String& message) {
    JsonDocument doc;
    doc["success"] = false;
    doc["error"] = message;
    sendJsonResponse(request, code, doc);
}

void RoboarmWebServer::sendJsonSuccess(AsyncWebServerRequest* request, const String& message) {
    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = message;
    sendJsonResponse(request, 200, doc);
}

void RoboarmWebServer::buildStatusJson(JsonDocument& doc) {
    doc["enabled"] = motors.isEnabled();
    doc["moving"] = motors.isAnyMoving();

    JsonObject positions = doc["positions"].to<JsonObject>();
    JsonObject targets = doc["targets"].to<JsonObject>();
    JsonObject distances = doc["distances"].to<JsonObject>();

    for (int i = 0; i < MOTOR_COUNT; i++) {
        String key = "j" + String(i + 1);
        positions[key] = motors.getPosition(i);
        targets[key] = motors.getTargetPosition(i);
        distances[key] = motors.getDistanceToGo(i);
    }

    doc["ip"] = WiFi.localIP().toString();
    doc["uptime"] = millis() / 1000;
}

void RoboarmWebServer::buildConfigJson(JsonDocument& doc) {
    doc["motor_count"] = MOTOR_COUNT;
    doc["enable_pin"] = MOTORS_ENABLE_PIN;

    JsonArray motors_arr = doc["motors"].to<JsonArray>();

    for (int i = 0; i < MOTOR_COUNT; i++) {
        const MotorConfig& cfg = MOTOR_CONFIGS[i];
        JsonObject motor = motors_arr.add<JsonObject>();

        motor["joint"] = i + 1;
        motor["name"] = cfg.name;
        motor["step_pin"] = cfg.stepPin;
        motor["dir_pin"] = cfg.dirPin;
        motor["steps_per_rev"] = cfg.stepsPerRev;
        motor["max_speed"] = cfg.maxSpeedHz;
        motor["acceleration"] = cfg.acceleration;
        motor["invert_dir"] = cfg.invertDir;
    }
}
