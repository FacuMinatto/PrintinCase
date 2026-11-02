#include "imu.h"
#include <math.h>

#define WHO_AM_I     0x75
#define PWR_MGMT_1   0x6B
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG  0x1B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H  0x43

#define MARGEN_WARNING 0.80

// =====================================================
// CLASE IMU
// =====================================================

Imu::Imu(Categoria cat, int direccion)
    : dirI2C(direccion),
      categoria(cat),
      escalaAccel(8192.0),   // Rango +/- 4g (evita saturación en 3.0g)
      escalaGiro(65.5),      // Rango +/- 500 °/s (evita saturación en 270 °/s)
      limiteAceleracion(0.0),
      limiteRotacion(0.0),
      aceleracionDinamica(0.0),
      rotacion(0.0),
      estadoAceleracion(NORMAL),
      estadoRotacion(NORMAL) {

    if (categoria == CAT_FRAGIL) {
        limiteAceleracion = 1.5;
        limiteRotacion = 90.0;
    } else if (categoria == CAT_NORMAL) {
        limiteAceleracion = 3.0;
        limiteRotacion = 270.0;
    } else {
        limiteAceleracion = 0.0;
        limiteRotacion = 0.0;
    }
}

bool Imu::iniciar() {
    Wire.begin();

    // 1. Verificar conexión I2C
    Wire.beginTransmission(dirI2C);
    if (Wire.endTransmission() != 0) {
        Serial.println("ERROR_SENSOR_NO_DETECTADO");
        return false;
    }

    // 2. Verificar identidad (0x68 = MPU6050, 0x70 = MPU6500, 0x71 = MPU9250)
    Wire.beginTransmission(dirI2C);
    Wire.write(WHO_AM_I);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)dirI2C, (uint8_t)1);

    if (!Wire.available()) {
        Serial.println("ERROR_IDENTIFICACION_SENSOR");
        return false;
    }

    byte id = Wire.read();
    if (id != 0x68 && id != 0x70 && id != 0x71) {
        Serial.print("ERROR_ID_DESCONOCIDO: 0x");
        Serial.println(id, HEX);
        return false;
    }

    // 3. Despertar sensor (PWR_MGMT_1 = 0)
    Wire.beginTransmission(dirI2C);
    Wire.write(PWR_MGMT_1);
    Wire.write(0x00);
    Wire.endTransmission();

    // 4. Configurar acelerómetro en +/- 4g (0x08)
    Wire.beginTransmission(dirI2C);
    Wire.write(ACCEL_CONFIG);
    Wire.write(0x08);
    Wire.endTransmission();

    // 5. Configurar giroscopio en +/- 500 °/s (0x08)
    Wire.beginTransmission(dirI2C);
    Wire.write(GYRO_CONFIG);
    Wire.write(0x08);
    Wire.endTransmission();

    Serial.println("SENSOR_IMU_OK");
    return true;
}

int16_t Imu::leer16Bits(byte registro) {
    Wire.beginTransmission(dirI2C);
    Wire.write(registro);
    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)dirI2C, (uint8_t)2);
    if (Wire.available() < 2) {
        return 0;
    }

    byte alto = Wire.read();
    byte bajo = Wire.read();
    return (int16_t)((alto << 8) | bajo);
}

void Imu::actualizar() {
    if (categoria == CAT_RESISTENTE) {
        return;
    }

    // Lectura de aceleraciones
    float ax = leer16Bits(ACCEL_XOUT_H) / escalaAccel;
    float ay = leer16Bits(ACCEL_XOUT_H + 2) / escalaAccel;
    float az = leer16Bits(ACCEL_XOUT_H + 4) / escalaAccel;

    float aceleracionTotal = sqrt(ax * ax + ay * ay + az * az);
    aceleracionDinamica = fabs(aceleracionTotal - 1.0);

    // Lectura de velocidades angulares
    float gx = leer16Bits(GYRO_XOUT_H) / escalaGiro;
    float gy = leer16Bits(GYRO_XOUT_H + 2) / escalaGiro;
    float gz = leer16Bits(GYRO_XOUT_H + 4) / escalaGiro;

    rotacion = sqrt(gx * gx + gy * gy + gz * gz);

    evaluarEstado(rotacion, limiteRotacion, estadoRotacion, "ROTACION");
    evaluarEstado(aceleracionDinamica, limiteAceleracion, estadoAceleracion, "ACELERACION");
}

void Imu::evaluarEstado(float valor, float limite, Estado &estadoActual, const char* parametro) {
    Estado nuevoEstado;

    if (limite <= 0.0) {
        return;
    }

    if (valor >= limite) {
        nuevoEstado = VIOLACION;
    } else if (valor >= limite * MARGEN_WARNING) {
        nuevoEstado = WARNING;
    } else {
        nuevoEstado = NORMAL;
    }

    if (nuevoEstado != estadoActual) {
        if (nuevoEstado == WARNING) {
            Serial.print("WARNING_");
            Serial.println(parametro);
        } else if (nuevoEstado == VIOLACION) {
            Serial.print("VIOLACION_");
            Serial.println(parametro);
        } else {
            Serial.print("NORMAL_");
            Serial.println(parametro);
        }
        estadoActual = nuevoEstado;
    }
}

void Imu::imprimir() {
    Serial.print("Accel Dinamica: ");
    Serial.print(aceleracionDinamica, 2);
    Serial.print(" g | Giro: ");
    Serial.print(rotacion, 2);
    Serial.println(" deg/s");
}

Estado Imu::obtenerEstadoAceleracion() const {
    return estadoAceleracion;
}

Estado Imu::obtenerEstadoRotacion() const {
    return estadoRotacion;
}

// =====================================================
// CLASE SENSOR TILT SW-520D
// =====================================================

SensorTilt::SensorTilt(int pinSensor, bool habilitado)
    : pin(pinSensor),
      activo(habilitado),
      contadorVibracion(0),
      inicioVentana(0),
      inclinacionDetectada(false) {
}

void SensorTilt::iniciar() {
    if (activo) {
        pinMode(pin, INPUT);
    }
}

void SensorTilt::actualizar() {
    if (!activo) {
        return;
    }

    inclinacionDetectada = false;
    int estadoTilt = digitalRead(pin);

    if (estadoTilt == HIGH) {
        if (inicioVentana == 0) {
            inicioVentana = millis();
        }
        contadorVibracion++;
    }

    if (inicioVentana != 0 && (millis() - inicioVentana >= 500)) {
        if (contadorVibracion >= 4) {
            inclinacionDetectada = true;
            Serial.println("ROTACION_SW520_DETECTADA");
        }
        contadorVibracion = 0;
        inicioVentana = 0;
    }
}

bool SensorTilt::huboInclinacion() const {
    return inclinacionDetectada;
}