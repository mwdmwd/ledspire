SKETCH=src/main.ino
FS_DIR=fs/
LIBS=$(ESP_LIBS)/ESP8266WiFi $(ESP_LIBS)/ArduinoOTA $(ESP_LIBS)/ESP8266mDNS $(ESP_LIBS)/ESP8266WebServer $(ESP_LIBS)/EEPROM

-include config.target.mk
