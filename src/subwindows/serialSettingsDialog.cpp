#include "serialSettingsDialog.h"

#include <QtWidgets>
#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qmessagebox.h>
#include <iostream>

static const char* blankString = "N/A";

// Create the serial settings dialog
// Settings are loaded if existing from the application settings
SerialSettingsDialog::SerialSettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_intValidator(new QIntValidator(0, 4000000, this)),
    serialPort(new QSerialPort(this)),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setMinimumSize(600, 330);
    this->setWindowTitle("Serial Port Settings");

    createForm();

    baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(applyButton, &QPushButton::clicked, this, &SerialSettingsDialog::apply);
    connect(cancelButton, &QPushButton::clicked, this, &SerialSettingsDialog::cancel);
    connect(clearButton, &QPushButton::clicked, textField, &QTextEdit::clear);
    connect(refreshButton, &QPushButton::clicked, this, &SerialSettingsDialog::updateDevices);

    connect(connectButton, &QPushButton::clicked, this, &SerialSettingsDialog::connectSerialPort);
    connect(disconnectButton, &QPushButton::clicked, this, &SerialSettingsDialog::disconnectSerialPort);

    connect(serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialSettingsDialog::showPortInfo);
    connect(baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialSettingsDialog::checkCustomBaudRatePolicy);
    connect(serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialSettingsDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();

    loadSettings();

    connect(serialPort, &QSerialPort::readyRead, this, &SerialSettingsDialog::readData);
}

// Connects the serial port with the configured settings, if successful, displays a connected message to the textfield
// Sends an onConnect signal on success
void SerialSettingsDialog::connectSerialPort() {

    updateSettings();

    const SerialSettingsDialog::Settings p = m_currentSettings;
    serialPort->setPortName(p.name);
    serialPort->setBaudRate(p.baudRate);
    serialPort->setDataBits(p.dataBits);
    serialPort->setParity(p.parity);
    serialPort->setStopBits(p.stopBits);
    serialPort->setFlowControl(p.flowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        textField->clear();

        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        //m_ui->actionConfigure->setEnabled(false);
        textField->append(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                  .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                  .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        emit onConnect();
    } else {
        QMessageBox::critical(this, tr("Error"), serialPort->errorString());

        textField->append((tr("Error while connecting to the serial port.")));
    }
}

// Disconnects the connected serial port
// Sends an onDisconnect signal
void SerialSettingsDialog::disconnectSerialPort() {

    if (serialPort->isOpen())
        serialPort->close();

    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    textField->append(tr("Disconnected."));

    emit onDisconnect();
}

// Reads data from the serial port line by line
// Displays the data on the textfield
void SerialSettingsDialog::readData()
{
    if(serialPort->canReadLine()) {
        textField->append(QString(serialPort->readLine()));
    }
}

// Slot that is used to send commands over the connected serial port
// Commands are strings, encoded using utf-8
void SerialSettingsDialog::sendCommand(QString cmd) {

    if(!serialPort->isOpen())
        return;

    QByteArray writeData = cmd.toUtf8();

    const qint64 bytesWritten = serialPort->write(writeData);

    if (bytesWritten == -1) {
        textField->append(QObject::tr("Failed to write the data to port %1, error: %2")
                .arg(serialPort->portName())
                .arg(serialPort->errorString()));

    } else if (bytesWritten != writeData.size()) {
        textField->append(QObject::tr("Failed to write the data to port %1, error: %2")
                                  .arg(serialPort->portName())
                                  .arg(serialPort->errorString()));
    }
}

void SerialSettingsDialog::createForm() {

    QGridLayout *mainLayout = new QGridLayout(this);

    QGroupBox *paramGroup = new QGroupBox("Parameters");
    QGridLayout *paramLayout = new QGridLayout;

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

    paramLayout->addWidget(baudRateLabel);
    paramLayout->addWidget(baudRateBox);
    paramLayout->addWidget(dataBitsLabel);
    paramLayout->addWidget(dataBitsBox);
    paramLayout->addWidget(parityLabel);
    paramLayout->addWidget(parityBox);
    paramLayout->addWidget(stopBitsLabel);
    paramLayout->addWidget(stopBitsBox);
    paramLayout->addWidget(flowControlLabel);
    paramLayout->addWidget(flowControlBox);

    paramGroup->setLayout(paramLayout);
    mainLayout->addWidget(paramGroup, 0, 1);

    QGroupBox *serialPortGroup = new QGroupBox("Serial Port");
    QGridLayout *serialPortLayout = new QGridLayout;

    QHBoxLayout *serialPortInfoListLayout = new QHBoxLayout;

    serialPortInfoListBox = new QComboBox();

    serialPortInfoListLayout->addWidget(serialPortInfoListBox);

    const QIcon refreshIcon = QIcon(":/icons/Breeze/actions/22/view-refresh.svg");
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

    connectButton = new QPushButton(tr("Connect"));
    disconnectButton = new QPushButton(tr("Disconnect"));
    applyButton = new QPushButton(tr("Apply and Close"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout->addWidget(connectButton);
    buttonsLayout->addWidget(disconnectButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout, 4, 0, 1, 2);

    setLayout(mainLayout);
}

SerialSettingsDialog::~SerialSettingsDialog() {
    disconnectSerialPort();
}

SerialSettingsDialog::Settings SerialSettingsDialog::settings() const
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

void SerialSettingsDialog::cancel() {
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

    baudRateBox->setCurrentText(applicationSettings->value("SerialSettings.baudRate", baudRateBox->itemText(0)).toString());

    dataBitsBox->setCurrentText(applicationSettings->value("SerialSettings.dataBits", dataBitsBox->itemText(0)).toString());

    parityBox->setCurrentText(applicationSettings->value("SerialSettings.parity", parityBox->itemText(0)).toString());

    stopBitsBox->setCurrentText(applicationSettings->value("SerialSettings.stopBits", stopBitsBox->itemText(0)).toString());

    flowControlBox->setCurrentText(applicationSettings->value("SerialSettings.flowControl", flowControlBox->itemText(0)).toString());

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
    return serialPort->isOpen();
}

void SerialSettingsDialog::updateDevices() {
    fillPortsInfo();
}

