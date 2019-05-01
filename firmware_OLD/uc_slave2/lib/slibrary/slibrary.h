/*
  * uc_slave2
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


#define SPEED_MAX 150   //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 5
#define  INT(x) (x-48)  //ascii convertion
#define iINT(x) (x+48)  //inverse ascii convertion

//Motor 1. variables para mover motor con velocidad y dirección determinadas.
int E1  = 5; //pwm pin D5
//Motor 2.
int E2  = 9; //pwm pin D9


//RST SETUP
uint8_t rst1 = 0;  uint8_t rst2 = 0;  uint8_t rst3 = 0;

//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;


uint8_t unload = 0;
uint8_t unload_save = 0;

uint8_t feed = 0;
uint8_t feed_save = 0;

String  message = "";
boolean stringComplete = false;


void receiveEvent() {
  while ( Wire.available() > 0 ) {
    byte x = Wire.read();
     message += (char) x;
     if ( (char) x == '\n' ) stringComplete = true;
  }
  //Serial.println(message);         // print the character
}

//for communication serial
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    message += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}


void bombas(int rst1, int rst2) {
  //bomba alimentacion
  if ( rst1 == 0) {
    if ( feed_save != feed ) {
      feed += 10;
      feed = map(feed, 40, 100, 102, 255);
      analogWrite(5, feed);
      feed_save = feed;
    }
  } else analogWrite(5, 0);

  //bomba descarga
  if ( rst2 == 0 ) {
    if ( unload_save != unload ) {
      unload += 5;
      unload = map(unload, 40, 100, 102, 255);
      analogWrite(9,unload);
      unload_save = unload;
    }
  } else analogWrite(9,0);

  return;
}


//clean strings
void clean_strings() {
  stringComplete = false;
  message    = "";
}

//Esquema I2C Concha y Toro:
//wphb015(-7) feed100unload100 mix1500(-7) temp150rst000 000(-3) dir111111(-9)
//feed100unload100temp150rst000 (29) => + "\n" = 30 bytes!
int validate_write() {
  if ( message[0] == 'w' ) {
    Serial.println("uc_slave2_echo w: " + message);
    return 1;
  }
  else {
    Serial.println("BAD");
    return 0;
  }
}


//wf100u100t029r110d111
//p0031d0002c02e0f0.4
void crumble() {
    if ( message[0] == 'w' && message[9] == 't' ) {
      feed   = message.substring(2,5).toInt();
      unload = message.substring(6,9).toInt();
      rst1 = int(INT(message[14]));  //rst_feed
      rst2 = int(INT(message[15]));  //rst_unload
      rst3 = int(INT(message[16]));  //rest_temp
    }
  return;
}
