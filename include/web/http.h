#include "HTTPClient.h"

String httpGETRequest(const char *serverName)
{
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin(serverName);

    http.setTimeout(CONST_HTTP_TIMEOUT);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    
    String payload = "{}";

    if (httpResponseCode > 0)
    {
       // Serial.print("HTTP Response code: ");
       // Serial.println(httpResponseCode);
        payload = http.getString();
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

void httpPOSTRequest(const char *serverName)
{
    Serial.println(serverName);
    Serial.println(httpGETRequest(serverName));
}

