#include "MCUSettingsDialog.h"
#include <QtWidgets>
#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/qmessagebox.h>
#include <iostream>
#include "../SVGIconColorAdjuster.h"

static const char* blankString = "N/A";

// Create the serial settings dialog
// Settings are loaded if existing from the application settings
MCUSettingsDialog::MCUSettingsDialog(ConnPoolCOM *connPoolCOM, ConnPoolUDP *connPoolUDP, QWidget *parent) :
    QDialog(parent),
    m_intValidator(new QIntValidator(0, 4000000, this)),
    connPoolUDP(connPoolUDP),
    connPoolCOM(connPoolCOM),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setMinimumSize(550, 550);
    this->setWindowTitle("Microcontroller Connection Settings");

    createForm();

    baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(applyButton, &QPushButton::clicked, this, &MCUSettingsDialog::apply);
    connect(clearButton, &QPushButton::clicked, textField, &QTextEdit::clear);
    connect(refreshButton, &QPushButton::clicked, this, &MCUSettingsDialog::updateDevices);

    connect(serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MCUSettingsDialog::showPortInfo);
    connect(baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MCUSettingsDialog::checkCustomBaudRatePolicy);
    connect(serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MCUSettingsDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();

    loadSettings();

    // GB: new solution uses connect later, not here
    //connect(serialPort, &QSerialPort::readyRead, this, &MCUSettingsDialogInst::readData);
}

