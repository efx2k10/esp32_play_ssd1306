
String utf8rus(String source)
{
    int i, k;
    String target;
    unsigned char n;
    char m[2] = {'0', '\0'};

    k = source.length();
    i = 0;

    while (i < k)
    {
        n = source[i];
        i++;

        if (n >= 0xC0)
        {
            switch (n)
            {
            case 0xD0:
            {
                n = source[i];
                i++;
                if (n == 0x81)
                {
                    n = 0xA8;
                    break;
                }
                if (n >= 0x90 && n <= 0xBF)
                    n = n + 0x2F;
                break;
            }
            case 0xD1:
            {
                n = source[i];
                i++;
                if (n == 0x91)
                {
                    n = 0xB8;
                    break;
                }
                if (n >= 0x7E && n <= 0x8F)
                    n = n + 0x6F;
                break;
            }
            }
        }
        m[0] = n;
        target = target + String(m);
    }
    return target;
}

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



