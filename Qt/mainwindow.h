#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void enviarDatos();
    void irAConfiguracion();
    void irAHistorial();
    void volverAlMenu();
    void limpiarHistorial();
    void descargarHistorial();
    
    // NUEVOS SLOTS PARA ARDUINO
    void solicitarHistorial();
    void leerDatosSerie();

private:
    void configurarUI();
    void aplicarEstilos();
    
    QWidget* crearPantallaMenu();
    QWidget* crearPantallaConfig();
    QWidget* crearPantallaHistorial();
    
    bool verificarPassword(); 

    QStackedWidget *stackedWidget;

    // Componentes de Configuración
    QComboBox *comboRotacion;
    QComboBox *comboAceleracion;
    QComboBox *comboHumedad;
    QDoubleSpinBox *spinTempMin;
    QDoubleSpinBox *spinTempMax;
    QDoubleSpinBox *spinMargen;
    QPushButton *btnEnviar;
    QPushButton *btnVolverConfig;

    // Componentes de Historial
    QTextEdit *txtHistorial;
    QPushButton *btnVolverHistorial;
    QPushButton *btnDescargar;
    QPushButton *btnLimpiar;
    QPushButton *btnSincronizar; // NUEVO BOTON

    QSerialPort *puertoSerie;
    QByteArray bufferSerie; // NUEVO: Buffer para no perder datos cortados
};

#endif // MAINWINDOW_H
