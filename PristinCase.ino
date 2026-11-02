#include "gps.h"
#include "imu.h"
#include "rtc.h"
#include "dht11.h"
#include "alarma.h"
#include "registro.h"

Gps miGps(4, 3);
Imu imu(CAT_NORMAL);        
SensorTilt tilt(A3, true);   
Dht11 sensorDht(2); 
ModuloRtc reloj(A0, A1, A2);

LedEstado ledImu(12, 11);
LedEstado ledTilt(10, 9);
LedEstado ledTemp(7, 5);
LedEstado ledHum(8, 6);

Buzzer buzzer(13);

Registro registro;

unsigned long ultimoGps = 0;
unsigned long ultimoImu = 0;
unsigned long ultimoDht = 0;
unsigned long ultimoRtc = 0;

void setup() {

  Serial.begin(9600);
  miGps.iniciar(9600);
  Serial.println("Modulo GPS iniciado, buscando satélites...");

  imu.iniciar();
  Serial.println("Modulo IMU iniciado.");

  tilt.iniciar();
  Serial.println("Modulo TILT iniciado.");

  sensorDht.iniciar();
  Serial.println("Modulo DHT11 iniciado.");

  reloj.iniciar(false);
  Serial.println("Modulo RTC iniciado.");

  registro.iniciar();
  Serial.println("Registro iniciado.");

  ledImu.iniciar();
  ledTilt.iniciar();
  ledTemp.iniciar();
  ledHum.iniciar();
  buzzer.iniciar();
}

void loop() {

  miGps.actualizar();
  imu.actualizar();
  tilt.actualizar();

  Estado estadoAccel = imu.obtenerEstadoAceleracion();
  Estado estadoGiro = imu.obtenerEstadoRotacion();
  Estado estadoImu = NORMAL;


  if (estadoAccel == VIOLACION || estadoGiro == VIOLACION) {
    estadoImu = VIOLACION;
  } 
  else if (estadoAccel == WARNING || estadoGiro == WARNING) {
    estadoImu = WARNING;
  } 
  else {
    estadoImu = NORMAL;
  }

  ledImu.mostrar(estadoImu);
  
  if (tilt.huboInclinacion() == true) {
    ledTilt.mostrar(VIOLACION);
  } else {
    ledTilt.mostrar(NORMAL);
  }
  
  Estado estadoTemp = sensorDht.obtenerEstadoTemperatura();
  Estado estadoHum = sensorDht.obtenerEstadoHumedad();


  registro.evaluar(
    estadoTemp, estadoHum, estadoImu, tilt.huboInclinacion(),
    sensorDht.obtenerTemperatura(), sensorDht.obtenerHumedad(),
    imu.obtenerAceleracionDinamica(), imu.obtenerRotacion(),
    reloj.obtenerFechaHora(),
    miGps.gps.location.lat(), miGps.gps.location.lng()
  );


  ledTemp.mostrar(estadoTemp);
  ledHum.mostrar(estadoHum);
  
  bool hayPeligro = false;
  
  if (estadoImu == VIOLACION || tilt.huboInclinacion() == true || estadoTemp == VIOLACION || estadoHum == VIOLACION) {
    hayPeligro = true;
  }
  
  buzzer.actualizarIntermitente(hayPeligro);

  if (millis() - ultimoGps > 1000) {
    ultimoGps = millis();
    miGps.imprimir();
  }

  if (millis() - ultimoImu >= 1000) {
    ultimoImu = millis();
    imu.imprimir();
  }

  if (millis() - ultimoDht >= 2000) {
    ultimoDht = millis();
    sensorDht.actualizar();
    sensorDht.imprimir();
  }

  if (millis() - ultimoRtc >= 1000) {
    ultimoRtc = millis();
    reloj.imprimir();
  }


  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "GET") {
        registro.enviarAQt();
    } 
    else if (comando == "CLEAR") {
        registro.borrarMemoria();
    }
    else if (comando.startsWith("SET,")) {
        // Acá parseas los parámetros que te envía Qt para configurar el sensor
    }
}


  /*                                //es para imprimir en el monitor serial
  if (Serial.available()) {         //o guardar sobre el qt

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    
    if (comando == "DESCARGAR") {
      registro.enviarAQt();
    } 
    else if (comando == "BORRAR") {
      registro.borrarMemoria();
    }
  }
  */
}
