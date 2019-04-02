#!/usr/bin/env python
# --*- coding: utf-8 -*--
import sys, zmq, time, logging

#logging.basicConfig(filename='/home/pi/vprocess4/log/communication.log', level=logging.INFO, format='%(asctime)s:%(levelname)s:%(message)s')

DIR="/home/pi/vprocess4/"

#5556: for download data
#5557: for upload data

#emit command for actuation zmq server (publisher): 'wph14.0feed100unload100mix100temp100rst111111dir111111\n'
#set_data = [ dato['alimentar'], dato['mezclar'], dato['ph'], dato['descarga'], dato['temperatura'], dato['alimentar_rst'], dato['mezclar_rst'], dato['ph_rst'], dato['descarga_rst'], dato['temperatura_rst'], dato['alimentar_dir'], dato['ph_dir'], dato['temperatura_dir'] ]

temp_save_set_data = None
tau_zmq_connect = 0.3 #0.3 [s]: no ha funcionado con menos
tau_zmq_while_read = 0.3

SPEED_MAX_MIX = 1500
SPEED_MAX = 100
TEMP_MAX  = 80

T_MAX = 1440
T_MIN = 2

BOMBA_REMONTAJE_T_MAX = T_MAX - 1
BOMBA_REMONTAJE_T_MIN = 1

CICLO_MAX = 20
CICLO_MIN = 1

PH_MIN = 1
PH_MAX = 14

command_save = "vacio"


#download data measures with client zmq
def published_setpoint(set_data):
    #Publisher set_data commands
    port = "5556"
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:%s" % port)
    topic   = 'w'
    #espero y envio valor
    time.sleep(tau_zmq_connect)
    socket.send_string("%s %s" % (topic, set_data))

    return True


def zmq_client():
    #####Listen measures
    port_sub = "5557"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    #espero y retorno valor
    time.sleep(tau_zmq_connect)

    return socket_sub.recv()


###### Funiones para intercambio de datos entre app.py y databse.py: datos desde la web"
def zmq_client_data_speak_website(data):
    #####Publisher part: publica las lecturas obtenidas por serial.
    port_pub = "5554"
    context_pub = zmq.Context()
    socket_pub = context_pub.socket(zmq.PUB)
    socket_pub.bind("tcp://*:%s" % port_pub)
    topic   = 'w'
    time.sleep(tau_zmq_connect)
    socket_pub.send_string("%s %s" % (topic, data))
    time.sleep(tau_zmq_while_read) #Tiempo de muestreo menor para todas las aplicaciones que recogen datos por ZMQ.
    return True


def zmq_client_data_listen_website():
    #####Listen measures
    port_sub = "5554"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    #espero y retorno valor
    time.sleep(tau_zmq_connect)
    data = socket_sub.recv(flags=zmq.NOBLOCK)
    return data
###### Funiones para intercambio de datos entre app.py y databse.py: datos desde la web"


def calibrate(var, coef):
    #coef[0| =: pendiente;  coef[1] =: intercepto
    #TRAMA: cXs88.88s99.99e ; m=88.888; n=99.99;
    #X = 0,1,2 (ph, od, temp); s = + ó -

    coef[0] = round(coef[0],2)
    coef[1] = round(coef[1],2)

    #sign for coef[0], coef[1]
    if coef[0] < 0:
        coef[0] = -coef[0]
        sign0   = '-'
    else:
        sign0   = '+'

    #sign for coef[1]
    if coef[1] < 0:
        coef[1] = -coef[1]
        sign1   = '-'
    else:
        sign1   = '+'

    #Format for coef[0]
    #Poner cero a la izquierda:
    if coef[0] < 10: #ej. coef[0] = 0.7; coef[0] = 1.1
        coef0text = '0' + str(coef[0]) # => coef[0] = 00.7; coef[0] = 01.1
    #limitar valor
    elif coef[0] >= 100: #aplica para el caso negativo y positivo por que arriba separe el signo.
        coef0text = '99.99'
    #si no es por que tiene formato como: "44.44"
    else:
        coef0text = str(coef[0])

    #Poner cero a la derecha:
    if  coef[0]-int(coef[0]) == 0 and coef[0] < 100:
        coef0text = coef0text + '0'
    elif coef0text[4:] is '':
        coef0text = coef0text + '0'

    #Format for coef[1]
    #Poner cero a la izquierda:
    if coef[1] < 10: #ej. coef[0] = 0.7; coef[0] = 1.1
        coef1text = '0' + str(coef[1]) # => coef[0] = 00.7; coef[0] = 01.1
    #limitar valor
    elif coef[1] >= 100:
        coef1text = '99.99'
    #sino es por que tiene formato 44.44:
    else:
        coef1text = str(coef[1])

    #Poner cero a la derecha:
    if coef[1]-int(coef[1])==0 and coef[1] < 100:
        coef1text = coef1text + '0'
    elif coef1text[4:] is '':
        coef1text = coef1text + '0'

    coef_cook = 'c' + str(var) + sign0 + coef0text + sign1 + coef1text + 'e'

    #guardo coef_cook en un archivo para depurar
    try:
        f = open(DIR + "coef_m_n.txt","w")
        f.write(coef_cook + '\n')
        f.close()
        published_setpoint(coef_cook)

    except:
        pass
        #logging.info("no se pudo guardar set de calibrate()")



