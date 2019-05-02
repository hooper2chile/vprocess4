#include "Arduino.h"
#include "Wire.h"
#include "SoftwareSerial.h"
#include "rgb_lcd.h"

SoftwareSerial mySerial(2, 3);  //RX(Digital2), TX(Digital3) Software serial port.

rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion

#define SPEED_MAX 100.0 //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 1
#define TEMP_MAX  60

#define REMONTAJE_PIN  4
#define AGUA_FRIA      10 //D10 = rele 1 (cable rojo)
#define AGUA_CALIENTE  11 //D11 = rele 2 (cable amarillo)

#define k0 0.1
#define k1 0.2//0.2
#define k2 0.3//0.3
#define k3 0.4//0.4
#define k4 0.5
#define k5 0.6
#define k6 0.7
#define k7 0.8
#define k8 0.9
#define k9 1.0

#define Gap_temp0 0.5
#define Gap_temp1 1.0    //1�C
#define Gap_temp2 2.0
#define Gap_temp3 3.0
#define Gap_temp4 4.0
#define Gap_temp5 5.0
#define Gap_temp6 7.0    //6.0
#define Gap_temp7 9.0    //7.0
#define Gap_temp8 11.0   //8.0
#define Gap_temp9 13.0   //9.0

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

uint8_t mytemp_set = 0;
uint8_t mytemp_set_save = 0;

uint8_t unload = 0;
uint8_t unload_save = 0;

uint8_t feed = 0;
uint8_t feed_save = 0;

uint8_t u_temp_save = 0;


//variables control temperatura
float dTemp  = 0;
float Temp_ = 0;
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

void remontaje(int pump_enable) {
  if (pump_enable) digitalWrite(REMONTAJE_PIN, LOW);
  else digitalWrite(REMONTAJE_PIN, HIGH);

  return;
}

//Control temperatura para agua fria y caliente
void control_temp(int rst3) {
  if (rst3 == 0) {
    //touch my delta temp
    dTemp = mytemp_set - Temp_;

    //CASO: necesito calentar por que setpoint es inferior a la medicion
    if ( dTemp >= -0.1 ) {
      delay(1);
      digitalWrite(AGUA_FRIA, HIGH);
      delay(1);
      digitalWrite(AGUA_CALIENTE, LOW);
    }
    //CASO: necesito enfriar por que medicion es mayor a setpoint
    else if ( dTemp < 0.2 ) {
      delay(1);
      digitalWrite(AGUA_FRIA, LOW);
      delay(1);
      digitalWrite(AGUA_CALIENTE, HIGH);
      dTemp = (-1)*dTemp;
    }

    if ( dTemp <= Gap_temp0 )
      u_temp = k0*umbral_temp;
    else if ( dTemp <= Gap_temp1 )
      u_temp = k1*umbral_temp;
    else if ( dTemp <= Gap_temp2 )
      u_temp = k2*umbral_temp;
    else if ( dTemp <= Gap_temp3 )
      u_temp = k3*umbral_temp;
    else if ( dTemp <= Gap_temp4 )
      u_temp = k4*umbral_temp;
    else if ( dTemp <= Gap_temp5 )
      u_temp = k5*umbral_temp;
    else if ( dTemp <= Gap_temp6 )
      u_temp = k6*umbral_temp;
    else if ( dTemp <= Gap_temp7 )
      u_temp = k7*umbral_temp;
    else if ( dTemp <= Gap_temp8 )
      u_temp = k8*umbral_temp;
    else if ( dTemp > Gap_temp9  )
      u_temp = k9*umbral_temp;
  }
  else {
    //el sistema se deja stanby
    digitalWrite(AGUA_CALIENTE, HIGH);
    digitalWrite(AGUA_FRIA, HIGH);
  }

  u_temp_save = int(u_temp);

  return;
}

//Esquema I2C Concha y Toro:
//TRAMA-Proceso  : wf000u000t029r111d000  //21 caracteres: 22 sumando el '\n'
//TRAMA-Remontaje: p1440d0001c03e1f0.2    //19 caracteres: 20 sumando el '\n'
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

//function for transform numbers to string format of message
String format_message(int var) {
  //reset to svar string
  svar = "";
  if (var < 10)
    svar = "00"+ String(var);
  else if (var < 100)
    svar = "0" + String(var);
  else
    svar = String(var);
  return svar;
}

void broadcast_setpoint(uint8_t select) {
  //se prepara el setpoint para el renvio hacia uc_step_motor.
  uset_temp = format_message(u_temp_save); //string variable for control: uset_temp_save

  switch (select) {
    case 0: //only re-tx and update pid uset's.
      new_write0 = "";
      //new_write0 = new_write.substring(0,3) + uset_ph + new_write.substring(7,34) + uset_temp + new_write.substring(37,55) + "\n";
      new_write0 = "wf" + new_write.substring(2,5) + 'u' + new_write.substring(6,9) + 't' + uset_temp + 'r' + new_write.substring(14);
      mySerial.print(new_write0);
      break;

    case 1: //update command and re-tx.
      new_write  = "";
      //new_write = message.substring(0,3) + uset_ph + message.substring(7,34) + uset_temp + message.substring(37,55) + "\n";
      new_write  = "wf" + message.substring(2,5)  + 'u' + message.substring(6,9)    + 't' + uset_temp + 'r' + message.substring(14);
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


int validate_write() {
  if ( message[0] == 'w' ) {
    Serial.println("uc_slave_echo w : " + message);
    Serial.println("uc_slave_forward: " + new_write0);
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
