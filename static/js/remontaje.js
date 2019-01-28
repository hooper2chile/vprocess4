$(document).ready(function() {

    namespace = '/biocl';

    // Connect to the Socket.IO server.
    var socket = io.connect(location.protocol + '//' +
	    		       document.domain + ':' +
		             location.port + namespace);

    //SECCION MEDICIONES
    //Se escuchan las mediciones de ph, OD, Temp.
    socket.on('Medidas', function(msg) {
        $('#med1').text('pH: '    + msg.data[0]          ).html();
        $('#med2').text('OD: '    + msg.data[1] + ' %'   ).html();
        $('#med3').text('Temp: '  + msg.data[2] + ' º[C]').html();
    });

    //SECCION AUTOCLAVE
    //se emiten los setpoints hacia el servidor
    $('form#ac_setpoints').submit(function(event) {
        socket.emit('ac_setpoints',
                    { ac_temp: $('#temp').val(),
                      ac_time: $('#time').val(),
                      temp_en: $('#temp_enable').is(':checked'),
                      time_en: $('#time_enable').is(':checked')
                     });

        //para depurar
        console.log('Emitiendo Valores: temp, timer, good checked: ');
        console.log($('#time').val());
        return false;
    });

    //se escuchan desde el servidor los setpoints aplicados
    //para ser desplegados en todos los clientes.
    socket.on('ac_setpoints', function(msg) {
        document.getElementById('temp').value   = msg.set[0];
        document.getElementById('time').value   = msg.set[1];
        document.getElementById('temp_enable').checked = msg.set[2];
	      document.getElementById('time_enable').checked = msg.set[3];

        $('#temp_set' ).text('Temp  Set: ' + msg.save[0] + '[ºC] ' ).html();
        $('#time_set' ).text('Timer Set: ' + msg.save[1] + '[MIN]' ).html();

        //para depurar
        console.log('Checkeds Recibidos');
        console.log($('#time').val());
    });

    $('#temp_set').css({ 'color': 'red', 'font-size': '110%' });
    $('#time_set').css({ 'color': 'red', 'font-size': '110%' });


    //se emiten señal de reinicio, apagado, grabacion y limpiaza hacia el servidor
    $('form#process').submit(function(event) {
        socket.emit('power',
                    { action  : $('select[name=selection]').val(),
                      checked : $('#confirm').is(':checked')
                    });
      //para depurar
      //  console.log('Emitiendo Valores de Acción');
      //  console.log($('select[name=selection]').val())
      //  console.log($('#confirm').is(':checked'));
      return false;
    });
});
