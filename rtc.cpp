#include "rtc.h"

ModuloRtc::ModuloRtc(int pinDat, int pinClk, int pinCe)
    : conexion(pinDat, pinClk, pinCe),
      rtc(conexion),
      inicializado(false) {
}

bool ModuloRtc::iniciar(bool forzarAjuste) {
    rtc.Begin();

    if (rtc.GetIsWriteProtected()) {
        rtc.SetIsWriteProtected(false);
    }

    if (!rtc.GetIsRunning()) {
        rtc.SetIsRunning(true);
    }

    if (forzarAjuste || !rtc.IsDateTimeValid()) {
        rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
        Serial.println("RTC: ajustado con fecha y hora de compilacion");
    }

    inicializado = rtc.IsDateTimeValid();
    if (!inicializado) {
        Serial.println("RTC: lectura invalida; revisar conexiones y alimentacion");
        return false;
    }

    Serial.println("RTC: inicializado correctamente");
    return true;
}

String ModuloRtc::obtenerFechaHora() {
    if (!inicializado) {
        return "00/00/0000 00:00:00";
    }

    RtcDateTime ahora = rtc.GetDateTime();
    if (!ahora.IsValid()) {
        return "00/00/0000 00:00:00";
    }

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02u/%02u/%04u %02u:%02u:%02u",
             (unsigned int)ahora.Day(),
             (unsigned int)ahora.Month(),
             (unsigned int)ahora.Year(),
             (unsigned int)ahora.Hour(),
             (unsigned int)ahora.Minute(),
             (unsigned int)ahora.Second());
    return String(buffer);
}

void ModuloRtc::imprimir() {
    Serial.println(obtenerFechaHora());
}

bool ModuloRtc::estaInicializado() const {
    return inicializado;
}
