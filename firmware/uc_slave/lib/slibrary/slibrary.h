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

#define ON  LOW
#define OFF HIGH
#define AGUA_FRIA      10 //D12
#define AGUA_CALIENTE  11 //D11
#define REMONTAJE_ON   digitalWrite(2,LOW)
#define REMONTAJE_OFF  digitalWrite(2,HIGH)

#define Gap_temp0 0.5
#define Gap_temp1 1.0       //1ºC
#define Gap_temp2 2.0
#define Gap_temp3 3.0
#define Gap_temp4 5.0

// RST setup
uint8_t rst1 = 0;  uint8_t rst2 = 0;  uint8_t rst3 = 1;

//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;

uint8_t pump_enable = 0;
uint8_t mytemp_set  = 0;
uint8_t mytemp_set_save = 0;

//variables control temperatura
float dTemp  = 0;
float dTemp2 = 0;
float Temp_ = 0;
uint8_t u_temp = 0;
float umbral_temp = SPEED_MAX;
//fin variables control temperatura

String message = "";
String state = "";
boolean stringComplete = false;


//Pines bomba remontaje uc_slave
int PWM1 = 3;
int IN1  = 2;
int IN2  = 4;
//fin pines de bomba remontaje

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
}

//Control temperatura para agua fria y caliente
void control_temp(int rst3) {
  if (rst3 == 0) {
    //touch my delta temp
    dTemp = mytemp_set - Temp_;

    //CASO: necesito calentar por que setpoint es inferior a la medicion
    if ( dTemp >= 0.0 ) {
      delay(10);
      digitalWrite(AGUA_FRIA, OFF);
      delay(10);
      digitalWrite(AGUA_CALIENTE, ON);

      if ( dTemp <= Gap_temp1 )
        u_temp = 0.20*umbral_temp; //20%
      else if ( dTemp <= Gap_temp2 )
        u_temp = 0.35*umbral_temp; //35%
      else if ( dTemp <= Gap_temp3 )
        u_temp = 0.50*umbral_temp; //50%
      else if ( dTemp <= Gap_temp4 )
        u_temp = 0.75*umbral_temp; //75%
      else if ( dTemp > Gap_temp4 )
        u_temp = umbral_temp;     //100%
    }

    //CASO: necesito enfriar por que medicion es mayor a setpoint
    else if ( dTemp < 0.0 ) {
      delay(10);
      digitalWrite(AGUA_FRIA, ON);
      delay(10);
      digitalWrite(AGUA_CALIENTE, OFF);

      dTemp2 = (-1)*dTemp;

      if ( dTemp2 <= Gap_temp1 )
        u_temp = 0.20*umbral_temp; //20%
      else if ( dTemp2 <= Gap_temp2 )
        u_temp = 0.35*umbral_temp; //35%
      else if ( dTemp2 <= Gap_temp3 )
        u_temp = 0.50*umbral_temp; //50%
      else if ( dTemp2 <= Gap_temp4 )
        u_temp = 0.75*umbral_temp; //75%
      else if ( dTemp2 > Gap_temp4 )
        u_temp = umbral_temp;     //100%
    }

    u_temp = map(u_temp, 0, umbral_temp, 0, 255);
    speed_motor(PWM2, u_temp, IN3, IN4);

  }
  else speed_motor(PWM2, 0, IN3, IN4);

  return;
}

int validate_write() {
  if ( message[0] == 'w' ) {
    Serial.println("uc_slave_echo w: " + message);
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
      rst3 = int(INT(message[16]));
      viewer_message_slave(String(mytemp_set) + ' ' + String(rst3) + ' ' + String(dir3),1);
    }

    else if ( message[0] == 'p' && message[13] == 'e' ) {
      pump_enable = INT(message[14]);
      viewer_message_slave(String(pump_enable),2);
    }

    else if (message[0] == 't') {
      Temp_ = message.substring(1).toFloat();
      viewer_message_slave(String(Temp_),3);
    }

  return;
}

//clean strings
void clean_strings() {
  stringComplete = false;

  message    = "";
}