void MCUSettingsDialog::connectCOM(const ConnPoolCOMInstanceSettings &p) {

    int index = connPoolCOM->setupAndOpenConnection(p, ConnPoolPurposeFlag::CAMERA_TRIGGER);
    if(index >= 0) {
        textField->clear();
        textField->append(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                  .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                  .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        connPoolCOMIndex = index;
        connPoolCOM->subscribeListener(index, this, SLOT(readData(QString, quint64)));

        sendCommand("<SX>"); // To surely stop any triggering if it is still going on on the microcontroller side
        setLimitationsWhileConnected(true);
        emit onConnect();
    } else {
        if(connPoolCOM->getInstance(connPoolCOMIndex) != nullptr) {
            QString errMsg = connPoolCOM->getInstance(connPoolCOMIndex)->errorString();
            QMessageBox::critical(this, tr("Error"), connPoolCOM->getInstance(connPoolCOMIndex)->errorString());
        }
        textField->append((tr("Error while connecting to the specified COM port. It is possibly already in use by another application.")));
    }
}

void MCUSettingsDialog::doConnect() {
    updateSettings();
    if(currentConnectionMethod == ConnectionMethod::COM) {
        connectCOM(m_currentSettingsCOM);
    } else {
        connectUDP(m_currentSettingsUDP);
    }
}

void MCUSettingsDialog::doDisconnect() {
    if(currentConnectionMethod == ConnectionMethod::COM) {
        disconnectCOM();
    } else {
        disconnectUDP();
    }
}

// Disconnects the connected serial port
// Sends an onDisconnect signal
void MCUSettingsDialog::disconnectCOM() {

    if(connPoolCOMIndex < 0) {
        qDebug() << "MCUSettingsDialogInst::disconnectCOM(): connPoolCOMIndex value is invalid";
        return;
    }
    if(connPoolCOMIndex >= 0 && connPoolCOM->getInstance(connPoolCOMIndex)->isOpen()) {
        connPoolCOM->unsubscribeListener(connPoolCOMIndex, this, SLOT(readData(QString, quint64)));
        connPoolCOM->closeConnection(connPoolCOMIndex, ConnPoolPurposeFlag::CAMERA_TRIGGER);
        //connPoolCOM->getInstance(connPoolCOMIndex)->close();
        connPoolCOMIndex = -1;
    }
    textField->append(tr("Disconnected."));
    setLimitationsWhileConnected(false);
    emit onDisconnect();
}

void MCUSettingsDialog::connectUDP(const ConnPoolUDPInstanceSettings &p) {

    int index = connPoolUDP->setupAndOpenConnection(p, ConnPoolPurposeFlag::CAMERA_TRIGGER);
    if(index >= 0) {
        textField->clear();
        textField->append(tr("Connected to %1 : %2").arg(p.ipAddress.toString()).arg(p.portNumber));
        connPoolUDPIndex = index;
        connPoolUDP->subscribeListener(index, this, SLOT(readData(QString, quint64)));
        sendCommand("<SX>"); // To surely stop any triggering if it is still going on on the microcontroller side
        setLimitationsWhileConnected(true);
        emit onConnect();
    } else {
        if(connPoolUDP->getInstance(connPoolUDPIndex) != nullptr) {
            QString errMsg = connPoolUDP->getInstance(connPoolUDPIndex)->errorString();
            QMessageBox::critical(this, tr("Error"), connPoolUDP->getInstance(connPoolUDPIndex)->errorString());
        }
        textField->append((tr("Error while connecting to the specified UDP port. It is possibly already in use by another application.")));
    }
}

void MCUSettingsDialog::disconnectUDP() {
    if(connPoolUDPIndex < 0) {
        qDebug() << "MCUSettingsDialogInst::disconnectUDP(): connPoolUDPIndex value is invalid";
        return;
    }
    if(connPoolUDPIndex >= 0 && connPoolUDP->getInstance(connPoolUDPIndex)->isOpen()) {
        connPoolUDP->unsubscribeListener(connPoolUDPIndex, this, SLOT(readData(QString, quint64)));
        connPoolUDP->closeConnection(connPoolUDPIndex, ConnPoolPurposeFlag::CAMERA_TRIGGER);
        //connPoolUDP->getInstance(connPoolUDPIndex)->close();
        connPoolUDPIndex = -1;
    }
    textField->append(tr("Disconnected."));
    setLimitationsWhileConnected(false);
    emit onDisconnect();
}

// Reads data from the serial port line by line
// Displays the data on the textfield
void MCUSettingsDialog::readData(QString msg, quint64 timestamp)
{
    // GB TODO: this is gotten by readAll, not readLine... is it okay?
    textField->append(msg);

    /*if(connPoolCOM->getInstance(connPoolCOMIndex)->canReadLine()) {
        textField->append(QString(connPoolCOM->getInstance(connPoolCOMIndex)->readLine()));
    }*/
}

void MCUSettingsDialog::sendCommand(QString cmd) {
    if(currentConnectionMethod == ConnectionMethod::COM) {
        sendCommandCOM(cmd);
    } else {
        sendCommandUDP(cmd);
    }
}

void MCUSettingsDialog::sendCommandUDP(QString cmd) {

    if(connPoolUDPIndex < 0 || !connPoolUDP->getInstance(connPoolUDPIndex)->isOpen())
        return;
    QByteArray writeData = cmd.toUtf8();
    const qint64 bytesWritten = connPoolUDP->writeToInstance(connPoolUDPIndex, writeData);

    if (bytesWritten == -1) {
        textField->append(QObject::tr("Failed to write the data to port %1 : %2, error: %3")
                                  .arg(connPoolUDP->getInstance(connPoolUDPIndex)->objectName())
                                  .arg(connPoolUDP->getInstance(connPoolUDPIndex)->localPort())
                                  .arg(connPoolUDP->getInstance(connPoolUDPIndex)->errorString()));

    } else if (bytesWritten != writeData.size()) {
        textField->append(QObject::tr("Failed to write the data to port %1 : %2, error: %3")
                                  .arg(connPoolUDP->getInstance(connPoolUDPIndex)->objectName())
                                  .arg(connPoolUDP->getInstance(connPoolUDPIndex)->localPort())
                                  .arg(connPoolUDP->getInstance(connPoolUDPIndex)->errorString()));
    }
}

// Slot that is used to send commands over the connected serial port
// Commands are strings, encoded using utf-8
void MCUSettingsDialog::sendCommandCOM(QString cmd) {

    if(connPoolCOMIndex < 0 || !connPoolCOM->getInstance(connPoolCOMIndex)->isOpen())
        return;
    QByteArray writeData = cmd.toUtf8();
    const qint64 bytesWritten = connPoolCOM->writeToInstance(connPoolCOMIndex, writeData);

    if (bytesWritten == -1) {
        textField->append(QObject::tr("Failed to write the data to port %1, error: %2")
                .arg(connPoolCOM->getInstance(connPoolCOMIndex)->portName())
                .arg(connPoolCOM->getInstance(connPoolCOMIndex)->errorString()));

    } else if (bytesWritten != writeData.size()) {
        textField->append(QObject::tr("Failed to write the data to port %1, error: %2")
                                  .arg(connPoolCOM->getInstance(connPoolCOMIndex)->portName())
                                  .arg(connPoolCOM->getInstance(connPoolCOMIndex)->errorString()));
    }
}

void MCUSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    UDPRadioButton = new QRadioButton("Connect via UDP / Ethernet cable");
    //UDPRadioButton->setChecked();
    connect(UDPRadioButton, SIGNAL(toggled(bool)), this, SLOT(onUDPRadioButtonChecked(bool)));
    mainLayout->addWidget(UDPRadioButton);

    UDPGroup = new QGroupBox();
    QFormLayout *UDPLayout = new QFormLayout();

    QHBoxLayout *UDPRow1Layout = new QHBoxLayout();

    QString udpIp = "192.168.40.200";
    int udpPort = 7000;

    //UDPLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Fixed));

    udpIpLabel = new QLabel(tr("IP address:"));
    udpIpBox = new IPCtrl();
    udpIpBox->setValue(udpIp);

    UDPRow1Layout->addWidget(udpIpLabel);
    UDPRow1Layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed));
    UDPRow1Layout->addWidget(udpIpBox);
    UDPRow1Layout->addSpacerItem(new QSpacerItem(60, 20, QSizePolicy::Fixed));

    udpPortLabel = new QLabel(tr("Port:"));
    udpPortBox = new QSpinBox();
    udpPortBox->setMinimum(1);
    udpPortBox->setMaximum(9999);
    udpPortBox->setSingleStep(1);
    udpPortBox->setValue(udpPort);
    udpPortBox->setFixedWidth(70);

    UDPRow1Layout->addWidget(udpPortLabel);
    UDPRow1Layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed));
    UDPRow1Layout->addWidget(udpPortBox);
    UDPRow1Layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));

    UDPLayout->addRow(UDPRow1Layout);

    QLabel *udpNoteLabel = new QLabel(tr("The IP address specified here will be used for sending to, while incoming data is filtered for it.\nThe port is where commands are sent to, while it is also the same port used for listening to the MCU."));
    UDPLayout->addRow(udpNoteLabel);

    UDPGroup->setLayout(UDPLayout);
    mainLayout->addWidget(UDPGroup);


    COMRadioButton = new QRadioButton("Connect via COM / Serial cable (or USB for emulated serial)");
    //COMRadioButton->setChecked();
    connect(COMRadioButton, SIGNAL(toggled(bool)), this, SLOT(onCOMRadioButtonChecked(bool)));
    mainLayout->addWidget(COMRadioButton);

    COMGroup = new QGroupBox();
    QGridLayout *serialLayout = new QGridLayout(this);

    serialInfoFrame = new QFrame();
    QGridLayout *serialInfoLayout = new QGridLayout;

    QHBoxLayout *serialPortInfoListLayout = new QHBoxLayout;

    serialPortInfoListBox = new QComboBox();
    serialPortInfoListBox->setFixedWidth(70);

    serialPortInfoListLayout->addWidget(serialPortInfoListBox);

    const QIcon refreshIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/view-refresh.svg"), applicationSettings);
    refreshButton = new QPushButton(""); // view-refresh.svg
    refreshButton->setIcon(refreshIcon);
    refreshButton->setFixedWidth(32);

    serialPortInfoListLayout->addWidget(refreshButton);

    serialInfoLayout->addLayout(serialPortInfoListLayout, 0, 0);

    descriptionLabel  = new QLabel();
    locationLabel  = new QLabel();
    manufacturerLabel  = new QLabel();
    pidLabel  = new QLabel();
    serialNumberLabel  = new QLabel();
    vidLabel = new QLabel();

    serialInfoLayout->addWidget(descriptionLabel);
    serialInfoLayout->addWidget(locationLabel);
    serialInfoLayout->addWidget(manufacturerLabel);
    serialInfoLayout->addWidget(pidLabel);
    serialInfoLayout->addWidget(serialNumberLabel);
    serialInfoLayout->addWidget(vidLabel);

    serialInfoFrame->setLayout(serialInfoLayout);
    serialLayout->addWidget(serialInfoFrame, 0, 0);

    paramFrame = new QFrame();
    QFormLayout *paramLayout = new QFormLayout;

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

    baudRateBox->setFixedWidth(70);
    dataBitsBox->setFixedWidth(70);
    flowControlBox->setFixedWidth(70);
    parityBox->setFixedWidth(70);
    stopBitsBox->setFixedWidth(70);

    paramLayout->addRow(baudRateLabel, baudRateBox);
    paramLayout->addRow(dataBitsLabel, dataBitsBox);
    paramLayout->addRow(parityLabel, parityBox);
    paramLayout->addRow(stopBitsLabel, stopBitsBox);
    paramLayout->addRow(flowControlLabel, flowControlBox);

    localEchoCheckBox = new QCheckBox("Local echo");
    paramLayout->addWidget(localEchoCheckBox);

    paramFrame->setLayout(paramLayout);
    serialLayout->addWidget(paramFrame, 0, 1);

    COMGroup->setLayout(serialLayout);
    mainLayout->addWidget(COMGroup);


    textField = new QTextEdit();
    textField->setText("Not connected.");
    textField->setMinimumHeight(70);
    textField->setTextInteractionFlags(Qt::NoTextInteraction);
    mainLayout->addWidget(textField);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    clearButton = new QPushButton(tr("Clear Output"));
    applyButton = new QPushButton(tr("Apply and Close"));

    buttonsLayout->addWidget(clearButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    buttonsLayout->addWidget(applyButton);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    this->resize(this->minimumSize());
}

