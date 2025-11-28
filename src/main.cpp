#include <WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "BME280.h"
#include "PH4502C.h"
#include "TS300B.h"

#define ONE_WIRE_BUS  4
#define PIN_SDA       21  
#define PIN_SCL       22  
#define PIN_PH_SENSOR 36  
#define PIN_TURBIDITY 39
const byte PUMP = 2;

const char* ssid        = "Wifi Magister Terapan";
const char* password    = "";
const char* mqtt_server = "broker.emqx.io";
const char* mqtt_topic_data    = "iot/water_quality/data";   // Topik Publish Data
const char* mqtt_topic_control = "iot/water_quality/relay";  // Topik Subscribe Kontrol

WiFiClient espClient;
PubSubClient client(espClient);

bool pumpState = false;      // Status Relay (false=OFF, true=ON)

unsigned long lastMsg = 0;
const long interval = 1000;   // Interval kirim data (1 detik)
#define MSG_BUFFER_SIZE  (200)
char msg[MSG_BUFFER_SIZE];

unsigned long previousMillis = 0;

// kalibrasi pH
const float PH7_VOLTAGE = 2.335; // pH 7
const float PH4_VOLTAGE = 3.083; // pH 4

// kalibrasi Turbidity
const float CALIB_CLEAR_VOLT = 1.500; // Air Jernih (0 NTU)
const float CALIB_DIRTY_VOLT = 0.178; // Air Keruh (1000 NTU)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
PH4502C phMeter(PIN_PH_SENSOR);
TS300B  turbidity(PIN_TURBIDITY);
BME280  BME280_Temp(0x76); // Alamat I2C 0x76

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void SerialMonitor();
void Send_to_Dashboard ();

void setup() {
  Serial.begin(115200);
  // Setup Relay Pin
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, LOW);

  setup_wifi();
  
  // Setup MQTT
  client.setServer(mqtt_server, 1883); // Port 1883 untuk ESP32 (TCP)
  client.setCallback(callback);        // Callback

  // Setup ADC
  analogReadResolution(12);       // Resolusi 12-bit (0-4095)
  analogSetAttenuation(ADC_11db); // Range 0-3.3V
  
  // Init DS18B20
  sensors.begin();
  // Init BME280
  if (!BME280_Temp.begin(PIN_SDA, PIN_SCL)) {
    Serial.println("[ERROR] Sensor BME280");
  }
  // Init Turbidity
  turbidity.begin();
  turbidity.setCalibration(CALIB_CLEAR_VOLT, CALIB_DIRTY_VOLT);
  // Init pH
  phMeter.begin();
  phMeter.setCalibration(PH7_VOLTAGE, PH4_VOLTAGE);
}

void loop() {
  Send_to_Dashboard();
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    randomSeed(micros()); // Seed agar random() lebih acak
  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Konversi payload ke String agar mudah dicek
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.println(messageTemp);

  // --- LOGIKA KONTROL POMPA ---
  // Jika pesan = "ON", nyalakan Relay (LOW atau HIGH tergantung modul relay Anda)
  // Kebanyakan modul relay aktif LOW (LOW = Nyala)
  if (String(topic) == mqtt_topic_control) {
    if(messageTemp == "ON"){
      digitalWrite(pumpState, HIGH); // Sesuaikan HIGH/LOW dengan relay Anda
      pumpState = true;
      Serial.println("Action: Relay ON");
    }
    else if(messageTemp == "OFF"){
      digitalWrite(pumpState, LOW);
      pumpState = false;
      Serial.println("Action: Relay OFF");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      
      client.subscribe(mqtt_topic_control);
      Serial.print("Subscribed to: ");
      Serial.println(mqtt_topic_control);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void SerialMonitor() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    
    previousMillis = currentMillis;
    sensors.requestTemperatures();
    // float pH_voltage = phMeter.getVoltage();
    // float Turbidity_voltage = turbidity.getVoltage();
    
    float tempC = sensors.getTempCByIndex(0);
    float tempBME280 = BME280_Temp.getTemperature();
    float humBME280  = BME280_Temp.getHumidity();
    float pressBME280 = BME280_Temp.getPressure();

    float ntuValue = turbidity.getNTU();
    float phValue = phMeter.getPH();

    Serial.print("DSTemp: ");     Serial.print(tempC);
    Serial.print(" | Temp: ");    Serial.print(tempBME280);
    Serial.print(" | Hum: ");     Serial.print(humBME280);
    Serial.print(" | press: ");   Serial.print(pressBME280);
    Serial.print(" | PH: ");      Serial.print(phValue, 2);
    Serial.print(" | NTU: ");     Serial.println(ntuValue, 0);
}

//Dummy function to simulate data sending
void Send_to_Dashboard (){
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Wajib dipanggil agar bisa terima pesan

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float tempWater = 25.0 + (random(0, 50) / 10.0); 
    float tempAir = 28.0 + (random(0, 40) / 10.0);
    float hum = 60.0 + (random(0, 200) / 10.0);
    float press = 1008.0 + random(0, 8);
    float phVal = 6.5 + (random(0, 200) / 100.0);
    float ntuVal = random(0, 50);

    // Format: {"temp_water":25.5, "temp_air":28.2, "hum":65.0, "press":1010, "ph":7.25, "ntu":10}
    snprintf(msg, MSG_BUFFER_SIZE, 
      "{\"temp_water\":%.2f, \"temp_air\":%.2f, \"hum\":%.1f, \"press\":%.0f, \"ph\":%.2f, \"ntu\":%.0f}",
      tempWater, tempAir, hum, press, phVal, ntuVal
    );

    Serial.print("Publish message: ");
    Serial.println(msg);
    
    client.publish(mqtt_topic_data, msg);
  }
}