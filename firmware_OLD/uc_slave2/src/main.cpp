/*
  * uc_slave2: DC Motors with encoders
  *======================
  * RESET, DIR SETUP
  * rst3, dir2: FREE
  * rst1, dir1: myfeed
  * rst4, dir456: unload  (dir4 para unload, no se usa)
  * rst2, dir456: mymix
  * rst5, dir3: mytemp
  * rst6, dir6: FREE !

  write by: Felipe Hooper
  Electronic Engineer

*/


#include <slibrary.h>
#include <avr/wdt.h>

void setup() {
  wdt_disable();

  Wire.begin(3);  //se inicia i2c slave2 on direccion: 3
  Wire.onReceive(receiveEvent); // data slave recieved

  Serial.begin(9600);
  message.reserve(65);

  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);
  analogWrite(E1, 0);
  analogWrite(E2, 0);



  wdt_enable(WDTO_8S);
}


void loop() {
  if ( stringComplete ) {
   if ( validate_write() ) {
     //se "desmenuza" el command de setpoints
     crumble();
     bombas(rst1, rst2);
   }
   else {
     Serial.println("BAD message");
   }
   clean_strings();
   wdt_reset();
 }
 //wdt_reset();
}
