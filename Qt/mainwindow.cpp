#include "mainwindow.h"
#include <QApplication>
#include <QPalette>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    configurarUI();
    aplicarEstilos();
    setWindowTitle("Configurador de Transporte");
    setMinimumSize(420, 480);

    puertoSerie = new QSerialPort(this);
    puertoSerie->setPortName("/dev/ttyUSB0");
    puertoSerie->setBaudRate(QSerialPort::Baud115200);
    puertoSerie->setDataBits(QSerialPort::Data8);
    puertoSerie->setParity(QSerialPort::NoParity);
    puertoSerie->setStopBits(QSerialPort::OneStop);
    puertoSerie->setFlowControl(QSerialPort::NoFlowControl);

    // NUEVO: Conectar la señal de recepción de datos
    connect(puertoSerie, &QSerialPort::readyRead, this, &MainWindow::leerDatosSerie);

    if (puertoSerie->open(QIODevice::ReadWrite)) {
        qDebug() << "Puerto serie abierto correctamente.";
    } else {
        qDebug() << "Error: No se pudo abrir el puerto serie al iniciar.";
    }
}

MainWindow::~MainWindow() {
    if (puertoSerie->isOpen()) {
        puertoSerie->close();
    }
}

// ... [mantén tu función configurarUI(), crearPantallaMenu() y crearPantallaConfig() intactas] ...
void MainWindow::configurarUI() {
    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(crearPantallaMenu());      
    stackedWidget->addWidget(crearPantallaConfig());    
    stackedWidget->addWidget(crearPantallaHistorial()); 
    setCentralWidget(stackedWidget);
}

QWidget* MainWindow::crearPantallaMenu() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QPushButton *btnIrHistorial = new QPushButton("Ver Historial");
    btnIrHistorial->setMinimumHeight(50);
    QPushButton *btnIrConfig = new QPushButton("Configurar Parámetros");
    btnIrConfig->setMinimumHeight(50);

    layout->addWidget(btnIrHistorial);
    layout->addWidget(btnIrConfig);

    connect(btnIrHistorial, &QPushButton::clicked, this, &MainWindow::irAHistorial);
    connect(btnIrConfig, &QPushButton::clicked, this, &MainWindow::irAConfiguracion);
    return widget;
}

QWidget* MainWindow::crearPantallaConfig() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layoutPrincipal = new QVBoxLayout(widget);
    layoutPrincipal->setContentsMargins(16, 16, 16, 16);
    layoutPrincipal->setSpacing(14);

    // Grupo de Sensores
    QGroupBox *grupoSensores = new QGroupBox("Perfiles de Entorno");
    QFormLayout *layoutSensores = new QFormLayout();
    
    comboRotacion = new QComboBox();
    comboRotacion->addItem("Sensible a la rotación");
    comboRotacion->addItem("Indistinto");

    comboAceleracion = new QComboBox();
    comboAceleracion->addItem("Muy Frágil");
    comboAceleracion->addItem("Frágil");
    comboAceleracion->addItem("Resistente");

    comboHumedad = new QComboBox();
    comboHumedad->addItem("Seco (20-50%)");
    comboHumedad->addItem("Húmedo (60-80%)");
    comboHumedad->addItem("Indistinto (20-90%)");

    layoutSensores->addRow("Límite de Rotación:", comboRotacion);
    layoutSensores->addRow("Sensibilidad de Impacto:", comboAceleracion);
    layoutSensores->addRow("Perfil de Humedad:", comboHumedad);
    grupoSensores->setLayout(layoutSensores);

    // Grupo de Temperatura
    QGroupBox *grupoTemperatura = new QGroupBox("Rango Térmico (°C)");
    QFormLayout *layoutTemperatura = new QFormLayout();

    spinTempMin = new QDoubleSpinBox();
    spinTempMin->setRange(-50.0, 100.0);
    spinTempMin->setValue(2.0);

    spinTempMax = new QDoubleSpinBox();
    spinTempMax->setRange(-50.0, 100.0);
    spinTempMax->setValue(8.0);

    spinMargen = new QDoubleSpinBox();
    spinMargen->setRange(0.0, 10.0);
    spinMargen->setValue(1.5);

    layoutTemperatura->addRow("Temperatura Mínima:", spinTempMin);
    layoutTemperatura->addRow("Temperatura Máxima:", spinTempMax);
    layoutTemperatura->addRow("Margen Riesgoso (±):", spinMargen);
    grupoTemperatura->setLayout(layoutTemperatura);

    // Botones de acción
    QHBoxLayout *layoutBotones = new QHBoxLayout();
    btnVolverConfig = new QPushButton("Volver");
    btnVolverConfig->setMinimumHeight(38);
    
    btnEnviar = new QPushButton("Cargar Parámetros");
    btnEnviar->setMinimumHeight(38);

    layoutBotones->addWidget(btnVolverConfig);
    layoutBotones->addWidget(btnEnviar);

    connect(btnEnviar, &QPushButton::clicked, this, &MainWindow::enviarDatos);
    connect(btnVolverConfig, &QPushButton::clicked, this, &MainWindow::volverAlMenu);

    layoutPrincipal->addWidget(grupoSensores);
    layoutPrincipal->addWidget(grupoTemperatura);
    layoutPrincipal->addLayout(layoutBotones);

    return widget; // Esto corrige el warning del retorno
}

