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
              case 'w':
                daqmx();
                forward();  //broadcast_setpoint() de vprocess4 es diferente al de vprocess6! ajustar al 6 aqui tambien!
                control_temp();         //lo mismo que en control_ph()
                co2_sensor();
                rtds_sensors();
                //clean_strings();
                break;


              case 'c':
                calibrate_sensor();
                Serial.println("CALIBRADO!");
                //clean_strings();
                break;

              case 'u':
                actuador_umbral();
                //clean_strings();
                break;
              /*
              case 'p': //remontaje set
                //Serial.println("p :" + message); //debug
                daqmx();
                broadcast_setpoint(1);
                break;
              */
              default:
                //clean_strings();
                break;
          }
          //wdt_reset(); //nuevo
          //PORTB = 0<<PB0;
      }
      else {
        Serial.println("bad validate:" + message);
      }
    clean_strings();
    wdt_reset();
  }
}
