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
#include <TimerOne.h>
#include <slibrary.h>


void setup() {
  wdt_disable();

  Wire.begin(2);  //se inicia i2c slave
  Wire.onReceive(receiveEvent); // data slave recieved

  Serial.begin(9600);

/*
  Timer1.initialize(TIME_T);
  Timer1.attachInterrupt(motor_control);
*/

// LCD CONFIG
// set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
// Print a message to the LCD.
//lcd.setCursor(columna, fila); columna: [1,16]   fila: [0,1]
  lcd.setCursor(0, 0); //
  lcd.print("wf100u100");
  lcd.setCursor(0, 1);
  lcd.print("t150r111d111");
  delay(1000);


  message.reserve(65);
  wdt_enable(WDTO_8S);
}



void loop() {
  //viewer_message_slave();

  if ( stringComplete ) {
    if ( /*validate_write()*/1 ) {
      //Serial.println("Good message");
      Serial.println(message);
      viewer_message_slave(message);
      //se "desmenuza" el command de setpoints
      crumble();

      //###################################################################################
      //Codigo para bomba remontaje

      if (pump_enable) speed_motor(PWM1, 150, IN1, IN2);
      else speed_motor(PWM1, 0, IN1, IN2);



      //###################################################################################


      //###################################################################################
      //Codigo para motores DC.-





      //###################################################################################
      clean_strings();
      wdt_reset();
    }

    else {
      Serial.println("BAD message");
      clean_strings();
    }
  }
}
