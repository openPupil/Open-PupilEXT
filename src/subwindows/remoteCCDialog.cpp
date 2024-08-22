#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/QLabel>
#include <QtWidgets/QtWidgets>
#include "remoteCCDialog.h"
#include "../SVGIconColorAdjuster.h"

#include "../mainwindow.h" // IMPORTANT: Can only be placed here


RemoteCCDialog::RemoteCCDialog(
    ConnPoolCOM *connPoolCOM,
    ConnPoolUDP *connPoolUDP,
    QWidget *parent) :
    QDialog(parent),
    mainWindow(parent),
    connPoolCOM(connPoolCOM),
    connPoolUDP(connPoolUDP),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setMinimumSize(310, 400);
    this->setWindowTitle("Remote Control Connection");

    //qDebug() << pupilDetection;
    //pupilDetection = dynamic_cast<MainWindow*>(parent)->pupilDetectionWorker;
    //qDebug() << pupilDetection;

    createForm();

    updateCOMDevices();
    fillCOMParameters();

    loadSettings();

}

RemoteCCDialog::~RemoteCCDialog() {
    
}

void RemoteCCDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    QGroupBox *udpGroup = new QGroupBox("UDP");
    QFormLayout *udpLayout = new QFormLayout;

    QLabel *udpIpLabel = new QLabel(tr("IP address:"));
    udpIpBox = new IPCtrl();
    //udpIpBox->setFixedWidth(140);

    //udpIpBox->text();
    udpIpBox->setValue("0.0.0.0");

    udpLayout->addRow(udpIpLabel, udpIpBox);

    QLabel *udpPortLabel = new QLabel(tr("Port:"));
    udpPortBox = new QSpinBox();
    udpPortBox->setMinimum(1);
    udpPortBox->setMaximum(9999);
    udpPortBox->setSingleStep(1);
    udpPortBox->setValue(6900);
    udpPortBox->setFixedWidth(70);

    udpLayout->addRow(udpPortLabel, udpPortBox);

    QWidget *widgetsRow1 = new QWidget();
    QHBoxLayout *UDPbuttonsLayout = new QHBoxLayout();
    connectUDPButton = new QPushButton("Start listening");
    disconnectUDPButton = new QPushButton("Stop listening");
    UDPbuttonsLayout->addWidget(connectUDPButton);
    UDPbuttonsLayout->addWidget(disconnectUDPButton);
    widgetsRow1->setLayout(UDPbuttonsLayout);
    udpLayout->addWidget(widgetsRow1);
    disconnectUDPButton->setEnabled(false);

    udpGroup->setLayout(udpLayout);
    mainLayout->addWidget(udpGroup);

    //////

    QGroupBox *comGroup = new QGroupBox("COM");
    QFormLayout *comLayout = new QFormLayout;



    QLabel *comPortLabel = new QLabel(tr("Serial port:"));

    serialPortInfoListBox = new QComboBox();
    //comPortLayout->addWidget(serialPortInfoListBox);

    const QIcon refreshIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/view-refresh.svg"), applicationSettings);
    refreshButton = new QPushButton(""); // view-refresh.svg
    refreshButton->setIcon(refreshIcon);
    refreshButton->setMinimumWidth(42);


    QHBoxLayout *comRow1 = new QHBoxLayout;
    comRow1->addWidget(serialPortInfoListBox);
    QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
    comRow1->addSpacerItem(sp1);
    comRow1->addWidget(refreshButton);
    //hwTriggerGroupLayout->addRow(comPortLabel, comRow1);

    comLayout->addRow(comPortLabel, comRow1);
    //comPortLayout->addWidget(refreshButton);




    QLabel *baudRateLabel = new QLabel(tr("Baudrate"));
    QLabel *dataBitsLabel = new QLabel(tr("Data bits"));
    QLabel *parityLabel = new QLabel(tr("Parity"));
    QLabel *stopBitsLabel = new QLabel(tr("Stop bits"));
    QLabel *flowControlLabel = new QLabel(tr("Flow control"));

    baudRateBox = new QComboBox();
    dataBitsBox = new QComboBox();
    flowControlBox = new QComboBox();
    parityBox = new QComboBox();
    stopBitsBox = new QComboBox();

    serialPortInfoListBox->setMinimumWidth(150);
    baudRateBox->setMinimumWidth(100);
    dataBitsBox->setMinimumWidth(100);
    flowControlBox->setMinimumWidth(100);
    parityBox->setMinimumWidth(100);
    stopBitsBox->setMinimumWidth(100);

    comLayout->addRow(baudRateLabel, baudRateBox);
    comLayout->addRow(dataBitsLabel, dataBitsBox);
    comLayout->addRow(parityLabel, parityBox);
    comLayout->addRow(stopBitsLabel, stopBitsBox);
    comLayout->addRow(flowControlLabel, flowControlBox);


    QWidget *widgetsRow2 = new QWidget();
    QHBoxLayout *COMbuttonsLayout = new QHBoxLayout();
    connectCOMButton = new QPushButton("Start listening");
    disconnectCOMButton = new QPushButton("Stop listening");
    COMbuttonsLayout->addWidget(connectCOMButton);
    COMbuttonsLayout->addWidget(disconnectCOMButton);
    widgetsRow2->setLayout(COMbuttonsLayout);
    comLayout->addWidget(widgetsRow2);
    disconnectCOMButton->setEnabled(false);

    comGroup->setLayout(comLayout);
    mainLayout->addWidget(comGroup);

    connect(connectUDPButton, SIGNAL(clicked()), this, SLOT(onConnectUDPClick()));
    connect(disconnectUDPButton, SIGNAL(clicked()), this, SLOT(onDisconnectUDPClick()));
    connect(connectCOMButton, SIGNAL(clicked()), this, SLOT(onConnectCOMClick()));
    connect(disconnectCOMButton, SIGNAL(clicked()), this, SLOT(onDisconnectCOMClick()));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(updateCOMDevices()));

    setLayout(mainLayout);
}

