#include "Arduino.h"
#include <Wire.h>
#include "Adafruit_ADS1015.h"
Adafruit_ADS1115 ads1(0x49);
//Adafruit_ADS1115 ads2(0x49);

#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion

#define SPEED_MIN 2.0
#define SPEED_MAX 150     //[RPM]
#define TEMP_MAX  130     //[ºC]

#define PGA1 0.125F
#define PGA2 0.0625F
#define alpha 0.2F

#define VOLTAGE_REF 5.0F     // before: 5  // Reference voltage for analog read
#define RS 10.0F             // Shunt resistor value (in ohms)
#define mA 1000.0F              // for sensors 4-20mA
#define K 1.0 / (10.0 * RS )

#define Gap_temp0 0.5
#define Gap_temp1 1.0       //1ºC
#define Gap_temp2 2.0
#define Gap_temp3 3.0
#define Gap_temp4 5.0


String  message     = "";  String  new_write   = "";  String  new_write0   = "";

boolean stringComplete = false;  // whether the string is complete

String  ph_var      = "";   String  ph_set      = "";
String  feed_var    = "";   String  feed_set    = "";
String  unload_var  = "";   String  unload_set  = "";
String  mix_var     = "";   String  mix_set     = "";
String  temp_var    = "";   String  temp_set    = "";

//Re-formatting
String  ph_select = "";
String  uset_temp = "";
String  uset_ph   = "";
String  svar      = "";

float   myphset   = 0;
float   mytempset = 0;

uint8_t myfeed    = 0;
uint8_t myunload  = 0;
uint16_t mymix    = 0;

//RESET SETUP
char rst1 = 1;  char rst2 = 1;  char rst3 = 1;
char rst4 = 1;  char rst5 = 1;  char rst6 = 1;

//DIRECTION SETUP
char dir1 = 1;  char dir2 = 1;  char dir3 = 1;
char dir4 = 1;  char dir5 = 1;  char dir6 = 1;


float umbral_a = SPEED_MAX;
float umbral_b = SPEED_MAX;
float umbral_temp = SPEED_MAX;

// for incoming serial data
float Byte0 = 0;  char cByte0[15] = "";  //por que no a 16?
float Byte1 = 0;  char cByte1[15] = "";
float Byte2 = 0;  char cByte2[15] = "";
float Byte3 = 0;  char cByte3[15] = "";
float Byte4 = 0;  char cByte4[15] = "";
float Byte5 = 0;  char cByte5[15] = "";
float Byte6 = 0;  char cByte6[15] = "";
float Byte7 = 0;  char cByte7[15] = "";  //for Temp2

//calibrate function()
char  var = '0';
float m = 0;
float n = 0;

//Ahora Temp1 //Anterio: pH=:(m0,n0)
float m0 = +12.02;  //+0.864553;//+0.75;
float n0 = -46.28;  //-3.634006;//-3.5;

//oD=:(m1,n1)
float m1 = +6.02;
float n1 = -20.42;

//Temp2=:(m2,n2)
float m2 = +12.02; //+11.0;
float n2 = -46.28; //-42.95;

float Iph = 0;
float Iod = 0;
float Itemp1 = 0;
float Itemp2 = 0;

//   DEFAULT:
float pH    = 0;//m0*Iph    + n0;      //   ph = 0.75*IpH   - 3.5
float oD    = 0;//m1*Iod    + n1;

float Temp1 = m0*Itemp1 + n0;      // Temp = 5.31*Itemp - 42.95;
float Temp2 = m2*Itemp2 + n2;
float Temp_ = 0.5 * ( Temp1 + Temp2 );


//variable for control
float u_temp = 0;
float u_ph = 0;
float dTemp= 0;
float dpH  = 0;


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

void i2c_send_command(String command, uint8_t slave) {
  Wire.beginTransmission(slave); // transmit to device #slave: [1,2]
  Wire.write(command.c_str());   // sends value byte
  Wire.endTransmission();        // stop transmitting
}