# var = 1 => ph
# var = 2 => temp
# e = end
def actuador(var,u_set):

    if var == 1:
        u_set[0] = int(u_set[0])
        u_set[1] = int(u_set[1])

        #format for u_set[0]
        if u_set[0] >= 10 and u_set[0] < 100:
            u_set_0 = '0' + str(u_set[0])

        elif u_set[0] > 0 and u_set[0] < 10:
            u_set_0 = '00' + str(u_set[0])

        elif u_set[0] >= 100 and u_set[0] <= SPEED_MAX:
            u_set_0 = str(u_set[0])

        else:
            u_set_0 = str(1)


        #format for u_set[1]
        if u_set[1] >= 10 and u_set[1] < 100:
            u_set_1 = '0' + str(u_set[1])

        elif u_set[1] > 0 and u_set[1] < 10:
            u_set_1 = '00' + str(u_set[1])

        elif u_set[1] >= 100 and u_set[1] <= SPEED_MAX:
            u_set_1 = str(u_set[1])

        else:
            u_set_1 = str(1)

        #creation for threshold actuador of ph command
        u_cook = 'u' + str(var) + 'a' + u_set_0 + 'b' + u_set_1 + 'e'


    elif var == 2:
        u_set[0] = int(u_set[0])
        u_set[1] = 0

        #format for u_set[0]
        if u_set[0] >= 10 and u_set[0] < 100:
            u_set_0 = '0' + str(u_set[0])

        elif u_set[0] > 0 and u_set[0] < 10:
            u_set_0 = '00' + str(u_set[0])

        elif u_set[0] >= 100 and u_set[0] <= SPEED_MAX:
            u_set_0 = str(u_set[0])

        else:
            u_set_0 = str(1)

        #creation for threshold actuador of temp command
        u_cook = 'u' + str(var) + 't' + u_set_0 + 'e'

    try:
        f = open(DIR + "actuador.txt","w")
        f.write(u_cook + '\n')
        f.close()
        published_setpoint(u_cook);

    except:
        pass
        #logging.info("no se pudo guardar set de actuador()")


