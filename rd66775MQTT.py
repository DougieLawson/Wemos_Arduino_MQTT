#!/usr/bin/python
import paho.mqtt.client as mqtt
import sys
from time import sleep

def on_connect(mqtts, obj, flags, rc):
  print("Conn rc: "+str(rc))

def on_publish(mqtts, obj, mid):
  print("mid: "+str(mid))

def on_message(mqtts, obj, msg):
  global mqttp, broker, port
  print ("Message received on topic: "+str(msg.topic)+" with QoS: "+str(msg.qos)+" and payload: "+str(msg.payload))
  sleep(10)
  print ("Sleep done")
  mqttp.connect(broker, port, 60)
  mqttp.publish("arduino/in/ReadBoth", "go")

def on_subscribe(mqtts, obj, mid, granted_qos):
  print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_log(mqtts, obj, level, string):
  print(string)

def main():

  global mqttp, broker, port
  broker = "192.168.3.14"
  port = 1883
 
  mqtts = mqtt.Client("ard_sub")
  mqttp = mqtt.Client("ard_pub")
 
  mqtts.on_message = on_message
  mqtts.on_connect = on_connect
  mqtts.on_publish = on_publish
  mqtts.on_subscribe = on_subscribe

  mqtts.connect(broker, port, 60)
  mqtts.subscribe("arduino/out/+", 0)
#  mqttp.publish("arduino/in/ReadC", "go")

  rc = 0
  while rc == 0:
    rc = mqtts.loop_forever()

  return 0 

if __name__ == "__main__":
   sys.exit(main())
