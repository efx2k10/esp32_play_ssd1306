#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
<meta charset="UTF-8">
<title>YMusic</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<title>ESP Play</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
    <link rel="stylesheet" href="http://efx2k10gma.temp.swtest.ru/esp32/html/style.css" media="all" />
    <script src="http://efx2k10gma.temp.swtest.ru/esp32/html/script.js" ></script>
</head>
<body>
<div id="wrapper">
<div id="app">
<div id="status_bar"></div>
<div id="setting"></div>
<div id="loader"></div>
<div id="img"></div>
<div id="play"></div>
<div id="btn"></div>
<div id="pan"></div>
<div id="pan2"></div>
<div id="pan3"></div>
<div id="pan4"></div>
<div id="pan5"></div>
</div>  
</div>  
</body>
</html>
)rawliteral";



void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "__ping__") == 0)
            ws.textAll("__pong__");
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

