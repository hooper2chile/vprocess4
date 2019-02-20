/*
*  uc_master
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
  //ads.setGain(GAIN_TWO);    // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV


  DDRB = DDRB | (1<<PB0) | (1<<PB5);
  PORTB = (0<<PB0) | (1<<PB5);

  message.reserve(65);
  wdt_enable(WDTO_8S);
}


void loop() {
  //viewer_message_master();

  if ( stringComplete  ) {
      if ( validate() ) {
          PORTB = 1<<PB0;

          switch ( message[0] ) {
              case 'r':
                hamilton_sensors();
                daqmx();
                control_temp();
                //control_temp_pid();
                broadcast_setpoint(0);
                break;

              case 'w':
                write_crumble();
                control_temp();
                broadcast_setpoint(1);
                tx_reply();
                //setpoint();
		            //daqmx();
                break;

              case 'c':
                sensor_calibrate();
                break;

              case 'u':
                actuador_umbral();
                break;

              case 'a': //autoclave set
                remontaje_setpoints();
                break;


              default:
                break;
          }

          PORTB = 0<<PB0;
      }
      else {
        Serial.println("bad validate");
      }

    clean_strings();
    wdt_reset();
  }
}
