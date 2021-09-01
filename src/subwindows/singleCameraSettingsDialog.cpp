
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include "singleCameraSettingsDialog.h"

#include <iostream>
#include <exception>
#include <stdexcept>

// Create a new camera settings dialog window
// Takes the camera which is configured, and the serial settings dialog for the hardware trigger handling
// Loads camera settings if available from the application settings
SingleCameraSettingsDialog::SingleCameraSettingsDialog(SingleCamera *camera, SerialSettingsDialog *serialSettings, QWidget *parent) :
        QDialog(parent), serialSettings(serialSettings),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    singleCamera = camera;

    settingsDirectory = QDir(applicationSettings->value("SingleCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    setMinimumSize(600, 450);

    setWindowTitle(QString("[%1] Camera Settings").arg(singleCamera->getFriendlyName()));

    createForm();

    connect(gainInputBox, SIGNAL(valueChanged(double)), singleCamera, SLOT(setGainValue(double)));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), singleCamera, SLOT(setExposureTimeValue(int)));
    connect(framerateEnabled, SIGNAL(toggled(bool)), singleCamera, SLOT(enableAcquisitionFrameRate(bool)));
    connect(framerateInputBox, SIGNAL(valueChanged(int)), singleCamera, SLOT(setAcquisitionFPSValue(int)));

    connect(saveButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::saveButtonClick);
    connect(loadButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::loadButtonClick);

    connect(startHWButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::startHardwareTrigger);
    connect(stopHWButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::stopHardwareTrigger);


    connect(gainAutoOnceButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::autoGainOnce);
    connect(exposureAutoOnceButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::autoExposureOnce);

    connect(serialConfigButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::onSerialConfig);

    loadSettings();
    updateForms();
}

void SingleCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *hwTriggerGroup = new QGroupBox("Hardware Trigger");
    QFormLayout *hwTriggerGroupLayout = new QFormLayout();

    hwTriggerEnabled = new QCheckBox("Enable Hardware Trigger");
    hwTriggerEnabled->setChecked(singleCamera->isHardwareTriggerEnabled());
    hwTriggerEnabled->setEnabled(false);

    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), singleCamera, SLOT(enableHardwareTrigger(bool)));
    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), this, SLOT(onHardwareTriggerCheckbox(bool)));

    lineSourceBox = new QComboBox();

    lineSourceBox->addItem(QString("Select Line Source"));
    for(int i=1; i<7;i++) {
        lineSourceBox->addItem(QString("Line") + QString::number(i));
    }
    //lineSourceBox->setCurrentText(QString::fromStdString(camera->getLineSource().c_str()));
    //lineSourceBox->setEnabled(false);
    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), lineSourceBox, SLOT(setEnabled(bool)));
    connect(lineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));

    hwTriggerGroupLayout->addRow(hwTriggerEnabled, lineSourceBox);

    serialConfigButton = new QPushButton("Serial Connection");

    hwTriggerGroupLayout->addRow("", serialConfigButton);

    QLabel *triggerFramerateLabel = new QLabel(tr("Trigger FPS:"));
    triggerFramerateInputBox = new QSpinBox();
    triggerFramerateInputBox->setMinimum(std::max(1, singleCamera->getAcquisitionFPSMin()));
    triggerFramerateInputBox->setMaximum(singleCamera->getAcquisitionFPSMax());
    triggerFramerateInputBox->setSingleStep(1);
    triggerFramerateInputBox->setValue(singleCamera->getAcquisitionFPSValue());
    triggerFramerateInputBox->setEnabled(false);

    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), triggerFramerateInputBox, SLOT(setEnabled(bool)));

    hwTriggerGroupLayout->addRow(triggerFramerateLabel, triggerFramerateInputBox);

    QLabel *triggerTimeSpanLabel = new QLabel(tr("Trigger Runtime [min]:"));
    triggerTimeSpanInputBox = new QDoubleSpinBox();
    triggerTimeSpanInputBox->setMinimum(0);
    triggerTimeSpanInputBox->setMaximum(std::numeric_limits<double>::max());
    triggerTimeSpanInputBox->setSingleStep(0.1);
    triggerTimeSpanInputBox->setValue(1);
    triggerTimeSpanInputBox->setEnabled(false);

    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), triggerTimeSpanInputBox, SLOT(setEnabled(bool)));

    hwTriggerGroupLayout->addRow(triggerTimeSpanLabel, triggerTimeSpanInputBox);


    startHWButton = new QPushButton("Start");
    stopHWButton = new QPushButton("Stop");
    bool serialConnected = serialSettings->isConnected();
    startHWButton->setEnabled(serialConnected);
    stopHWButton->setEnabled(serialConnected);

    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), startHWButton, SLOT(setEnabled(bool)));
    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), stopHWButton, SLOT(setEnabled(bool)));

    hwTriggerGroupLayout->addRow(startHWButton, stopHWButton);

    hwTriggerGroup->setLayout(hwTriggerGroupLayout);
    mainLayout->addWidget(hwTriggerGroup);


    QGroupBox *analogGroup = new QGroupBox("Analog Control");
    QFormLayout *analogLayout = new QFormLayout();
    QHBoxLayout *gainInputLayout = new QHBoxLayout();

    QLabel *gainLabel = new QLabel(tr("Gain [dB]:"));
    gainLabel->setMinimumWidth(50);

    gainInputBox = new QDoubleSpinBox();
    gainInputBox->setMinimum(singleCamera->getGainMin());
    gainInputBox->setMaximum(singleCamera->getGainMax());
    gainInputBox->setSingleStep(0.01);
    gainInputBox->setValue(singleCamera->getGainValue());

    gainInputLayout->addWidget(gainInputBox);
    analogLayout->addRow(gainLabel, gainInputLayout);

    gainAutoOnceButton = new QPushButton("Auto Gain (Once)");
    analogLayout->addRow(gainAutoOnceButton);

    analogGroup->setLayout(analogLayout);
    mainLayout->addWidget(analogGroup);


    QGroupBox *acquisitionGroup = new QGroupBox("Acquisition Control");
    QFormLayout *acquisitionLayout = new QFormLayout;
    QHBoxLayout *exposureInputLayout = new QHBoxLayout;
    QLabel *exposureLabel = new QLabel(tr("Exposure [us]:"));
    exposureLabel->setMinimumWidth(50);
    exposureInputBox = new QSpinBox();
    exposureInputBox->setMinimum(singleCamera->getExposureTimeMin());
    exposureInputBox->setMaximum(singleCamera->getExposureTimeMax());
    exposureInputBox->setSingleStep(1);
    exposureInputBox->setValue(singleCamera->getExposureTimeValue());

    exposureInputLayout->addWidget(exposureInputBox);
    acquisitionLayout->addRow(exposureLabel, exposureInputLayout);

    exposureAutoOnceButton = new QPushButton("Auto Exposure (Once)");
    acquisitionLayout->addRow(exposureAutoOnceButton);

    QLabel *frameRateLabel = new QLabel("Resulting Framerate:");
    frameRateValueLabel = new QLabel(QString::number(singleCamera->getResultingFrameRateValue()));

    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));
    connect(gainInputBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));

    acquisitionLayout->addRow(frameRateLabel, frameRateValueLabel);

    framerateEnabled = new QCheckBox("Limit acquisition framerate");
    framerateEnabled->setChecked(singleCamera->isEnabledAcquisitionFrameRate());

    acquisitionLayout->addRow(framerateEnabled);

    QHBoxLayout *framerateInputLayout = new QHBoxLayout;
    QLabel *framerateLabel = new QLabel(tr("Framerate:"));
    framerateLabel->setMinimumWidth(50);
    framerateInputBox = new QSpinBox();
    QSpacerItem *sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    framerateInputLayout->addWidget(framerateInputBox);
    framerateInputLayout->addSpacerItem(sp);
    framerateInputBox->setMinimum(std::max(1, singleCamera->getAcquisitionFPSMin()));
    framerateInputBox->setMaximum(singleCamera->getAcquisitionFPSMax());
    framerateInputBox->setSingleStep(1);
    framerateInputBox->setValue(singleCamera->getAcquisitionFPSValue());
    framerateInputBox->setEnabled(singleCamera->isEnabledAcquisitionFrameRate());

    acquisitionLayout->addRow(framerateLabel, framerateInputLayout);

    connect(framerateEnabled, SIGNAL(toggled(bool)), framerateInputBox, SLOT(setEnabled(bool)));

    acquisitionGroup->setLayout(acquisitionLayout);
    mainLayout->addWidget(acquisitionGroup);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    saveButton = new QPushButton(tr("Save to File"));
    loadButton = new QPushButton(tr("Load from File"));

    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addWidget(loadButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
}