// ACTUALIZADO: Pantalla de historial
QWidget* MainWindow::crearPantallaHistorial() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layoutPrincipal = new QVBoxLayout(widget);

    txtHistorial = new QTextEdit();
    txtHistorial->setReadOnly(true);
    // NUEVO: Encabezados adaptados exactamente a tu struct de Arduino
    txtHistorial->setPlainText("FechaHora,Latitud,Longitud,Temperatura,Humedad,Aceleracion,Rotacion,Causa\n");

    QHBoxLayout *layoutBotones = new QHBoxLayout();
    
    btnVolverHistorial = new QPushButton("Volver");
    btnSincronizar = new QPushButton("Sincronizar Arduino"); // NUEVO
    btnDescargar = new QPushButton("Descargar CSV");
    btnLimpiar = new QPushButton("Limpiar Historial");

    layoutBotones->addWidget(btnVolverHistorial);
    layoutBotones->addWidget(btnSincronizar);
    layoutBotones->addWidget(btnDescargar);
    layoutBotones->addWidget(btnLimpiar);

    connect(btnVolverHistorial, &QPushButton::clicked, this, &MainWindow::volverAlMenu);
    connect(btnSincronizar, &QPushButton::clicked, this, &MainWindow::solicitarHistorial);
    connect(btnDescargar, &QPushButton::clicked, this, &MainWindow::descargarHistorial);
    connect(btnLimpiar, &QPushButton::clicked, this, &MainWindow::limpiarHistorial);

    layoutPrincipal->addWidget(txtHistorial);
    layoutPrincipal->addLayout(layoutBotones);

    return widget;
}

// --- NUEVA LÓGICA DE PUERTO SERIE (Lectura de Arduino) ---

void MainWindow::leerDatosSerie() {
    // Leemos todo lo que llega y lo guardamos en un buffer 
    bufferSerie.append(puertoSerie->readAll());

    // Procesamos línea por línea (por si llega fragmentado)
    while (bufferSerie.contains('\n')) {
        int posEnter = bufferSerie.indexOf('\n');
        QString linea = QString::fromUtf8(bufferSerie.left(posEnter)).trimmed();
        bufferSerie.remove(0, posEnter + 1);

        // Si la línea es un incidente de tu Arduino
        if (linea.startsWith("INCIDENTE;")) {
            // Quitamos la palabra "INCIDENTE;"
            linea.replace("INCIDENTE;", "");
            // Reemplazamos los punto y coma por comas para que sea un CSV perfecto
            linea.replace(";", ",");
            // Lo agregamos a la pantalla
            txtHistorial->append(linea);
        } 
        else if (linea == "MEMORIA_BORRADA") {
            QMessageBox::information(this, "Arduino", "La memoria EEPROM ha sido borrada.");
        }
    }
}

void MainWindow::solicitarHistorial() {
    if (puertoSerie->isOpen()) {
        // Le mandamos un comando al Arduino para que empiece a escupir los datos
        puertoSerie->write("GET\n");
    } else {
        QMessageBox::warning(this, "Error", "Puerto serie cerrado.");
    }
}

