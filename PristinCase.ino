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
  Serial.setTimeout(50);
  miGps.iniciar(9600);
  Serial.println(F("Modulo GPS iniciado, buscando satélites..."));

  imu.iniciar();
  Serial.println(F("Modulo IMU iniciado."));

  tilt.iniciar();
  Serial.println(F("Modulo TILT iniciado."));

  sensorDht.iniciar();
  Serial.println(F("Modulo DHT11 iniciado."));

  reloj.iniciar(false);
  Serial.println(F("Modulo RTC iniciado."));

  registro.iniciar();
  Serial.println(F("Registro iniciado."));

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

// --- LECTURA DE COMANDOS DESDE QT ---
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim(); 
    
    // Filtro para ignorar ruidos vacíos
    if (comando.length() == 0) return;

    // 1. ECO DE DEPURACIÓN: El Arduino devuelve a Qt exactamente lo que leyó
    Serial.print(F("INFO_DEBUG;Llego_al_Arduino:["));
    Serial.print(comando);
    Serial.println(F("]"));
    
    if (comando == "GET" || comando == "DESCARGAR") {
      registro.enviarAQt();
    } 
    else if (comando == "CLEAR" || comando == "BORRAR") {
      registro.borrarMemoria();
      ledImu.resetear();
      ledTilt.resetear();
      ledTemp.resetear();
      ledHum.resetear();
    }
    else if (comando.startsWith("SET,")) {
      int idx1 = 4;
      int idx2 = comando.indexOf(',', idx1);
      int idRotacion = comando.substring(idx1, idx2).toInt();
      
      idx1 = idx2 + 1;
      idx2 = comando.indexOf(',', idx1);
      int idAceleracion = comando.substring(idx1, idx2).toInt();
      
      idx1 = idx2 + 1;
      idx2 = comando.indexOf(',', idx1);
      int idHumedad = comando.substring(idx1, idx2).toInt();
      
      idx1 = idx2 + 1;
      idx2 = comando.indexOf(',', idx1);
      float tMin = comando.substring(idx1, idx2).toFloat();
      
      idx1 = idx2 + 1;
      idx2 = comando.indexOf(',', idx1);
      float tMax = comando.substring(idx1, idx2).toFloat();
      
      idx1 = idx2 + 1;
      float margen = comando.substring(idx1).toFloat();

      // Enviamos los valores a las clases
      Categoria catImu = (Categoria)(idAceleracion + 1);
      imu.setCategoria(catImu);
      
      sensorDht.setPerfilHumedad(idHumedad);
      sensorDht.setLimitesTemperatura(tMin, tMax, margen);
      
      tilt.setSensibilidad(idRotacion == 0);

      ledImu.resetear();
      ledTilt.resetear();
      ledTemp.resetear();
      ledHum.resetear();
      
      // 2. CONFIRMACIÓN DE PARSEO
      Serial.print(F("INFO_DEBUG;Parseo_OK_TMin:"));
      Serial.print(tMin);
      Serial.print(F("_TMax:"));
      Serial.println(tMax);
      Serial.println(F("OK_DATOS_RECIBIDOS"));
    }
  }
}