
import time, zmq, logging, datetime, serial

logging.basicConfig(filename='/home/pi/vprocess4/testing.log', level=logging.INFO, format='%(asctime)s:%(levelname)s:%(message)s')


tau_zmq_while_write = 0.25 #0.5=500 [ms]
tau_zmq_connect = 0.5
tau_serial      = 0.01 #0.08   #0.02  #  0.01=10 [ms]
tau_dtr         = 1

save_setpoint1 = 'wf000u000t000r111e0f0.0'
setpoint_reply_uc = save_setpoint1




def listen_setpoint(): #escuchar
    port_sub = "5556"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    time.sleep(tau_zmq_connect)

    string = ""
    #string = ['','','','']
    try:
        string= socket_sub.recv(flags=zmq.NOBLOCK).split()
        print string

    except zmq.Again:
        pass

    time.sleep(tau_zmq_while_write)
    return string




def rs232():
    global save_setpoint1
    global setpoint_reply_uc

    flag = False
    while not flag:
        try:
            print "Opening SerialPort!"
            logging.info("---------------------------Try Open SerialPort-USB------------------------------------------------------")

            ser = serial.Serial(port='/dev/ttyUSB0', timeout = 1, baudrate=9600)
            ser.setDTR(True)
            time.sleep(tau_dtr)
            ser.setDTR(False)
            time.sleep(tau_dtr)

            logging.info("------------------------------- DTR SET READY ---------------------------------------------------------")
            logging.info("Post DTR SET READY: flag = %s", flag)
            print "SerialPort DTR is ready!"

            flag = ser.is_open
            print "Flag de SerialPort is: %s", flat

            if flag:
                logging.info('CONEXION SERIAL EXITOSA, flag= %s', flag)

            #Action for write AND read command of setpoint + remontaje (29-09-19) to serial port
            while ser.is_open:

                #setpoint = listen_setpoint()
                print setpoint

                try:
                    #escribiendo setpoint al uc_master de sensores
                    ser.write(setpoint + '\n')
                    logging.info("Write_to_uc: %s", setpoint)

                    #leyendo la respuesta del uc_master al comando "setpoint" anterior
                    result = ser.readline().split()
                    logging.info("Reply_from_uc: %s ", result)

                    save_setpoint1 = setpoint
                    logging.info("************* Se actualizan save_setpoint1 (write setpoint to UC): %s   *************", save_setpoint1)
                    print "setpoint writed"

                except:
                    print "except 1"
                    logging.info("no se pudo escribir al uc")
                    save_setpoint1 = setpoint
                    logging.info("the last setpoint save")
                    #ser.close()
                    #flag = False

                time.sleep(tau_serial)

        except serial.SerialException:
            print "conexion serial no realizada. except 2"
            logging.info("Sin Conexion Serial")
            flag = False
            time.sleep(2)

    logging.info("Fin de rs232().py")
    return True




    def main():
        rs232()

        return True


    if __name__ == "__main__":
        while True:
            main()
