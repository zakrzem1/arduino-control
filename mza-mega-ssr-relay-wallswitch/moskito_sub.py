import paho.mqtt.client as mqtt
import logging
logger = logging.getLogger(__name__)
mqtt_topic = ""

def on_connect(client, userdata, flags, result_code):
  global mqtt_topic
  logger.info("Connected with result code " + str(result_code))
  subscribed = client.subscribe(mqtt_topic)
  #info ('subscribed sucess = '+str(mqtt.MQTT_ERR_SUCCESS == subscribed[0])+' msgID: '+str(subscribed[1]))

def on_message(client, userdata, msg):
  logger.info(str(client) + str(userdata))
  logger.info(msg.topic + " on_message " + str(msg.payload))
  operation = str(msg.payload, 'utf-8')
  global on_command
  on_command(operation)

def on_subscribe(mosq, obj, mid, granted_qos):
  logger.info("[on_subscribe] Subscribed: " + str(mid) + " " + str(granted_qos))

def on_publish(mqttc, obj, mid):
  print("published mid: " + str(mid))
  pass

def start_client(mqtt_broker_host, mqtt_topic_to_subscribe, on_command_fn):
  global mqtt_topic
  global on_command
  on_command = on_command_fn
  mqtt_topic=mqtt_topic_to_subscribe
  client = mqtt.Client()
  client.enable_logger(logger)
  client.on_connect = on_connect
  client.on_message = on_message
  client.on_subscribe = on_subscribe
  client.on_publish = on_publish
  client.connect(mqtt_broker_host, 1883, 60)
  client.loop_start()
  return client

