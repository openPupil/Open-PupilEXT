#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

/**
    Partially code used from https://doc.qt.io/qt-5/qtserialport-terminal-example.html
    @authors Moritz Lode, Qt Company Ltd., Gabor Benyei, Attila Boncser
*/

#include <QDialog>
#include <QSerialPort>
#include <QtGui/QIntValidator>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/qtextedit.h>
#include <QtCore/QSettings>

#include "../connPoolCOM.h"

QT_BEGIN_NAMESPACE

/**
    Settings window (dialog) for the serial connection establishment, represents an open serial connection

slots:
    sendCommand(): Slot to receive string that is send over the open serial connection

signals:
    onConnect(): Signal send at opening a serial connection
    onDisconnect(): Signal send to closing a serial connection
*/

#define SERIAL_DEF_BAUDRATE 3
#define SERIAL_DEF_DATABITS 3
#define SERIAL_DEF_PARITY 0
#define SERIAL_DEF_STOPBITS 0
#define SERIAL_DEF_FLOWCONTROL 0


class SerialSettingsDialog : public QDialog {
    Q_OBJECT

public:

    /*struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };*/

    explicit SerialSettingsDialog(ConnPoolCOM *connPoolCOM, QWidget *parent = nullptr);
    ~SerialSettingsDialog() override;

    ConnPoolCOMInstanceSettings settings() const;

    bool isCOMConnected();

private:

    ConnPoolCOM *connPoolCOM;
    int connPoolCOMIndex = -1;

    ConnPoolCOMInstanceSettings m_currentSettings;
    QIntValidator *m_intValidator = nullptr;

    QSettings *applicationSettings;

    QGroupBox *paramGroup;
    QGroupBox *serialPortGroup;

    QLabel *descriptionLabel;
    QLabel *locationLabel;
    QLabel *manufacturerLabel;
    QLabel *pidLabel;
    QLabel *serialNumberLabel;
    QLabel *vidLabel;
    QTextEdit *textField;

    QPushButton *applyButton;
    QPushButton *clearButton;
    QPushButton *refreshButton;

    QCheckBox *localEchoCheckBox;

    QComboBox *serialPortInfoListBox;
    QComboBox *baudRateBox;
    QComboBox *dataBitsBox;
    QComboBox *flowControlBox;
    QComboBox *parityBox;
    QComboBox *stopBitsBox;

    void createForm();
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

    void saveSettings();
    void loadSettings();

private slots:

    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void readData(QString msg, quint64 timestamp); // GB modified
    void updateDevices();

    void setLimitationsWhileConnected(bool state);

public slots:

    void connectCOM(const ConnPoolCOMInstanceSettings &p);
    void connectSerialPort();
    void disconnectCOM();
    void sendCommand(QString cmd);

signals:

    void onConnect();
    void onDisconnect();

};

#endif // SETTINGSDIALOG_H
