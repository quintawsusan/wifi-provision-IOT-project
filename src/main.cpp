#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <secrets.h>

const int ledPin = 2;
WebServer server(80);

String page()
{
    return "<!DOCTYPE html>"
           "<html lang='en'>"
           "<head>"
           "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
           "<title>WIFI Provisioning</title>"
           "<style>"
           "body {"
           "    color: white;"
           "    background: #0d1117;"
           "    font-family: monospace;"
           "}"
           ".esp{"
           "    background: #131a22;"
           "    color: white;"
           "    font-size: 20px;"
           "    border: 1px solid #1f6b2c;"
           "    padding: 40px;"
           "    display: flex;"
           "justify-content: center;"
           "}"
           ".grid {"
           "    display: flex;"
           "    gap: 16px;"
           "    margin-top: 18px;"
           "}"
           ".card {"
           "    flex: 1;"
           "    border-radius: 4px;"
           "    padding: 14px;"
           "    background: #131a22;"
           "    min-height: 150px;"
           "padding-left: 30px;"
           "}"
           ".networks{"
           "padding-left: 90px;"
           "}"
           "#netList{"
           "    font-size: 14px;"
           "    margin: 12px 0;"
           "    min-height: 60px;"
           "}"
           ".net-row{"
           "    padding: 5px 0;"
           "    border-bottom: 1px solid #2a3542;"
           "}"
           "button {"
           "    width: 30%;"
           "    padding: 8px;"
           "    margin-top: 8px;"
           "    background: #131a22;"
           "    border: 1px solid #1f6b2c;"
           "    color: #39d353;"
           "    border-radius: 5px;"
           "    cursor: pointer;"
           "}"
           "button:hover {"
           "    background: rgba(57, 211, 83, 0.1);"
           "}"
           "#onoff {"
           "    width: 60%;"
           "    padding: 8px;"
           "    background: whitesmoke;"
           "    border-radius: 3px;"
           "    box-sizing: border-box;"
           "}"
           "#state{"
           "    padding-top: 10px;"
           "}"
           "</style>"
           "</head>"

           "<body>"
           "<div class='esp'>"
           "  <h2>ESP Information</h2>"
           "</div>"
           "<div class='grid'>"
           "  <div class='card'>"
           "<div class='networks'>"
           "    <h3>Nearby Available Networks</h3>"
           "    <div id='netList'>No scan yet.</div>"
           "    <button onclick='scanNetworks()'>Scan Me</button>"
           "</div>"
           "  </div>"
           "  <div class='card'>"
           "    <h3>LED Controls</h3>"
           "<form action='/send'>"
           "    <input id='onoff' name='led' placeholder='Type on or off'>"
           "    <button type='submit'>Send</button>"
           "</form>"
           "    <div id='state'>Current state: %status%</div>"
           "  </div>"
           "</div>"

           "<script>"
           "function scanNetworks(){"
           "  document.getElementById('netList').innerText = 'Scanning...';"
           "  fetch('/scan').then(r => r.json()).then(data => {"
           "    let html = '';"
           "    data.forEach(n => {"
           "      html += \"<div class='net-row'>\" + n.ssid + ' &mdash; ' + n.rssi + ' dBm ' + (n.secure ? '&#128274;' : '') + '</div>';"
           "    });"
           "    document.getElementById('netList').innerHTML = html || 'No networks found.';"
           "  });"
           "}"
           "</script>"

           "</body>"
           "</html>";
}

void handleRoot()
{
    String html = page();

    if (digitalRead(ledPin))
        html.replace("%status%", "ON");
    else
        html.replace("%status%", "OFF");


    server.send(200, "text/html", html);
}

void handleScan()
{
    int n = WiFi.scanNetworks();

    int indices[n];
    for (int i = 0; i < n; i++) indices[i] = i;

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (WiFi.RSSI(indices[j]) < WiFi.RSSI(indices[j + 1]))
            {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }

    String json = "[";
    for (int i = 0; i < n; i++)
    {
        int idx = indices[i];
        json += "{\"ssid\":\"" + WiFi.SSID(idx) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(idx)) + ",";
        json += "\"secure\":" + String(WiFi.encryptionType(idx) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
        if (i < n - 1) json += ",";
    }
    json += "]";

    WiFi.scanDelete(); 
    server.send(200, "application/json", json);
}


void handleOn()
{
    digitalWrite(ledPin, HIGH);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleOff()
{
    digitalWrite(ledPin, LOW);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleSend()
{
    String value = server.arg("led");
    if (value == "on")
        digitalWrite(ledPin, HIGH);


    if (value == "off")
        digitalWrite(ledPin, LOW);


    server.sendHeader("Location", "/");
    server.send(303);
}

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
    server.on("/", handleRoot);
    server.on("/scan", handleScan);
    server.on("/on", handleOn);
    server.on("/off", handleOff);
    server.on("/send", handleSend);
    server.begin();
}

void loop()
{
    server.handleClient();
}

