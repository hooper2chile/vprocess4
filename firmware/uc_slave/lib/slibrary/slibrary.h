/*
  * uc_slave
  *======================
  * RESET, DIR SETUP
  * rst3, dir2: rst3 is myph_a (acido), dir2 is for two bombs
  * rst1, dir1: myfeed
  * rst4, dir456: unload  (dir4 para unload, no se usa)
  * rst2, dir456: mymix
  * rst5, dir3: mytemp
  * rst6, dir6: rst6 is myph_b, dir2 is for two bombs  dir6 is FREE !


    write by: Felipe Hooper
    Electronic Engineer
*/


#include "Arduino.h"
#include <Wire.h>


#include "rgb_lcd.h"
rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;


#define TIME_T    20  //TIME_T [us]

#define SPEED_MAX 150   //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 1
#define TEMP_MAX  60

#define  INT(x) (x-48)  //ascii convertion
#define iINT(x) (x+48)  //inverse ascii convertion

// RST setup
uint8_t rst1 = 0;  uint8_t rst2 = 0;  uint8_t rst3 = 0;

//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;

uint8_t mymix    = 0;
uint8_t mytemp   = 0;
uint8_t pump_enable = 0;

uint8_t mytemp_save   = 0;


String  message = "";
boolean stringComplete = false;

char    ph_var     = "";   String  ph_set     = "";
String  feed_var   = "";   String  feed_set   = "";
String  unload_var = "";   String  unload_set = "";
String  mix_var    = "";   String  mix_set    = "";
String  temp_var   = "";   String  temp_set   = "";


//Pines bomba remontaje uc_slave
const int PWM1 = 3;
const int IN1  = 2;
const int IN2  = 4;
//fin pines de remontaje


void receiveEvent() {
  while ( Wire.available() > 0 ) {
    byte x = Wire.read();
     message += (char) x;
     if ( (char) x == '\n' ) stringComplete = true;
  }
  //Serial.println(message);         // print the character
}

//clean strings
void clean_strings() {
  stringComplete = false;

  ph_var     = "";  feed_var = "";  unload_var = "";  temp_var = "";
  mix_var    = "";  ph_set   = "";  feed_set   = "";  temp_set = "";
  unload_set = "";  mix_set  = "";  message    = "";
}

int validate_write() {

  if ( message[0] == 'w' )
  {
    //Serial.println("GOOD");
    return 1;
  }

  else {
    //Serial.println("BAD");
    return 0;
  }
}

//wf100u100t150r111d111
//p0031d0002c02e0f0.4
void viewer_message_slave (String command_lcd) {
    lcd.setCursor(0, 0);
    lcd.print(command_lcd.substring(0,9));
    lcd.setCursor(0, 1);
    lcd.print(command_lcd.substring(9,21));

    return;
}


void speed_motor(int *PWM_PIN, int *pwm_set, int *A, int *B) {
    digitalWrite (A, LOW );
    digitalWrite (B, HIGH);

    analogWrite(PWM_PIN, (int) pwm_set);
}


//HAY QUE REHACER crumble para el nuevo esquema I2C
void crumble() {
    if (message[0] == 'w') {
      if (message[14] == 't') {
        mytemp = message.substring(15,18).toInt();
        rst3 = INT(message[21]);
        dir3 = INT(message[25]);
      }
    }

    else if (message[0] == 'p') {
       if (message[13] == 'e') pump_enable = INT(message[14]);
       if (message[15] == 'p') mymix = message.substring(16,19).toInt();
    }

  return;
}
