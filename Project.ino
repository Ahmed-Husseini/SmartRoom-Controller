#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define ssid "HOME 2"
#define password "AAE@2021"

#define DHTPIN 21
#define FanPIN 19
#define CPIN 18
#define ACPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8  // 8-bit resolution: duty cycle 0–255

const char broker[] = "broker.emqx.io";
const int port = 1883;

long long last_time = 0;

float Temperature = 0;
float Humidity = 0;
bool ON = false;
bool Auto = false;
int Speed = 0;
bool Direction = false;
float MaxTemp = 0;
float MaxHum = 0;
bool Alarm = false;

WiFiClient wificlient;
PubSubClient client(wificlient);

void callback(const char topic[], byte* payload, unsigned int length) {
  String msg = String((char*)payload).substring(0, length);

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(msg);

  if (strcmp(topic, "/FanController/ON") == 0) {
    ON = (msg == "ON");
  }

  else if (strcmp(topic, "/FanController/Auto") == 0) {
    Auto = (msg == "Auto");
  }

  else if (strcmp(topic, "/FanController/Speed") == 0) {
    Speed = msg.toInt();
  }

  else if (strcmp(topic, "/FanController/Direction") == 0) {
    Direction = (msg == "Clockwise");
  }

  else if (strcmp(topic, "/FanController/MaxTemp") == 0) {
    MaxTemp = msg.toFloat();
  }

  else if (strcmp(topic, "/FanController/MaxHum") == 0) {
    MaxHum = msg.toFloat();
  }
}

void setup() {
  pinMode(FanPIN, OUTPUT);
  pinMode(CPIN, OUTPUT);
  pinMode(ACPIN, OUTPUT);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(FanPIN, PWM_CHANNEL);

  dht.begin();

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println("IP: " + WiFi.localIP().toString());

  client.setServer(broker, port);
  client.setCallback(callback);

  Serial.print("\nConnecting to broker");
  while (!client.connect("tyujnb")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to broker");

  client.subscribe("/FanController/ON");
  client.subscribe("/FanController/Auto");
  client.subscribe("/FanController/Speed");
  client.subscribe("/FanController/Direction");
  client.subscribe("/FanController/MaxTemp");
  client.subscribe("/FanController/MaxHum");

  Serial.println("\nReady to publish messages");

  last_time = millis();
}

void loop() {
  client.loop();
  if ((millis() - last_time) >= 2000) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    if (!isnan(temp))
      Temperature = temp;
    if (!isnan(hum))
      Humidity = hum;
    last_time = millis();
  }
  String tempStr = String(Temperature, 2);
  String humStr = String(Humidity, 2);
  client.publish("/FanController/Temp", tempStr.c_str());
  client.publish("/FanController/Humidity", humStr.c_str());

    if ((Temperature > MaxTemp) || (Humidity > MaxHum)) {
      client.publish("/FanController/Alarm", "Yes");
      Alarm = true;
  }
  else {
      client.publish("/FanController/Alarm", "No");
      Alarm = false;
  }

  int pwmValue = 0;

  if (!Auto) {
    if (ON)
      pwmValue = Speed; // User-defined speed
    else
      pwmValue = 0;
  } else {
    if (Alarm) {
      int tempRation = ((Temperature - MaxTemp) / MaxTemp) * 100;
      int humRation = ((Humidity - MaxHum) / MaxHum) * 100;
      if (tempRation > humRation)
        pwmValue = tempRation;
      else
        pwmValue = humRation;
    }
    else
      pwmValue = 0;
  }

pwmValue = constrain(pwmValue, 0, 255); // Ensure it's within 8-bit range
ledcWrite(PWM_CHANNEL, pwmValue);


  if (Direction) {
    digitalWrite(CPIN, HIGH);
    digitalWrite(ACPIN, LOW);
  }
  else {
    digitalWrite(CPIN, LOW);
    digitalWrite(ACPIN, HIGH);
  }

}
