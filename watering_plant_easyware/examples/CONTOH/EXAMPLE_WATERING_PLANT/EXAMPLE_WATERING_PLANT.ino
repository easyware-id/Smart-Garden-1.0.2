#include "Easyware_PROJECT19052026.h"

//LIBRARY ini dibuat dan diuji oleh R.SA dan disahkan oleh Easyware Electronics
//dengan tujuan mempermudah kehidupan WNI

//library Automatic Watering Plant


// --- MODIFIKASI TAMPILAN SESUKA HATI DISINI ---
// void desainSaya(float t, float h, int soil) {
//   if (_currentDisplay == OLED) {
//     displayOLED->clearDisplay();
//     displayOLED->setCursor(30, 0);                               <----------kalau oled begini
//     displayOLED->print("MY PROJECT"); // Judul custom
//     displayOLED->setCursor(0, 20);
//     displayOLED->print("Temp: "); displayOLED->print(t);
//     displayOLED->display();
//   } 
//   else if (_currentDisplay == LCD_I2C) {
//     displayLCD->setCursor(0,0);
//     displayLCD->print("Suhu: "); displayLCD->print(t);          <------kalau LCD seperti ini
//   }
// }                                                              Tidak pakai if else juga bisa atur atur aja

void setup() {
  main_PARAM(OLED, DHT11, A2, A0, false); //  <---------kalau ini saja pakai tampilan bawaan
            //A                     A
            //|                      \ Aktifkan kalau mau liat RAW data (tapi hanya diSERIAL_ONLY)
            //|                       
  //bisa diubah dengan OLED, LCD, DAN SERIAL


  // Aktifkan baris ini jika ingin pakai desain di atas
  //setCustomDisplay(desainSaya); <--------- kalau pakai buatan sendiri sesuaikan dengan nama functionnya
  
  prog_SETUP(7); // <------- masukan pin relay kalau ada, jika tidak tulis aja -1
}

void loop() {
  // 30 berarti relay menyala saat kelembapan tanah 30% atau lebih rendah.
  // 1023 dan 200 itu untuk kalibrasi sensor soil, masukan RAW data paling kering dan paling basah
  prog_MAIN(30, 1023, 200); 
  delay(1000);   // kasih delay
}
