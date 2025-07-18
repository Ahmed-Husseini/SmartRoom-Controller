#include <WiFi.h>                 // WiFi library for network connection
#include <PubSubClient.h>         // MQTT client library
#include <DHT.h>                  // DHT sensor library

// WiFi credentials
#define ssid "HOME 2"
#define password "AAE@2021"

// Pin definitions
#define DHTPIN 21
#define FanPIN 19
#define CPIN 18
#define ACPIN 5

// DHT sensor setup
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// PWM configuration
#define PWM_FREQ 25000
#define PWM_RESOLUTION 8  // 8-bit resolution: duty cycle 0–255
int ledChannel;

// MQTT broker settings
const char broker[] = "broker.emqx.io"; // Using free public broker
const int port = 1883;

// Variables for time and environment readings
long long last_time = 0;
float Temperature = 0;
float Humidity = 0;

// Control flags and values
bool ON = false;
bool Auto = false;
int Speed = 0;
bool Direction = false;
float MaxTemp = 0;
float MaxHum = 0;
bool Alarm = false;

// WiFi and MQTT client objects
WiFiClient wificlient;
PubSubClient client(wificlient);


// MQTT callback function to handle incoming messages
void callback(const char topic[], byte* payload, unsigned int length) {
  String msg = String((char*)payload).substring(0, length);

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(msg);

  // Handle different subscribed topics
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


// Initial setup function
void setup() {
  pinMode(FanPIN, OUTPUT);
  pinMode(CPIN, OUTPUT);
  pinMode(ACPIN, OUTPUT);

  // Attach PWM to fan pin
  ledChannel = ledcAttach(FanPIN, PWM_FREQ, PWM_RESOLUTION);

  dht.begin();  // Initialize DHT sensor

  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println("IP: " + WiFi.localIP().toString());

  // Connect to MQTT broker
  client.setServer(broker, port);
  client.setCallback(callback);

  Serial.print("\nConnecting to broker");
  while (!client.connect("tyujnb")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to broker");

  // Subscribe to relevant topics
  client.subscribe("/FanController/ON");
  client.subscribe("/FanController/Auto");
  client.subscribe("/FanController/Speed");
  client.subscribe("/FanController/Direction");
  client.subscribe("/FanController/MaxTemp");
  client.subscribe("/FanController/MaxHum");

  Serial.println("\nReady to publish messages");

  last_time = millis();
}


// Main loop function
void loop() {
  client.loop();  // Handle incoming/outgoing MQTT messages

  // Read temperature and humidity every 2 seconds
  if ((millis() - last_time) >= 2000) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    if (!isnan(temp))
      Temperature = temp;
    if (!isnan(hum))
      Humidity = hum;
    last_time = millis();
  }

  // Publish temperature and humidity values
  String tempStr = String(Temperature, 2);
  String humStr = String(Humidity, 2);
  client.publish("/FanController/Temp", tempStr.c_str());
  client.publish("/FanController/Humidity", humStr.c_str());

  // Determine if alarm should be triggered
  if ((Temperature > MaxTemp) || (Humidity > MaxHum)) {
    client.publish("/FanController/Alarm", "Yes");
    Alarm = true;
  }
  else {
    client.publish("/FanController/Alarm", "No");
    Alarm = false;
  }

  // PWM and fan speed logic
  int pwmValue = 0;
  int tempRation = 0;
  int humRation = 0;

  if (!Auto) {
    if (ON)
      pwmValue = map(Speed, 0, 100, 0, 255);
    else
      pwmValue = 0;
  } else {
    if (Alarm) {
      if ((Temperature - MaxTemp) > 0)
        tempRation = (int)(((Temperature - MaxTemp) / MaxTemp) * 255);
      if ((Humidity - MaxHum) > 0)
        humRation = (int)(((Humidity - MaxHum) / MaxHum) * 255);
      pwmValue = max(tempRation, humRation);
    }
    else
      pwmValue = 0;
  }

  // Clamp PWM value between 0 and 255
  pwmValue = constrain(pwmValue, 0, 255);

  // Write PWM value to fan pin
  ledcWrite(FanPIN , pwmValue);

  // Set fan direction
  if (Direction) {
    digitalWrite(CPIN, HIGH);
    digitalWrite(ACPIN, LOW);
  }
  else {
    digitalWrite(CPIN, LOW);
    digitalWrite(ACPIN, HIGH);
  }
}
