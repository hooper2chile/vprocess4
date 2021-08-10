#!usr/bin/env python
# -*- coding: utf-8 -*-
#vprocess4c - CyT: Felipe Hooper 8 Agosto 2021
'''
    Adquisición de datos por sockets (ZMQ) para la base de datos.
'''
import os, sys, time, datetime, sqlite3, sqlitebck, logging, zmq

DIR="/home/pi/vprocess4c"

logging.basicConfig(filename = DIR + '/log/mydatabase2.log', level=logging.INFO, format='%(asctime)s:%(levelname)s:%(message)s')
TIME_MIN_BD = 0.01 #1  # 1 [s]
flag_database = "False"
flag_database_local = False




def main():

    topicfilter = "w"
    #####Listen measures - estructura para zmq listen de ficha_producto ################
    port_sub1 = "5554"
    context_sub1 = zmq.Context()
    socket_sub1 = context_sub1.socket(zmq.SUB)
    socket_sub1.connect ("tcp://localhost:%s" % port_sub1)
    socket_sub1.setsockopt(zmq.SUBSCRIBE, topicfilter)

    #####Listen measures - estructura para zmq listen real_data (measures from myserial.py)
    port_sub2 = "5557"
    context_sub2 = zmq.Context()
    socket_sub2 = context_sub2.socket(zmq.SUB)
    socket_sub2.connect ("tcp://localhost:%s" % port_sub2)
    socket_sub2.setsockopt(zmq.SUBSCRIBE, topicfilter)
    ####################################################################################

    connector = sqlite3.connect(':memory:', detect_types = sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES)
    c = connector.cursor()
    time.sleep(1)




    first_time = time.strftime("Fecha__%d-%m-%y__Hora__%H_%M_%S")
    BACKUP     = True
    T_SEG      = datetime.datetime.now()
    T_elapsed  = datetime.datetime.now()



    while True:
        ##############   ZMQ connection for download data ficha_producto ###############
        try:
            ficha_producto = socket_sub1.recv(flags=zmq.NOBLOCK).split()[1];
            ficha_producto = ficha_producto.split(",")
            ficha_producto_save = ficha_producto
            #ficha_producto_save = ficha_producto
            #log para depuracion

            f = open(DIR + "/ficha_producto_basedatos.txt","a+")
            f.write(str(ficha_producto) + "__up__" + "\n" )
            f.close()

        except zmq.Again:
            pass


        ##############  ZMQ connection for download real_data (measures from myserial.py)
        try:
            temporal = socket_sub2.recv(flags=zmq.NOBLOCK).split()
            if temporal != "":
                real_data = temporal

                #f = open(DIR + "/real_data_basedatos.txt","a+")
                #f.write(str(real_data) + "__up__" + "\n" )
                #f.close()

        except zmq.Again:
            pass
        ########################## ZMQ connections #######################################




        # CREAR TABLAS SI NO EXISTEN!!!!
        c.execute('CREATE TABLE IF NOT EXISTS PROCESO (ID INTEGER PRIMARY KEY autoincrement, FECHA TIMESTAMP NOT NULL, HORA TIMESTAMP NOT NULL, T_MOSTO REAL, T_SOMBRERO REAL, T_Promedio REAL, T_Setpoint REAL, Flujo_REMONTAJE REAL, Densidad REAL, Yan REAL, pH REAL, Brix REAL, Acidez REAL, Lote REAL, Dosis REAL, Bomba1 REAL, Bomba2 REAL, Electrovalvula_Aire REAL, CEPA REAL, FLUJO_AIRE REAL, co2 REAL)')

        #CREACION DE TABLAS TEMP1(Sombrero), TEMP2(Mosto), TEMP_ (promedio). CADA ITEM ES UNA COLUMNA
        c.execute('CREATE TABLE IF NOT EXISTS T_SOMBRERO (ID INTEGER PRIMARY KEY autoincrement, FECHA_HORA TIMESTAMP NOT NULL, MAGNITUD REAL)')
        c.execute('CREATE TABLE IF NOT EXISTS T_MOSTO    (ID INTEGER PRIMARY KEY autoincrement, FECHA_HORA TIMESTAMP NOT NULL, MAGNITUD REAL)')
        c.execute('CREATE TABLE IF NOT EXISTS T_PROMEDIO (ID INTEGER PRIMARY KEY autoincrement, FECHA_HORA TIMESTAMP NOT NULL, MAGNITUD REAL)')

        logging.info("Se crearon las tablas!!!")

        #se guardan las tablas agregados en la db si no existian
        connector.commit()


        try:
            real_data_1 = (float(real_data[3]) + float(real_data[2]))/2  #temperatura promedio entre T_MOSTO y T_SOMBRERO
            real_data_2 =  float(real_data[4])                       #medicion de co2 [ppm]
            #comp1 = float(ficha_producto[12])#round(float(ficha_producto[14])*ficha_producto[12],2)                                                                                                                                                                            #Densidad_Opt       #TASA_Crec          #Etanol       #sustrato_i       #mezclador           #bomba1             #bomba2          #Temp_setpoint     #Temp_measure     #pH_setpoint     #pH_measure

            logging.info("DATOS FLOAT SE insertan datos en db")
            print "Datos Float obtenidos !!!!"


        except:
            print "Datos Float NO fueron obtenidos !!!!"
            logging.info("no se pudo insertar datos en db ??????????????")



        #INSERCION DE LOS DATOS MEDIDOS, TABLA FULL CON TODA LA DATA, NULL es para el ID
        try:
            c.execute("INSERT INTO PROCESO  VALUES (NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", (datetime.datetime.now().strftime("%Y-%m-%d"), datetime.datetime.now().strftime("%H:%M:%S"), round(float(real_data[3]),2), round(float(real_data[2]),2), round(real_data_1,2), ficha_producto[9], ficha_producto[12], ficha_producto[0], ficha_producto[1], ficha_producto[2], ficha_producto[3], ficha_producto[4], ficha_producto[7], ficha_producto[8], ficha_producto[10], ficha_producto[11], ficha_producto[13], ficha_producto[6], ficha_producto[5], real_data_2 ))
            #Insercion solo de los datos de sensores
            print "TABLA GRANDE OK!!!! en db"

        except:
            print "no se pudo insertar datos TABLA GRANDE  en db"



        try:
            #Insercion solo de los datos de sensores
            c.execute("INSERT INTO T_MOSTO    VALUES (NULL,?,?)", (datetime.datetime.now().strftime("%Y-%m-%d,%H:%M:%S"), round(float(real_data[3]),2)))
            c.execute("INSERT INTO T_SOMBRERO VALUES (NULL,?,?)", (datetime.datetime.now().strftime("%Y-%m-%d,%H:%M:%S"), round(float(real_data[2]),2)))
            c.execute("INSERT INTO T_PROMEDIO VALUES (NULL,?,?)", (datetime.datetime.now().strftime("%Y-%m-%d,%H:%M:%S"), round(float(real_data[1]),2)))

            print "Datos Insertados !!!"
            logging.info("se insertaron todos los datos en db")

            #time.sleep(0.5)

        except:
            #print "no se pudo insertar dato en db"
            print "Fallo al Insertar Dato !!!"
            logging.info("no se pudo insertar datos en db")

        #se guardan los datos agregados en la db
        connector.commit()


        '''
        if T_SEG - T_elapsed.second <= 0 :
            BACKUP = True
            T_elapsed = 0
        else:
            BACKUP = False
            T_elapsed = datetime.datetime.now()

        print BACKUP


        BACKUP = True
        if BACKUP :
            first_time = time.strftime("Fecha__%d-%m-%y__Hora__%H_%M_%S")
        else:
            first_time = "__NULO__"
        '''


        #Backup DB in RAM to DISK SD
        if BACKUP:

            filedb='/home/pi/vprocess4c/database/backup__' + first_time + '__.db'

            bck = sqlite3.connect(filedb)
            sqlitebck.copy(connector, bck)

            try:
                os.system('sqlite3 -header -csv %s "select * from T_SOMBRERO;" > /home/pi/vprocess4c/csv/%s' % (filedb,filedb[28:-3])+'T_SOMBRERO.csv' )
                os.system('sqlite3 -header -csv %s "select * from T_MOSTO;"    > /home/pi/vprocess4c/csv/%s' % (filedb,filedb[28:-3])+'T_MOSTO.csv'    )
                os.system('sqlite3 -header -csv %s "select * from T_PROMEDIO;" > /home/pi/vprocess4c/csv/%s' % (filedb,filedb[28:-3])+'T_PROMEDIO.csv' )

                os.system('sqlite3 -header -csv %s "select * from PROCESO;"    > /home/pi/vprocess4c/csv/%s' % (filedb,filedb[28:-3])+'PROCESO.csv' )

                logging.info("\n Backup CSV REALIZADO \n")

            except:
                logging.info("\n Backup FULL NO REALIZADO, NO REALIZADO \n")


            try:
                #Se guarda el nombre de la db para ser utilizado en app.py
                f = open(DIR + "/name_db.txt","w")
                f.write(filedb + '\n')
                f.close()

            except:
                #print "no se pudo guardar el nombre de la DB para ser revisada en app.py"
                logging.info("no se pudo guardar el nombre de la DB para ser revisada en app.py")




if __name__ == "__main__":
    main()
