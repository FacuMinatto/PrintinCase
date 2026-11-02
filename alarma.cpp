#include "alarma.h"

LedEstado::LedEstado(int r, int v) : pinRojo(r), pinVerde(v), peorEstado(NORMAL) {
}

void LedEstado::iniciar() {
    pinMode(pinRojo, OUTPUT);
    pinMode(pinVerde, OUTPUT);
    apagar();
}

void LedEstado::apagar() {
    analogWrite(pinRojo, 0);
    analogWrite(pinVerde, 0);
}

void LedEstado::resetear() {
    peorEstado = NORMAL;
}

void LedEstado::mostrar(Estado nuevoEstado) {
    if (nuevoEstado > peorEstado) {
        peorEstado = nuevoEstado;
    }

    if (peorEstado == NORMAL) {
        analogWrite(pinRojo, 0);
        analogWrite(pinVerde, 255);
    } 
    else if (peorEstado == WARNING) {
        analogWrite(pinRojo, 255);
        analogWrite(pinVerde, 40);
    } 
    else if (peorEstado == VIOLACION) {
        analogWrite(pinRojo, 255);
        analogWrite(pinVerde, 0);
    }
}

Buzzer::Buzzer(int p) : pin(p), estadoBuzzer(LOW), ultimoCambio(0) {
}

void Buzzer::iniciar() {
    pinMode(pin, OUTPUT);
    apagar();
}

void Buzzer::encender() {
    digitalWrite(pin, HIGH);
    estadoBuzzer = HIGH;
}

void Buzzer::apagar() {
    digitalWrite(pin, LOW);
    estadoBuzzer = LOW;
}

void Buzzer::actualizarIntermitente(bool activar) {
    if (activar == false) {
        apagar();
        return;
    }

    unsigned long tiempoActual = millis();

    if (tiempoActual - ultimoCambio >= 250) {
        ultimoCambio = tiempoActual;

        if (estadoBuzzer == LOW) {
            estadoBuzzer = HIGH;
        } else {
            estadoBuzzer = LOW;
        }

        digitalWrite(pin, estadoBuzzer);
    }
}