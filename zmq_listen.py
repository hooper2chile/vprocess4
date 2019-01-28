#!/usr/bin/env python
#--*- coding: utf-8 -*--

import zmq, time, os

tau_zmq_connect     = 0.5


#escucha los zmq emitidos por myserial.py
port_sub = "5557"
context_sub = zmq.Context()
socket_sub = context_sub.socket(zmq.SUB)
socket_sub.connect ("tcp://localhost:%s" % port_sub)
topicfilter = "w"
socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
time.sleep(tau_zmq_connect)

string = ['','','','']

'''
  Byte0 = pH;
  Byte1 = oD;
  Byte2 = Temp1;
  Byte3 = Iph;
  Byte4 = Iod;
  Byte5 = Itemp1;
  Byte6 = Itemp2;

  Byte7 = Temp2; //Nuevo
'''

os.system("clear")
while True:
   
    string = socket_sub.recv().split()
    os.system("clear")
    print "\n   pH,     oD,   Temp1,    Iph,   Iod,   Itemp1,   Itemp2,  Temp2"
    #print string[1:8]
    print string[1:9]
    print "\n"
    print "Itemp1     Itemp2"
    print string[6] + "       " + string[7] + "\n\n"

    print "Temp1      Temp2"
    print string[3] + "       " + string[8] + "\n"

    time.sleep(0.25)
