#pragma once

/**
    @author Gabor Benyei, Attila Boncser
*/

#include <QDialog>
#include <QtWidgets/QComboBox>
#include <QtCore>
#include <QtWidgets/QPushButton>

#include <QtCore/QSettings>

#include <QSerialPort>
#include <QSerialPortInfo>

#include "IPCtrl.h"
#include "../connPoolCOM.h"
#include "../connPoolUDP.h"

#include "../pupilDetection.h"
#include "../dataWriter.h"
#include "../imageWriter.h"
#include "../dataStreamer.h"
#include "../recEventTracker.h"
#include "../supportFunctions.h"



/**
    In this dialog the user can specify the means of connecting to another computer for listening
    for remote commands. This way, PupilEXT machine (like a "host PC") can be controlled from 
    the machine running e.g. Matlab or PsychoPy experiment code.
    Some of the code regarding serial connection was copied from MCUSettingsDialogInst.

    NOTE: used several parts of the code from former serialSettingsDialog made by ML
*/
class RemoteCCDialog : public QDialog {
    Q_OBJECT

public:

    //explicit RemoteCCDialog(ConnPoolUDP *connPoolUdp, ConnPoolCOM *connPoolCOM, QWidget *parent = nullptr);
    explicit RemoteCCDialog( 
        ConnPoolCOM *connPoolCOM,
        ConnPoolUDP *connPoolUDP,
        QWidget *parent = nullptr);

    ~RemoteCCDialog() override;


private:

    QWidget *mainWindow;

    ConnPoolUDP *connPoolUDP;
    int connPoolUDPIndex = -1;

    ConnPoolCOM *connPoolCOM;
    int connPoolCOMIndex = -1;

    void createForm();

    QSettings *applicationSettings;

    IPCtrl *udpIpBox;
    QSpinBox *udpPortBox;

    ConnPoolUDPInstanceSettings m_currentSettingsUDP;
    ConnPoolCOMInstanceSettings m_currentSettingsCOM;

    QPushButton *refreshButton;

    QComboBox *serialPortInfoListBox;
    QComboBox *baudRateBox;
    QComboBox *dataBitsBox;
    QComboBox *flowControlBox;
    QComboBox *parityBox;
    QComboBox *stopBitsBox;

    QPushButton *connectUDPButton;
    QPushButton *disconnectUDPButton;
    QPushButton *connectCOMButton;
    QPushButton *disconnectCOMButton;

    void fillCOMParameters();

    void updateSettings();

    void saveSettings();
    void loadSettings();

private slots:
    void updateCOMDevices();

    //void readData(const QString &msg);
    void interpretCommand(const QString &msg, const quint64 &timestamp);

//public slots:
//    void sendCommand(QString cmd);

public slots:
    bool isAnyConnected();
    bool isUDPConnected();
    bool isCOMConnected();

    void onConnectUDPClick();
    void onDisconnectUDPClick();
    void onConnectCOMClick();
    void onDisconnectCOMClick();

    void connectUDP(const ConnPoolUDPInstanceSettings &p);
    void connectCOM(const ConnPoolCOMInstanceSettings &p);
    void disconnectUDP();
    void disconnectCOM();

signals:

    void UDPmessageReceived(QString str, quint64 timestamp);

    //void onUDPConnect();
    //void onUDPDisconnect();
    //void onCOMConnect();
    //void onCOMDisconnect();
    void onConnStateChanged();

    void connectRemoteUDP(QString ip, QString port);
    void connectRemoteCOM(ConnPoolCOMInstanceSettings p);
    void connectStreamUDP(QString ip, QString port, QString dataContainer);
    void connectStreamCOM(ConnPoolCOMInstanceSettings p);
    void disconnectRemoteUDP();
    void disconnectRemoteCOM();
    void disconnectStreamUDP();
    void disconnectStreamCOM();

};