void RemoteCCDialog::connectUDP(const ConnPoolUDPInstanceSettings &p) {

    int index = connPoolUDP->setupAndOpenConnection(p, ConnPoolPurposeFlag::REMOTE_CONTROL);
    if(index >= 0) {
        connectUDPButton->setEnabled(false);
        disconnectUDPButton->setEnabled(true);

        connPoolUDPIndex = index;
        connPoolUDP->subscribeListener(index, this, SLOT(interpretCommand(QString, quint64)));

        //emit onCOMConnect();
        emit onConnStateChanged();
    } else {
        if(connPoolUDP->getInstance(connPoolUDPIndex) != nullptr) {
            QString errMsg = connPoolUDP->getInstance(connPoolUDPIndex)->errorString();
            QMessageBox::critical(this, tr("Error"), connPoolUDP->getInstance(connPoolUDPIndex)->errorString());
        }
        qDebug() << "Error while connecting to the specified UDP port. It is possibly already in use by another application.";
    }
}

void RemoteCCDialog::onConnectUDPClick() {
    updateSettings();
    connectUDP(m_currentSettingsUDP);
}

void RemoteCCDialog::onDisconnectUDPClick() {
    disconnectUDP();
}

void RemoteCCDialog::disconnectUDP() {
    if(connPoolUDPIndex < 0) {
        qDebug() << "RemoteCCDialog::disconnectUDP(): connPoolUDPIndex value is invalid";
        return;
    }

    if(connPoolUDP->getInstance(connPoolUDPIndex)->isOpen()) {
        connPoolUDP->unsubscribeListener(connPoolUDPIndex, this, SLOT(interpretCommand(QString, quint64)));
        connPoolUDP->closeConnection(connPoolUDPIndex, ConnPoolPurposeFlag::REMOTE_CONTROL);
    }
    connPoolUDPIndex = -1;

    connectUDPButton->setEnabled(true);
    disconnectUDPButton->setEnabled(false);

    //emit onUDPDisconnect();
    emit onConnStateChanged();
}

void RemoteCCDialog::connectCOM(const ConnPoolCOMInstanceSettings &p) {
    int index = connPoolCOM->setupAndOpenConnection(p, ConnPoolPurposeFlag::REMOTE_CONTROL);
    if(index >= 0) {
        connectCOMButton->setEnabled(false);
        disconnectCOMButton->setEnabled(true);

        connPoolCOMIndex = index;
        connPoolCOM->subscribeListener(index, this, SLOT(interpretCommand(QString, quint64)));

        //emit onCOMConnect();
        emit onConnStateChanged();
    } else {
        if(connPoolCOM->getInstance(connPoolCOMIndex) != nullptr) {
            QString errMsg = connPoolCOM->getInstance(connPoolCOMIndex)->errorString();
            QMessageBox::critical(this, tr("Error"), connPoolCOM->getInstance(connPoolCOMIndex)->errorString());
        }
        qDebug() << "Error while connecting to the specified COM port. It is possibly already in use by another application.";
    }
}

