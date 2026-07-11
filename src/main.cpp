#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

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
           "}"
           ".esp{"
           "    color: #131a22;"
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
           "    height: 150px;"
           "padding-left: 30px;"
           "}"
           ".networks{"
           "padding-left: 90px;"
           "}"
           "button {"
           "    width: 30%;"
           "    padding: 8px;"
           "    margin-top: 8px;"
           "    background: #131a22;"
           "    border: 1px solid #1f6b2c;"
           "    color: #39d353;"
           "    border-radius: 5px;"
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
           "    <button>Scan Me</button>"
           "</div>"
           "  </div>"
           "  <div class='card'>"
           "    <h3>LED Controls</h3>"
           "    <input id='onoff' placeholder='Type on or off'>"
           "    <button>Send</button>"
           "    <div id='state'>Current state: %status%</div>"
           "  </div>"
           "</div>"
           "</body>"
           "</html>";
}

void handleRoot()
{
    String html = page();

    html.replace("%networks%", "Click Scan Networks");

    if (digitalRead(ledPin))
        html.replace("%status%", "ON");
    else
        html.replace("%status%", "OFF");

    server.send(200, "text/html", html);
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
    server.on("/on", handleOn);
    server.on("/off", handleOff);
    server.on("/send", handleSend);
    server.begin();
}

void loop()
{
    server.handleClient();
}