//desmenuza el string de comandos
void write_crumble() {   //falta definir si se hara el control de temp aca, lo mismo el remontaje.
  mytempset = message.substring(34, 37).toFloat();
  mymix     = message.substring(26, 30).toInt();
  //setting rst
  rst1 = INT(message[40]);  rst2 = INT(message[41]);  rst3 = INT(message[42]);

  return;
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

  Serial.println( String(umbral_a) + '_' + String(umbral_b) + '_' + String(umbral_temp) );
  return;
}


void hamilton_sensors() {
  //Filtros de media exponencial
  Itemp1  = alpha * (PGA1 * K ) * ads1.readADC_SingleEnded(0) + (1 - alpha) * Itemp1;
  Itemp2  = alpha * (PGA1 * K ) * ads1.readADC_SingleEnded(1) + (1 - alpha) * Itemp2;

  //Update measures
  Temp1 = m0 * Itemp1 + n0;
  Temp2 = m2 * Itemp2 + n2;
  Temp_ = 0.5 * ( Temp1 + Temp2 );

  //pH    = m0 * Iph    + n0;
  //oD    = m1 * Iod    + n1;

  return;
}


void tx_reply(){
  //tx of measures
  Serial.print(cByte0);  Serial.print("\t");
  Serial.print(cByte1);  Serial.print("\t");
  Serial.print(cByte2);  Serial.print("\t");
  Serial.print(cByte3);  Serial.print("\t");
  Serial.print(cByte4);  Serial.print("\t");
  Serial.print(cByte5);  Serial.print("\t");
  Serial.print(cByte6);  Serial.print("\t");
  Serial.print(cByte7);  Serial.print("\t");

  Serial.print("\n");
}

void daqmx() {
  //data adquisition measures
 /*
  Byte0 = pH;
  Byte1 = oD;
  Byte2 = Temp1;
  Byte3 = Iph;
  Byte4 = Iod;
  Byte5 = Itemp1;
  Byte6 = Itemp2;
  Byte7 = Temp2;   //Nuevo: para promedio movil o exponencial
*/
  Byte0 = Temp_;
  Byte1 = Temp1;
  Byte2 = Temp2;
  Byte3 = Iph;
  Byte4 = Iod;
  Byte5 = Itemp1;
  Byte6 = Itemp2;
  Byte7 = oD;

  dtostrf(Byte0, 7, 2, cByte0);
  dtostrf(Byte1, 7, 2, cByte1);
  dtostrf(Byte2, 7, 2, cByte2);
  dtostrf(Byte3, 7, 2, cByte3);
  dtostrf(Byte4, 7, 2, cByte4);
  dtostrf(Byte5, 7, 2, cByte5);
  dtostrf(Byte6, 7, 2, cByte6);
  dtostrf(Byte7, 7, 2, cByte7);

  tx_reply();
  return;
}