void RemoteCCDialog::onConnectCOMClick() {
    updateSettings();
    connectCOM(m_currentSettingsCOM);
}

void RemoteCCDialog::onDisconnectCOMClick() {
    disconnectCOM();
}

void RemoteCCDialog::disconnectCOM() {
    if(connPoolCOMIndex < 0) {
        qDebug() << "RemoteCCDialog::disconnectCOM(): connPoolCOMIndex value is invalid";
        return;
    }

    if(connPoolCOM->getInstance(connPoolCOMIndex)->isOpen()) {
        connPoolCOM->unsubscribeListener(connPoolCOMIndex, this, SLOT(interpretCommand(QString, quint64)));
        connPoolCOM->closeConnection(connPoolCOMIndex, ConnPoolPurposeFlag::REMOTE_CONTROL);
    }
    connPoolCOMIndex = -1;

    connectCOMButton->setEnabled(true);
    disconnectCOMButton->setEnabled(false);

    //emit onCOMDisconnect();
    emit onConnStateChanged();
}

void RemoteCCDialog::fillCOMParameters() {
    baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    baudRateBox->setCurrentIndex(3);
    //baudRateBox->addItem(tr("Custom"));

    dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    dataBitsBox->setCurrentIndex(3);

    parityBox->addItem(tr("None"), QSerialPort::NoParity);
    parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

bool RemoteCCDialog::isAnyConnected() {
    return (isUDPConnected() || isCOMConnected());
}

bool RemoteCCDialog::isUDPConnected() {
    if(connPoolUDPIndex < 0 || !connPoolUDP->getInstance(connPoolUDPIndex)->isOpen()) /*||
        connPoolUDP->getInstance(connPoolUDPIndex)->state() == QAbstractSocket::ClosingState || 
        connPoolUDP->getInstance(connPoolUDPIndex)->state() == QAbstractSocket::UnconnectedState )*/
        return false;
    else
        return true;
}

bool RemoteCCDialog::isCOMConnected() {
    if(connPoolCOMIndex < 0 || !connPoolCOM->getInstance(connPoolCOMIndex)->isOpen())
        return false;
    else
        return true;
}

// Update current settings with the configuration from the form
void RemoteCCDialog::updateSettings()
{
    m_currentSettingsUDP.ipAddress = QHostAddress(udpIpBox->getValue());
    m_currentSettingsUDP.portNumber = udpPortBox->value();

    m_currentSettingsCOM.name = serialPortInfoListBox->currentText();

    if (baudRateBox->currentIndex() == 4) {
        m_currentSettingsCOM.baudRate = baudRateBox->currentText().toInt();
    } else {
        m_currentSettingsCOM.baudRate = static_cast<QSerialPort::BaudRate>(
                    baudRateBox->itemData(baudRateBox->currentIndex()).toInt());
    }
    m_currentSettingsCOM.stringBaudRate = QString::number(m_currentSettingsCOM.baudRate);

    m_currentSettingsCOM.dataBits = static_cast<QSerialPort::DataBits>(
                dataBitsBox->itemData(dataBitsBox->currentIndex()).toInt());
    m_currentSettingsCOM.stringDataBits = dataBitsBox->currentText();

    m_currentSettingsCOM.parity = static_cast<QSerialPort::Parity>(
                parityBox->itemData(parityBox->currentIndex()).toInt());
    m_currentSettingsCOM.stringParity = parityBox->currentText();

    m_currentSettingsCOM.stopBits = static_cast<QSerialPort::StopBits>(
                stopBitsBox->itemData(stopBitsBox->currentIndex()).toInt());
    m_currentSettingsCOM.stringStopBits = stopBitsBox->currentText();

    m_currentSettingsCOM.flowControl = static_cast<QSerialPort::FlowControl>(
                flowControlBox->itemData(flowControlBox->currentIndex()).toInt());
    m_currentSettingsCOM.stringFlowControl = flowControlBox->currentText();

    //m_currentSettings.localEchoEnabled = localEchoCheckBox->isChecked();

    saveSettings();
}

// Loads the serial port settings from application settings
// BG: code mostly copied from MCUSettingsDialogInst
void RemoteCCDialog::loadSettings() {

    udpIpBox->setValue(applicationSettings->value("RemoteControlConnection.UDP.ipAddress", udpIpBox->getValue()).toString());
    udpPortBox->setValue(applicationSettings->value("RemoteControlConnection.UDP.portNumber", udpPortBox->value()).toInt());

    serialPortInfoListBox->setCurrentText(applicationSettings->value("RemoteControlConnection.COM.name", serialPortInfoListBox->itemText(0)).toString());
    baudRateBox->setCurrentText(applicationSettings->value("RemoteControlConnection.COM.baudRate", baudRateBox->itemText(3)).toString());
    dataBitsBox->setCurrentText(applicationSettings->value("RemoteControlConnection.COM.dataBits", dataBitsBox->itemText(0)).toString());
    parityBox->setCurrentText(applicationSettings->value("RemoteControlConnection.COM.parity", parityBox->itemText(0)).toString());
    stopBitsBox->setCurrentText(applicationSettings->value("RemoteControlConnection.COM.stopBits", stopBitsBox->itemText(0)).toString());
    flowControlBox->setCurrentText(applicationSettings->value("RemoteControlConnection.COM.flowControl", flowControlBox->itemText(0)).toString());
    //localEchoCheckBox->setChecked(applicationSettings->value("RemoteControlConnection.COM.localEchoEnabled", localEchoCheckBox->isChecked()).toBool());
    updateSettings();
}

// Saves serial port settings to application settings
// BG: code mostly copied from MCUSettingsDialogInst
void RemoteCCDialog::saveSettings() {
    applicationSettings->setValue("RemoteControlConnection.UDP.ipAddress", udpIpBox->getValue());
    applicationSettings->setValue("RemoteControlConnection.UDP.portNumber", udpPortBox->value());
    
    applicationSettings->setValue("RemoteControlConnection.COM.name", serialPortInfoListBox->currentText());
    applicationSettings->setValue("RemoteControlConnection.COM.baudRate", baudRateBox->currentText().toInt());
    applicationSettings->setValue("RemoteControlConnection.COM.dataBits", dataBitsBox->currentText().toInt());
    applicationSettings->setValue("RemoteControlConnection.COM.parity", parityBox->currentText());
    applicationSettings->setValue("RemoteControlConnection.COM.stopBits", stopBitsBox->currentText().toFloat());
    applicationSettings->setValue("RemoteControlConnection.COM.flowControl", flowControlBox->currentText());
    //applicationSettings->setValue("RemoteControlConnection.COM.localEchoEnabled", localEchoCheckBox->isChecked());
}

/*
bool RemoteCCDialog::isCOMConnected() {
    return serialPort->isOpen();
}
*/

void RemoteCCDialog::updateCOMDevices() {
    serialPortInfoListBox->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        serialPortInfoListBox->addItem(info.portName());
    }

    // NOTE: also add item(s) assigned from our internal COM pool (which can be reused)
    // later NOTE: no need for this, because qt also seems to show the ports that PupilEXT has already opened
//    std::vector<QString> poolPortNames = connPoolCOM->getOpenedNames();
//    for(int i=0; i<poolPortNames.size(); i++) {
//        serialPortInfoListBox->addItem(poolPortNames[i]);
//    }
}