MCUSettingsDialog::~MCUSettingsDialog() {
    disconnectCOM();
}

//ConnPoolCOMInstanceSettings MCUSettingsDialog::getCOMsettings() const {
//    return m_currentSettingsCOM;
//}

// Updates the form text with information on the serial port
void MCUSettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    const QStringList list = serialPortInfoListBox->itemData(idx).toStringList();
    descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void MCUSettingsDialog::apply()
{
    updateSettings();
    close();
}

void MCUSettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !baudRateBox->itemData(idx).isValid();
    baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        baudRateBox->clearEditText();
        QLineEdit *edit = baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void MCUSettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !serialPortInfoListBox->itemData(idx).isValid();
    serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        serialPortInfoListBox->clearEditText();
}

void MCUSettingsDialog::fillPortsParameters()
{
    baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    baudRateBox->addItem(tr("Custom"));
    baudRateBox->setCurrentIndex(SERIAL_DEF_BAUDRATE);

    dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    dataBitsBox->setCurrentIndex(SERIAL_DEF_DATABITS);

    parityBox->addItem(tr("None"), QSerialPort::NoParity);
    parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);
    parityBox->setCurrentIndex(SERIAL_DEF_PARITY);

    stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    stopBitsBox->setCurrentIndex(SERIAL_DEF_STOPBITS);

    flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    flowControlBox->setCurrentIndex(SERIAL_DEF_FLOWCONTROL);
}

