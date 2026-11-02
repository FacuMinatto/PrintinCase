#ifndef RTC_H
#define RTC_H

#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

class ModuloRtc {
public:
    // DAT = 2, CLK = 12, CE/RST = 8 por defecto
    ModuloRtc(int pinDat = 2, int pinClk = 12, int pinCe = 8);

    bool iniciar(bool forzarAjuste = false);
    String obtenerFechaHora();
    void imprimir();
    bool estaInicializado() const;

private:
    ThreeWire conexion;
    RtcDS1302<ThreeWire> rtc;
    bool inicializado;
};

#endif
