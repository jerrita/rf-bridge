#include "switch.h"

#define LED D4
#define VERSION "0.1"

// -------------------- Configurations -----------------------------
const long UPDTAE_INTERVALS = 30 * 1000;

const char *WiFi_SSID = "Ireina";
const char *WiFi_Password = "pa5sw0rd";

const PROGMEM char *MQTT_CLIENT_ID = "switch433";
const PROGMEM char *MQTT_SERVER_IP = "homeassistant";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char *MQTT_USER = "mqtt";
const PROGMEM char *MQTT_PASSWORD = "mqtt";

const char *MQTT_COMMAND_TOPIC = "home/switch433/set";
const char *MQTT_STATE_TOPIC = "home/switch433";

const char *DEVICE_ON = "ON";
const char *DEVICE_OFF = "OFF";
// ------------------------------------------------------------------

bool switchState;
long lastTime = 0;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void switch_on()
{
  switchState = true;
  digitalWrite(LED, LOW);
}

void switch_off()
{
  switchState = false;
  digitalWrite(LED, HIGH);
}

bool publish_switch_status()
{
  return client.publish(MQTT_STATE_TOPIC,
                        switchState ? DEVICE_ON : DEVICE_OFF,
                        true);
}

void callback(char *p_topic, byte *p_payload, unsigned int p_length)
{
  if (!strcmp(p_topic, MQTT_COMMAND_TOPIC))
  {
    if (!strncmp(DEVICE_ON, (char *)p_payload, p_length))
    {
      LOG_DEBUG("Switch: Trigger ON");
      switch_on();
      publish_switch_status();
    }
    else if (!strncmp(DEVICE_OFF, (char *)p_payload, p_length))
    {
      LOG_DEBUG("Switch: Trigger OFF");
      switch_off();
      publish_switch_status();
    }
    else
    {
      LOG_WARN("Switch: Unknown payload!");
    }
  }
}

void mqtt_reconnect()
{
  while (!client.connected())
  {
    LOG_DEBUG("MQTT Connecting...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
    {
      LOG_INFO("MQTT Connected.");
      publish_switch_status();
      client.subscribe(MQTT_COMMAND_TOPIC);
    }
    else
    {
      LOG_DEBUG("MQTT Connection lost.");
      LOG_DEBUG("Retry in 5 seconds...");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(LED, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.setHostname("switch433");
  WiFi.begin(WiFi_SSID, WiFi_Password);
  WiFi.setAutoReconnect(true);

  LOG_INFO("Connecting wifi...");
  while (!WiFi.isConnected())
  {
    delay(1000);
    LOG_INFO("Waiting...");
  }
  LOG_INFO("WiFi connected.");
  LOG_INFO("WiFi IP:", WiFi.localIP().toString());

  switchState = false;
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
  LOG_INFO("Setup Done. VERSION:", VERSION);
}

void loop()
{
  if (!client.connected())
    mqtt_reconnect();
  client.loop();

  if (millis() - lastTime > UPDTAE_INTERVALS)
  {
    publish_switch_status();
    lastTime = millis();
  }
}