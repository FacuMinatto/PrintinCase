#include "registro.h"

Registro::Registro()
    : cantidadRegistros(0),
      inicioTemp(0),
      inicioHum(0),
      guardadoTemp(false),
      guardadoHum(false),
      guardadoImu(false),
      guardadoTilt(false) {
}

void Registro::iniciar() {
    cantidadRegistros = EEPROM.read(0);
    if (cantidadRegistros > maxRegistros) {
        cantidadRegistros = 0;
        EEPROM.update(0, 0);
    }
}

void Registro::evaluar(Estado estadoTemp, Estado estadoHum, Estado estadoImu, bool huboVuelco,
                       float temp, float hum, float accel, float giro,
                       String fechaHora, float lat, float lng) {

    if (!guardadoTemp) {
        if (estadoTemp == VIOLACION) {
            if (inicioTemp == 0) {
                inicioTemp = millis();
            }
            if (millis() - inicioTemp >= 5000) {
                Incidente inc;
                fechaHora.toCharArray(inc.fechaHora, sizeof(inc.fechaHora));
                inc.latitud = lat;
                inc.longitud = lng;
                inc.temperatura = temp;
                inc.humedad = hum;
                inc.aceleracion = accel;
                inc.rotacion = giro;
                strcpy(inc.causa, "TEMP");

                guardarEnEEPROM(inc);
                guardadoTemp = true;
            }
        } else {
            inicioTemp = 0;
        }
    }

    if (!guardadoHum) {
        if (estadoHum == VIOLACION) {
            if (inicioHum == 0) {
                inicioHum = millis();
            }
            if (millis() - inicioHum >= 5000) {
                Incidente inc;
                fechaHora.toCharArray(inc.fechaHora, sizeof(inc.fechaHora));
                inc.latitud = lat;
                inc.longitud = lng;
                inc.temperatura = temp;
                inc.humedad = hum;
                inc.aceleracion = accel;
                inc.rotacion = giro;
                strcpy(inc.causa, "HUM");

                guardarEnEEPROM(inc);
                guardadoHum = true;
            }
        } else {
            inicioHum = 0;
        }
    }

    if (!guardadoImu && estadoImu == VIOLACION) {
        Incidente inc;
        fechaHora.toCharArray(inc.fechaHora, sizeof(inc.fechaHora));
        inc.latitud = lat;
        inc.longitud = lng;
        inc.temperatura = temp;
        inc.humedad = hum;
        inc.aceleracion = accel;
        inc.rotacion = giro;
        strcpy(inc.causa, "IMPACTO");

        guardarEnEEPROM(inc);
        guardadoImu = true;
    }

    if (!guardadoTilt && huboVuelco) {
        Incidente inc;
        fechaHora.toCharArray(inc.fechaHora, sizeof(inc.fechaHora));
        inc.latitud = lat;
        inc.longitud = lng;
        inc.temperatura = temp;
        inc.humedad = hum;
        inc.aceleracion = accel;
        inc.rotacion = giro;
        strcpy(inc.causa, "VUELCO");

        guardarEnEEPROM(inc);
        guardadoTilt = true;
    }
}

void Registro::guardarEnEEPROM(const Incidente &nuevo) {
    if (cantidadRegistros >= maxRegistros) {
        return;
    }

    int direccion = 1 + (cantidadRegistros * sizeof(Incidente));
    EEPROM.put(direccion, nuevo);

    cantidadRegistros++;
    EEPROM.update(0, cantidadRegistros);
}

void Registro::enviarAQt() {
    Serial.println("INICIO_DATOS");
    for (int i = 0; i < cantidadRegistros; i++) {
        Incidente inc;
        int direccion = 1 + (i * sizeof(Incidente));
        EEPROM.get(direccion, inc);

        Serial.print("INCIDENTE;");
        Serial.print(inc.fechaHora);
        Serial.print(";");
        Serial.print(inc.latitud, 6);
        Serial.print(";");
        Serial.print(inc.longitud, 6);
        Serial.print(";");
        Serial.print(inc.temperatura, 1);
        Serial.print(";");
        Serial.print(inc.humedad, 1);
        Serial.print(";");
        Serial.print(inc.aceleracion, 2);
        Serial.print(";");
        Serial.print(inc.rotacion, 2);
        Serial.print(";");
        Serial.println(inc.causa);
    }
    Serial.println("FIN_DATOS");
}

void Registro::borrarMemoria() {
    EEPROM.update(0, 0);
    cantidadRegistros = 0;
    guardadoTemp = false;
    guardadoHum = false;
    guardadoImu = false;
    guardadoTilt = false;
    Serial.println("MEMORIA_BORRADA");
}