###############################################################################
#Se reciben localmente los datos de rm_sets desde app.py
def cook_remontaje(rm_sets):
    global command_save

    rm_sets[0] = int(rm_sets[0])    #periodo
    rm_sets[1] = int(rm_sets[1])    #rm_duracion
    rm_sets[2] = int(rm_sets[2])    #rm_ciclo
    rm_sets[3] = float(rm_sets[3])  #rm_flujo
    rm_sets[5] = int(rm_sets[5])    #rm_enable

    #rm_sets[5] = int(rm_sets[4])   #pichicateo para hacer que la bomba funcione directo con la pagina, sin calculo de tiempos

    command = None

    periodo  = 2
    duracion = 1
    ciclo  = 1
    flujo  = 0
    enable = 0

    try:
        #limites de periodo
        if rm_sets[0] >= T_MAX:
            rm_sets[0] = T_MAX
        elif rm_sets[0] <= T_MIN:
            rm_sets[0] = T_MIN
        #str de periodo
        if rm_sets[0] < 10 and rm_sets[0] >= 0:
            rm_sets[0] = '000' + str(rm_sets[0])
        elif rm_sets[0] < 100 and rm_sets[0] > 0:
            rm_sets[0] = '00' + str(rm_sets[0])
        elif rm_sets[0] < 1000 and rm_sets[0] > 0:
            rm_sets[0] = '0' + str(rm_sets[0])
        periodo = str(rm_sets[0])
        rm_sets[0] = int(rm_sets[0])


        #limites de duracion
        if rm_sets[1] >= BOMBA_REMONTAJE_T_MAX:
            rm_sets[1] = BOMBA_REMONTAJE_T_MAX
        elif rm_sets[1] < BOMBA_REMONTAJE_T_MIN:
            rm_sets[1] = BOMBA_REMONTAJE_T_MIN
        #str de duracion
        if rm_sets[1] < 10 and rm_sets[1] >= 0:
            rm_sets[1] = '000' + str(rm_sets[1])
        elif rm_sets[1] < 100 and rm_sets[1] > 0:
            rm_sets[1] = '00' + str[rm_sets[1]]
        elif rm_sets[1] < 1000 and rm_sets[1] > 0:
            rm_sets[1] = '0' + str[rm_sets[1]]
        duracion = str(rm_sets[1])
        rm_sets[1] = int(rm_sets[1])


        #limites de ciclo
        if rm_sets[2] >= CICLO_MAX:
            rm_sets[2] = CICLO_MAX
        elif rm_sets[2] < CICLO_MIN:
            rm_sets[2] = CICLO_MIN
        #str de ciclo
        if rm_sets[2] < 10 and rm_sets[2] >= 0:
            rm_sets[2] = '0' + str(rm_sets[2])
        ciclo = str(rm_sets[2])
        rm_sets[2] = int(rm_sets[2])

        #limites de flujo (ver escala flujemetro manual) rm_sets[3]
        if rm_sets[3] < 0:
            rm_sets[3] = 0
        elif rm_sets[3] >= 3:
            rm_sets[3] = 3
        #str de flujo
        flujo = str(rm_sets[3])


        #ajustando flag rm_enable (rm_sets[3])
        if rm_sets[5] is 1:
            rm_sets[5] = 1
        else:
            rm_sets[5] = 0
        #str flag rm_enable
        enable = str(rm_sets[5])


        #se construye el string de autoclavado
        command = 'p' + periodo + 'd' + duracion + 'c' + ciclo + 'e' + enable + 'f' + flujo + '\n'

        if command_save != command:
            published_setpoint(command)
            command_save = command

        #se respalda el comando
        #f = open(DIR + "remontaje_string.txt","a+")
     	#f.write(str(command) + '\n')  #agregar la hora a este string
    	#f.close()

    except:
        logging.info("no se pudo generar el command para remontaje_string")
        pass

    return True
###############################################################################





def cook_setpoint(set_data):
    #format string
    #convert true or false in checkbox to 0 or 1
    for i in range(5,13):
        if set_data[i] is True:
            set_data[i] = 1
        else:
            set_data[i] = 0
                                                                                                            # + str(set_data[7]) = rst6
    string_rst = str(set_data[5]) + str(set_data[6]) + str(set_data[7]) + str(set_data[8]) + str(set_data[9]) + str(set_data[7])
    string_dir = str(set_data[10])+ str(set_data[11])+ str(set_data[12]) + '111'


######### para vprocess4 #####################################################
                #       alimentar  +     ¿?           +  temperatura
    string_dir2 = str(set_data[10])+ str(set_data[11])+ str(set_data[12])

                #       alimentar  +   descarga       +  temperatura
    string_rst2 = str(set_data[5]) + str(set_data[8]) + str(set_data[9])
