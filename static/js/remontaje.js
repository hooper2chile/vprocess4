$(document).ready(function() {

    namespace = '/biocl';

    // Connect to the Socket.IO server.
    var socket = io.connect(location.protocol + '//' +
	    		       document.domain + ':' +
		             location.port + namespace);

    //SECCION MEDICIONES
    //Se escuchan las mediciones de ph, OD, Temp.
    /*
    socket.on('Medidas', function(msg) {
        $('#med1').text('T. Promedio: ' + msg.data[0] + ' º[C]').html();
        $('#med2').text('T. Sombrero: ' + msg.data[1] + ' º[C]').html();
        $('#med3').text('T. Mosto   : ' + msg.data[2] + ' º[C]').html();
    });
    */

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

        $('#rm_periodo_div_id' ).text('Periodo         : ' + msg.save[0] + '[MIN]').html();
        $('#rm_duracion_div_id').text('Tiempo encendido: ' + msg.save[1] + '[MIN]').html();
        $('#rm_ciclo_div_id'   ).text('Total Dias      : ' + msg.save[2] + '[DIAS]').html();
        $('#rm_flujo_div_id'   ).text('Flujo           : ' + msg.save[3] + '[L/min]').html();

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







/**************nuevas funciones CyT 4-8-21 ***************************************/
//se emiten los setpoints de bomba hacia el servidor
$('form#aire_formulario').submit(function(event) {
    socket.emit('aire_setpoints',
                { a_periodo : $('#a_periodo_input_id').val(),
                  a_duracion: $('#a_duracion_input_id').val(),
                  a_ciclo   : $('#a_ciclo_input_id').val(),
                  //a_flujo   : $('#b_flujo_input_id').val(),
                  a_enable  : $('#a_enable_input_id').is(':checked')
                });

    //para depurar
    console.log('Emitiendo Valores aire: periodo, duracion, ciclo, enable checked:');
    console.log($('#a_periodo_input_id').val());
    console.log($('#a_duracion_input_id').val());
    console.log($('#a_ciclo_input_id').val());
    //console.log($('#b_flujo_input_id').val());
    console.log($('#a_enable_input_id').is(':checked'));
    return false;
});

//se escuchan desde el servidor los setpoints aplicados
//para ser desplegados en todos los clientes.
socket.on('aire_setpoints', function(msg) {
    document.getElementById('a_periodo_input_id' ).value  = msg.set[0];
    document.getElementById('a_duracion_input_id').value  = msg.set[1];
    document.getElementById('a_ciclo_input_id').value     = msg.set[2];
    //document.getElementById('a_flujo_input_id').value     = msg.set[3];
    document.getElementById('a_enable_input_id').checked  = msg.set[4];

    $('#a_periodo_div_id' ).text('Periodo         : ' + msg.save[0] + '[MIN]').html();
    $('#a_duracion_div_id').text('Tiempo encendido: ' + msg.save[1] + '[MIN]').html();
    $('#a_ciclo_div_id'   ).text('Total Dias      : ' + msg.save[2] + '[DIAS]').html();
    //$('#b_flujo_div_id'   ).text('Flujo           : ' + msg.save[3] + '[L/min]').html();

    //para depurar
    console.log('Checkeds Ya Recibidos');
    console.log($('#b_periodo_input_id').val());
    console.log($('#b_duracion_input_id').val());
    console.log($('#b_ciclo_input_id').val());
    //console.log($('#b_flujo_input_id').val());
});

$('#a_periodo_div_id' ).css({ 'color': 'white', 'font-size': '110%' });
$('#a_duracion_div_id').css({ 'color': 'white', 'font-size': '110%' });
$('#a_ciclo_div_id'   ).css({ 'color': 'white', 'font-size': '110%' });
//$('#a_flujo_div_id'   ).css({ 'color': 'white', 'font-size': '110%' });



/**************nuevas funciones CyT 4-8-21 ***************************************/
//se emiten los setpoints de bomba hacia el servidor
$('form#bomba_formulario').submit(function(event) {
    socket.emit('bomba_setpoints',
                { b_periodo : $('#b_periodo_input_id').val(),
                  b_duracion: $('#b_duracion_input_id').val(),
                  b_ciclo   : $('#b_ciclo_input_id').val(),
                  //b_flujo   : $('#b_flujo_input_id').val(),
                  b_enable  : $('#b_enable_input_id').is(':checked')
                });

    //para depurar
    console.log('Emitiendo Valores Bomba: periodo, duracion, ciclo, enable checked:');
    console.log($('#b_periodo_input_id').val());
    console.log($('#b_duracion_input_id').val());
    console.log($('#b_ciclo_input_id').val());
    //console.log($('#b_flujo_input_id').val());
    console.log($('#b_enable_input_id').is(':checked'));
    return false;
});

//se escuchan desde el servidor los setpoints aplicados
//para ser desplegados en todos los clientes.
socket.on('bomba_setpoints', function(msg) {
    document.getElementById('b_periodo_input_id' ).value  = msg.set[0];
    document.getElementById('b_duracion_input_id').value  = msg.set[1];
    document.getElementById('b_ciclo_input_id').value     = msg.set[2];
    //document.getElementById('b_flujo_input_id').value     = msg.set[3];
    document.getElementById('b_enable_input_id').checked  = msg.set[4];

    $('#b_periodo_div_id' ).text('Periodo         : ' + msg.save[0] + '[MIN]').html();
    $('#b_duracion_div_id').text('Tiempo encendido: ' + msg.save[1] + '[MIN]').html();
    $('#b_ciclo_div_id'   ).text('Total Dias      : ' + msg.save[2] + '[DIAS]').html();
    //$('#a_flujo_div_id'   ).text('Flujo           : ' + msg.save[3] + '[L/min]').html();

    //para depurar
    console.log('Checkeds Ya Recibidos');
    console.log($('#b_periodo_input_id').val());
    console.log($('#b_duracion_input_id').val());
    console.log($('#b_ciclo_input_id').val());
    //console.log($('#a_flujo_input_id').val());
});

$('#b_periodo_div_id' ).css({ 'color': 'white', 'font-size': '110%' });
$('#b_duracion_div_id').css({ 'color': 'white', 'font-size': '110%' });
$('#b_ciclo_div_id'   ).css({ 'color': 'white', 'font-size': '110%' });
//$('#a_flujo_div_id'   ).css({ 'color': 'white', 'font-size': '110%' });










});