// TODO: something nicer?
void RemoteCCDialog::interpretCommand(const QString &msg, const quint64 &timestamp) {

    MainWindow *w = dynamic_cast<MainWindow*>(mainWindow);

    qDebug() << "Message is the following: \n" << msg;

    if(msg.isEmpty()) {
        qDebug() << "Received an empty datagram for a remote control command. No action performed.";
        return;
    }
    
    if(msg.at(0) == 'T' || msg.at(0) == 't') { // trigger signal for trial stepping, need to happen fast
        // note: qstring cannot use str[0] == 'T'
        w->PRGincrementTrialCounter(timestamp);
        return;
    }

    QString str = SupportFunctions::simplifyReceivedMessage(msg);

    // NOTE: toLower() is used to make identification of e.g. pup.det.algorithms safer

    if(str[0].toLower() == 'm' && str.size()>=3) { // receive arbitrary message to be saved into output data file
        w->PRGlogRemoteMessage(timestamp, str.mid(2, str.length()-2).toLower());
        return;
    }

    if(str[0].toLower() == 'a' && str.size()>=2) { // performing actions just like when interacting with GUI
        if(str[1].toLower() == '1' && str.size()>=4) { // open single camera device
            w->PRGopenSingleCamera(str.mid(3, str.length()-3).toLower());
        } else if(str[1].toLower() == '2' && str.size()>=4) { // open stereo camera device
            QString twoNames = str.mid(3, str.length()-3).toLower();
            QRegExp separator("[,|;]");
            QStringList subStrings = twoNames.split(separator);
            if(subStrings.length() >= 2)
                w->PRGopenStereoCamera(subStrings[0], subStrings[1]);
        } else if(str[1].toLower() == 'w' && str.size()>=4) { // open single webcam
            w->PRGopenSingleWebcam(str.mid(3, str.length()-3).toInt());
        } else if(str[1].toLower() == 't' && str.size()==2) { // start pupil tracking
            w->PRGtrackStart();
        } else if(str[1].toLower() == 'x' && str.size()==2) { // stop pupil tracking
            w->PRGtrackStop();
        } else if(str[1].toLower() == 'r' && str.size()==2) { // start csv recording
            w->PRGrecordStart();
        } else if(str[1].toLower() == 's' && str.size()==2) { // stop csv recording
            w->PRGrecordStop();
        } else if(str[1].toLower() == 'v' && str.size()==2) { // start data streaming
            w->PRGstreamStart();
        } else if(str[1].toLower() == 'c' && str.size()==2) { // stop data streaming
            w->PRGstreamStop();
        } else if(str[1].toLower() == 'm' && str.size()==2) { // start image recording
            w->PRGrecordImageStart();
        } else if(str[1].toLower() == 'a' && str.size()==2) { // stop image recording
            w->PRGrecordImageStop();
        } else if(str[1].toLower() == 'd' && str.size()==2) { // disconnect from camera
            w->PRGcloseCamera();
        } else if(str[1].toLower() == 'f' && str.size()==2) { // force reset trial counter
            w->PRGforceResetTrialCounter(timestamp);
        }
        return;
    } 
    
    if(str[0].toLower() == 'g' && str.size()>=4) { // changing general settings or basic runtime variables
        if(str[1].toLower() == 'p') { // set image output path, no toLower()
            w->PRGsetOutPath(str.mid(3, str.length()-3));
        } else if(str[1].toLower() == 'l') { // set logfile and path name, no toLower()
            w->PRGsetCsvPathAndName(str.mid(3, str.length()-3));
        } else if(str[1].toLower() == 'c') { // set global delimiter character, no toLower()
            w->PRGsetGlobalDelimiter(str.mid(3, str.length()-3));
        } else if(str[1].toLower() == 'i') { // set image output format
            w->PRGsetImageOutputFormat(str.mid(3, str.length()-3).toLower());
        }
        return;
    } 
    
    if(str[0].toLower() == 'p' && str.size()>=4) { // changing pupil detection settings
        if(str[1].toLower() == 'a') { // set pupil detection algorithm
            w->PRGsetPupilDetectionAlgorithm(str.mid(3, str.length()-3).toLower());
        } else if(str[1].toLower() == 'r') { // Use ROI Area Selection
            w->PRGsetPupilDetectionUsingROI(str.mid(3, str.length()-3).toLower());
        } else if(str[1].toLower() == 'o') { // Compute Additional Outline Confidence
            w->PRGsetPupilDetectionCompOutlineConf(str.mid(3, str.length()-3).toLower());
        }
        return;
    }

    if(str[0].toLower() == 'i' && str.size()>=4) { // changing camera-related and Image acquisition settings
        if(str[1].toLower() == 't' && str.size()>=4) { // set image acquisition triggering mode
            if(str[3].toLower() == 'h') { // hardware-based triggering
                w->PRGenableHWT(true);
            } if(str[3].toLower() == 's') { // software-based triggering
                w->PRGenableHWT(false);
            }
        } else if(str[1].toLower() == 'h') { // set hardware-based triggering settings
            if(str[2].toLower() == 'q') { // start image acquisition triggering
                w->PRGstartHWT();
            } else if(str[2].toLower() == 'y') { // stop image acquisition triggering
                w->PRGstopHWT();
            } else if(str[2].toLower() == 'l' && str.size()>=5 && str[4].digitValue() <=4 && str[4].digitValue() >=1) { // set line source
                w->PRGsetHWTlineSource(str[4].digitValue());
            } else if(str[2].toLower() == 'r' && str.size()>=5) {
                bool ok;
                float val = str.mid(4, str.length()-4).toFloat(&ok);
                if(!ok || (ok && val < 0.0f))
                    return;
                w->PRGsetHWTruntime(val);
            } else if(str[2].toLower() == 'f' && str.size()>=5) {
                bool ok;
                int val = str.mid(4, str.length()-4).toInt(&ok);
                if(!ok || (ok && val <= 0))
                    return;
                w->PRGsetHWTframerate(val);
            }
        } else if(str[1].toLower() == 's') { // set software-based triggering settings
            if(str[2].toLower() == 'c' && str.size()>=5 && str[4].digitValue() <=1 && str[4].digitValue() >=0) { // enable sofwtare triggering framerate limiting
                w->PRGenableSWTframerateLimiting(str.mid(4, str.length()-4).toLower());
            } else if(str[2].toLower() == 'f' && str.size()>=5) {
                bool ok;
                int val = str.mid(4, str.length()-4).toInt(&ok);
                if(!ok || (ok && val <= 0))
                    return;
                w->PRGsetSWTframerate(val);
            }
        } else if(str[1].toLower() == 'e' && str.size()>=4) { // set exposure
            bool ok;
            int val = str.mid(3, str.length()-3).toInt(&ok);
            if(!ok || (ok && val <= 0))
                return;
            w->PRGsetExposure(val);
        } else if(str[1].toLower() == 'g' && str.size()>=4) { // set gain
            bool ok;
            int val = str.mid(3, str.length()-3).toDouble(&ok);
            if(!ok || (ok && val <= 0))
                return;
            w->PRGsetGain(val);
        }
        return;
    }

    if(str[0].toLower() == 'c' && str.size()>=6) { // establish connection
        if(str[1].toLower() == 'r') { // for remote control
            if(str[2].toLower() == 'c') {
                if(str.mid(3,3).toLower() == 'udp' && str.size()>=8)
                    w->PRGconnectRemoteUDP(str.mid(7, str.length()-7));
                else if(str.mid(3,3).toLower() == 'com' && str.size()>=8)
                    w->PRGconnectRemoteCOM(str.mid(7, str.length()-7).toUpper());
            } else if(str[2].toLower() == 'd') {
                if(str.mid(3,3).toLower() == 'udp')
                    w->PRGdisconnectRemoteUDP();
                else if(str.mid(3,3).toLower() == 'com')
                    w->PRGdisconnectRemoteCOM();
            }
        } else if(str[1].toLower() == 's') { // for streaming
            if(str[2].toLower() == 'c') {
                if(str.mid(3,3).toLower() == 'udp' && str.size()>=8)
                    w->PRGconnectStreamUDP(str.mid(7, str.length()-7));
                else if(str.mid(3,3).toLower() == 'com' && str.size()>=8)
                    w->PRGconnectStreamCOM(str.mid(7, str.length()-7).toUpper());
            } else if(str[2].toLower() == 'd') {
                if(str.mid(3,3).toLower() == 'udp')
                    w->PRGdisconnectStreamUDP();
                else if(str.mid(3,3).toLower() == 'com')
                    w->PRGdisconnectStreamCOM();
            }
        } else if(str[1].toLower() == 'm') { // for Microcontroller ("camera serial") connection
            if(str[2].toLower() == 'c') {
                if(str.mid(3,3).toLower() == 'udp' && str.size()>=8)
                    w->PRGconnectMicrocontrollerUDP(str.mid(7, str.length()-7));
                else if(str.mid(3,3).toLower() == 'com' && str.size()>=8)
                    w->PRGconnectMicrocontrollerCOM(str.mid(7, str.length()-7).toUpper());
            } else if(str[2].toLower() == 'd') {
                w->PRGdisconnectMicrocontroller();
            }
        }
        return;
    }

    qDebug() << "Could not interpret remote control command. Message is the following: " << msg;
}

