#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi 설정
const char *ssid = "bssm_free";
const char *password = "bssm_free";

// MQTT 브로커 설정
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_topic = "slide/D~HT";

WiFiClient espClient;
PubSubClient client(espClient);

// DHT11 설정
#define DHTPIN 4 // NodeMCU D2 핀(GPIO4)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);

  // WiFi 연결
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  // DHT 시작
  dht.begin();

  // MQTT 초기화
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect()
{
  // MQTT 연결 시도
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client"))
    {
      Serial.println("MQTT Connected!");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // DHT11 데이터 읽기
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t))
  {
    // MQTT로 퍼블리시
    char payload[32];
    snprintf(payload, sizeof(payload), "Temp: %.2f C, Hum: %.2f %%", t, h);

    client.publish(mqtt_topic, payload);

    Serial.println(payload);
  }
  else
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  delay(5000); // 5초 간격 퍼블리시
}