void control_temp() {
  //for debug
  //mytemp  = 50;
  //touch my delta temp
  dTemp = mytempset - Temp1;

  if ( dTemp > 0.0 ) {
    if ( dTemp <= Gap_temp1 )
      u_temp = 0.20*umbral_temp;

    else if ( dTemp <= Gap_temp2 )
      u_temp = 0.35*umbral_temp; //50%

    else if ( dTemp <= Gap_temp3 )
      u_temp = 0.50*umbral_temp; //75%

    else if ( dTemp <= Gap_temp4 )
      u_temp = 0.75*umbral_temp; //100%

    else if ( dTemp > Gap_temp4 )
      u_temp = umbral_temp;
  }
  //dTemp < 0 => speed min in actuador temp
  else if ( dTemp <= 0.0 )
    u_temp = SPEED_MIN;

  return;
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


//Re-transmition commands to slave micro controller
void broadcast_setpoint(uint8_t select) {
  //se prepara el setpoint para el renvio hacia uc slave.
  format_message(u_temp);
  uset_temp = svar; //string variable for control: uset_temp

  switch (select) {
    case 0: //only re-tx and update pid uset's.
      new_write0 = "";
      new_write0 = new_write;
      i2c_send_command(new_write0, 2);
      break;

    case 1: //update command and re-tx.
      new_write  = "";
      new_write  = message.substring(7,23) + message.substring(30,42) + "\n";
      i2c_send_command(new_write, 2);
      break;

    default:
      break;
  }
  return;
}

/*
void setpoint() {
  //acá se leen los nuevos setpoint para los lazos de control
  write_crumble();
  control_temp();
  broadcast_setpoint(1);
  tx_reply();
  //Serial.println("A A A");
  //Serial.println("Good Setpoint Crumble");
  return;
}
*/


//wph08.3feed100unload100mix1500temp022rst111111dir111111
//wphb015feed100unload100mix1500temp150rst000000dir111111 (55 bytes)

//Esquema I2C Concha y Toro:
//wphb015(-7) feed100unload100 mix1500(-7) temp150rst000 000(-3) dir111111(-9)
//feed100unload100temp150rst000 (29) => + "\n" = 30 bytes!

void clean_strings() {
  //clean strings
  stringComplete = false;
  message   = "";
  uset_temp = "";
  uset_ph   = "";
  ph_select ="";
}


int validate() {
//message format write values: wph14.0feed100unload100mix1500temp100rst111111dir111111
    // Validate SETPOINT
    if (  message[0] == 'w'                     &&
          message.substring(1, 3)   == "ph"     &&
          message.substring(7, 11)  == "feed"   &&
          message.substring(14, 20) == "unload" &&
          message.substring(23, 26) == "mix"    &&
          message.substring(30, 34) == "temp"   &&
          message.substring(37, 40) == "rst"    &&
          message.substring(46, 49) == "dir"    &&

          //ph number
          ( message.substring(3, 7).toFloat() >= 0    ) &&
          ( message.substring(3, 7).toFloat() <= 14.0 ) &&

          //feed number
          ( message.substring(11, 14).toInt() >= 0         ) &&
          ( message.substring(11, 14).toInt() <= SPEED_MAX ) &&

          //unload number
          ( message.substring(20, 23).toInt() >= 0         ) &&
          ( message.substring(20, 23).toInt() <= SPEED_MAX ) &&

          //mix number
          ( message.substring(26, 30).toInt() >= 0    ) &&
          ( message.substring(26, 30).toInt() <= 1500 ) &&

          //temp number
          ( message.substring(34, 37).toInt() >= 0        ) &&
          ( message.substring(34, 37).toInt() <= TEMP_MAX ) &&

          //rst bits
          ( message[40] == iINT(1) || message[40] == iINT(0) ) &&
          ( message[41] == iINT(1) || message[41] == iINT(0) ) &&
          ( message[42] == iINT(1) || message[42] == iINT(0) ) &&
          ( message[43] == iINT(1) || message[43] == iINT(0) ) &&
          ( message[44] == iINT(1) || message[44] == iINT(0) ) &&
          ( message[45] == iINT(1) || message[45] == iINT(0) ) &&

          //dir bits
          ( message[49] == iINT(1) || message[49] == iINT(0) ) &&
          ( message[50] == iINT(1) || message[50] == iINT(0) ) &&
          ( message[51] == iINT(1) || message[51] == iINT(0) ) &&
          ( message[52] == iINT(1) || message[52] == iINT(0) ) &&
          ( message[53] == iINT(1) || message[53] == iINT(0) ) &&
          ( message[54] == iINT(1) || message[54] == iINT(0) )
        )
        { return 1; }

      // Validate CALIBRATE
      else if ( message[0]  == 'c' &&
               (message[2]  == '+' || message[2] == '-') &&
               (message[8]  == '+' || message[8] == '-') &&
                message[14] == 'e' &&
                message.substring(3,8 ).toFloat() < 100 &&
                message.substring(9,14).toFloat() < 100
              )
          return 1;

      //Validete umbral actuador ph: u1a001b001e
      else if ( message[0]  == 'u' && message[1] == '1' &&
                message[2]  == 'a' && message[6] == 'b' &&
                message[10] == 'e'
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
