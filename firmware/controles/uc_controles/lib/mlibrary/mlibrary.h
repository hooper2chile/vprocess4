#include "Arduino.h"
#include "Wire.h"
#include "SoftwareSerial.h"
//#include "rgb_lcd.h"

SoftwareSerial mySerial(2, 3);  //RX(Digital2), TX(Digital3) Software serial port.

/*
rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;
*/
#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion

#define SPEED_MAX 150.0 //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 1
#define TEMP_MAX  60

#define REMONTAJE_PIN  A0 //bomba remontaje
#define AGUA_FRIA      A1 //D10 = rele 1 (cable rojo)
#define AGUA_CALIENTE  A2 //D11 = rele 2 (cable amarillo)
#define ELECTROVALVULA_AIRE  A3 //electrovalvula neumatica para presion de aire

char relay_temp = "";
String  new_write   = "wf000u000t000r111d111\n";
String  new_write0  = "";

String message = "";
String state   = "";

boolean stringComplete = false;  // whether the string is complete

//Re-formatting
String  uset_temp = "";
String  svar      = "";

// RST setup
uint8_t rst1 = 1;       uint8_t rst2 = 1;       uint8_t rst3 = 1;
uint8_t rst1_save = 1;  uint8_t rst2_save = 1;  uint8_t rst3_save = 1;
//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;

uint8_t pump_enable = 0;
uint8_t aire_enable = 0;

float mytemp_set = 0;
float mytemp_set_save = 0;

uint8_t unload = 0;
uint8_t unload_save = 0;

uint8_t feed = 0;
uint8_t feed_save = 0;

uint8_t u_temp_save = 0;


//variables control temperatura
float dTemp  = 0;
float Temp_  = 0;
float u_temp = 0;
float umbral_temp = SPEED_MAX;
//fin variables control temperatura
//******


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

//for hardware serial
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    message += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}



void remontaje(int pump_enable) {
  if (pump_enable) digitalWrite(REMONTAJE_PIN, LOW);
  else digitalWrite(REMONTAJE_PIN, HIGH);

  return;
}


void neumatica(int aire_enable) {
  if (aire_enable == 1)  digitalWrite(A3, LOW );
  else                   digitalWrite(A3, HIGH);
}


//function for transform numbers to string format of message
void format_message(int var) {
  //reset to svar string
  svar = "";
  if (var < 10)
    svar = "00"+ String(var);
  else if (var < 100)
    svar = "0" + String(var);
  else
    svar = String(var);
  return;
}


void broadcast_setpoint(uint8_t select) {
  //se prepara el setpoint para el renvio hacia uc_step_motor.
  format_message(u_temp_save); //string variable for control: uset_temp_save
  uset_temp = svar;

  switch (select) {
    case 0: //only re-tx and update pid uset's.
      new_write0 = "";
      //new_write0 = "wf" + new_write.substring(2,5) + 'u' + new_write.substring(6,9) + 't' + uset_temp + 'r' + new_write.substring(14,17) + 'd' + "111\n";
      new_write0 = new_write;
      mySerial.print(new_write0);
      break;

    case 1: //update command and re-tx.
      new_write  = "";
      //new_write  = "wf" + message.substring(2,5)  + 'u' + message.substring(6,9)    + 't' + uset_temp + 'r' + message.substring(14,17)   + 'd' + "111\n";
      new_write = message;
      mySerial.print(new_write);
      break;

    default:
      break;
  }

  return;
}


void clean_strings() {
  //clean strings
  stringComplete = false;
  message   = "";
  uset_temp = "";
}



void crumble() {
    if ( message[0] == 'w' && message[9] == 't' ) {
      mytemp_set = message.substring(11,14).toFloat();
      feed   = message.substring(2,5).toInt();

      unload = message.substring(6,9).toInt();
      rst1 = int(INT(message[15]));  //rst_feed
      rst2 = int(INT(message[16]));  //rst_unload
      rst3 = int(INT(message[17]));  //rest_temp

      //trama remontaja unificada el 29-09-19 con la trama de setpoint
      pump_enable = INT(message[19]);
      relay_temp = message[10]; // c: caliente, e: frio, n: nada

      //aire_enable = INT(message[21]); //6 agosto 2021: falta agregarlo en la tratama que viene desde app.py/communication.py, luego en uc_sensores en la funcion forward()
    }
    return;
}


void reles_temp(){
  if (relay_temp == 'e') {
    digitalWrite(AGUA_CALIENTE, HIGH);
    delay(1);
    digitalWrite(AGUA_FRIA,      LOW);
    delay(1);
  }
  else if (relay_temp == 'c') {
    digitalWrite(AGUA_CALIENTE,  LOW);
    delay(1);
    digitalWrite(AGUA_FRIA,     HIGH);
    delay(1);
  }
  else {
    digitalWrite(AGUA_CALIENTE, HIGH);
    delay(1);
    digitalWrite(AGUA_FRIA,     HIGH);
    delay(1);
  }
  return;
}


int validate_write() {
  if ( message[0] == 'w' && message[9] == 't' && message[14] == 'r' && message[5] == 'u' && message[18] == 'e' && ( message[19] == '0' || message[19] == '1') ) {
    Serial.println("echo: " + message);
    return 1;
  }
  else {
    Serial.println("BAD command to uc_controles");
    return 0;
  }
}
