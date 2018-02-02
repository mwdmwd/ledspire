#include <stdint.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoOTA.h>

#include "wifi_creds.h"

#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14

#define SETUP_PIN(pin) do{pinMode(pin, OUTPUT);analogWrite(pin, 0);}while(0);

ESP8266WebServer server(80);
int r, g, b;
unsigned long long lastReconnectMillis;

uint16_t gammaCorr8to16(uint8_t val)
{
	return (uint16_t)(pow((float)val / 255.f, 2.8f) * 1023 + 0.5);
}

void setRGB(int nr, int ng, int nb)
{
	if(r != nr)
	{
		analogWrite(RED_PIN, gammaCorr8to16(nr)); r = nr;
	}
	if(g != ng)
	{
		analogWrite(GREEN_PIN, gammaCorr8to16(ng)); g = ng;
	}
	if(b != nb)
	{
		analogWrite(BLUE_PIN, gammaCorr8to16(nb)); b = nb;
	}
}

void setup()
{
	pinMode(2, OUTPUT); // Onboard LED
	pinMode(BLUE_PIN, OUTPUT); // For blinking
	SETUP_PIN(RED_PIN);
	SETUP_PIN(GREEN_PIN);
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	WiFi.setAutoReconnect(true);
	while(WiFi.status() != WL_CONNECTED)
	{
		delay(50);
		digitalWrite(2, !digitalRead(2));
		digitalWrite(BLUE_PIN, digitalRead(2));
	}
	digitalWrite(2, 1);
	digitalWrite(BLUE_PIN, 0);

	SETUP_PIN(RED_PIN);
	SETUP_PIN(GREEN_PIN);
	SETUP_PIN(BLUE_PIN);
	setRGB(128, 60, 224);

	SPIFFS.begin();

	server.on("/rssi", [](void) {
		server.send(200, "text/plain", String(WiFi.RSSI()));
	});
	
	server.on("/", [](void) {
		server.sendHeader("Location", "/index.html");
		server.send(301);
	});
	server.on("/c", [](void) {
		if(server.args() != 0)
		{
			int nr = atoi(server.arg("r").c_str());
			int ng = atoi(server.arg("g").c_str());
			int nb = atoi(server.arg("b").c_str());
			setRGB(nr, ng, nb);
			server.send(200, "text/plain", "OK");
		}
		else
		{
			char rbuf[12];
			snprintf(rbuf, 12, "%u,%u,%u", r, g, b);
			server.send(200, "text/plain", rbuf);
		}
	});
	
	server.serveStatic("/", SPIFFS, "/www/");
	server.begin();

	ArduinoOTA.setHostname("ledspire");
	ArduinoOTA.begin();
}

void loop()
{
	while(WiFi.status() != WL_CONNECTED)
	{
		delay(50);
		digitalWrite(2, !digitalRead(2));
		digitalWrite(BLUE_PIN, digitalRead(2));
	}

	server.handleClient();
	ArduinoOTA.handle();
}
