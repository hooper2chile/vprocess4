#!/usr/bin/env python
# --*- coding: utf-8 -*--
import sys, time, logging


def remontaje(rm_sets, ficha_producto):


        time_save1 = datetime.datetime.now()
        time_save2 = datetime.datetime.now()
        #################################################################################
        # Codigo para remontaje (calculo de tiempos)
        #bucle principal de tiempo
        #if rm_sets[4] == 1: #se habilita el remontaje con enable de la app.py
        if rm_sets[4] == 1:
            ciclo_seg    = int(rm_sets[2])*3600*24             #se configura variable "ciclo"    de dias a segundos.
            duracion_seg = int(rm_sets[1])*60                  #se configura variable "duracion" de minutos a segundos.
            periodo_seg  = int(rm_sets[0])*60                  #se configura variable "periodo"  de munutos a segundos.

            if ciclo_seg - ciclo_elapsed > 0:
                #GPIO.output(PIN_CICLO, True)
                c = datetime.datetime.now() - time_save1
                ciclo_elapsed = c.days*3600*24 + c.seconds     #tiempo transcurrido

                ###################################################################################
                # Codigo reentrante para periodo y duracion de bomba remontaje
                d = datetime.datetime.now() - time_save2
                if flag_duty_cycle == 1:
                    time_save2 = datetime.datetime.now()
                    d = datetime.datetime.now() - time_save2
                    flag_duty_cycle = 0

                if (duracion_seg - d.seconds) > 0:             #descuento tiempo del duty cycle, tiempo en alto.
                    rm_sets[5] = 1                             #aca encender el pin (enciendo bomba)


                elif (periodo_seg - d.seconds) > 0:            #vencido el tiempo en alto, descuento tiempo del periodo, tiempo en bajo.
                    rm_sets[5] = 0                             #aca apagar el pin (apago la bomba)


                else:
                    flag_duty_cycle = 1
                ###################################################################################

            else:
                rm_sets[4] = 0   #finalizado el ciclo, se debe apagar el enable global y Con cada cambio en los parametros, se vuelven a emitir a todos los clientes.
                socketio.emit('remontaje_setpoints', {'set': rm_sets, 'save': rm_save }, namespace='/biocl', broadcast=True)
                #time.sleep(0.01)

        else:
            ciclo_elapsed = 0
            rm_sets[5] = 0

            flag_duty_cycle = 1

            time_save1 = datetime.datetime.now()
            time_save2 = datetime.datetime.now()

            #time.sleep(0.05)
            # Codigo para remontaje (calculo de tiempos)
            #################################################################################




        return rm_sets
