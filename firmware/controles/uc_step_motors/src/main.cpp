/*
* uc_step_motor
* 01/05/2019
* ======================
* write by: Felipe Hooper
* Electronic Engineer
*/


/* nueva trama: 6-8-21
w f 0 2 0 u 0 2 0 t n  0  0  3  r  1  1  1  e  0
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
*/

#include <avr/wdt.h>
#include <TimerOne.h>
#include <slibrary.h>


void setup() {
  wdt_disable();

  Serial.begin(9600);

  DDRB = DDRB | (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
  DDRC = DDRC | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);
  DDRD = DDRD | (1<<PD3) | (1<<PD4) | (1<<PD5);



  Timer1.initialize(TIME_T);
  Timer1.attachInterrupt(motor_control);

  message.reserve(65);
  wdt_enable(WDTO_8S);
}



void loop() {
  if ( stringComplete ) {
    if ( validate_write() ) {
      Serial.println("Good message: " +  message);

      //se "desmenuza" el command de setpoints
      crumble();


      //Time setup for counters:
      if ( myfeed != myfeed_save ) {
        time_setup(myfeed, &count_m3_set, &count_m3);
        myfeed_save = myfeed;
      }

      if ( myunload != myunload_save ) {
        time_setup(myunload, &count_m4_set, &count_m4);
        myunload_save = myunload;
      }

      if ( mytemp != mytemp_save ) {
        time_setup(mytemp, &count_m5_set, &count_m5);  //setear en otra función que reciba este mensaje desde un lazo de control
        mytemp_save = mytemp;
      }

      //RST and DIR SETTING:
      //feed: rst1=0 (enable); dir1=1 (cw), else ccw.
      setup_dir_rst( _BV(RST_FEED), _BV(DIR_FEED),
                     &myfeed, &rst1, &dir1,
                     &PORTC,  &PORTC );

      //unload: rst4, dir4=null, PORT_2 = NULL.
      setup_dir_rst( _BV(RST_UNLOAD), _BV(NULL),
                     &myunload, &rst2, &dir2,
                     &PORTC,    &PORTC );

      //temp: rst5, dir3: PORT_1 distinto a PORT_2.
      setup_dir_rst( _BV(RST_TEMP), _BV(DIR_TEMP),
                     &mytemp, &rst3, &dir3,
                     &PORTC,  &PORTB );


      clean_strings();
      wdt_reset();
    }

    else {
      Serial.println("BAD message");
      clean_strings();
    }

  }


}
