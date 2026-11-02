#ifndef REGISTRO_H
#define REGISTRO_H

#include <Arduino.h>
#include <EEPROM.h>
#include "imu.h"

struct Incidente {
    char fechaHora[20];
    float latitud;
    float longitud;
    float temperatura;
    float humedad;
    float aceleracion;
    float rotacion;
    char causa[8];
};

class Registro {
public:
    Registro();

    void iniciar();
    
    void evaluar(Estado estadoTemp, Estado estadoHum, Estado estadoImu, bool huboVuelco,
                 float temp, float hum, float accel, float giro,
                 String fechaHora, float lat, float lng);

    void enviarAQt();
    void borrarMemoria();

private:
    int cantidadRegistros;
    const int maxRegistros = 20;

    unsigned long inicioTemp;
    unsigned long inicioHum;

    bool guardadoTemp;
    bool guardadoHum;
    bool guardadoImu;
    bool guardadoTilt;

    void guardarEnEEPROM(const Incidente &nuevo);
};

#endif