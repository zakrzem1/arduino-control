import paho.mqtt.client as mqtt
import logging
logger = logging.getLogger(__name__)
mqtt_topics=[]
qos = 0
def on_connect(client, userdata, flags, result_code):
  global mqtt_topics
  global qos
  global subscribed
  logger.info("Connected with result code " + str(result_code))
  for topic in mqtt_topics:
    # subscribed = 
    client.subscribe(topic)
  #info ('subscribed sucess = '+str(mqtt.MQTT_ERR_SUCCESS == subscribed[0])+' msgID: '+str(subscribed[1]))

def on_message(client, userdata, msg):
  logger.info("on_message client: " + str(client) + ", userdata: " + str(userdata))
  logger.info(" topic: "+ msg.topic + ", payload: " + str(msg.payload))
  global on_command
  on_command(msg.topic, str(msg.payload, 'utf-8'))

def on_subscribe(mosq, obj, mid, granted_qos):
  logger.info("[on_subscribe] Subscribed: " + str(mid) + " " + str(granted_qos))

def on_publish(mqttc, obj, mid):
  print("published mid: " + str(mid))
  pass

def start_client(mqtt_broker_host, mqtt_topics_to_subscribe, on_command_fn):
  global mqtt_topics
  global on_command
  on_command = on_command_fn
  mqtt_topics = mqtt_topics_to_subscribe
  client = mqtt.Client()
  client.enable_logger(logger)
  client.on_connect = on_connect
  client.on_message = on_message
  client.on_subscribe = on_subscribe
  client.on_publish = on_publish
  client.connect(mqtt_broker_host, 1883, 60)
  client.loop_start()
  return client

