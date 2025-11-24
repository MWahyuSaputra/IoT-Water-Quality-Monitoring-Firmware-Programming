#include "PH4502C.h"

// Constructor
PH4502C::PH4502C(uint8_t pin) {
  _pin = pin;
  // Nilai Default Kalibrasi (Jika user lupa setCalibration)
  _v_ph7 = 2.500; 
  _v_ph4 = 3.000;
  _recalculateParams(); // Hitung slope default
}

void PH4502C::begin() {
  pinMode(_pin, INPUT);
}

void PH4502C::setCalibration(float v_ph7, float v_ph4) {
  _v_ph7 = v_ph7;
  _v_ph4 = v_ph4;
  // Setiap kali data kalibrasi berubah, hitung ulang m dan c
  _recalculateParams();
}

void PH4502C::_recalculateParams() {
  // Rumus Garis Linear: pH = m * Voltage + c
  // m (Slope) = (pH2 - pH1) / (V2 - V1)
  _slope = (7.0 - 4.0) / (_v_ph7 - _v_ph4);
  
  // c (Intercept) = pH - (m * V)
  _intercept = 7.0 - (_slope * _v_ph7);
}

float PH4502C::getVoltage() {
  return _readStableVoltage();
}

float PH4502C::getPH(float temperature) {
  float voltage = _readStableVoltage();

  // 1. Hitung pH Raw (Tanpa Suhu)
  float ph_raw = (_slope * voltage) + _intercept;

  // 2. Kompensasi Suhu (Nernst Equation)
  // pH koreski = pH_raw + (pH_raw - 7) * (T - 25) * 0.003
  float ph_compensated = ph_raw + (ph_raw - 7.0) * (temperature - 25.0) * 0.003;

  return ph_compensated;
}

// --- FUNGSI FILTER (Sama dengan TS300B untuk konsistensi) ---
float PH4502C::_readStableVoltage() {
  int buffer_arr[_samples];

  // 1. Sampling
  for (int i = 0; i < _samples; i++) {
    buffer_arr[i] = analogRead(_pin);
    delay(10); 
  }

  // 2. Sorting (Bubble Sort)
  for (int i = 0; i < _samples - 1; i++) {
    for (int j = i + 1; j < _samples; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        int temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // 3. Median Filter (Ambil tengah)
  int medianADC = buffer_arr[_samples / 2];

  // 4. Konversi ke Volt (Standard ESP32 3.3V)
  // Jika di PCB pakai divider 1k:1k, kalikan dengan 2.0 di sini!
  // Untuk skenario "Direct Connection" atau yang sudah dikompensasi di v_ph7:
  float volts = (medianADC / 4095.0) * 3.3;

  return volts;
}