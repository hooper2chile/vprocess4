#include "Arduino.h"
#include <Wire.h>
#include "Adafruit_ADS1015.h"
Adafruit_ADS1115 ads1(0x49);
//Adafruit_ADS1115 ads2(0x49);

#include <SoftwareSerial.h>
#define rx_rtd1 4
#define tx_rtd1 5
SoftwareSerial rtd1(rx_rtd1, tx_rtd1);


#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion

#define SPEED_MIN 2.0
#define SPEED_MAX 150     //[RPM]
#define TEMP_MAX  60      //[ºC]
#define NOISE     0.01    //mA

#define PGA1 0.125F
#define PGA2 0.0625F
#define alpha 0.2F

#define RS 10.0F             // Shunt resistor value (in ohms)
#define K 1.0 / (10.0 * RS )

String message     = "";
String new_write   = "";
String new_write_w = "wf000u000t000r111e0f0.0t18.1\n";
//String new_write_t = "20.0\n";

boolean stringComplete = false;  // whether the string is complete

//RESET SETUP
uint8_t rst1 = 1;  uint8_t rst2 = 1;  uint8_t rst3 = 1;

//DIRECTION SETUP
char dir1 = 1;  char dir2 = 1;  char dir3 = 1;

// for incoming serial data
float Byte0 = 0;  char cByte0[15] = "";  //por que no a 16?
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
float m = 0;
float n = 0;

//Ahora Temp1 //T.Sombrero //Anterior: pH=:(m0,n0)
float m0 = +12.05; //+12.02;  //+0.864553;//+0.75;
float n0 = -47.72; //-46.28;  //-3.634006;//-3.5;

//oD=:(m1,n1)
float m1 = +6.02;
float n1 = -20.42;

//Temp2=:(m2,n2) //T.Mosto
float m2 = +7.47;   //+12.02; //+11.0;
float n2 = -22.38;  //-46.28; //-42.95;

float Iph = 0;
float Iod = 0;
float Itemp1 = 0;
float Itemp2 = 0;

float umbral_a, umbral_b, umbral_temp;

//   DEFAULT:
float pH    = 0;//m0*Iph    + n0;      //   ph = 0.75*IpH   - 3.5
float oD    = 0;//m1*Iod    + n1;

float Temp1 = m0*Itemp1 + n0;      // Temp = 5.31*Itemp - 42.95;
float Temp2 = m2*Itemp2 + n2;
float Temp_ = 0;//0.5 * ( Temp1 + Temp2 );

float flujo = 0.0;

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

void i2c_send_command(String command, uint8_t slave) {   //slave = 2: slave tradicional. 3 es el nuevo
  Wire.beginTransmission(slave); // transmit to device #slave: [2,3]
  Wire.write(command.c_str());   // sends value byte
  Wire.endTransmission();        // stop transmitting
}

//c2+00.75-03.50e // c: (0=>temp1) (1=>od) (2=>temp2)
void sensor_calibrate(){
  //calibrate function for "message"
  var = message[1];
  m   = message.substring(2,8 ).toFloat();
  n   = message.substring(8,14).toFloat();

  switch (var) {
    case '0': //pH case for calibration //ver si usa este caso para calibrar temp2
      m0 = m;
      n0 = n;
      break;

    case '1': //Oxigen disolve case for calibration
      m1 = m;
      n1 = n;
      break;

    case '2': //Temperature1 case for calibration
      m2 = m;
      n2 = n;
      break;

    default:
      break;
  }

  Serial.println("Sensor calibrated");
  return;
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

/*
void hamilton_sensors() {
  //Filtros de media exponencial
  //Itemp1  = alpha * (PGA1 * K ) * ads1.readADC_SingleEnded(0) + (1 - alpha) * Itemp1;
  //Itemp2  = alpha * (PGA1 * K ) * ads1.readADC_SingleEnded(1) + (1 - alpha) * Itemp2;

  Itemp1 = (PGA1*K) * ads1.readADC_SingleEnded(0);
  Itemp2 = (PGA1*K) * ads1.readADC_SingleEnded(1);

  if (Itemp1 >= 4.5 && Itemp1 <= 12.0)   //5.5mA y 12mA
     Temp1 = m0 * Itemp1 + n0;
  else Temp1 = 0;


  if (Itemp2 >= 4.5 && Itemp2 <= 12.0)   //4.4mA y 12mA
     Temp2 = m2 * Itemp2 + n2;
  else Temp2 = 0;


  //Update measures
  Temp1 = m0 * Itemp1 + n0;
  Temp2 = m2 * Itemp2 + n2;

  if ( 0.5 * ( Temp1 + Temp2 ) > 0 )
	  Temp_ = 0.5 * (Temp1 + Temp2);
  else 	  Temp_ = 0.0;

  return;
}
*/

float temporal = 0;
float temperature = 2;
float dato = 10;
String sensorstring = "";
boolean sensor_string_complete = false;

void rtd_sensors() {
// PARA LEER DATOS y/o RESPUESTAS DESDE EL ATLAS_SCIENTIFIC
//=========================================================
while (rtd1.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
  char inchar = (char)rtd1.read();              //get the char we just received
  sensorstring += inchar;                           //add the char to the var called sensorstring
  if (inchar == '\r') {                             //if the incoming character is a <CR>
    sensor_string_complete = true;                  //set the flag
  }
}

//PARA ASEGURAR DATO NUMERICO, DESCARTA STRING BASURA
if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
  //Serial.println(sensorstring);                     //send that string to the PC's serial monitor
  Serial.println(temperature);
  if (isdigit(sensorstring[0])) {                   //if the first character in the string is a digit
    Temp_ = sensorstring.toFloat();           //convert the string to a floating point number so it can be evaluated by the Arduino
  }
  sensorstring = "";                                //clear the string
  sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
}


	return;
}


