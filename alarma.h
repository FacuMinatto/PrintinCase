#ifndef ALARMA_H
#define ALARMA_H

#include <Arduino.h>
#include "imu.h"

class LedEstado {
public:
    LedEstado(int pinRojo, int pinVerde);

    void iniciar();
    void mostrar(Estado estado);
    void apagar();

private:
    int pinRojo;
    int pinVerde;
    Estado peorEstado;
};

class Buzzer {
public:
    Buzzer(int pinBuzzer = 13);

    void iniciar();
    void encender();
    void apagar();
    void actualizarIntermitente(bool activar);

private:
    int pin;
    int estadoBuzzer;
    unsigned long ultimoCambio;
};

#endif