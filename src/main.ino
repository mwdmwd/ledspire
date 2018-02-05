#include <stdint.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <ArduinoOTA.h>

#include "conf.h"
#include "wifi_creds.h"

#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14

#define SETUP_PIN(pin) do{pinMode(pin, OUTPUT);analogWrite(pin, 0);}while(0);

ESP8266WebServer server(80);
unsigned long long lastReconnectMillis;

uint16_t gammaCorr8to16(uint8_t val)
{
	return (uint16_t)(pow((float)val / 255.f, 2.8f) * 1023 + 0.5);
}

void setRGB(int nr, int ng, int nb)
{
	if(conf.r != nr)
	{
		analogWrite(RED_PIN, gammaCorr8to16(nr)); conf.r = nr;
	}
	if(conf.g != ng)
	{
		analogWrite(GREEN_PIN, gammaCorr8to16(ng)); conf.g = ng;
	}
	if(conf.b != nb)
	{
		analogWrite(BLUE_PIN, gammaCorr8to16(nb)); conf.b = nb;
	}
}

void restoreRGB(void)
{
	analogWrite(RED_PIN, gammaCorr8to16(conf.r));
	analogWrite(GREEN_PIN, gammaCorr8to16(conf.g));
	analogWrite(BLUE_PIN, gammaCorr8to16(conf.b));
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
	WiFi.setAutoReconnect(false);
	WiFi.setSleepMode(WIFI_NONE_SLEEP);
	while(WiFi.status() != WL_CONNECTED)
	{
		delay(50);
		digitalWrite(2, !digitalRead(2));
		digitalWrite(BLUE_PIN, digitalRead(2));

		if(millis() >= 30000) // Waited 30s and no connection...
			ESP.reset(); // Give up and restart
	}
	digitalWrite(2, 1);
	digitalWrite(BLUE_PIN, 0);

	SETUP_PIN(RED_PIN);
	SETUP_PIN(GREEN_PIN);
	SETUP_PIN(BLUE_PIN);

	EEPROM.begin(CONF_SIZE);
	if(confValid())
		confRead();
	else
		confWrite();
	restoreRGB();

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
			snprintf(rbuf, 12, "%u,%u,%u", conf.r, conf.g, conf.b);
			server.send(200, "text/plain", rbuf);
		}
	});
	
	server.on("/prog", HTTP_GET, [](void) {
		Dir progDir = SPIFFS.openDir("/prog");
		String out = "";
		uint8_t first = 1;
		while(progDir.next())
		{
			if(!first)
				out += "\n";
			first = 0;
			out += &progDir.fileName().c_str()[6]; // Strip /prog/
		}
		server.send(200, "text/plain", out);
	});

	server.on("/prog", HTTP_POST, [](void) {
		String fileName = server.arg("name");
		String content = server.arg("plain");
		if(fileName == "" || content == "") {server.send(500); return;}

		File pf = SPIFFS.open("/prog/" + fileName, "w");
		if(!pf) {server.send(500); return;}
		pf.write((const uint8_t*)content.c_str(), content.length());
		pf.close();
		server.send(200);
	});

	server.on("/prog", HTTP_DELETE, [](void) {
		String fileName = server.arg("name");
		if(fileName == "") {server.send(500); return;}
		if(SPIFFS.remove("/prog/" + fileName))
			server.send(200);
		else
			server.send(500);
	});

	server.serveStatic("/prog/", SPIFFS, "/prog/");
	server.serveStatic("/", SPIFFS, "/www/");
	server.begin();

	ArduinoOTA.setHostname("ledspire");
	ArduinoOTA.begin();
}

void loop()
{
	if(WiFi.status() != WL_CONNECTED)
	{
		WiFi.disconnect();
		delay(5000);
		WiFi.begin();
		while(true)
		{
			delay(50);
			digitalWrite(2, !digitalRead(2));
			digitalWrite(BLUE_PIN, digitalRead(2));

			if(WiFi.status() == WL_CONNECTED) // Going to break out
			{
				restoreRGB(); // Restore color
				break;
			}
		}
	}

	server.handleClient();
	ArduinoOTA.handle();
	confUpdate();
}
