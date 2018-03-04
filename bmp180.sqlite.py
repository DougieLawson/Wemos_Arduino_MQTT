#!/usr/bin/python

import time
import bmp180
import sqlite3 as sql
import paho.mqtt.client as mqtt

mqttpub = mqtt.Client()

def mqttPublish(topic, payload):

    broker="192.168.3.14"
    port = 1883

    rc = mqttpub.connect(broker, port, 60)
    tries = 5
    while rc !=0:
        try:
            rc = mqttpub.reconnect()
            if rc != 0:
                rc = mqttpub.connect(broker, port, 60)
            if rc != 0:
                raise Exception('connectionError')
        except:
            tries -= 1
            mqttpub.disconnect()
            if tries == 0:
                raise Exception('connectionException')

    mqttpub.publish(topic, payload)


time4 = ''
BMPsensor = bmp180.BMP180()

sensorData = sql.connect('/srv/bmp180/sensordata.db')
sensorData.execute("PRAGMA journal_mode=WAL")
sensorData.execute("VACUUM")
bmp_data = sensorData.cursor()

def tick():

    while True:
        global time4
        press = BMPsensor.get_pressure()
        temp = BMPsensor.get_temperature()
        time1 = time.strftime('%M')
        if int(time1) % 3 == 0:
            time2 = time.strftime('%H:%M:%S')
            time3 = time.strftime('%S')
            if time3 == '00': # exactly on the minute
                if time2 != time4:
                    time4 = time2 # Make sure we only run once
                    mqttPublish("falcon/temperature", str(temp)+" "+time2)
                    mqttPublish("falcon/pressure", str(press)+" "+time2)
                    bmp_data.execute("insert into bmp_data (date_time, temp, pressure) values(datetime('now','localtime'), ?, ?)", (temp, press))
                    sensorData.commit()
        time.sleep(0.3)

tick()
