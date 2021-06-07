# -*- coding: utf-8 -*-
"""
Created on Tue Jun  6 16:53:48 2021

@author: Jared Sylvia
"""
import time
from paho.mqtt import client as mqtt_client

broker = '10.10.0.1'
port = 1883
statusTopic = 'ceis114/trafficLight/status'
controlTopic = 'ceis114/trafficLight/control'
client_id = 'pythonTrafficLight'

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    # Set Connecting Client ID
    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        payload = msg.payload.decode()
        print(f"Received `{payload}` from `{msg.topic}` topic")
        if('Cross' in payload):
            crosswalk(payload)
            
    client.subscribe(statusTopic)
    client.on_message = on_message
    

def publish(client, message):
    time.sleep(1)
    result = client.publish(controlTopic, message)
    status = result[0]
    if status == 0:
        print(f"Send `{message}` to topic `{controlTopic}`")
    else:
        print(f"Failed to send message to topic {controlTopic}")
        
def crosswalk(direction):
    client = connect_mqtt()
    timer = 10
    if(direction == 'nsCross'):
        
        while timer:
            print(str(timer), 'seconds until light change.')
            time.sleep(1)
            timer -= 1
        publish(client, 'ns')
    
    if(direction == 'ewCross'):
        
        while timer:
            print(str(timer), 'seconds until light change.')
            time.sleep(1)
            timer -= 1
        publish(client, 'ew')
    print(direction)
    
        
def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()
    
if __name__ == '__main__':
    run()