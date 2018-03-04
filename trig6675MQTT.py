#!/usr/bin/python
import paho.mqtt.client as mqtt
import sys
from time import sleep

broker = "192.168.3.14"
port = 1883
 
mqttp = mqtt.Client("time_pub")
 
while True:
  mqttp.connect(broker, port, 60)
  sleep(0.1)
  mqttp.publish("arduino/in/ReadBoth", "go")
  sleep(3.3)
  mqttp.publish("arduino/in/ReadC", "go")
  sleep(3.3)
  mqttp.publish("arduino/in/ReadF", "go")
  sleep(3.3)

