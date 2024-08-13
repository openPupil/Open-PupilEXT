#pragma once

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
#include <QRadioButton>
#include <QSpinBox>
#include "../connPoolCOM.h"
#include "../connPoolUDP.h"
#include "IPCtrl.h"

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


class MCUSettingsDialog : public QDialog {
    Q_OBJECT

public:

    enum class ConnectionMethod {
        UDP = 1,
        COM = 2,
    };

    explicit MCUSettingsDialog(ConnPoolCOM *connPoolCOM, ConnPoolUDP *connPoolUDP, QWidget *parent = nullptr);
    ~MCUSettingsDialog() override;

//    ConnPoolCOMInstanceSettings getCOMsettings() const;

    bool isConnected();

private:

    ConnectionMethod currentConnectionMethod = ConnectionMethod::COM;

    ConnPoolUDP *connPoolUDP;
    int connPoolUDPIndex = -1;

    ConnPoolCOM *connPoolCOM;
    int connPoolCOMIndex = -1;

    ConnPoolCOMInstanceSettings m_currentSettingsCOM;
    ConnPoolUDPInstanceSettings m_currentSettingsUDP;
    QIntValidator *m_intValidator = nullptr;

    QSettings *applicationSettings;

    QGroupBox *UDPGroup;
    QLabel *udpIpLabel;
    QLabel *udpPortLabel;
    IPCtrl *udpIpBox;
    QSpinBox *udpPortBox;

    QRadioButton *UDPRadioButton;
    QRadioButton *COMRadioButton;

    QGroupBox *COMGroup;
    QFrame *paramFrame;
    QFrame *serialInfoFrame;

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

    bool isCOMConnected();
    bool isUDPConnected();

private slots:

    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void readData(QString msg, quint64 timestamp);
    void updateDevices();

    void onUDPRadioButtonChecked(bool state);
    void onCOMRadioButtonChecked(bool state);

    void setLimitationsWhileConnected(bool state);

    void disconnectUDP();
    void disconnectCOM();

    void sendCommandUDP(QString cmd);
    void sendCommandCOM(QString cmd);

public slots:

    void selectConnectionMethod(ConnectionMethod connectionMethod);

    // NOTE: cannot call these connect() and disconnect() because it would mess up the Qt signal-slot connect and disconnect functions
    void doConnect();
    void doDisconnect();

    void connectUDP(const ConnPoolUDPInstanceSettings &p);
    void connectCOM(const ConnPoolCOMInstanceSettings &p);

    void sendCommand(QString cmd);

signals:

    void onConnect();
    void onDisconnect();

};
