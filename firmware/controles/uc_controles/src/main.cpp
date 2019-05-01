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
  mySerial.begin(9600);
  //mixer1.begin(9600);

  message.reserve(65);

  DDRB = DDRB | (1<<PB0) | (1<<PB5);
  PORTB = (0<<PB0) | (1<<PB5);

  wdt_enable(WDTO_8S);
}

void loop() {
  if ( stringComplete  ) {
      if ( validate_write() ) {
        //se "desmenuza" el command de setpoints
        crumble();
        //###################################################################################
        //Codigo para bomba remontaje
        remontaje(pump_enable);
        //###################################################################################
        //nuevo control de temperatura con agua fria y caliente (no PID)
        control_temp(rst3);
        // fin control temperatura

        broadcast_setpoint(1);
        //PORTB = 0<<PB0;
      }
      else {
        Serial.println("bad validate");
      }

    clean_strings();
    wdt_reset();
  }
}
