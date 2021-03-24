
#include <iostream>
#include <pylon/TlFactory.h>
#include <pylon/Container.h>
#include "stereoCameraSettingsDialog.h"

// Creates a new stereo camera settings dialog
// The dialog setups the stereo camera and starts fetching image frames through a hardware trigger
// A stereo camera consists of two physical cameras, which are chosen in this widget
// For the hardware trigger a connection to a microcontroller is necessary which is handled through the SerialSettings object
// This is ensured in this form by disabling the start hardware trigger buttons until the cameras are opened
StereoCameraSettingsDialog::StereoCameraSettingsDialog(StereoCamera *camera, SerialSettingsDialog *serialSettings, QWidget *parent) :
        QDialog(parent), camera(camera), serialSettings(serialSettings),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    settingsDirectory = QDir(applicationSettings->value("StereoCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    setMinimumSize(600, 450);

    setWindowTitle(QString("Stereo Camera Settings"));

    createForm();

    connect(gainInputBox, SIGNAL(valueChanged(double)), camera, SLOT(setGainValue(double)));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), camera, SLOT(setExposureTimeValue(int)));
    connect(framerateEnabled, SIGNAL(toggled(bool)), camera, SLOT(enableAcquisitionFrameRate(bool)));
    connect(framerateInputBox, SIGNAL(valueChanged(int)), camera, SLOT(setAcquisitionFPSValue(int)));

    connect(saveButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::saveButtonClick);
    connect(loadButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::loadButtonClick);

    connect(startHWButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::startHardwareTrigger);
    connect(stopHWButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::stopHardwareTrigger);


    connect(gainAutoOnceButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::autoGainOnce);
    connect(exposureAutoOnceButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::autoExposureOnce);

    connect(serialConfigButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::onSerialConfig);

    // Update the device list from which main and secondary camera are chosen
    updateDevicesBox();
    loadSettings();
    updateForms();
}

void StereoCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    QGroupBox *cameraGroup = new QGroupBox("Camera Selection");
    QFormLayout *cameraLayout = new QFormLayout();

    QLabel *lineSourceLabel = new QLabel(tr("Trigger Source:"));
    lineSourceBox = new QComboBox();

    lineSourceBox->addItem(QString("Select Line Source"));
    for(int i=1; i<7;i++) {
        lineSourceBox->addItem(QString("Line") + QString::number(i));
    }

    connect(lineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));

    cameraLayout->addRow(lineSourceLabel, lineSourceBox);

    QLabel *mainCameraLabel = new QLabel(tr("Main:"));
    mainCameraBox = new QComboBox();
    QLabel *secondaryCameraLabel = new QLabel(tr("Secondary:"));
    secondaryCameraBox = new QComboBox();

    cameraLayout->addRow(mainCameraLabel, mainCameraBox);
    cameraLayout->addRow(secondaryCameraLabel, secondaryCameraBox);

    updateDevicesButton = new QPushButton(tr("Refresh Devices"));
    connect(updateDevicesButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::updateDevicesBox);
    cameraLayout->addRow(updateDevicesButton);

    QWidget *tmp = new QWidget();
    QHBoxLayout *cameraButtonLayout = new QHBoxLayout();
    openButton = new QPushButton(tr("Open"));
    connect(openButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::openStereoCamera);
    cameraButtonLayout->addWidget(openButton);
    openButton->setDisabled(true);
    closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::closeStereoCamera);
    cameraButtonLayout->addWidget(closeButton);
    closeButton->setDisabled(true);
    tmp->setLayout(cameraButtonLayout);
    cameraLayout->addWidget(tmp);

    updateDevicesBox();

    cameraGroup->setLayout(cameraLayout);
    mainLayout->addWidget(cameraGroup);


    hwTriggerGroup = new QGroupBox("Hardware Trigger");
    QFormLayout *hwTriggerGroupLayout = new QFormLayout();

    serialConfigButton = new QPushButton("Serial Connection");

    hwTriggerGroupLayout->addWidget(serialConfigButton);

    QLabel *triggerFramerateLabel = new QLabel(tr("Trigger FPS:"));
    triggerFramerateInputBox = new QSpinBox();
    triggerFramerateInputBox->setMinimum(1);
    triggerFramerateInputBox->setMaximum(999);
    triggerFramerateInputBox->setSingleStep(1);
    triggerFramerateInputBox->setEnabled(false);

    hwTriggerGroupLayout->addRow(triggerFramerateLabel, triggerFramerateInputBox);

    QLabel *triggerTimeSpanLabel = new QLabel(tr("Trigger Runtime [min]:"));
    triggerTimeSpanInputBox = new QDoubleSpinBox();
    triggerTimeSpanInputBox->setMinimum(0);
    triggerTimeSpanInputBox->setMaximum(std::numeric_limits<double>::max());
    triggerTimeSpanInputBox->setSingleStep(0.1);
    triggerTimeSpanInputBox->setValue(1);
    triggerTimeSpanInputBox->setEnabled(false);

    hwTriggerGroupLayout->addRow(triggerTimeSpanLabel, triggerTimeSpanInputBox);

    QWidget *hwTmp = new QWidget();
    QHBoxLayout *hwButtonLayout = new QHBoxLayout();
    startHWButton = new QPushButton("Start");
    stopHWButton = new QPushButton("Stop");
    bool serialConnected = serialSettings->isConnected();
    startHWButton->setEnabled(serialConnected);
    stopHWButton->setEnabled(serialConnected);
    hwButtonLayout->addWidget(startHWButton);
    hwButtonLayout->addWidget(stopHWButton);

    hwTmp->setLayout(hwButtonLayout);
    hwTriggerGroupLayout->addWidget(hwTmp);

    hwTriggerGroup->setLayout(hwTriggerGroupLayout);
    mainLayout->addWidget(hwTriggerGroup);
    hwTriggerGroup->setDisabled(true);


    analogGroup = new QGroupBox("Analog Control");
    QFormLayout *analogLayout = new QFormLayout();
    QHBoxLayout *gainInputLayout = new QHBoxLayout();

    QLabel *gainLabel = new QLabel(tr("Gain [dB]:"));
    gainLabel->setMinimumWidth(50);

    // We do not set its value yet, because the camera may not have a valid value yet (not opened)
    gainInputBox = new QDoubleSpinBox();
    gainInputBox->setMinimum(0.0);
    //gainInputBox->setMaximum(floor(camera->getGainMax()));
    gainInputBox->setSingleStep(0.01);
    //gainInputBox->setValue(camera->getGainValue());

    gainInputLayout->addWidget(gainInputBox);
    //gainInputLayout->addWidget(gainSlider);
    analogLayout->addRow(gainLabel, gainInputLayout);

    gainAutoOnceButton = new QPushButton("Auto Gain (Once)");
    analogLayout->addRow(gainAutoOnceButton);

    analogGroup->setLayout(analogLayout);
    analogGroup->setDisabled(true);
    mainLayout->addWidget(analogGroup);


    acquisitionGroup = new QGroupBox("Acquisition Control");
    QFormLayout *acquisitionLayout = new QFormLayout;
    QHBoxLayout *exposureInputLayout = new QHBoxLayout;
    QLabel *exposureLabel = new QLabel(tr("Exposure [us]:"));
    exposureLabel->setMinimumWidth(50);
    exposureInputBox = new QSpinBox();
    exposureInputBox->setMinimum(1);
    exposureInputBox->setMaximum(10000000);
    exposureInputBox->setSingleStep(100);
    //exposureInputBox->setValue(camera->getExposureTimeValue());

    exposureInputLayout->addWidget(exposureInputBox);
    acquisitionLayout->addRow(exposureLabel, exposureInputLayout);

    exposureAutoOnceButton = new QPushButton("Auto Exposure (Once)");
    acquisitionLayout->addRow(exposureAutoOnceButton);

    QLabel *frameRateLabel = new QLabel("Resulting Framerate:");
    frameRateValueLabel = new QLabel(QString::number(0));

    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));
    connect(gainInputBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));

    acquisitionLayout->addRow(frameRateLabel, frameRateValueLabel);

    framerateEnabled = new QCheckBox("Limit acquisition framerate");
    framerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());

    acquisitionGroup->setDisabled(true);
    acquisitionLayout->addRow(framerateEnabled);


    QHBoxLayout *framerateInputLayout = new QHBoxLayout;
    QLabel *framerateLabel = new QLabel(tr("Framerate:"));
    framerateLabel->setMinimumWidth(50);
    framerateInputBox = new QSpinBox();
    QSpacerItem *sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    framerateInputLayout->addWidget(framerateInputBox);
    framerateInputLayout->addSpacerItem(sp);
    framerateInputBox->setMinimum(1);
    framerateInputBox->setMaximum(999);
    framerateInputBox->setSingleStep(1);
    //framerateInputBox->setValue(camera->getAcquisitionFPSValue());
    //framerateInputBox->setEnabled(camera->isEnabledAcquisitionFrameRate());

    acquisitionLayout->addRow(framerateLabel, framerateInputLayout);

    connect(framerateEnabled, SIGNAL(toggled(bool)), framerateInputBox, SLOT(setEnabled(bool)));

    acquisitionGroup->setLayout(acquisitionLayout);
    mainLayout->addWidget(acquisitionGroup);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    saveButton = new QPushButton(tr("Save to File"));
    loadButton = new QPushButton(tr("Load from File"));
    saveButton->setDisabled(true);
    loadButton->setDisabled(true);

    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addWidget(loadButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
}