void tx_reply(){
  Serial.flush();
  //tx of measures
  Serial.print(cByte0);  Serial.print("\t");
  Serial.print(cByte1);  Serial.print("\t");
  Serial.print(cByte2);  Serial.print("\t");
  Serial.print(cByte3);  Serial.print("\t");
  Serial.print(cByte4);  Serial.print("\t");
  Serial.print(cByte5);  Serial.print("\t");
  Serial.print(cByte6);  Serial.print("\t");
  Serial.print(cByte7);  Serial.print("\t");
//nuevo
  Serial.print(new_write_w);  Serial.print("\t");

  //Serial.print(message);
  Serial.print("\n");
}

void daqmx() {
  //data adquisition measures
  Byte0 = Temp_;
  Byte1 = temporal;
  Byte2 = dato;
  Byte3 = Iph;
  Byte4 = Iod;
  Byte5 = Itemp1;
  Byte6 = Itemp2;
  Byte7 = flujo;

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

//Re-transmition commands to slave micro controller
void broadcast_setpoint(uint8_t select) {
  switch (select) {
    case 0: //only re-tx and update uset's.
      //se actualiza medicion de temperatura para enviarla a uc_slave
      new_write = "";
      new_write = new_write_w;
      i2c_send_command(new_write, 2); //va hacia uc_slave
      break;

    case 1: //update command and re-tx.
      new_write_w  = "";
      new_write_w  = message.substring(0,23) + "t" + String(Temp_) + "\n";  //message;// + "\n";
      i2c_send_command(new_write_w, 2);
      break;

    default:
      break;
  }
  return;
}

//Esquema I2C Concha y Toro:
//TRAMA-Proceso  : wf000u000t009r000e1f0.2

void clean_strings() {
  //clean strings
  stringComplete = false;
  message   = "";
}

// Validate and crumble SETPOINT
int validate() {
    //message format write values: wf100u100t150r111d111
    if ( message[0] == 'w' ) {
            rst1 = int(INT(message[14]));  //rst_feed
            rst2 = int(INT(message[15]));  //rst_unload
            rst3 = int(INT(message[16]));  //rest_temp
            return 1;
    }
    // Validate CALIBRATE
    else if ( message[0]  == 'c' &&
             (message[2]  == '+' || message[2] == '-') &&
             (message[8]  == '+' || message[8] == '-') &&
              message[14] == 'e' &&
              message.substring(3,8 ).toFloat() < 100 &&
              message.substring(9,14).toFloat() < 100
            )
            return 1;

    //Validete umbral actuador temp: u2t003e
    else if ( message[0] == 'u' && message[1] == '2' &&
              message[2] == 't' && message[6] == 'e'
            )
          return 1;

   // Validate READING
    else if ( message[0] == 'r' )
          return 1;

    // NOT VALIDATE
    else
          return 0;
}
