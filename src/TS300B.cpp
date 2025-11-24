#include "TS300B.h"

// Constructor: Menyimpan pin
TS300B::TS300B(uint8_t pin) {
  _pin = pin;
  // Default calibration values
  _v_clear = 1.500; 
  _v_dirty = 0.100;
}

// Init
void TS300B::begin() {
  pinMode(_pin, INPUT);
}

// Setter untuk Kalibrasi
void TS300B::setCalibration(float v_clear, float v_dirty) {
  _v_clear = v_clear;
  _v_dirty = v_dirty;
}

// Mengambil tegangan stabil (Public API)
float TS300B::getVoltage() {
  return _readStableVoltage();
}

// Menghitung NTU (Public API)
float TS300B::getNTU() {
  float voltage = _readStableVoltage();
  float ntu = 0;

  // Logika Mapping Linear Terbalik
  if (voltage >= _v_clear) {
    ntu = 0; // Sangat Jernih
  } else if (voltage <= _v_dirty) {
    ntu = 1000; // Sangat Keruh
  } else {
    ntu = 1000.0 * (_v_clear - voltage) / (_v_clear - _v_dirty);
  }
  
  return ntu;
}

// Median Filter & Smoothing
float TS300B::_readStableVoltage() {
  int buffer_arr[_samples];

  //Sampling
  for (int i = 0; i < _samples; i++) {
    buffer_arr[i] = analogRead(_pin);
    delay(5); // Jeda stabilisasi sinyal
  }

  // Sorting (Bubble Sort) - Kecil ke Besar
  for (int i = 0; i < _samples - 1; i++) {
    for (int j = i + 1; j < _samples; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        int temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // Ambil Median
  int medianADC = buffer_arr[_samples / 2];

  // Konversi ke Volt (Standard ESP32)
  // Menggunakan referensi 3.3V sesuai standar hardware
  float volts = (medianADC / 4095.0) * 3.3;

  return volts;
}