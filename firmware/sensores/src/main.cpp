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
  ads1.begin();
  //ads2.begin();
  //                                           ADS1015  ADS1115
  //                                           -------  -------
  ads1.setGain(GAIN_ONE);      // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  //ads2.setGain(GAIN_ONE);
  //ads.setGain(GAIN_TWO);     // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV

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
              case 'r':
                rtds_sensors();
                daqmx();
                broadcast_setpoint(0);
		            //broadcast_setpoint(1); //probando el 2 de marzo de 2020
                //calibrate_sensor();
                //Serial.println("CALIBRADO!");
                break;

              case 'w':
                //Serial.println("w: " + message); //debug
                daqmx();
                broadcast_setpoint(1);
                break;

              case 'c':
                calibrate_sensor();
                Serial.println("CALIBRADO!");
                break;

              case 'u':
                actuador_umbral();
                break;
              /*
              case 'p': //remontaje set
                //Serial.println("p :" + message); //debug
                daqmx();
                broadcast_setpoint(1);
                break;
              */
              default:
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
