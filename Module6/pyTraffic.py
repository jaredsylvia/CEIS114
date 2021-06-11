# -*- coding: utf-8 -*-
"""
Created on Tue Jun  6 16:53:48 2021

@author: Jared Sylvia
"""
import time
from paho.mqtt import client as mqtt_client
import threading as th

broker = '10.10.0.1'
port = 1883
statusTopic = 'ceis114/trafficLight/status'
controlTopic = 'ceis114/trafficLight/control'
line0Topic = 'ceis114/trafficLight/line0'
line1Topic = 'ceis114/trafficLight/line1'

clientIDs = []
clients = []
nclients=2
mqtt_client.Client.connected_flag=False
client_idSub = 'pythonTrafficLightSub'
client_idPub = 'pythonTrafficLightPub'

def connect_mqtt(client_id, broker, port):
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
        
        
        if(payload == 'nsCross'):
            publish(client, 'North/South', line0Topic)
            publish(client, 'Don\'t Walk', line1Topic)
            th.Thread(target=crosswalk(payload, client))    
        elif(payload == 'ewCross'):
            publish(client, 'East/West', line0Topic)
            publish(client, 'Don\'t Walk', line1Topic)
            th.Thread(target=crosswalk(payload, client))
        
            
            
    client.subscribe(statusTopic)
    client.on_message = on_message
    

def publish(client, message, topic):
    time.sleep(1)
    result = client.publish(topic, message)
    status = result[0]
    if status == 0:
        print(f"Send `{message}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

def on_disconnect(client, userdata,rc=0):
    print("DisConnected result code "+str(rc))
    client.loop_stop()
        
    
def crosswalk(direction, client):
    timer = 10
    
    if(direction == 'nsCross'):
        
        while timer:
            print(str(timer), 'seconds until light change.')
            time.sleep(1)
            timer -= 1
        publish(client, 'ns', controlTopic)
        
    if(direction == 'ewCross'):
        
        while timer:
            print(str(timer), 'seconds until light change.')
            time.sleep(1)
            timer -= 1
        publish(client, 'ew', controlTopic)
    print(direction)
    
        
def run():
    
    for i  in range(nclients):
        cname ="Client"+str(i)
        client = connect_mqtt(cname, broker, port)
        clients.append(client)
    for client in clients:
        client.connect(broker)
        print(client)
        
        #client.loop_start()
         
    # subClient = connect_mqtt(client_idSub, broker, port)
    # pubClient = connect_mqtt(client_idPub, broker, port)
    subscribe(clients[0])
    th.Thread(target=clients[0].loop_forever())
    
if __name__ == '__main__':
    try:
        run()
    except KeyboardInterrupt:
        for client in clients:
            client.loop_stop()