#include "BME280.h"

// Constructor
BME280::BME280(uint8_t addr) {
  _i2c_addr = addr;
}

// Inisialisasi
bool BME280::begin(int sda_pin, int scl_pin) {
  // Inisialisasi I2C pada pin spesifik ESP32
  Wire.begin(sda_pin, scl_pin);

  // Coba hubungkan ke sensor
  // Mengembalikan true jika sukses, false jika gagal
  return _bme.begin(_i2c_addr, &Wire);
}

float BME280::getTemperature() {
  return _bme.readTemperature();
}

float BME280::getHumidity() {
  return _bme.readHumidity();
}

float BME280::getPressure() {
  // Driver mengembalikan Pascal (Pa), kita konversi ke hPa (dibagi 100)
  return _bme.readPressure() / 100.0F;
}

float BME280::getAltitude(float seaLevel) {
  return _bme.readAltitude(seaLevel);
}