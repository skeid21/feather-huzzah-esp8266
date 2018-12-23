#include <ESP8266mDNS.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string>

const char *ssid = "";
const char *password = "";

ESP8266WebServer server(80);

const int led = 4;
const int badRequestLed = 0;
const int goodRequestLed = 2;
const char * rootPage = "\
<html>\
<button id=\"ledButton\"></button>\
<script language=\"JavaScript\">\
    let button = document.getElementById(\"ledButton\");\
    const statusUrl = \"/status\";\
    const switchLEDUrl = \"/switch\";\
    function switchLEDStatus() {\
        const switchRequest = new XMLHttpRequest();\
        switchRequest.onreadystatechange = (e) => {\
            if (switchRequest.responseText == \"on\") {\
                button.innerHTML = \"Turn LED Off\";\
            } else {\
                button.innerHTML = \"Turn LED On\";\
                ledOn = true;\
            }\
        };\
        switchRequest.open(\"GET\", switchLEDUrl);\
        switchRequest.send();\
    };\
    function init() {\
        const statusRequest = new XMLHttpRequest();\
        statusRequest.onreadystatechange = (e) => {\
            if (statusRequest.responseText == \"on\") {\
                button.innerHTML = \"Turn LED Off\";\
            } else {\
                button.innerHTML = \"Turn LED On\";\
                ledOn = true;\
            }\
        };\
        statusRequest.open(\"GET\", statusUrl);\
        statusRequest.send();\
        button.onclick = switchLEDStatus;\
    }\
    window.onload = init;\
</script>\
</html>\
";
void onReceivedRequest(bool goodRequest) {
  if (goodRequest)
  {
    digitalWrite(badRequestLed, HIGH);
    digitalWrite(goodRequestLed, LOW);
  } else {
    digitalWrite(badRequestLed, LOW);
    digitalWrite(goodRequestLed, HIGH);
  }
}
void handleRoot()
{
  onReceivedRequest(true);
  server.send(200, "text/html", rootPage);
}

void handleNotFound()
{

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  onReceivedRequest(false);
  server.send(404, "text/plain", message);
}
bool ledOn = false;
void setup(void)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  pinMode(badRequestLed, OUTPUT);
  pinMode(goodRequestLed, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("led"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/switch", []() {
    String ledStatus;
    if (ledOn)
    {
      ledStatus = "off";
      digitalWrite(led, LOW);
    }
    else 
    {
      ledStatus = "on";
      digitalWrite(led, HIGH);
    }
    ledOn = !ledOn;
    onReceivedRequest(true);
    server.send(200, "text/plain", ledStatus);
  });

  server.on("/status", []() {
    String ledStatus = "on";
    if (ledOn == false)
    {
      ledStatus = "off";
    }
    onReceivedRequest(true);
    server.send(200, "test/plain", ledStatus);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
  server.handleClient();
}
