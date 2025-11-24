#ifndef PH4502C_H
#define PH4502C_H

#include <Arduino.h>

class PH4502C {
  public:
    /**
     * Constructor
     * @param pin: Pin ADC (GPIO 36/39/etc)
     */
    PH4502C(uint8_t pin);

    /**
     * Inisialisasi Pin
     */
    void begin();

    /**
     * Memasukkan data kalibrasi 2 titik (pH 7 dan pH 4)
     * Fungsi ini akan otomatis menghitung Slope (m) dan Intercept (c)
     * @param v_ph7: Tegangan saat probe di buffer pH 7.0
     * @param v_ph4: Tegangan saat probe di buffer pH 4.0
     */
    void setCalibration(float v_ph7, float v_ph4);

    /**
     * Mendapatkan tegangan stabil (untuk proses debugging/kalibrasi)
     */
    float getVoltage();

    /**
     * Mendapatkan nilai pH Akhir (Sudah dikompensasi suhu)
     * @param temperature: Suhu air saat ini (dari DS18B20). Default 25.0 C.
     */
    float getPH(float temperature = 25.0);

  private:
    uint8_t _pin;
    float _v_ph7;
    float _v_ph4;
    float _slope;
    float _intercept;
    const int _samples = 30; // Jumlah sampel smoothing

    // Fungsi internal kalkulasi Slope & Intercept
    void _recalculateParams();
    
    // Fungsi internal pembacaan ADC stabil
    float _readStableVoltage();
};

#endif