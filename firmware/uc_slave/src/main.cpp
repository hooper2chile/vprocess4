/*
 * uc_slave: DC Motors with encoders
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
#include <avr/wdt.h>
#include <slibrary.h>

void setup() {
 wdt_disable();

 message.reserve(65);

 //pinout setting para puente H motor bomba temperatura
 pinMode(PWM2, OUTPUT);
 pinMode(IN3, OUTPUT);
 pinMode(IN4, OUTPUT);
 analogWrite(PWM2, 0);
 digitalWrite(IN3, HIGH);
 digitalWrite(IN4, HIGH);

 //ventilador
 pinMode(13, OUTPUT);
 digitalWrite(13, HIGH);
 //pinout setting para puente H motor bomba temperatura

 //electrovavulas control de temparatura
 pinMode(AGUA_FRIA, OUTPUT);
 pinMode(AGUA_CALIENTE, OUTPUT);
 digitalWrite(AGUA_FRIA, HIGH);
 digitalWrite(AGUA_CALIENTE, HIGH);
 //electrovavulas control de temparatura

 //bomba remontaje
 pinMode(REMONTAJE_PIN, OUTPUT);
 digitalWrite(REMONTAJE_PIN, HIGH); //inicio apagado de bomba remontaje
 //bomba remontaje

 //setting de giro bombas alimentacion y descarga
 pinMode(5, OUTPUT);
 pinMode(9, OUTPUT);
 pinMode(12, OUTPUT);
 digitalWrite(12, HIGH); //IN2 e IN4 a VCC. IN1 e IN3 estan a GND.
 //setting de giro bombas alimentacion y descarga

 Wire.begin(2);  //se inicia i2c slave con direccion: 2
 Wire.onReceive(receiveEvent); // data slave recieved

 Serial.begin(9600);

 lcd.begin(16, 2);
 lcd.setRGB(colorR, colorG, colorB);

 wdt_enable(WDTO_8S);
}

void loop() {
 if ( stringComplete ) {
   if ( validate_write() ) {
     //se "desmenuza" el command de setpoints
     crumble();
     cooler(rst1, rst2, rst3);

     //###################################################################################
     //Codigo para bomba remontaje
     remontaje(pump_enable);
     //###################################################################################

     //nuevo control de temperatura con agua fria y caliente (no PID)
     control_temp(rst3);
     // fin control temperatura

     //###################################################################################
     bombas(rst1, rst2);
     //###################################################################################
     wdt_reset();
   }

   else {
     Serial.println("BAD message");
   }
   clean_strings();
 }
 //wdt_reset();
}