######### para vprocess4 #####################################################

    global temp_save_set_data

    try:
        set_data[0] = int(set_data[0])   #alimentar
        set_data[1] = int(set_data[1])   #mezclar
        set_data[2] = float(set_data[2]) #ph
        set_data[3] = int(set_data[3])   #descarga
        set_data[4] = int(set_data[4])   #temperatura

        #rst setting
        set_data[5] = int(set_data[5])  #alimentar_rst
        set_data[6] = int(set_data[6])  #mezclar_rst
        set_data[7] = int(set_data[7])  #ph_rst
        set_data[8] = int(set_data[8])  #descarga_rst
        set_data[9] = int(set_data[9])  #temperatura_rst

        #dir setting
        set_data[10]= int(set_data[10]) #alimentar_dir
        set_data[11]= int(set_data[11]) #ph_dir
        set_data[12]= int(set_data[12]) #temperatura_dir

        temp_save_set_data = set_data

    except ValueError:
        set_data = temp_save_set_data #esto permite reenviar el ultimo si hay una exception
        #logging.info("exception de set_data")

    #threshold setting:
    #alimentar
    if set_data[0] > SPEED_MAX:     #SPEED_MAX_MIX:
        set_data[0] = SPEED_MAX     #SPEED_MAX_MIX

    elif set_data[0] < 0:
        set_data[0] = 0

    #Mezclar
    if set_data[1] > SPEED_MAX_MIX:  #cambiar a SPEED_MAX_MIX cuando este listo el protocolo que tambien hay que modificar
        set_data[1] = SPEED_MAX_MIX

    elif set_data[1] < 0:
        set_data[1] = 0

    #pH
    if set_data[2] > PH_MAX:
        set_data[2] = PH_MAX

    elif set_data[2] < PH_MIN:
        set_data[2] = PH_MIN

    #descarga
    if set_data[3] > SPEED_MAX:
        set_data[3] = SPEED_MAX

    elif set_data[3] < 0:
        set_data[3] = 0

    #temperatura
    if set_data[4] > TEMP_MAX:
        set_data[4] = TEMP_MAX

    elif set_data[4] < 0:
        set_data[4] = 0



    #format seetting:
    if set_data[0] < 10:
        string_feed = '00' + str(set_data[0])
    elif set_data[0] < 100:
        string_feed = '0'  + str(set_data[0])
    else:
        string_feed = str(set_data[0])



    if set_data[1] < 10:
        string_mix = '000' + str(set_data[1])
    elif set_data[1] < 100:
        string_mix = '00'  + str(set_data[1])
    elif set_data[1] < 1000:
        string_mix = '0'   + str(set_data[1])
    else:
        string_mix = str(set_data[1])



    ph_dec = str(set_data[2]-int(set_data[2]))[1:]
    if ph_dec == '.0':
        if set_data[2] < 10:
            string_ph = '0' + str(set_data[2])
        else:
            string_ph = str(set_data[2])

    else:
        if set_data[2] < 10:
            string_ph = '0' + str(set_data[2])
        else:
            string_ph = str(set_data[2])



    if set_data[3] < 10:
        string_unload = '00' + str(set_data[3])
    elif set_data[3] < 100:
        string_unload = '0'  + str(set_data[3])
    else:
        string_unload = str(set_data[3])



    if set_data[4] < 10:
        string_temp = '00' + str(set_data[4])
    elif set_data[4] < 100:
        string_temp = '0'  + str(set_data[4])
    else:
        string_temp = str(set_data[4])



    #command = 'wph' + string_ph + 'feed' + string_feed + 'unload' + string_unload + 'mix' + string_mix + \
    #          'temp' + string_temp + 'rst' + string_rst + 'dir' + string_dir + '\n'

    #vprocess4
    command = 'w' + 'f' + string_feed + 'u' + string_unload + 't' + string_temp + 'r' + string_rst2 + 'd' + string_dir2 + '\n'

    logging.info('\n' + command + '\n')

    #published for put in queue and write in serial port
    published_setpoint(command)

    try:
        f = open(DIR + "command.txt","a+")
        f.write(command)
        f.close()

    except OSError:
        pass
        #logging.info("no se pudo guardar el command en el archivo de texto")

    return True




def main():
    while True:
        pass
        time.sleep(0.05)
        #logging.info("communication.py")


if __name__ == '__main__':
    main()