// Called when the window is closed
// We do not close the window to preserve the settings throughout a session, instead only hide it
void StereoCameraSettingsDialog::reject() {
    saveSettings();
    //QDialog::reject();
    hide();
}

// Closing for good, save the settings and close the camera if not already
void StereoCameraSettingsDialog::accept() {
    saveSettings();

    closeStereoCamera();
    QDialog::accept();
}

StereoCameraSettingsDialog::~StereoCameraSettingsDialog() = default;

// Updates the physical device list using the pylon library
void StereoCameraSettingsDialog::updateDevicesBox() {

    mainCameraBox->setDisabled(false);
    secondaryCameraBox->setDisabled(false);

    CTlFactory& TlFactory = CTlFactory::GetInstance();

    mainCameraBox->clear();
    secondaryCameraBox->clear();

    TlFactory.EnumerateDevices(lstDevices);

    if(!lstDevices.empty()) {
        for(auto const &device: lstDevices) {
            mainCameraBox->addItem(device.GetFriendlyName().c_str());
            secondaryCameraBox->addItem(device.GetFriendlyName().c_str());
        }
    } else {
        mainCameraBox->addItem("No devices found.");
        secondaryCameraBox->addItem("No devices found.");
        mainCameraBox->setDisabled(true);
        secondaryCameraBox->setDisabled(true);
    }
}

void StereoCameraSettingsDialog::updateForms() {

    if(camera->isOpen()) {

        framerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
        framerateInputBox->setMinimum(std::max(1, camera->getAcquisitionFPSMin()));
        framerateInputBox->setMaximum(camera->getAcquisitionFPSMax());
        framerateInputBox->setValue(camera->getAcquisitionFPSValue());

        gainInputBox->setMinimum(floor(camera->getGainMin()));
        gainInputBox->setMaximum(floor(camera->getGainMax()));
        gainInputBox->setValue(camera->getGainValue());

        exposureInputBox->setMinimum(camera->getExposureTimeMin());
        exposureInputBox->setMaximum(camera->getExposureTimeMax());
        exposureInputBox->setValue(camera->getExposureTimeValue());
    }
}

// Saves the main camera settings to file
// In stereo cameras, the settings are only saved and loaded for the main camera, and then set for the secondary camera too
void StereoCameraSettingsDialog::saveButtonClick() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Camera Setting File"), "", tr("PFS files (*.pfs)"));

    if(!filename.isEmpty()) {
        QFileInfo fileInfo(filename);

        // check if filename has extension
        if(fileInfo.suffix().isEmpty()) {
            filename = filename + ".pfs";
        }
        camera->saveMainToFile(filename.toStdString().c_str());
    }
}

// Selects a file and loads its camera settings
// In stereo cameras, the settings are only loaded for the main camera, which are then set for the secondary camera too
void StereoCameraSettingsDialog::loadButtonClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Camera Setting File"), "", tr("PFS files (*.pfs)"));

    if(!filename.isEmpty()) {

        camera->loadMainFromFile(filename.toStdString().c_str());
        updateForms();
    }
}

