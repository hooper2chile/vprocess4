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

#define TIME_T    20    //TIME_T [us]

#define SPEED_MAX 150.0 //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 1
#define TEMP_MAX  60

#define  INT(x) (x-48)  //ascii convertion
#define iINT(x) (x+48)  //inverse ascii convertion


#define REMONTAJE_PIN  4
#define AGUA_FRIA      10 //D10 = rele 1 (cable rojo)
#define AGUA_CALIENTE  11 //D11 = rele 2 (cable amarillo)
#define REMONTAJE_ON   digitalWrite(2,LOW)
#define REMONTAJE_OFF  digitalWrite(2,HIGH)

#define Gap_temp0 0.5
#define Gap_temp1 1.0       //1ºC
#define Gap_temp2 2.0
#define Gap_temp3 3.0
#define Gap_temp4 4.0
#define Gap_temp5 5.0
#define Gap_temp6 6.0
#define Gap_temp7 7.0
#define Gap_temp8 8.0
#define Gap_temp9 9.0

// RST setup
uint8_t rst1 = 1;  uint8_t rst2 = 1;  uint8_t rst3 = 1;
uint8_t rst1_save = 1;  uint8_t rst2_save = 1;  uint8_t rst3_save = 1;
//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;

uint8_t pump_enable = 0;

uint8_t mytemp_set = 0;
uint8_t mytemp_set_save = 0;

uint8_t unload = 0;
uint8_t unload_save = 0;

uint8_t feed = 0;
uint8_t feed_save = 0;

//variables control temperatura
float dTemp  = 0;
float Temp_ = 0;
uint8_t u_temp = 0;
float umbral_temp = SPEED_MAX;
//fin variables control temperatura

String message = "";
String state = "";
boolean stringComplete = false;


//Pines bomba temperatura uc_slave
int PWM2 = 6;
int IN3  = 7;
int IN4  = 8;
//fin pines de bomba temperatura

