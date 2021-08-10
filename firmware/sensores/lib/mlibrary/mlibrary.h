#include "Arduino.h"
#include <Wire.h>

#define rtd1 101
#define rtd2 102
#define co2s 105

#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion

#define SPEED_MIN 2.0
#define SPEED_MAX 150     //[RPM]
#define TEMP_MAX  60      //[ºC]


#define AGUA_FRIA      A1 //D10 = rele 1 (cable rojo)
#define AGUA_CALIENTE  A2 //D11 = rele 2 (cable amarillo)
#define VENTILADOR     A3 //ventilador

#define Gap_temp0 0.5
#define Gap_temp1 1.0    //1C
#define Gap_temp2 2.0
#define Gap_temp3 3.0
#define Gap_temp4 4.0
#define Gap_temp5 5.0
#define Gap_temp6 7.0    //6.0
#define Gap_temp7 9.0    //7.0
#define Gap_temp8 11.0   //8.0
#define Gap_temp9 13.0   //9.0

#define Gap_pH_0  0.05
#define Gap_pH_1  0.10     // 0.1 (pH)
#define Gap_pH_2  0.50
#define Gap_pH_3  0.75
#define Gap_pH_4  1.00
#define Gap_pH_5  2.00

float myph_set   = 0;
int myfeed_set   = 0;
int myunload_set = 0;
int mymix_set    = 0;

uint8_t u_temp_save = 0;
float mytemp_set = 25;
float dTemp  = 0;
int u_temp = 0;

String ph_select   = "n";
String temp_select = "n";
String svar = "";
float u_ph = 0;
float dpH  = 0;
float pH   = 0;

String message     = "";
String new_write_w = "wf000u000tn000r111e0\n";


boolean stringComplete = false;  // whether the string is complete

//RESET SETUP
uint8_t rst1 = 1;  uint8_t rst2 = 1;  uint8_t rst3 = 1;

//DIRECTION SETUP
char dir1 = 1;  char dir2 = 1;  char dir3 = 1;

// for incoming serial data
float Byte0 = 0;  char cByte0[15] = "";
float Byte1 = 0;  char cByte1[15] = "";
float Byte2 = 0;  char cByte2[15] = "";
float Byte3 = 0;  char cByte3[15] = "";
float Byte4 = 0;  char cByte4[15] = "";
float Byte5 = 0;  char cByte5[15] = "";
float Byte6 = 0;  char cByte6[15] = "";
float Byte7 = 0;  char cByte7[15] = "";  //for Temp2
//nuevo
float Byte8 = 0;  char cByte8[15] = "";  //for setpont confirmation //no se necesita: 28-9-19

//calibrate function()
char  var = '0';
float umbral_a, umbral_b, umbral_temp;



float Temp1 = 0;
float Temp2 = 0;
float Temp_ = 0.5 * ( Temp1 + Temp2 );

float Temp1_save = 0;
float Temp2_save = 0;
unsigned int l,j,k,m;

float flujo = 0.0;


byte received_from_computer = 0; //we need to know how many characters have been received.
byte serial_event = 0;           //a flag to signal when data has been received from the pc/mac/other.
byte code = 0;                   //used to hold the I2C response code.

char RTD_data[20];
char RTD_data1[20];
char RTD_data2[20];               //we make a 20 byte character array to hold incoming data from the RTD circuit.

//co2 variables
char co2_data[20];
float co2   = 0;



byte in_char = 0;                //used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte i = 0;                      //counter used for RTD_data array.

int time_ = 600;                 //used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.