void MCUSettingsDialog::fillPortsInfo()
{
    serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        serialPortInfoListBox->addItem(list.first(), list);
    }

    serialPortInfoListBox->addItem(tr("Custom"));

    // NOTE: also add item(s) assigned from our internal COM pool (which can be reused)
    // later NOTE: no need for this, because qt also seems to show the ports that PupilEXT has already opened
//    std::vector<QString> poolPortNames = connPoolCOM->getOpenedNames();
//    for(int i=0; i<poolPortNames.size(); i++) {
//        serialPortInfoListBox->addItem(poolPortNames[i]);
//    }
}


// Update current settings with the configuration from the form
void MCUSettingsDialog::updateSettings() {

    m_currentSettingsUDP.ipAddress = QHostAddress(udpIpBox->getValue());

    m_currentSettingsUDP.portNumber = udpPortBox->value();

    //

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

    m_currentSettingsCOM.localEchoEnabled = localEchoCheckBox->isChecked();

    saveSettings();
}

// Loads the serial port settings from application settings
void MCUSettingsDialog::loadSettings() {

    currentConnectionMethod = (ConnectionMethod)applicationSettings->value("MCUConnection.ConnectionMethod", QString::number((int)ConnectionMethod::COM)).toInt();
    if(currentConnectionMethod == ConnectionMethod::COM) {
        COMRadioButton->setChecked(true);
        UDPRadioButton->setChecked(false);
    } else {
        COMRadioButton->setChecked(false);
        UDPRadioButton->setChecked(true);
    }

    udpIpBox->setValue(applicationSettings->value("MCUConnection.UDP.ipAddress", udpIpBox->getValue()).toString());
    udpPortBox->setValue(applicationSettings->value("MCUConnection.UDP.portNumber", udpPortBox->value()).toInt());

    serialPortInfoListBox->setCurrentText(applicationSettings->value("MCUConnection.COM.name", serialPortInfoListBox->itemText(0)).toString());
    baudRateBox->setCurrentText(applicationSettings->value("MCUConnection.COM.baudRate", baudRateBox->itemText(SERIAL_DEF_BAUDRATE)).toString());
    dataBitsBox->setCurrentText(applicationSettings->value("MCUConnection.COM.dataBits", dataBitsBox->itemText(SERIAL_DEF_DATABITS)).toString());
    parityBox->setCurrentText(applicationSettings->value("MCUConnection.COM.parity", parityBox->itemText(SERIAL_DEF_PARITY)).toString());
    stopBitsBox->setCurrentText(applicationSettings->value("MCUConnection.COM.stopBits", stopBitsBox->itemText(SERIAL_DEF_STOPBITS)).toString());
    flowControlBox->setCurrentText(applicationSettings->value("MCUConnection.COM.flowControl", flowControlBox->itemText(SERIAL_DEF_FLOWCONTROL)).toString());
    localEchoCheckBox->setChecked(applicationSettings->value("MCUConnection.COM.localEchoEnabled", localEchoCheckBox->isChecked()).toBool());
    updateSettings();
}

