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

/*
#include "rgb_lcd.h"
rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;
*/

#define TIME_T    20  //TIME_T [us]

#define SPEED_MAX 150   //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 1
#define TEMP_MAX  60

#define  INT(x) (x-48)  //ascii convertion
#define iINT(x) (x+48)  //inverse ascii convertion


uint8_t rst1 = 0;  uint8_t rst2 = 0;  uint8_t rst3 = 0;
uint8_t rst4 = 0;  uint8_t rst5 = 0;  uint8_t rst6 = 0;

//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;
uint8_t dir4 = 1;  uint8_t dir5 = 1;  uint8_t dir6 = 1;

uint8_t myphset  = 0;
uint8_t myph_a    = 0;
uint8_t myph_b    = 0;
uint8_t myph_a_save = 0;
uint8_t myph_b_save = 0;


uint8_t myfeed   = 0;
uint8_t myunload = 0;
uint8_t mymix    = 0;
uint8_t mytemp   = 0;

uint8_t myfeed_save   = 0;
uint8_t mytemp_save   = 0;
uint8_t myunload_save = 0;

uint8_t myph1_save    = 0;
uint8_t myph2_save    = 0;

String  message = "";
boolean stringComplete = false;

char    ph_var     = "";   String  ph_set     = "";
String  feed_var   = "";   String  feed_set   = "";
String  unload_var = "";   String  unload_set = "";
String  mix_var    = "";   String  mix_set    = "";
String  temp_var   = "";   String  temp_set   = "";


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
//message format write values: wpha140feed100unload100mix1500temp100rst111111dir111111
//wphb040feed010unload010mix1500temp010rst000000dir111111

//Esquema I2C Concha y Toro:
//wphb015(-7) feed100unload100 mix1500(-7) temp150rst000 000(-3) dir111111(-9)
//feed100unload100temp150rst000 (29) => + "\n" = 30 bytes!
int validate_write() {

  if (
    message[0] == 'w'                     &&

    message.substring(1, 3)   == "ph"     &&
    message.substring(7, 11)  == "feed"   &&
    message.substring(14, 20) == "unload" &&
    message.substring(23, 26) == "mix"    &&
    message.substring(30, 34) == "temp"   &&
    message.substring(37, 40) == "rst"    &&
    message.substring(46, 49) == "dir"    &&

    //ph number
    ( message[3] == 'a' || message[3] == 'b'         ) &&
    ( message.substring(4, 7).toFloat() <= SPEED_MAX ) &&


    //feed number
    ( message.substring(11, 14).toInt() >= 0   ) &&
    ( message.substring(11, 14).toInt() <= SPEED_MAX ) &&

    //unload number
    ( message.substring(20, 23).toInt() >= 0   ) &&
    ( message.substring(20, 23).toInt() <= SPEED_MAX ) &&

    //mix number
    ( message.substring(26, 30).toInt() >= 0   ) &&
    ( message.substring(26, 30).toInt() <= 1500) &&

    //temp number
    ( message.substring(34, 37).toInt() >= 0   ) &&
    ( message.substring(34, 37).toInt() <= SPEED_MAX ) &&

    //rst bits
    ( message[40] == iINT(1) || message[40] == iINT(0) ) &&
    ( message[41] == iINT(1) || message[41] == iINT(0) ) &&
    ( message[42] == iINT(1) || message[42] == iINT(0) ) &&
    ( message[43] == iINT(1) || message[43] == iINT(0) ) &&
    ( message[44] == iINT(1) || message[44] == iINT(0) ) &&
    ( message[45] == iINT(1) || message[45] == iINT(0) ) &&

    //dir bits
    ( message[49] == iINT(1) || message[49] == iINT(0) ) &&
    ( message[50] == iINT(1) || message[50] == iINT(0) ) &&
    ( message[51] == iINT(1) || message[51] == iINT(0) ) &&
    ( message[52] == iINT(1) || message[52] == iINT(0) ) &&
    ( message[53] == iINT(1) || message[53] == iINT(0) ) &&
    ( message[54] == iINT(1) || message[54] == iINT(0) ))
  {
    //Serial.println("GOOD");
    return 1;
  }

  else {
    //Serial.println("BAD");
    return 0;
  }
}

/*
void viewer_message_slave () {
  lcd.setCursor(10, 2);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
}
*/


//HAY QUE REHACER crumble para el nuevo esquema I2C
void crumble() {  //se puede alivianar usando .toFloat() directamente despues de substring
  //Serial.println("good");

  feed_var = message.substring(7, 11);
  feed_set = message.substring(11, 14);

  unload_var = message.substring(14, 20);
  unload_set = message.substring(20, 23);

  mix_var = message.substring(23, 26);    //variable remontaje
  mix_set = message.substring(26, 30);    //variable remontaje

  temp_var = message.substring(30, 34);
  temp_set = message.substring(34, 37);


  myfeed   = feed_set.toInt();
  myunload = unload_set.toInt();
  mymix    = mix_set.toInt();             //variable remontaje
  mytemp   = temp_set.toInt();

  //setting rst
  rst1 = INT(message[40]);  rst2 = INT(message[41]);  rst3 = INT(message[42]);
  rst4 = INT(message[43]);  rst5 = INT(message[44]);  rst6 = INT(message[45]);

  //setting dir
  dir1 = INT(message[49]);  dir2 = INT(message[50]);  dir3 = INT(message[51]);
  dir4 = INT(message[52]);  dir5 = INT(message[53]);  dir6 = INT(message[54]);

  return;
}