//nueva funcion 4-8-21 FH. medicion de co2
void co2_sensor() {
  Wire.beginTransmission(co2s);                                                 //call the circuit by its ID number.
  Wire.write('r');

  Wire.endTransmission();                                                       //end the I2C data transmission.
  if (strcmp('r', "sleep") != 0) {                                              //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(co2s, 20, 1);                                              //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();                                                         //the first byte is the response code, we read this separately.
     i = 0;
     while (Wire.available()) {            //are there bytes to receive.
      in_char = Wire.read();              //receive a byte.
      co2_data[i] = in_char;              //load this byte into our array.
      i += 1;                             //incur the counter for the array element.
      if (in_char == 0) {                 //if we see that we have been sent a null command.
        i = 0;                            //reset the counter i to 0.
        break;                            //exit the while loop.
      }
    }
    co2 = atof(co2_data);
  }
  serial_event = false;                   //reset the serial event flag.
  return;
}




void rtd1_sensor() {
  Wire.beginTransmission(rtd1);                                                 //call the circuit by its ID number.
  Wire.write('r');
  Wire.endTransmission();                                                       //end the I2C data transmission.
  i = 0;
  if (strcmp('r', "sleep") != 0) {                                              //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(rtd1, 20, 1);                                              //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();                                                         //the first byte is the response code, we read this separately.

    while (Wire.available()) {
      in_char = Wire.read();
      RTD_data1[i] = in_char;
      i += 1;
      if (in_char == 0) {
        i = 0;
        break;
      }
    }
  Temp1 = atof(RTD_data1);
  }
  serial_event = false;                   //reset the serial event flag.
  return;
}


void rtd2_sensor() {
  Wire.beginTransmission(rtd2);                                                 //call the circuit by its ID number.
  Wire.write('r');
  Wire.endTransmission();                                                       //end the I2C data transmission.
  i = 0;
  if (strcmp('r', "sleep") != 0) {                                              //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(rtd2, 20, 1);                                              //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();                                                         //the first byte is the response code, we read this separately.

    while (Wire.available()) {            //are there bytes to receive.
      in_char = Wire.read();              //receive a byte.
      RTD_data2[i] = in_char;              //load this byte into our array.
      i += 1;                             //incur the counter for the array element.
      if (in_char == 0) {                 //if we see that we have been sent a null command.
        i = 0;                            //reset the counter i to 0.
        break;                            //exit the while loop.
      }
    }
    Temp2 = atof(RTD_data2);
  }
  serial_event = false;                   //reset the serial event flag.
  return;
}


rtds_sensors(){
  rtd1_sensor();
  rtd2_sensor();
  return Temp_ = 0.5 * (Temp1 + Temp2);
}


void calibrate_sensor() {
  // comunicacion a sensor 1
  Wire.beginTransmission(rtd1);
  Wire.write("cal,25.5");
  Wire.endTransmission();                                                       //end the I2C data transmission.

  if (strcmp("cal,25.5", "sleep") != 0) {                                     //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(rtd1, 20, 1);                                              //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();                                                         //the first byte is the response code, we read this separately.

    while (Wire.available()) {            //are there bytes to receive.
      in_char = Wire.read();              //receive a byte.
      RTD_data[i] = in_char;              //load this byte into our array.
      i += 1;                             //incur the counter for the array element.
      if (in_char == 0) {                 //if we see that we have been sent a null command.
        i = 0;                            //reset the counter i to 0.
        break;                            //exit the while loop.
      }
    }
  }

  // comunicacion a sensor 2
  Wire.beginTransmission(rtd2);
  Wire.write("cal,25.5");
  Wire.endTransmission();                                                       //end the I2C data transmission.

  if (strcmp("cal,25.5", "sleep") != 0) {                                     //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
    delay(time_);                                                               //wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(rtd2, 20, 1);                                              //call the circuit and request 20 bytes (this may be more than we need)
    code = Wire.read();                                                         //the first byte is the response code, we read this separately.

    while (Wire.available()) {            //are there bytes to receive.
      in_char = Wire.read();              //receive a byte.
      RTD_data[i] = in_char;              //load this byte into our array.
      i += 1;                             //incur the counter for the array element.
      if (in_char == 0) {                 //if we see that we have been sent a null command.
        i = 0;                            //reset the counter i to 0.
        break;                            //exit the while loop.
      }
    }
  }
  serial_event = false;                   //reset the serial event flag.
  return;
}