void StereoCameraSettingsDialog::autoGainOnce() {
    camera->autoGainOnce();
    gainInputBox->setValue(camera->getGainValue());
}

void StereoCameraSettingsDialog::autoExposureOnce() {
    camera->autoExposureOnce();
    exposureInputBox->setValue(camera->getExposureTimeValue());
}

void StereoCameraSettingsDialog::updateFrameRateValue() {
    frameRateValueLabel->setText(QString::number(camera->getResultingFrameRateValue()));

    if(startHWButton->isEnabled()) {
        triggerFramerateInputBox->setMaximum(static_cast<int>(floor(camera->getResultingFrameRateValue())));
        framerateInputBox->setMaximum(static_cast<int>(floor(camera->getResultingFrameRateValue())));
    }
}

void StereoCameraSettingsDialog::onLineSourceChange(int index) {
    if(index!=0) {
        camera->setLineSource(lineSourceBox->itemText(index).toStdString().c_str());
        triggerFramerateInputBox->setDisabled(false);
        triggerTimeSpanInputBox->setDisabled(false);
        openButton->setDisabled(false);
        closeButton->setDisabled(false);
    } else {
        triggerFramerateInputBox->setDisabled(true);
        triggerTimeSpanInputBox->setDisabled(true);
        startHWButton->setDisabled(true);
        stopHWButton->setDisabled(true);
        openButton->setDisabled(true);
        closeButton->setDisabled(true);
    }
}

// Starts the hardware trigger
// To start a hardware trigger signal for the camera, the command string is build and send over the serial port to the microcontroller
// The command structure is: <TX...X...> with the number of images and their delay specified i.e. <TX1000X33> for thousand frame with ~30fps
void StereoCameraSettingsDialog::startHardwareTrigger() {

    double runtime = triggerTimeSpanInputBox->value();
    double fps = triggerFramerateInputBox->value();

    int delay = (int)(((1000.0f/fps)*1000.0f) / 2.0f);
    int count = (int)(runtime*60000000)/(delay*2);


    QString cmd = "<TX"+ QString::number(count) +"X"+ QString::number(delay) +">";
    std::cout<<"Sending hardware trigger command: "<<cmd.toStdString()<<std::endl;

    emit onHardwareTriggerEnable();
    emit onHardwareTriggerStart(cmd);

    stopHWButton->setEnabled(true);
    startHWButton->setEnabled(false);
}

// Stops the hardware trigger signal by sending a stop signal to the microcontroller
void StereoCameraSettingsDialog::stopHardwareTrigger() {

    emit onHardwareTriggerDisable();
    emit onHardwareTriggerStop(QString("<SX>"));

    stopHWButton->setEnabled(false);
    startHWButton->setEnabled(true);
}

// Opens the stereo camera system, which means that both selected cameras are attached to the stereo camera object and opened, started to fetch images
// Beware that the opening of the stereo camera must happen BEFORE the start of the hardware trigger signal to ensure a sync image signal
// This is ensured in this form by disabling the start hardware trigger buttons until the cameras are opened
void StereoCameraSettingsDialog::openStereoCamera() {

    int mainCameraIndex = mainCameraBox->currentIndex();
    int secondaryCameraIndex = secondaryCameraBox->currentIndex();

    if(mainCameraIndex != secondaryCameraIndex) {

        try {
            camera->attachCameras(lstDevices[mainCameraIndex], lstDevices[secondaryCameraIndex]);
        } catch (const GenericException &e) {
            // Error handling.
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;

            QMessageBox::critical(this, "Device Error", e.GetDescription());

            return;
        }

        // Activate all settings groups underneath
        analogGroup->setDisabled(false);
        acquisitionGroup->setDisabled(false);
        saveButton->setDisabled(false);
        loadButton->setDisabled(false);

        // Its important to open the camera here not earlier, as loading config overrides the config in open
        camera->open();

        loadSettings();

        updateForms();

        hwTriggerGroup->setDisabled(false);
    } else {
        std::cout<<"Error cant use same camera with stereo system."<<std::endl;
    }
}

