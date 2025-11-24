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

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);       // Resolusi 12-bit (0-4095)
  analogSetAttenuation(ADC_11db); // Range 0-3.3V

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

    Serial.print("DSTemp: ");
    Serial.print(tempC);
    Serial.print(" | Temp: ");
    Serial.print(tempBME280);
    Serial.print(" | Hum: ");
    Serial.print(humBME280);
    Serial.print(" | press: ");
    Serial.print(pressBME280);
    Serial.print(" | PH: ");
    Serial.print(phValue, 2);
    Serial.print(" | NTU: ");
    Serial.print(ntuValue, 0);
    Serial.println(" NTU");
  }
}