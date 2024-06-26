#include "bridge.h"

#define LED D4
#define VERSION "0.1"

#define RECV_PIN D3
#define SEND_PIN D10

// -------------------- Configurations -----------------------------
const PROGMEM char *WiFi_SSID = "Ireina";
const PROGMEM char *WiFi_Password = "pa5sw0rd";

const PROGMEM char *MQTT_CLIENT_ID = "rf-bridge";
const PROGMEM char *MQTT_SERVER_IP = "homeassistant";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char *MQTT_USER = "mqtt";
const PROGMEM char *MQTT_PASSWORD = "mqtt";

const char *MQTT_COMMAND_TOPIC = "home/rf-bridge/set";
const char *MQTT_STATE_TOPIC = "home/rf-bridge";
// ------------------------------------------------------------------

WiFiClient wifiClient;
PubSubClient client(wifiClient);
RCSwitch recvSwitch, sendSwitch;

void blink()
{
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
}

void callback(char *p_topic, byte *p_payload, unsigned int p_length)
{
  if (!strcmp(p_topic, MQTT_COMMAND_TOPIC))
  {
    char *payload = (char *)p_payload;
    char *separator = strchr(payload, '|');

    if (separator)
    {
      *separator = '\0';
      unsigned length = atoi(payload);
      unsigned long code = strtoul(separator + 1, nullptr, 16);
      LOG_INFO("Send:", length, '|', code);
      sendSwitch.send(code, length);
      blink();
    }
  }
}

void rc_recv()
{
  blink();
  String data = recvSwitch.getReceivedBitlength() + '|' + String(recvSwitch.getReceivedValue(), HEX);
  LOG_INFO("Received:", data);
  LOG_INFO("Protocol:", recvSwitch.getReceivedProtocol());
  recvSwitch.resetAvailable();
  client.publish(MQTT_STATE_TOPIC, data.c_str(), data.length());
}

void mqtt_reconnect()
{
  while (!client.connected())
  {
    LOG_DEBUG("MQTT Connecting...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
    {
      LOG_INFO("MQTT Connected.");
      client.subscribe(MQTT_COMMAND_TOPIC);
    }
    else
    {
      LOG_WARN("MQTT Connection lost.");
      LOG_WARN("Retry in 5 seconds...");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  recvSwitch.enableReceive(RECV_PIN);
  sendSwitch.enableTransmit(SEND_PIN);

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

  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
  LOG_INFO("Setup Done. Version:", VERSION);
  blink();
}

void loop()
{
  if (!client.connected())
    mqtt_reconnect();
  client.loop();
  if (recvSwitch.available())
    rc_recv();
}
