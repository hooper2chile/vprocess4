$(document).ready(function() {

    namespace = '/biocl';

    // Connect to the Socket.IO server.
    var socket = io.connect(location.protocol + '//' +
	    		       document.domain + ':' +
		             location.port + namespace);

    //SECCION MEDICIONES
    //Se escuchan las mediciones de ph, OD, Temp.
    socket.on('Medidas', function(msg) {
        $('#med1').text('Temp_: ' + msg.data[0] + ' º[C]').html();
        $('#med2').text('Temp1: ' + msg.data[1] + ' º[C]').html();
        $('#med3').text('Temp2: ' + msg.data[2] + ' º[C]').html();
    });

    //SECCION AUTOCLAVE
    //se emiten los setpoints hacia el servidor
    $('form#remontaje_formulario').submit(function(event) {
        socket.emit('remontaje_setpoints',
                    { rm_periodo : $('#rm_periodo_input_id').val(),
                      rm_duracion: $('#rm_duracion_input_id').val(),
                      rm_ciclo   : $('#rm_ciclo_input_id').val(),
                      rm_flujo   : $('#rm_flujo_input_id').val(),
                      rm_enable  : $('#rm_enable_input_id').is(':checked')
                    });

        //para depurar
        console.log('Emitiendo Valores: periodo, duracion, ciclo, flujo, enable checked:');
        console.log($('#rm_periodo_input_id').val());
        console.log($('#rm_duracion_input_id').val());
        console.log($('#rm_ciclo_input_id').val());
        console.log($('#rm_flujo_input_id').val());
        console.log($('#rm_enable_input_id').is(':checked'));
        return false;
    });

    //se escuchan desde el servidor los setpoints aplicados
    //para ser desplegados en todos los clientes.
    socket.on('remontaje_setpoints', function(msg) {
        document.getElementById('rm_periodo_input_id' ).value  = msg.set[0];
        document.getElementById('rm_duracion_input_id').value  = msg.set[1];
        document.getElementById('rm_ciclo_input_id').value     = msg.set[2];
        document.getElementById('rm_flujo_input_id').value     = msg.set[3];
        document.getElementById('rm_enable_input_id').checked  = msg.set[4];

        $('#rm_periodo_div_id' ).text('Periodo_ : ' + msg.save[0] + '[MIN]').html();
        $('#rm_duracion_div_id').text('Duración_: ' + msg.save[1] + '[MIN]').html();
        $('#rm_ciclo_div_id'   ).text('Ciclo_   : ' + msg.save[2] + '[MIN]').html();
        $('#rm_flujo_div_id'   ).text('Flujo_   : ' + msg.save[3] + '[L/min]').html();

        //para depurar
        console.log('Checkeds Ya Recibidos');
        console.log($('#rm_periodo_input_id').val());
        console.log($('#rm_duracion_input_id').val());
        console.log($('#rm_ciclo_input_id').val());
        console.log($('#rm_flujo_input_id').val());
    });

    $('#rm_periodo_div_id' ).css({ 'color': 'white', 'font-size': '110%' });
    $('#rm_duracion_div_id').css({ 'color': 'white', 'font-size': '110%' });
    $('#rm_ciclo_div_id'   ).css({ 'color': 'white', 'font-size': '110%' });
    $('#rm_flujo_div_id'   ).css({ 'color': 'white', 'font-size': '110%' });

});