//for communication i2c
void receiveEvent() {
  while ( Wire.available() > 0 ) {
    byte x = Wire.read();
     message += (char) x;
     if ( (char) x == '\n' ) {
      stringComplete = true;
     }
  }  //Serial.println(message);         // print the character
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
//wf100u100t150r111d111
//wf100u100t022r000d000
//wf100u100t029r111d111
//p0031d0002c02e0f0.4
//p0031d0002c02e1f0.4
void viewer_message_slave (String command_lcd, int num) {
    if ( num == 1 ) {      //wf
      lcd.setCursor(0,0);
      lcd.print(command_lcd);
    }
    else if ( num == 2 ) { //pump_enable
      lcd.setCursor(10,0);
      lcd.print(command_lcd);
    }
    else if ( num == 3 ) { //Temp_
      lcd.setCursor(1,1);
      lcd.print(command_lcd);
    }
    return;
}

void speed_motor(int *PWM_PIN, int *pwm_set, int *A, int *B) {
    digitalWrite (A, LOW );
    digitalWrite (B, HIGH);

    analogWrite(PWM_PIN, (int) pwm_set);  // ¿NO hay que usarlas con * (punteros)?
    return;
}

void remontaje(int pump_enable) {
  if (pump_enable) digitalWrite(REMONTAJE_PIN, LOW);
  else digitalWrite(REMONTAJE_PIN, HIGH);

  return;
}

//ventilación puente-H
void cooler(int rst1, int rst2, int rst3) {
  if ( (rst1 == 1) && (rst2 == 1) && (rst3 == 1) ) digitalWrite(13, HIGH);
  else digitalWrite(13, LOW );

  return;
}

void bombas(int rst1, int rst2) {
  //bomba alimentacion
  if ( rst1 == 0) {
    if ( feed_save != feed ) {
      feed = map(feed+20, 0, SPEED_MAX, 0, 255);
      analogWrite(5, feed);
      feed_save = feed;
    }
  } else analogWrite(5, 0);

  //bomba descarga
  if ( rst2 == 0 ) {
    if ( unload_save != unload ) {
      unload = map(unload, 0, SPEED_MAX, 0, 255);
      analogWrite(9,unload);
      unload_save = unload;
    }
  } else analogWrite(9,0);

  return;
}

//Control temperatura para agua fria y caliente
void control_temp(int rst3) {
  if (rst3 == 0) {
    //touch my delta temp
    dTemp = mytemp_set - Temp_;

    //CASO: necesito calentar por que setpoint es inferior a la medicion
    if ( dTemp > 0 ) {
      delay(1);
      digitalWrite(AGUA_FRIA, HIGH);
      delay(1);
      digitalWrite(AGUA_CALIENTE, LOW);
    }
    //CASO: necesito enfriar por que medicion es mayor a setpoint
    else if ( dTemp < 0 ) {
      delay(1);
      digitalWrite(AGUA_FRIA, LOW);
      delay(1);
      digitalWrite(AGUA_CALIENTE, HIGH);
      dTemp = (-1)*dTemp;
    }

    if ( dTemp <= Gap_temp1 )
      u_temp = 0.28*umbral_temp;
    else if ( dTemp <= Gap_temp2 )
      u_temp = 0.31*umbral_temp;
    else if ( dTemp <= Gap_temp3 )
      u_temp = 0.35*umbral_temp;
    else if ( dTemp <= Gap_temp4 )
      u_temp = 0.45*umbral_temp;
    else if ( dTemp <= Gap_temp5 )
      u_temp = 0.50*umbral_temp;
    else if ( dTemp <= Gap_temp6 )
      u_temp = 0.60*umbral_temp;
    else if ( dTemp <= Gap_temp7 )
      u_temp = 0.65*umbral_temp;
    else if ( dTemp <= Gap_temp8 )
      u_temp = 0.75*umbral_temp;

    else if ( dTemp > Gap_temp9  )
      u_temp = 0.85*umbral_temp;

    u_temp = map(u_temp, 0, umbral_temp, 0, 255);
    speed_motor(PWM2, u_temp, IN3, IN4);

  }
  else {
    speed_motor(PWM2, 0, IN3, IN4);  //el sistema se deja stanby
    digitalWrite(AGUA_CALIENTE, HIGH);
    digitalWrite(AGUA_FRIA, HIGH);
  }
  return;
}

int validate_write() {
  if ( message[0] == 'w' ) {
    Serial.println("uc_slave_echo w: " + message);
    //String Reset = String(rst1)+String(rst2)+String(rst3);
    //Serial.println(Reset);
    return 1;
  }
  else if ( message[0] == 'p') {
    Serial.println("uc_slave_echo p: " + message);
    return 1;
  }
  else if ( message[0] == 't') {
    Serial.println("uc_slave_echo t: " + message);
    return 1;
  }

  else {
    //Serial.println("BAD");
    return 0;
  }
}

//wf100u100t150r111d111
//wf100u100t025r000d000
//wf100u100t029r110d111
//p0031d0002c02e0f0.4
//p0031d0002c02e1f0.4
void crumble() {
    if ( message[0] == 'w' && message[9] == 't' ) {
      mytemp_set = message.substring(10,13).toInt();
      feed   = message.substring(2,5).toInt();
      unload = message.substring(6,9).toInt();
      rst1 = int(INT(message[14]));  //rst_feed
      rst2 = int(INT(message[15]));  //rst_unload
      rst3 = int(INT(message[16]));  //rest_temp
      //viewer_message_slave(String(mytemp_set) + ' ' + String(rst3) + ' ' + String(dir3),1);
    }

    else if ( message[0] == 'p' && message[13] == 'e' ) {
      pump_enable = INT(message[14]);
      //viewer_message_slave(String(pump_enable),2);
    }

    else if (message[0] == 't') {
      Temp_ = message.substring(1).toFloat();
      //viewer_message_slave(String(Temp_),3);
    }

  return;
}

//clean strings
void clean_strings() {
  stringComplete = false;

  message    = "";
}
