#include "serialSettingsDialog.h"

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
SerialSettingsDialog::SerialSettingsDialog(ConnPoolCOM *connPoolCOM, QWidget *parent) :
    QDialog(parent),
    m_intValidator(new QIntValidator(0, 4000000, this)),
    connPoolCOM(connPoolCOM),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setMinimumSize(520, 380);
    this->setWindowTitle("Camera Serial Connection Settings");

    createForm();

    baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(applyButton, &QPushButton::clicked, this, &SerialSettingsDialog::apply);
    connect(clearButton, &QPushButton::clicked, textField, &QTextEdit::clear);
    connect(refreshButton, &QPushButton::clicked, this, &SerialSettingsDialog::updateDevices);

    connect(serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialSettingsDialog::showPortInfo);
    connect(baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialSettingsDialog::checkCustomBaudRatePolicy);
    connect(serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialSettingsDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();

    loadSettings();

    // GB: new solution uses connect later, not here
    //connect(serialPort, &QSerialPort::readyRead, this, &SerialSettingsDialog::readData);
}

// Connects the serial port with the configured settings, if successful, displays a connected message to the textfield
// Sends an onConnect signal on success
void SerialSettingsDialog::connectSerialPort() {

    updateSettings();

    const ConnPoolCOMInstanceSettings p = m_currentSettings;
    
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
        textField->append((tr("Error while connecting to the serial port.")));
    }
}

// Disconnects the connected serial port
// Sends an onDisconnect signal
void SerialSettingsDialog::disconnectSerialPort() {

    if(connPoolCOMIndex < 0) {
        qDebug() << "SerialSettingsDialog::disconnectSerialPort(): connPoolCOMIndex value is invalid";
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

// Reads data from the serial port line by line
// Displays the data on the textfield
void SerialSettingsDialog::readData(QString msg, quint64 timestamp)
{
    // GB TODO: this is gotten by readAll, not readLine... is it okay?
    textField->append(msg);

    /*if(connPoolCOM->getInstance(connPoolCOMIndex)->canReadLine()) {
        textField->append(QString(connPoolCOM->getInstance(connPoolCOMIndex)->readLine()));
    }*/
}

// Slot that is used to send commands over the connected serial port
// Commands are strings, encoded using utf-8
void SerialSettingsDialog::sendCommand(QString cmd) {

    //if(!serialPort->isOpen())
    if(connPoolCOMIndex < 0 || !connPoolCOM->getInstance(connPoolCOMIndex)->isOpen())
        return;
    QByteArray writeData = cmd.toUtf8();
    const qint64 bytesWritten = connPoolCOM->getInstance(connPoolCOMIndex)->write(writeData);
    //const qint64 bytesWritten = serialPort->write(writeData);

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

void SerialSettingsDialog::createForm() {

    QGridLayout *mainLayout = new QGridLayout(this);

    paramGroup = new QGroupBox("Parameters");
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

    paramGroup->setLayout(paramLayout);
    mainLayout->addWidget(paramGroup, 0, 1);

    serialPortGroup = new QGroupBox("Serial Port");
    QGridLayout *serialPortLayout = new QGridLayout;

    QHBoxLayout *serialPortInfoListLayout = new QHBoxLayout;

    serialPortInfoListBox = new QComboBox();
    serialPortInfoListBox->setFixedWidth(70);

    serialPortInfoListLayout->addWidget(serialPortInfoListBox);

    const QIcon refreshIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/view-refresh.svg"), applicationSettings);
    refreshButton = new QPushButton(""); // view-refresh.svg
    refreshButton->setIcon(refreshIcon);
    refreshButton->setFixedWidth(22);

    serialPortInfoListLayout->addWidget(refreshButton);

    serialPortLayout->addLayout(serialPortInfoListLayout, 0, 0);

    descriptionLabel  = new QLabel();
    locationLabel  = new QLabel();
    manufacturerLabel  = new QLabel();
    pidLabel  = new QLabel();
    serialNumberLabel  = new QLabel();
    vidLabel = new QLabel();

    serialPortLayout->addWidget(descriptionLabel);
    serialPortLayout->addWidget(locationLabel);
    serialPortLayout->addWidget(manufacturerLabel);
    serialPortLayout->addWidget(pidLabel);
    serialPortLayout->addWidget(serialNumberLabel);
    serialPortLayout->addWidget(vidLabel);

    serialPortGroup->setLayout(serialPortLayout);
    mainLayout->addWidget(serialPortGroup, 0, 0);

    QGroupBox *optionsGroup = new QGroupBox("Options");
    QHBoxLayout *optionsLayout = new QHBoxLayout();

    localEchoCheckBox = new QCheckBox("Local echo");
    optionsLayout->addWidget(localEchoCheckBox);

    optionsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    clearButton = new QPushButton(tr("Clear Output"));
    optionsLayout->addWidget(clearButton);

    optionsGroup->setLayout(optionsLayout);
    mainLayout->addWidget(optionsGroup, 1, 0, 1, 2);

    textField = new QTextEdit();
    textField->setText("Not connected.");
    textField->setTextInteractionFlags(Qt::NoTextInteraction);
    mainLayout->addWidget(textField, 2, 0, 2, 2);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    applyButton = new QPushButton(tr("Apply and Close"));

    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    buttonsLayout->addWidget(applyButton);

    mainLayout->addLayout(buttonsLayout, 4, 0, 1, 2);

    setLayout(mainLayout);

    this->resize(600, 370);
}

SerialSettingsDialog::~SerialSettingsDialog() {
    disconnectSerialPort();
}

ConnPoolCOMInstanceSettings SerialSettingsDialog::settings() const
{
    return m_currentSettings;
}

// Updates the form text with information on the serial port
void SerialSettingsDialog::showPortInfo(int idx)
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

void SerialSettingsDialog::apply()
{
    updateSettings();
    close();
}

void SerialSettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !baudRateBox->itemData(idx).isValid();
    baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        baudRateBox->clearEditText();
        QLineEdit *edit = baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void SerialSettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !serialPortInfoListBox->itemData(idx).isValid();
    serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        serialPortInfoListBox->clearEditText();
}

void SerialSettingsDialog::fillPortsParameters()
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

void SerialSettingsDialog::fillPortsInfo()
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

    // GB added begin
    // NOTE: also add item(s) assigned from our internal COM pool (which can be reused)
    // later NOTE: no need for this, because qt also seems to show the ports that PupilEXT has already opened
//    std::vector<QString> poolPortNames = connPoolCOM->getOpenedNames();
//    for(int i=0; i<poolPortNames.size(); i++) {
//        serialPortInfoListBox->addItem(poolPortNames[i]);
//    }
    // GB added end
}


// Update current settings with the configuration from the form
void SerialSettingsDialog::updateSettings()
{
    m_currentSettings.name = serialPortInfoListBox->currentText();

    if (baudRateBox->currentIndex() == 4) {
        m_currentSettings.baudRate = baudRateBox->currentText().toInt();
    } else {
        m_currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    baudRateBox->itemData(baudRateBox->currentIndex()).toInt());
    }
    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);

    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                dataBitsBox->itemData(dataBitsBox->currentIndex()).toInt());
    m_currentSettings.stringDataBits = dataBitsBox->currentText();

    m_currentSettings.parity = static_cast<QSerialPort::Parity>(
                parityBox->itemData(parityBox->currentIndex()).toInt());
    m_currentSettings.stringParity = parityBox->currentText();

    m_currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                stopBitsBox->itemData(stopBitsBox->currentIndex()).toInt());
    m_currentSettings.stringStopBits = stopBitsBox->currentText();

    m_currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                flowControlBox->itemData(flowControlBox->currentIndex()).toInt());
    m_currentSettings.stringFlowControl = flowControlBox->currentText();

    m_currentSettings.localEchoEnabled = localEchoCheckBox->isChecked();

    saveSettings();
}

