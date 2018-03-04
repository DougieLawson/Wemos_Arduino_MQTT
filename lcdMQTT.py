#!/usr/bin/python

import mosquitto
import os
import time
import sys
import pifacecommon
import pifacecad

def on_message(mqtts, userd, msg):
  lcd_message = msg.payload.decode("utf-8")+ "                    "
  print ("Message received on topic: "+msg.topic+" with QoS: "+str(msg.qos)+" and payload: "+lcd_message)
  cad.__init__()
  cad.lcd.backlight_on()
  cad.lcd.blink_off()
  cad.lcd.cursor_off()
  while len(lcd_message) > 20:
     print ("1:{l1}".format(l1=lcd_message[:20]))
     cad.lcd.set_cursor(15,0)
     cad.lcd.write("{l1}".format(l1=lcd_message[:20]))
     if len(lcd_message) > 20:
        cad.lcd.set_cursor(15,1)
        print ("2:{l2}".format(l2=lcd_message[20:40]))
        cad.lcd.write("{l2}".format(l2=lcd_message[20:40]))
     else: 
        cad.lcd.write("                    ") 
     for i in range(0,20):
        cad.lcd.move_left()
        time.sleep(0.4)
     cad.lcd.home()

     lcd_message = lcd_message[20:]

def main():

  global cad
  cad = pifacecad.PiFaceCAD()

  broker = "192.168.3.14"
  port = 1883

  mypid = os.getpid()
  sub_uniq = "subclient_"+str(mypid)
  mqtts = mosquitto.Mosquitto(sub_uniq)
  mqtts.on_message = on_message

  mqtts.connect(broker, port, 60, True)
  mqtts.subscribe("test/lcd", 0)

  rc = 0
  while rc == 0:
    rc = mqtts.loop()

  return 0 

if __name__ == "__main__":
   sys.exit(main())
