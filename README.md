Arduino, Wemos and MQTT sample code
===================================

&copy; 2018 Dougie Lawson, All rights reserved


* bmp180.sqlite.py -- read a BMP180 sensor, write a SQLite3 db and publish the results every three minutes.
* esp8266_MQTT.ino -- basics of getting MQTT running on an ESP8266
* lcdMQTT.py -- subscribe MQTT, display on PiFaceCAD SPI LCD
* MAX6675_LCD.ino -- read a 6675 thermocouple, send results on serial
* rd66775MQTT.py  -- read serial, publish 6675 readings on MQTT
* trig6675MQTT.py -- nudge an Arduino to publish results every three seconds
* wemos_LCDthermoMQTT.ino -- read a MAX31855 thermocouple, publish on MQTT and a simple web server page.
* Wemos_MQTT_LCD.ino -- subscribe to an MQTT topic, print results on I2C LCD
* Wemos_RFID.ino -- sample for reading MF-RC522 cards and publishing to MQTT

* index.html -- sample page for MQTT web sockets (needs JQuery and mqttws31)
* config.js -- config file for MQTT web sockets
* mqtt6675.ino -- subscribe to MQTT, publish 6675 readings when triggered by trig6675MQTT.py python program


Get mqttws31 from https://raw.githubusercontent.com/eclipse/paho.mqtt.javascript/master/src/mqttws31.js
