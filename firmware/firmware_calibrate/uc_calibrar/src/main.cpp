/*
*  uc_controles
*  Writed by: Felipe Hooper
*  Electronic Engineer
*/

#include <avr/wdt.h>
#include "mlibrary.h"


void setup() {
  wdt_disable();

  Serial.begin(9600);
  Wire.begin(); //se inicia i2c master

  DDRB = DDRB | (1<<PB0) | (1<<PB5);
  PORTB = (0<<PB0) | (1<<PB5);

  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);

  message.reserve(65);
  wdt_enable(WDTO_8S);

  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); //encender alimentacion sensores atlas i2c
}

void loop() {
  if ( serial_event  ) {
      if ( validate() ) {
          //PORTB = 1<<PB0;
          switch ( message[0] ) {
            case 'c':
                calibrate_sensor();
                Serial.println("CALIBRADO!");
                break;
          }
          //wdt_reset(); //nuevo
          //PORTB = 0<<PB0;
      }
      else {
        Serial.println("bad validate (agua a 25 [°C]):" + message);
      }
    atlas_sensors();
    clean_strings();
    wdt_reset();
  }
}
