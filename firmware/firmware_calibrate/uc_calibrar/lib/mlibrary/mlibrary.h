#include "Arduino.h"
#include <Wire.h>

#define rtd1 101
#define rtd2 102
#define co2s 105

#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion


String message     = "";
String new_write_w = "wf000u000tn000r111e0\n";


boolean stringComplete = false;  // whether the string is complete


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


atlas_sensors(){
  rtd1_sensor();
  rtd2_sensor();
  co2_sensor();
}


void calibrate_sensor() {
  // comunicacion a sensor 1
  Wire.beginTransmission(rtd1);
  Wire.write("cal,25");
  Wire.endTransmission();                                                       //end the I2C data transmission.

  if (strcmp("cal,25", "sleep") != 0) {                                     //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
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
  Wire.write("cal,25");
  Wire.endTransmission();                                                       //end the I2C data transmission.

  if (strcmp("cal,25", "sleep") != 0) {                                     //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
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



// Validate and crumble SETPOINT
int validate() {
    //mejorar el esta funcion, caso "w". julio 15-07-21. FH
    //message format write values: wf100u100t150r111d111
    // Validate CALIBRATE
    if ( message[0]  == 'c' )
            return 1;


    else
          return 0;
}