//for hardware serial
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    message += inChar;
    if (inChar == '\n') {
      serial_event = true;
    }
  }
}

void i2c_send_command(String command, uint8_t slave) {   //slave = 2: slave tradicional. 3 es el nuevo
  Wire.beginTransmission(slave); // transmit to device #slave: [2,3]
  Wire.write(command.c_str());   // sends value byte
  Wire.endTransmission();        // stop transmitting
}


//modifica los umbrales de cualquiera de los dos actuadores
void actuador_umbral(){
  //setting threshold ph: u1a160b141e
  if ( message[1] == '1' ) {

    umbral_a = 0; umbral_b = 0;
    umbral_a = message.substring(3,6).toFloat();
    umbral_b = message.substring(7,10).toFloat();

    if ( umbral_a <= SPEED_MIN )
      umbral_a = SPEED_MIN;
    else if ( umbral_a >= SPEED_MAX )
      umbral_a = SPEED_MAX;

    if ( umbral_b <= SPEED_MIN )
      umbral_b = SPEED_MIN;
    else if ( umbral_b >= SPEED_MAX )
      umbral_b = SPEED_MAX;
  }
  //setting threshold temp: u2t011e
  else if ( message[1] == '2' ) {
    umbral_temp = 0;
    umbral_temp = message.substring(3,6).toFloat();

    if ( umbral_temp <= SPEED_MIN )
      umbral_temp = SPEED_MIN;
    else if ( umbral_temp >= SPEED_MAX)
      umbral_temp = SPEED_MAX;
    else
      umbral_temp = umbral_temp;
  }
  Serial.println( "Umbral_Temp: " + String(umbral_temp) );
  //Serial.println( String(umbral_a) + '_' + String(umbral_b) + '_' + String(umbral_temp) );
  return;
}



void tx_reply(){
  //tx of measures
  Serial.print(cByte0);       Serial.print("\t");
  Serial.print(cByte1);       Serial.print("\t");
  Serial.print(cByte2);       Serial.print("\t");
  Serial.print(cByte3);       Serial.print("\t");
  //Serial.print(cByte4);       Serial.print("\t");
  //Serial.print(cByte5);       Serial.print("\t");
  //Serial.print(cByte6);       Serial.print("\t");
  //Serial.print(cByte7);       Serial.print("\t");
  Serial.print("\t");
  Serial.print("new_write_w: ");  Serial.print(new_write_w.substring(0,22));/* Serial.print("\t"); Serial.print("message: "); Serial.print(message.substring(0,19)); */
  Serial.println("");
}

void daqmx() {
  //data adquisition measures
  Byte0 = Temp_;
  Byte1 = Temp1;
  Byte2 = Temp2;
  Byte3 = co2;  //Co2_measure;
  Byte4 = l;//contador fallas antes de reset en rtd1;
  Byte5 = j;//contador fallas antes de reset en rtd2;
  Byte6 = m;//contador fallas antes de reset en co2;
  Byte7 = k;//contador fallas totales


  dtostrf(Byte0, 7, 2, cByte0);
  dtostrf(Byte1, 7, 2, cByte1);
  dtostrf(Byte2, 7, 2, cByte2);
  dtostrf(Byte3, 7, 2, cByte3);
  dtostrf(Byte4, 7, 2, cByte4);
  dtostrf(Byte5, 7, 2, cByte5);
  dtostrf(Byte6, 7, 2, cByte6);
  dtostrf(Byte7, 7, 2, cByte7);
  //dtostrf(Byte8, 7, 2, cByte8);

  tx_reply();
  return;
}

void clean_strings() {
  //clean strings
  serial_event = false;
  //new_write_w = message;
  //message     = "";
}


//function for transform numbers to string format of message
String format_message(int var) {
  //reset to svar string
  svar = "";

  if      (var < 10)   svar = "00" + String(var);
  else if (var < 100)  svar =  "0" + String(var);
  else                 svar = String(var);

  return svar;
}

