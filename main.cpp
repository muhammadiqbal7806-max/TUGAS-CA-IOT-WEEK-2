#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>


#define DHTPIN 4      
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define MQ2_PIN 34     

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int retry = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry++;
    if (retry > 30) {
      Serial.println("Restarting ESP32…");
      ESP.restart();
    }
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");

    String clientId = "ESP32_Client_";
    clientId += String(random(0xffff), HEX); 

    if (client.connect(clientId.c_str())) {
      Serial.println("connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" — retry in 3 seconds");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);

  pinMode(MQ2_PIN, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int mq2Value = analogRead(MQ2_PIN);
  if (!isnan(humidity) && !isnan(temperature)) {

    char tempStr[8];
    dtostrf(temperature, 1, 2, tempStr);
    client.publish("esp32/dht22/temperature", tempStr);

    char humStr[8];
    dtostrf(humidity, 1, 2, humStr);
    client.publish("esp32/dht22/humidity", humStr);

  } else {
    Serial.println("Failed to read DHT22 sensor!");
  }

  char mqStr[10];
  sprintf(mqStr, "%d", mq2Value);
  client.publish("esp32/mq2", mqStr);

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | Hum: ");
  Serial.print(humidity);
  Serial.print(" % | MQ2: ");
  Serial.println(mq2Value);

  delay(2000); 
}
