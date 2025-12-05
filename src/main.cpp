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

// GP2Y1010AUOF 설정
#define DUST_LED_PIN D4
#define DUST_ADC_PIN A0 

const int samplingTime = 280;
const int deltaTime = 40;
const int sleepTime = 9680;

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



  pinMode(DUST_LED_PIN, OUTPUT);
  digitalWrite(DUST_LED_PIN, HIGH);
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
  // --- GP2Y1010AUOF 데이터 읽기 (엄격한 타이밍 적용) ---
  digitalWrite(DUST_LED_PIN, LOW); // LED 켜기 (적외선 발사)
  delayMicroseconds(samplingTime); // 280us 대기
  
  float dustAdc = analogRead(DUST_ADC_PIN); // 값 읽기
  
  delayMicroseconds(deltaTime);    // 40us 대기
  digitalWrite(DUST_LED_PIN, HIGH); // LED 끄기
  delayMicroseconds(sleepTime);    // 9680us 대기

  // 미세 먼지 값 읽어오기
  float vo = dustAdc * (3.3 / 1024.0);

  float voc = 0.6; 
    float p = (vo - voc) * 200.0;

  if (p < 0)
  {
    p = 0.00;
  }


  if (!isnan(h) && !isnan(t))
  {
    // MQTT로 퍼블리시
    char payload[150];
    snprintf(payload, sizeof(payload),"{\"temperature\":%.2f,\"humidity\": %.2f, \"pm25\" : %.2f}", t, h, p);

    client.publish(mqtt_topic, payload);

    Serial.println(payload);
  }
  else
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  delay(1000); // 1초 간격 퍼블리시
}
 