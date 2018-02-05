ESP_ROOT=${HOME}/esp8266/arduino
CHIP=esp8266
BOARD=nodemcuv2
#F_CPU=160000000
FLASH_DEF=4M1M
LWIP_VARIANT=Prebuilt

UPLOAD_SPEED=256000
ESP_ADDR=ledspire.local
SKETCH=src/main.ino
FS_DIR=fs/

A=${ESP_ROOT}/libraries/
LIBS=$AESP8266WiFi $AArduinoOTA $AESP8266mDNS $AESP8266WebServer $AEEPROM
include makeEspArduino/makeEspArduino.mk
