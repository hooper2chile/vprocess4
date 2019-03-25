import time, zmq
tau_zmq_connect = 0.3

def zmq_client():
    #####Listen measures
    port_sub = "5557"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    
    while 1:
        #espero y retorno valor
        time.sleep(tau_zmq_connect)
        data = socket_sub.recv()
        print data

if __name__=='__main__':
    zmq_client()
