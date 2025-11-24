#ifndef BME280_H
#define BME280_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class BME280 {
  public:
    /**
     * Constructor
     * @param addr: Alamat I2C (Default 0x76, bisa 0x77)
     */
    BME280(uint8_t addr = 0x76);

    /**
     * Inisialisasi Sensor BME280
     * @param sda_pin: Pin SDA (Default 21 untuk ESP32)
     * @param scl_pin: Pin SCL (Default 22 untuk ESP32)
     * @return bool: True jika sensor ditemukan, False jika gagal
     */
    bool begin(int sda_pin = 21, int scl_pin = 22);

    // --- GETTER FUNCTIONS ---
    
    /** Mengambil Suhu (Celsius) */
    float getTemperature();

    /** Mengambil Kelembaban (%) */
    float getHumidity();

    /** Mengambil Tekanan Udara (hPa) */
    float getPressure();

    /** * Mengambil Estimasi Ketinggian (Meter)
     * @param seaLevel: Tekanan permukaan laut saat ini (Default 1013.25)
     */
    float getAltitude(float seaLevel = 1013.25);

  private:
    Adafruit_BME280 _bme; // Objek driver Adafruit
    uint8_t _i2c_addr;
};

#endif