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

  //bomba remontaje
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH); //inicio apagado de bomba remontaje
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
      if (pump_enable) digitalWrite(2,LOW);
      else digitalWrite(2,HIGH);
      //###################################################################################
      //###################################################################################
      //Codigo para motor DC temperatura.-
      //###################################################################################
      //nuevo control de temperatura con agua fria y caliente (no PID)
      control_temp(rst3);
      //speed_motor(PWM2, 200, IN3, IN4);
    }

    else {
      Serial.println("BAD message");
    }
    clean_strings();
  }
  wdt_reset();
}