void SingleCameraSettingsDialog::updateForms() {

    if(singleCamera->isOpen()) {

        gainInputBox->setMinimum(singleCamera->getGainMin());
        gainInputBox->setMaximum(singleCamera->getGainMax());
        gainInputBox->setValue(singleCamera->getGainValue());

        exposureInputBox->setMinimum(singleCamera->getExposureTimeMin());
        exposureInputBox->setMaximum(singleCamera->getExposureTimeMax());
        exposureInputBox->setValue(singleCamera->getExposureTimeValue());

        framerateEnabled->setChecked(singleCamera->isEnabledAcquisitionFrameRate());
        framerateInputBox->setMinimum(std::max(1, singleCamera->getAcquisitionFPSMin()));
        framerateInputBox->setMaximum(singleCamera->getAcquisitionFPSMax());
        framerateInputBox->setValue(singleCamera->getAcquisitionFPSValue());

        hwTriggerEnabled->setChecked(singleCamera->isHardwareTriggerEnabled());

        if(singleCamera->isHardwareTriggerEnabled()) {
            lineSourceBox->setCurrentText(QString::fromStdString(singleCamera->getLineSource().c_str()));
        }
    }
}

// Selects a output filename and saves a Basler camera specific pfs file containing its configuration
void SingleCameraSettingsDialog::saveButtonClick() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Feature File"), "", tr("PFS files (*.pfs)"));

    if(!filename.isEmpty()) {
        QFileInfo fileInfo(filename);

        // check if filename has extension
        if(fileInfo.suffix().isEmpty()) {
            filename = filename + ".pfs";
        }

        singleCamera->saveToFile(filename.toStdString().c_str());
    }
}

// Loading a Basler pfs camera settings file
void SingleCameraSettingsDialog::loadButtonClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature File"), "", tr("PFS files (*.pfs)"));

    if(!filename.isEmpty()) {

        singleCamera->loadFromFile(filename.toStdString().c_str());
        updateForms();
    }
}

void SingleCameraSettingsDialog::autoGainOnce() {

    singleCamera->autoGainOnce();
    gainInputBox->setValue(singleCamera->getGainValue());
}

void SingleCameraSettingsDialog::autoExposureOnce() {

    singleCamera->autoExposureOnce();
    exposureInputBox->setValue(singleCamera->getExposureTimeValue());
}

// Instead of rejecting the dialog, thus closing it, we only hide it and show it again, so that all settings of the current camera are still in the forms
void SingleCameraSettingsDialog::reject() {
    saveSettings();
    //QDialog::reject();
    hide();
}

void SingleCameraSettingsDialog::accept() {
    saveSettings();
    QDialog::accept();
}

void SingleCameraSettingsDialog::updateFrameRateValue() {
    frameRateValueLabel->setText(QString::number(singleCamera->getResultingFrameRateValue()));

    triggerFramerateInputBox->setMaximum(static_cast<int>(floor(singleCamera->getResultingFrameRateValue())));
    framerateInputBox->setMaximum(static_cast<int>(floor(singleCamera->getResultingFrameRateValue())));
}

void SingleCameraSettingsDialog::onLineSourceChange(int index) {
    if(index!=0) {
        singleCamera->setLineSource(lineSourceBox->itemText(index).toStdString().c_str());
        hwTriggerEnabled->setEnabled(true);
    } else {
        hwTriggerEnabled->setEnabled(false);
    }
}

// To start a hardware trigger signal for the camera, the command string is build and send over the serial port to the microcontroller
// The command structure is: <TX...X...> with the number of images and their delay specified i.e. <TX1000X33> for thousand frame with ~30fps
void SingleCameraSettingsDialog::startHardwareTrigger() {

    if(hwTriggerEnabled->isChecked()) {

        double runtime = triggerTimeSpanInputBox->value();
        double fps = triggerFramerateInputBox->value();

        // Calculate the delay and number of frames from the input values
        int delay = (int)(((1000.0f/fps)*1000.0f) / 2.0f);
        int count = (int)(runtime*60000000)/(delay*2);

        QString cmd = "<TX"+ QString::number(count) +"X"+ QString::number(delay) +">";
        std::cout<<"Sending hardware trigger command: "<<cmd.toStdString()<<std::endl;

        emit onHardwareTriggerStart(cmd);

        stopHWButton->setEnabled(true);
        startHWButton->setEnabled(false);
    }
}

