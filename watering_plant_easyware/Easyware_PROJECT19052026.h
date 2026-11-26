#ifndef _EASYWARE_PROJECT19052026H_
#define _EASYWARE_PROJECT19052026H_

#include "Arduino.h"
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>

enum DisplayType { OLED, LCD_I2C, SERIAL_ONLY };
const uint8_t MAX_RELAY = 8;
const uint16_t TANPA_RELAY = 0xFFFF;
const uint16_t EASYWARE_SOIL_ADC_MAX = 1023;

// Objek Global
DHT* dht;
Adafruit_SSD1306* displayOLED = nullptr;
LiquidCrystal_I2C* displayLCD = nullptr;

DisplayType _currentDisplay;
uint16_t _soilPin;
uint16_t _relayPins[MAX_RELAY];
uint8_t _jumlahRelay = 0;
bool _relayAktif = false;
bool _showRaw = false; // Variabel kontrol RAW data
float KELEMBAPAN;
float SUHU;

typedef void (*CustomDisplayFunc)(float t, float h, int s);
CustomDisplayFunc _userRender = nullptr;

// Menambahkan parameter showRaw (default false)
void main_PARAM(DisplayType type, uint8_t DHTTYPE, uint8_t DHTPIN, uint16_t soilPin, bool showRaw = false) {
    _currentDisplay = type;
    _soilPin = soilPin;
    _showRaw = showRaw; 
    dht = new DHT(DHTPIN, DHTTYPE);

    if (_currentDisplay == OLED) {
        displayOLED = new Adafruit_SSD1306(128, 64, &Wire, -1);
    } else if (_currentDisplay == LCD_I2C) {
        displayLCD = new LiquidCrystal_I2C(0x27, 16, 2);
    }
}

void setCustomDisplay(CustomDisplayFunc fungsiUser) {
    _userRender = fungsiUser;
}

void renderDisplay(float t, float h, int soilPersen, int soilRaw) {
    if (_currentDisplay == OLED && displayOLED) {
        displayOLED->clearDisplay();
        displayOLED->setTextSize(1);
        displayOLED->setTextColor(SSD1306_WHITE);
        displayOLED->setCursor(0,0);
        displayOLED->print(F("Temp: ")); displayOLED->print(t);
        displayOLED->setCursor(0,10);
        if (_showRaw) {
            displayOLED->print(F("Soil RAW: ")); displayOLED->print(soilRaw);
        } else {
            displayOLED->print(F("Soil: ")); displayOLED->print(soilPersen); displayOLED->print(F(" %"));
        }
        displayOLED->display();
    } 
    else if (_currentDisplay == LCD_I2C && displayLCD) {
        displayLCD->setCursor(0,0);
        displayLCD->print(F("Temp: ")); displayLCD->print((int)t); displayLCD->print(F(" C   "));
        displayLCD->setCursor(0,1);
        if (_showRaw) {
            displayLCD->print(F("Soil RAW: ")); displayLCD->print(soilRaw); displayLCD->print(F("  "));
        } else {
            displayLCD->print(F("Soil: ")); displayLCD->print(soilPersen); displayLCD->print(F(" %   "));
        }
    } 
}

void setupRelay(const uint16_t relayPins[], uint8_t jumlahRelay) {
    _jumlahRelay = 0;
    _relayAktif = false;

    if (relayPins == nullptr || jumlahRelay == 0) return;
    if (jumlahRelay > MAX_RELAY) jumlahRelay = MAX_RELAY;

    for (uint8_t i = 0; i < jumlahRelay; i++) {
        if (relayPins[i] == TANPA_RELAY) continue;

        _relayPins[_jumlahRelay] = relayPins[i];
        pinMode(_relayPins[_jumlahRelay], OUTPUT);
        digitalWrite(_relayPins[_jumlahRelay], HIGH);
        _jumlahRelay++;
    }

    _relayAktif = (_jumlahRelay > 0);
}

void setupSensorDisplay() {
    if (_showRaw || _currentDisplay == SERIAL_ONLY) {
        Serial.begin(9600);
    }

    if (dht) dht->begin();
    if (_currentDisplay == OLED && displayOLED) displayOLED->begin(SSD1306_SWITCHCAPVCC, 0x3C);
    if (_currentDisplay == LCD_I2C && displayLCD) { displayLCD->init(); displayLCD->backlight(); }
}

void prog_SETUP(uint16_t relayPin) {
    const uint16_t relayPins[] = { relayPin };
    setupRelay(relayPins, 1);
    setupSensorDisplay();
}

void prog_SETUP(const uint16_t relayPins[], uint8_t jumlahRelay) {
    setupRelay(relayPins, jumlahRelay);
    setupSensorDisplay();
}

void pilihJumlahRelay(const uint16_t relayPins[], uint8_t jumlahRelay) {
    setupRelay(relayPins, jumlahRelay);
}

void setRelayState(bool aktif) {
    if (!_relayAktif) return;

    for (uint8_t i = 0; i < _jumlahRelay; i++) {
        digitalWrite(_relayPins[i], aktif ? LOW : HIGH);
    }
}

void prog_MAIN(uint16_t BATAS_KERING, uint16_t b_HIGH, uint16_t b_LOW) {
    int soilRawAsli = analogRead(_soilPin);
    // Nilai RAW tampilan dibalik: kecil = tanah kering, besar = tanah basah.
    int soilRaw = EASYWARE_SOIL_ADC_MAX - soilRawAsli;
    
    if (dht) {
        KELEMBAPAN = dht->readHumidity();
        SUHU = dht->readTemperature();
    }

    // Nilai tampilan: 0% = paling kering, 100% = paling basah.
    // Parameter kalibrasi tetap memakai nilai ADC asli agar pemanggilan lama tetap kompatibel.
    int rawKering = EASYWARE_SOIL_ADC_MAX - b_HIGH;
    int rawBasah = EASYWARE_SOIL_ADC_MAX - b_LOW;
    int kelembapanSoil = map(soilRaw, rawKering, rawBasah, 0, 100);
    kelembapanSoil = constrain(kelembapanSoil, 0, 100);

    if (_relayAktif) {
        // BATAS_KERING adalah kelembapan minimum sebelum penyiraman dimulai.
        // Contoh 30: relay ON saat kelembapan tanah 30% atau lebih rendah.
        setRelayState(kelembapanSoil <= BATAS_KERING);
    }

    // LOGIKA SERIAL RAW DATA
    if (_showRaw) {
        Serial.print(F("[RAW] Soil: ")); Serial.print(soilRaw);
        Serial.print(F(" | Temp: ")); Serial.print(SUHU);
        Serial.print(F(" | Humi: ")); Serial.println(KELEMBAPAN);
    }

    if (_userRender != nullptr) {
        _userRender(SUHU, KELEMBAPAN, kelembapanSoil);
    } else {
        renderDisplay(SUHU, KELEMBAPAN, kelembapanSoil, soilRaw);
        // Default output serial jika mode SERIAL_ONLY tapi RAW tidak aktif
        if (_currentDisplay == SERIAL_ONLY && !_showRaw) {
            Serial.print(F("Soil: ")); Serial.print(kelembapanSoil);
            Serial.print(F("% | Temp: ")); Serial.println(SUHU);
        }
    }
}

#endif
