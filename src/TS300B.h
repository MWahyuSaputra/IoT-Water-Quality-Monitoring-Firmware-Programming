#ifndef TS300B_H
#define TS300B_H

#include <Arduino.h>

class TS300B {
  public:
    /**
     * Constructor
     * @param pin: Pin GPIO ESP32 dimana sensor terhubung
     */
    TS300B(uint8_t pin);

    /**
     * Inisialisasi sensor (Setting PinMode)
     */
    void begin();

    /**
     * Mengatur nilai kalibrasi tegangan
     * @param v_clear: Tegangan saat air jernih (0 NTU)
     * @param v_dirty: Tegangan saat gelap total/sangat keruh (1000 NTU)
     */
    void setCalibration(float v_clear, float v_dirty);

    /**
     * Membaca tegangan stabil setelah proses filtering (Median)
     * @return float: Tegangan dalam Volt (0.0 - 3.3V)
     */
    float getVoltage();

    /**
     * Membaca nilai kekeruhan akhir dalam NTU
     * @return float: Nilai NTU (0 - 1000)
     */
    float getNTU();

  private:
    uint8_t _pin;
    float _v_clear;
    float _v_dirty;
    const int _samples = 40;

    // Fungsi untuk membaca ADC Raw dengan Median Filter
    float _readStableVoltage();
};

#endif