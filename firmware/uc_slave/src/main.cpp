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

  Wire.begin(2);  //se inicia i2c slave con direccion: 2
  Wire.onReceive(receiveEvent); // data slave recieved

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);

  message.reserve(65);

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

  wdt_enable(WDTO_8S);
}

void loop() {
  if ( stringComplete ) {
    if ( validate_write() ) {
      //se "desmenuza" el command de setpoints
      crumble();

      //###################################################################################
      //Codigo para bomba remontaje
      if (pump_enable) digitalWrite(REMONTAJE_PIN, LOW);
      else digitalWrite(REMONTAJE_PIN, HIGH);
      //###################################################################################
      //###################################################################################
      //Codigo para motor DC temperatura.-

      //sera necesario?
      //###################################################################################
      //nuevo control de temperatura con agua fria y caliente (no PID)
      control_temp(rst3);
    }

    else {
      Serial.println("BAD message");
    }
    clean_strings();
  }
  wdt_reset();
}
