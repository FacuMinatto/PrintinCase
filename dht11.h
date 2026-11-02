#ifndef DHT11_H
#define DHT11_H

#include <Arduino.h>
#include <DHT.h>
#include "imu.h"

class Dht11 {
public:
    
    Dht11(int pin = 2, 
          float tMin = 2.0, float tMax = 30.0, float mTemp = 2.0,
          float hMin = 20.0, float hMax = 80.0, float mHum = 5.0);

    void iniciar();
    void actualizar();
    void imprimir();

    float obtenerTemperatura() const;
    float obtenerHumedad() const;
    bool esValido() const;

    Estado obtenerEstadoTemperatura() const;
    Estado obtenerEstadoHumedad() const;

private:
    DHT dht;
    float temperatura;
    float humedad;
    bool lecturaValida;

    float tempMin;
    float tempMax;
    float margenTemp;
    
    float humMin;
    float humMax;
    float margenHum;
    
    Estado estadoTemperatura;
    Estado estadoHumedad;

    void evaluarEstados();
};

#endif