// Stops the hardware trigger signal by sending a stop signal to the microcontroller
void SingleCameraSettingsDialog::stopHardwareTrigger() {

    emit onHardwareTriggerStop(QString("<SX>"));

    stopHWButton->setEnabled(false);
    startHWButton->setEnabled(true);
}

void SingleCameraSettingsDialog::onHardwareTriggerCheckbox(bool value) {

    if(value) {
        emit onHardwareTriggerEnable();
    } else {
        emit onHardwareTriggerDisable();
    }
}

void SingleCameraSettingsDialog::onSerialConnect() {
    startHWButton->setEnabled(true);
    stopHWButton->setEnabled(true);
}

void SingleCameraSettingsDialog::onSerialDisconnect() {
    startHWButton->setEnabled(false);
    stopHWButton->setEnabled(false);
}

void SingleCameraSettingsDialog::loadSettings() {

    hwTriggerEnabled->setChecked(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerEnabled", singleCamera->isHardwareTriggerEnabled()).toBool());
    singleCamera->enableHardwareTrigger(hwTriggerEnabled->isChecked());

    lineSourceBox->setCurrentText(applicationSettings->value("SingleCameraSettingsDialog.lineSource", QString::fromStdString(singleCamera->getLineSource().c_str())).toString());
    singleCamera->setLineSource(lineSourceBox->currentText().toStdString().c_str());

    triggerFramerateInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerFramerate", triggerFramerateInputBox->value()).toInt());
    triggerTimeSpanInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerTime", triggerTimeSpanInputBox->value()).toDouble());

    gainInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.analogGain", singleCamera->getGainValue()).toDouble());
    singleCamera->setGainValue(gainInputBox->value());

    exposureInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.analogExposure", singleCamera->getExposureTimeValue()).toInt());
    singleCamera->setExposureTimeValue(exposureInputBox->value());

    framerateEnabled->setChecked(applicationSettings->value("SingleCameraSettingsDialog.framerateEnabled", singleCamera->isEnabledAcquisitionFrameRate()).toBool());
    singleCamera->enableAcquisitionFrameRate(framerateEnabled->isChecked());

    framerateInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.acquisitionFramerate", singleCamera->getAcquisitionFPSValue()).toInt());
    singleCamera->setAcquisitionFPSValue(framerateInputBox->value());

    // TODO load the pfs file as an backup if no appication settings are available?
}

// Save camera settings to the application settings
// Also saves camera settings as pfs file into the application settings directory
void SingleCameraSettingsDialog::saveSettings() {

    applicationSettings->setValue("SingleCameraSettingsDialog.hwTriggerEnabled", hwTriggerEnabled->isChecked());
    applicationSettings->setValue("SingleCameraSettingsDialog.lineSource", lineSourceBox->currentText());
    applicationSettings->setValue("SingleCameraSettingsDialog.hwTriggerFramerate", triggerFramerateInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.hwTriggerTime", triggerTimeSpanInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.analogGain", gainInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.analogExposure", exposureInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.framerateEnabled", framerateEnabled->isChecked());
    applicationSettings->setValue("SingleCameraSettingsDialog.acquisitionFramerate", framerateInputBox->value());

    applicationSettings->setValue("SingleCameraSettingsDialog.settingsDirectory", settingsDirectory.path());

    QString configFile = settingsDirectory.filePath(singleCamera->getFriendlyName()+".pfs");
    configFile.replace(" ", "");
    std::cout<<"Saving config to settings directory: "<< configFile.toStdString() <<std::endl;
    singleCamera->saveToFile(configFile.toStdString().c_str());
}

void SingleCameraSettingsDialog::onSettingsChange() {
    loadSettings();
}

SingleCameraSettingsDialog::~SingleCameraSettingsDialog() = default;