// Loads the serial port settings from application settings
void SerialSettingsDialog::loadSettings() {
    serialPortInfoListBox->setCurrentText(applicationSettings->value("SerialSettings.name", serialPortInfoListBox->itemText(0)).toString());
    baudRateBox->setCurrentText(applicationSettings->value("SerialSettings.baudRate", baudRateBox->itemText(SERIAL_DEF_BAUDRATE)).toString());
    dataBitsBox->setCurrentText(applicationSettings->value("SerialSettings.dataBits", dataBitsBox->itemText(SERIAL_DEF_DATABITS)).toString());
    parityBox->setCurrentText(applicationSettings->value("SerialSettings.parity", parityBox->itemText(SERIAL_DEF_PARITY)).toString());
    stopBitsBox->setCurrentText(applicationSettings->value("SerialSettings.stopBits", stopBitsBox->itemText(SERIAL_DEF_STOPBITS)).toString());
    flowControlBox->setCurrentText(applicationSettings->value("SerialSettings.flowControl", flowControlBox->itemText(SERIAL_DEF_FLOWCONTROL)).toString());
    localEchoCheckBox->setChecked(applicationSettings->value("SerialSettings.localEchoEnabled", localEchoCheckBox->isChecked()).toBool());
    updateSettings();
}

// Saves serial port settings to application settings
void SerialSettingsDialog::saveSettings() {
    applicationSettings->setValue("SerialSettings.name", serialPortInfoListBox->currentText());
    applicationSettings->setValue("SerialSettings.baudRate", baudRateBox->currentText().toInt());
    applicationSettings->setValue("SerialSettings.dataBits", dataBitsBox->currentText().toInt());
    applicationSettings->setValue("SerialSettings.parity", parityBox->currentText());
    applicationSettings->setValue("SerialSettings.stopBits", stopBitsBox->currentText().toFloat());
    applicationSettings->setValue("SerialSettings.flowControl", flowControlBox->currentText());
    applicationSettings->setValue("SerialSettings.localEchoEnabled", localEchoCheckBox->isChecked());
}

bool SerialSettingsDialog::isConnected() {
    if(connPoolCOMIndex >= 0)
        return connPoolCOM->getInstance(connPoolCOMIndex)->isOpen();
    
    return false;
}

void SerialSettingsDialog::updateDevices() {
    fillPortsInfo();
}

void SerialSettingsDialog::setLimitationsWhileConnected(bool state) {
    paramGroup->setDisabled(state);
    serialPortGroup->setDisabled(state);
}

