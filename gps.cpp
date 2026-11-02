#include "gps.h"

Gps::Gps(int rx, int tx) : puertoGps(rx, tx) {
}

void Gps::iniciar(long baudios) {
    puertoGps.begin(baudios);
}

void Gps::actualizar() {
    while (puertoGps.available()) {
        gps.encode(puertoGps.read());
    }
}

void Gps::imprimir() {
    Serial.print("Satélites: ");
    Serial.print(gps.satellites.value());

    if (gps.location.isValid()) {
        Serial.print("   https://maps.google.com/?q=");
        Serial.print(gps.location.lat(), 6);
        Serial.print(",");
        Serial.println(gps.location.lng(), 6);
    } else {
        Serial.println("   sin posición todavía");
    }
}
