#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>

// Estados de los parámetros monitoreados
enum Estado {
    NORMAL,
    WARNING,
    VIOLACION
};

// Categorías de fragilidad
enum Categoria {
    CAT_FRAGIL = 1,
    CAT_NORMAL = 2,
    CAT_RESISTENTE = 3
};

class Imu {
public:
    Imu(Categoria categoria = CAT_NORMAL, int direccion = 0x68);

    bool iniciar();
    void actualizar();
    void imprimir();

    Estado obtenerEstadoAceleracion() const;
    Estado obtenerEstadoRotacion() const;

    float obtenerAceleracionDinamica() const { return aceleracionDinamica; }
    float obtenerRotacion() const { return rotacion; }

    void setCategoria(Categoria nuevaCat);

private:
    int dirI2C;
    Categoria categoria;

    float escalaAccel;
    float escalaGiro;

    float limiteAceleracion;
    float limiteRotacion;

    float aceleracionDinamica;
    float rotacion;

    Estado estadoAceleracion;
    Estado estadoRotacion;

    int16_t leer16Bits(byte registro);
    void evaluarEstado(float valor, float limite, Estado &estadoActual, const char* parametro);
};

class SensorTilt {
public:
    SensorTilt(int pinSensor = 4, bool habilitado = true);

    void iniciar();
    void actualizar();
    bool huboInclinacion() const;
    void setSensibilidad(bool hab);

private:
    int pin;
    bool activo;
    int contadorVibracion;
    unsigned long inicioVentana;
    bool inclinacionDetectada;
};

#endif