// Saves serial port settings to application settings
void MCUSettingsDialog::saveSettings() {

    applicationSettings->setValue("MCUConnection.ConnectionMethod", (int)currentConnectionMethod);

    applicationSettings->setValue("MCUConnection.UDP.ipAddress", udpIpBox->getValue());
    applicationSettings->setValue("MCUConnection.UDP.portNumber", udpPortBox->value());

    applicationSettings->setValue("MCUConnection.COM.name", serialPortInfoListBox->currentText());
    applicationSettings->setValue("MCUConnection.COM.baudRate", baudRateBox->currentText().toInt());
    applicationSettings->setValue("MCUConnection.COM.dataBits", dataBitsBox->currentText().toInt());
    applicationSettings->setValue("MCUConnection.COM.parity", parityBox->currentText());
    applicationSettings->setValue("MCUConnection.COM.stopBits", stopBitsBox->currentText().toFloat());
    applicationSettings->setValue("MCUConnection.COM.flowControl", flowControlBox->currentText());
    applicationSettings->setValue("MCUConnection.COM.localEchoEnabled", localEchoCheckBox->isChecked());
}

bool MCUSettingsDialog::isCOMConnected() {
    if(connPoolCOMIndex >= 0)
        return connPoolCOM->getInstance(connPoolCOMIndex)->isOpen();
    
    return false;
}

bool MCUSettingsDialog::isUDPConnected() {
    if(connPoolUDPIndex >= 0)
        return connPoolUDP->getInstance(connPoolUDPIndex)->isOpen();

    return false;
}

bool MCUSettingsDialog::isConnected() {

    return (isCOMConnected() || isUDPConnected());
}

void MCUSettingsDialog::updateDevices() {
    fillPortsInfo();
}

void MCUSettingsDialog::setLimitationsWhileConnected(bool state) {
//    paramFrame->setDisabled(state);
//    serialInfoFrame->setDisabled(state);
    COMGroup->setEnabled(!state && (currentConnectionMethod == ConnectionMethod::COM));
    UDPGroup->setEnabled(!state && (currentConnectionMethod == ConnectionMethod::UDP));
}

void MCUSettingsDialog::onUDPRadioButtonChecked(bool state) {
    if(state) {
        currentConnectionMethod = ConnectionMethod::UDP;
        COMGroup->setEnabled(false);
        UDPGroup->setEnabled(true);
    } else {
        currentConnectionMethod = ConnectionMethod::COM;
        COMGroup->setEnabled(true);
        UDPGroup->setEnabled(false);
    }
}

void MCUSettingsDialog::onCOMRadioButtonChecked(bool state) {
    if(state) {
        currentConnectionMethod = ConnectionMethod::COM;
        COMGroup->setEnabled(true);
        UDPGroup->setEnabled(false);
    } else {
        currentConnectionMethod = ConnectionMethod::UDP;
        COMGroup->setEnabled(false);
        UDPGroup->setEnabled(true);
    }
}

// For programmatically setting the currentConnectionMethod and corresponding GUI
void MCUSettingsDialog::selectConnectionMethod(ConnectionMethod connectionMethod) {
    if(connectionMethod == ConnectionMethod::COM) {
        onCOMRadioButtonChecked(true);
    } else {
        onUDPRadioButtonChecked(true);
    }
};