// --- Resto de las funciones ---

bool MainWindow::verificarPassword() {
    bool ok;
    QString texto = QInputDialog::getText(this, "Autenticación requerida", "Ingrese la contraseña de dueño:", QLineEdit::Password, "", &ok);
    if (ok && texto == "sudo") {
        return true;
    } else if (ok) {
        QMessageBox::warning(this, "Error", "Contraseña incorrecta.");
    }
    return false;
}

void MainWindow::irAConfiguracion() { if (verificarPassword()) stackedWidget->setCurrentIndex(1); }
void MainWindow::irAHistorial() { stackedWidget->setCurrentIndex(2); }
void MainWindow::volverAlMenu() { stackedWidget->setCurrentIndex(0); }

void MainWindow::limpiarHistorial() {
    if (verificarPassword()) {
        txtHistorial->clear();
        txtHistorial->setPlainText("FechaHora,Latitud,Longitud,Temperatura,Humedad,Aceleracion,Rotacion,Causa\n");
        
        // NUEVO: Le avisamos al Arduino que borre su EEPROM
        if (puertoSerie->isOpen()) {
            puertoSerie->write("CLEAR\n");
        }
        
        QMessageBox::information(this, "Éxito", "El historial local ha sido borrado.");
    }
}

void MainWindow::descargarHistorial() {
    QString nombreArchivo = QFileDialog::getSaveFileName(this, "Guardar Historial", "historial.csv", "Archivos CSV (*.csv)");
    if (nombreArchivo.isEmpty()) return;
    QFile archivo(nombreArchivo);
    if (archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream salida(&archivo);
        salida << txtHistorial->toPlainText();
        archivo.close();
        QMessageBox::information(this, "Éxito", "Historial guardado correctamente.");
    }
}

void MainWindow::enviarDatos() {
    // (Igual que antes)
    int idRotacion = comboRotacion->currentIndex();
    int idAceleracion = comboAceleracion->currentIndex();
    int idHumedad = comboHumedad->currentIndex();
    double tMin = spinTempMin->value();
    double tMax = spinTempMax->value();
    double margen = spinMargen->value();

    QString trama = "SET," + QString::number(idRotacion) + "," +
                    QString::number(idAceleracion) + "," + QString::number(idHumedad) + "," +
                    QString::number(tMin) + "," + QString::number(tMax) + "," + QString::number(margen) + "\n";

    if (puertoSerie->isOpen()) {
        puertoSerie->write(trama.toUtf8());
        QMessageBox::information(this, "Enviado", "Parámetros cargados al dispositivo.");
    }
}

// ... [mantén tu función aplicarEstilos() intacta] ...

void MainWindow::aplicarEstilos() {
    QPalette paletaOscura = qApp->palette();
    paletaOscura.setColor(QPalette::Window, QColor("#121212"));
    paletaOscura.setColor(QPalette::WindowText, QColor("#E0E0E0"));
    paletaOscura.setColor(QPalette::Base, QColor("#1A1A1A"));
    paletaOscura.setColor(QPalette::Button, QColor("#1F1F1F"));
    paletaOscura.setColor(QPalette::ButtonText, QColor("#E0E0E0"));
    qApp->setPalette(paletaOscura);

    QString qss =
        "QMainWindow, QWidget { background-color: #121212; color: #E0E0E0; font-size: 13px; }"
        "QLabel { background-color: transparent; }"
        "QGroupBox { background-color: #1A1A1A; border: 1px solid #2E2E2E; border-radius: 6px; margin-top: 8px; font-weight: bold; color: #A78BFA; }"
        "QComboBox, QDoubleSpinBox, QTextEdit { background-color: #242424; color: #F0F0F0; border: 1px solid #3A3A3A; border-radius: 4px; padding: 5px; }"
        "QComboBox:focus, QDoubleSpinBox:focus, QTextEdit:focus { border: 1px solid #7C3AED; }"
        "QPushButton { background-color: #1F1F1F; color: #FFFFFF; border: 1px solid #7C3AED; border-radius: 5px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7C3AED; }"
        "QPushButton:pressed { background-color: #5B21B6; }"
        "QInputDialog { background-color: #1A1A1A; }"; 

    this->setStyleSheet(qss);
}