//Re-transmition commands to slave micro controller
void forward() {
      //delay(10);
      new_write_w = "";                                                                  /*message desde 13 hasta el final*/
      new_write_w = message.substring(0,9) + "t" + temp_select + format_message(u_temp) + message.substring(13,22) + "\n";
      i2c_send_command(new_write_w, 2); //va hacia uc_controles
      message = "";
      //delay(10);
  return;
}


//Control temperatura para agua fria y caliente
void control_temp() {
  if (rst3 == 0) {
    //touch my delta temp
    dTemp = mytemp_set - Temp_;

    //CASO: necesito calentar por que setpoint es inferior a la medicion
    if ( dTemp >= -0.1 ) {
      temp_select = "c"; //calentar
      /*
      delay(1);
      digitalWrite(AGUA_FRIA, HIGH);
      delay(1);
      digitalWrite(AGUA_CALIENTE, LOW);
      */
    }
    //CASO: necesito enfriar por que medicion es mayor a setpoint
    else if ( dTemp < 0.2 ) {
      temp_select = "e"; //enfriar
      /*
      delay(1);
      digitalWrite(AGUA_FRIA, LOW);
      delay(1);
      digitalWrite(AGUA_CALIENTE, HIGH);
      */
      dTemp = (-1)*dTemp;
    }

    if      ( dTemp <= Gap_temp0 ) u_temp = 90;
    else if ( dTemp <= Gap_temp1 ) u_temp = 95;
    else if ( dTemp <= Gap_temp2 ) u_temp = 100;
    else if ( dTemp <= Gap_temp3 ) u_temp = 110;
    else if ( dTemp <= Gap_temp4 ) u_temp = 120;
    else if ( dTemp <= Gap_temp5 ) u_temp = 130;
    else if ( dTemp <= Gap_temp6 ) u_temp = 135;
    else if ( dTemp <= Gap_temp7 ) u_temp = 140;
    else if ( dTemp <= Gap_temp8 ) u_temp = 145;
    else if ( dTemp  > Gap_temp9 ) u_temp = SPEED_MAX;
  }

  else {
    temp_select = "n";
    //el sistema se deja stanby
    //digitalWrite(AGUA_CALIENTE, HIGH);
    //digitalWrite(AGUA_FRIA, HIGH);
  }

  u_temp_save = int(u_temp);
  return;
}



//Esquema I2C Concha y Toro:
//TRAMA-Proceso  : wf000u000tX009r000e1a1, X={c,e,n}

/* nueva trama: 6-8-21: app.py --> myserial.py --> uc_sensores
w f 0 2 0 u 0 2 0 t 0  0  3  r  1  1  1  e  0  a  1
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21
*/

/* nueva trama: 6-8-21, uc_sensores --> uc_controles!!!
w f 0 2 0 u 0 2 0 t n  0  0  3  r  1  1  1  e  0  a  1
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21
*/
// Validate and crumble SETPOINT
int validate() {
    //mejorar el esta funcion, caso "w". julio 15-07-21. FH
    //message format write values: wf100u100t150r111d111
    if ( message[0] == 'w' && message[1] == 'f' && message[5] == 'u' && message[9] == 't' && message[13] == 'r' && message[17]=='e' && (message[18]=='0' || message[18]=='1') )
    {
            rst1 = int(INT(message[14]));  //rst_feed
            rst2 = int(INT(message[15]));  //rst_unload
            rst3 = int(INT(message[16]));  //rst_temp

            mytemp_set   = message.substring(10,13).toFloat();

            return 1;
    }
    // Validate CALIBRATE
    else if ( message[0]  == 'c' )
            return 1;

    //Validete umbral actuador temp: u2t003e
    else if ( message[0] == 'u' && message[1] == '2' &&
              message[2] == 't' && message[6] == 'e'
            )
          return 1;

     // NOT VALIDATE
    else
          return 0;
}
