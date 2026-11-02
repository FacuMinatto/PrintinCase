#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

class Gps {
public:
    Gps(int rx = 4, int tx = 3);

    void iniciar(long baudios = 9600);
    void actualizar();
    void imprimir();

    TinyGPSPlus gps;

private:
    SoftwareSerial puertoGps;
};

#endif