// Closes the stereo camera, stops the hardware trigger (by sending signal over serial port)
void StereoCameraSettingsDialog::closeStereoCamera() {

    camera->close();
    // Also stop the HW trigger signals
    stopHardwareTrigger();
    // Disable all camera settings groups underneath
    analogGroup->setDisabled(true);
    acquisitionGroup->setDisabled(true);
    hwTriggerGroup->setDisabled(true);
}

// Slot receiving a signal from the serialsettings that a serial port is now connected
void StereoCameraSettingsDialog::onSerialConnect() {
    if(camera->isOpen()) {
        startHWButton->setEnabled(true);
        stopHWButton->setEnabled(true);
    }
}

// Slot receiving a signal from the serialsettings that a serial port is now disconnected
void StereoCameraSettingsDialog::onSerialDisconnect() {
    startHWButton->setEnabled(false);
    stopHWButton->setEnabled(false);
}

// Slot receiving signal that the settings changed, reload settings
void StereoCameraSettingsDialog::onSettingsChange() {
    loadSettings();
}

// Loads settings from the application setting
// Update the camera from these settings
void StereoCameraSettingsDialog::loadSettings() {

    mainCameraBox->setCurrentText(applicationSettings->value("StereoCameraSettingsDialog.mainCamera", mainCameraBox->currentText()).toString());
    secondaryCameraBox->setCurrentText(applicationSettings->value("StereoCameraSettingsDialog.secondaryCamera", secondaryCameraBox->currentText()).toString());

    lineSourceBox->setCurrentText(applicationSettings->value("StereoCameraSettingsDialog.lineSource", QString::fromStdString(camera->getLineSource().c_str())).toString());
    camera->setLineSource(lineSourceBox->currentText().toStdString().c_str());

    triggerFramerateInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.hwTriggerFramerate", triggerFramerateInputBox->value()).toInt());
    triggerTimeSpanInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.hwTriggerTime", triggerTimeSpanInputBox->value()).toDouble());

    gainInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.analogGain", camera->getGainValue()).toDouble());
    camera->setGainValue(gainInputBox->value());

    exposureInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.analogExposure", camera->getExposureTimeValue()).toInt());
    camera->setExposureTimeValue(exposureInputBox->value());

    framerateEnabled->setChecked(applicationSettings->value("StereoCameraSettingsDialog.framerateEnabled", camera->isEnabledAcquisitionFrameRate()).toBool());
    camera->enableAcquisitionFrameRate(framerateEnabled->isChecked());

    framerateInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.acquisitionFramerate", camera->getAcquisitionFPSValue()).toInt());
    camera->setAcquisitionFPSValue(framerateInputBox->value());

    // TODO load the pfs file as an backup if no appication settings are available?
}

// Saves settings to application settings
// As a backup the main camera settings are also saved to file
void StereoCameraSettingsDialog::saveSettings() {

    applicationSettings->setValue("StereoCameraSettingsDialog.mainCamera", mainCameraBox->currentText());
    applicationSettings->setValue("StereoCameraSettingsDialog.secondaryCamera", secondaryCameraBox->currentText());

    applicationSettings->setValue("StereoCameraSettingsDialog.lineSource", lineSourceBox->currentText());
    applicationSettings->setValue("StereoCameraSettingsDialog.hwTriggerFramerate", triggerFramerateInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.hwTriggerTime", triggerTimeSpanInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.analogGain", gainInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.analogExposure", exposureInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.framerateEnabled", framerateEnabled->isChecked());
    applicationSettings->setValue("StereoCameraSettingsDialog.acquisitionFramerate", framerateInputBox->value());

    applicationSettings->setValue("StereoCameraSettingsDialog.settingsDirectory", settingsDirectory.path());

    if(camera->isOpen()) {
        QString mainName = QString::fromStdString(lstDevices[mainCameraBox->currentIndex()].GetFriendlyName().c_str());
        QString configFile = settingsDirectory.filePath(mainName+".pfs");
        configFile.replace(" ", "");
        std::cout<<"Saving config to settings directory: "<< configFile.toStdString() <<std::endl;
        camera->saveMainToFile(configFile.toStdString().c_str());
    }
}
