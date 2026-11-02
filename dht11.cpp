#include "dht11.h"

Dht11::Dht11(int pin, float tMin, float tMax, float mTemp, float hMin, float hMax, float mHum)
    : dht(pin, DHT11),
      temperatura(0.0),
      humedad(0.0),
      lecturaValida(false),
      tempMin(tMin),
      tempMax(tMax),
      margenTemp(mTemp),
      humMin(hMin),
      humMax(hMax),
      margenHum(mHum),
      estadoTemperatura(NORMAL),
      estadoHumedad(NORMAL) {
}

void Dht11::iniciar() {
    dht.begin();
}

void Dht11::actualizar() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        lecturaValida = false;
        return;
    }

    humedad = h;
    temperatura = t;
    lecturaValida = true;

    evaluarEstados();
}

void Dht11::evaluarEstados() {
    if (temperatura < tempMin || temperatura > tempMax) {
        estadoTemperatura = VIOLACION;
    } 
    else if (temperatura < (tempMin + margenTemp) || temperatura > (tempMax - margenTemp)) {
        estadoTemperatura = WARNING;
    } 
    else {
        estadoTemperatura = NORMAL;
    }
    if (humedad < humMin || humedad > humMax) {
        estadoHumedad = VIOLACION;
    } 
    else if (humedad < (humMin + margenHum) || humedad > (humMax - margenHum)) {
        estadoHumedad = WARNING;
    } 
    else {
        estadoHumedad = NORMAL;
    }
}

void Dht11::imprimir() {
    if (!lecturaValida) {
        Serial.println("Error al leer el sensor DHT11");
        return;
    }

    Serial.print("Humedad: ");
    Serial.print(humedad, 1);
    Serial.print(" %, Temp: ");
    Serial.print(temperatura, 1);
    Serial.println(" Celsius");
}

float Dht11::obtenerTemperatura() const {
    return temperatura;
}

float Dht11::obtenerHumedad() const {
    return humedad;
}

bool Dht11::esValido() const {
    return lecturaValida;
}

Estado Dht11::obtenerEstadoTemperatura() const {
    return estadoTemperatura;
}
Estado Dht11::obtenerEstadoHumedad() const {
    return estadoHumedad;
}