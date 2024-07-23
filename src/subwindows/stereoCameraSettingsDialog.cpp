
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
        QDialog(parent), 
        camera(camera), 
        serialSettings(serialSettings),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    settingsDirectory = QDir(applicationSettings->value("StereoCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    setMinimumSize(430, 680);

    setWindowTitle(QString("Stereo Camera Settings"));

    createForm();

    // BG: moved all connect() calls to the end of createForm() for clarity

    // Update the device list from which main and secondary camera are chosen
    updateDevicesBox();
    loadSettings();
    updateForms();
}

void StereoCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    QGroupBox *cameraGroup = new QGroupBox("Camera Selection");
    QFormLayout *cameraLayout = new QFormLayout();

    QLabel *mainCameraLabel = new QLabel(tr("Main:"));
    mainCameraBox = new QComboBox();
    QLabel *secondaryCameraLabel = new QLabel(tr("Secondary:"));
    secondaryCameraBox = new QComboBox();

    cameraLayout->addRow(mainCameraLabel, mainCameraBox);
    cameraLayout->addRow(secondaryCameraLabel, secondaryCameraBox);
    
    // BG modified begin
    // BG NOTE: modified to fit on smaller screens
    QWidget *tmp = new QWidget();
    QHBoxLayout *cameraButtonLayout = new QHBoxLayout();
    updateDevicesButton = new QPushButton(tr("Refresh Devices"));
    cameraButtonLayout->addWidget(updateDevicesButton);
    openButton = new QPushButton(tr("Open"));
    cameraButtonLayout->addWidget(openButton);
    //openButton->setDisabled(true); // BG
    closeButton = new QPushButton(tr("Close"));
    cameraButtonLayout->addWidget(closeButton);
    //closeButton->setDisabled(true); // BG
    tmp->setLayout(cameraButtonLayout);
    cameraLayout->addWidget(tmp);
    // BG modified end

    updateDevicesBox();

    cameraGroup->setLayout(cameraLayout);
    mainLayout->addWidget(cameraGroup);


    hwTriggerGroup = new QGroupBox("Hardware Trigger");
    QFormLayout *hwTriggerGroupLayout = new QFormLayout();

    // BG modified begin
    // BG NOTE: modified to fit in less rows, so can fit better on smaller screens. Also moved a few lines here for clarity

    // // BG moved begin
    QLabel *lineSourceLabel = new QLabel(tr("Trigger Source:"));
    lineSourceBox = new QComboBox();

    // BG: shortened from text "Select line source" to fit better
    lineSourceBox->addItem(QString("Select"));
    for(int i=1; i<7;i++) {
        lineSourceBox->addItem(QString("Line") + QString::number(i));
    }
    lineSourceBox->setFixedWidth(80); // BG
    serialConfigButton = new QPushButton("Serial Connection");

    QHBoxLayout *trigConfigRow1 = new QHBoxLayout;
    trigConfigRow1->addWidget(lineSourceBox);
    trigConfigRow1->addWidget(serialConfigButton);
    hwTriggerGroupLayout->addRow(lineSourceLabel, trigConfigRow1); 
    // // BG moved end

    QLabel *triggerFramerateLabel = new QLabel(tr("Trigger FPS:"));
    triggerFramerateInputBox = new QSpinBox();
    triggerFramerateInputBox->setMinimum(1);
    triggerFramerateInputBox->setMaximum(999);
    triggerFramerateInputBox->setSingleStep(1);
    triggerFramerateInputBox->setEnabled(false);
    triggerFramerateInputBox->setFixedWidth(80); // BG

    QLabel *triggerTimeSpanLabel = new QLabel(tr("Trigger Runtime [min]:"));
    triggerTimeSpanInputBox = new QDoubleSpinBox();
    triggerTimeSpanInputBox->setMinimum(0);
    triggerTimeSpanInputBox->setMaximum(std::numeric_limits<double>::max());
    triggerTimeSpanInputBox->setSingleStep(0.1);
    triggerTimeSpanInputBox->setEnabled(false);
    triggerTimeSpanInputBox->setFixedWidth(70); // BG

    QHBoxLayout *trigConfigRow2 = new QHBoxLayout;
    trigConfigRow2->addWidget(triggerFramerateInputBox);
    QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
    trigConfigRow2->addSpacerItem(sp1);
    trigConfigRow2->addWidget(triggerTimeSpanLabel);
    trigConfigRow2->addWidget(triggerTimeSpanInputBox);
    hwTriggerGroupLayout->addRow(triggerFramerateLabel, trigConfigRow2);

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
    // BG modified end

    hwTriggerGroup->setLayout(hwTriggerGroupLayout);
    mainLayout->addWidget(hwTriggerGroup);
    //hwTriggerGroup->setDisabled(true); // BG


    analogGroup = new QGroupBox("Analog Control");
    QFormLayout *analogLayout = new QFormLayout();
    QHBoxLayout *gainInputLayout = new QHBoxLayout();

    QLabel *gainLabel = new QLabel(tr("Gain [dB]:"));
    gainLabel->setMinimumWidth(80);

    // BG modified begin
    // BG NOTE: Modified to fit on smaller screens too
    // We do not set its value yet, because the camera may not have a valid value yet (not opened)
    gainInputBox = new QDoubleSpinBox();
    gainInputBox->setMinimum(0.0);
    //gainInputBox->setMaximum(floor(camera->getGainMax()));
    gainInputBox->setSingleStep(0.01);
    //gainInputBox->setValue(camera->getGainValue());

    gainAutoOnceButton = new QPushButton("Auto Gain (Once)");
    gainInputLayout->addWidget(gainInputBox);
    gainInputLayout->addWidget(gainAutoOnceButton);
    analogLayout->addRow(gainLabel, gainInputLayout);
    // BG modified end

    analogGroup->setLayout(analogLayout);
    //analogGroup->setDisabled(true); // BG
    mainLayout->addWidget(analogGroup);


    acquisitionGroup = new QGroupBox("Acquisition Control");
    QVBoxLayout *acquisitionLayout = new QVBoxLayout;
    
    QHBoxLayout *exposureInputLayout = new QHBoxLayout;
    // BG: we are in unicode, so can use greek mu sign. Previously it was written as "Exposure [us]"
    exposureLabel = new QLabel(tr("Exposure [µs]:")); 
    exposureLabel->setFixedWidth(80);
    exposureInputBox = new QSpinBox();
    exposureInputBox->setMinimum(camera->getExposureTimeMin());
    exposureInputBox->setMaximum(camera->getExposureTimeMax());
    exposureInputBox->setSingleStep(1);
    exposureInputBox->setValue(camera->getExposureTimeValue());
    exposureInputBox->setFixedWidth(70);

    // BG modified begin
    // BG NOTE: Modified to fit on smaller screens too
    exposureAutoOnceButton = new QPushButton("Auto Exposure (Once)");
    exposureInputLayout->addWidget(exposureLabel);
    exposureInputLayout->addWidget(exposureInputBox);
    exposureInputLayout->addWidget(exposureAutoOnceButton);
    acquisitionLayout->addLayout(exposureInputLayout);
    // BG modified end

    QHBoxLayout *imageROIlayoutHBlock = new QHBoxLayout;

    QVBoxLayout *imageROIlayoutNestedVBlock1 = new QVBoxLayout;

    // BG added begin
//    QSpacerItem *sp2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
//    imageROIlayoutRow2->addSpacerItem(sp2);
    QHBoxLayout *imageROIlayoutRow1 = new QHBoxLayout;
    imageROIwidthLabel = new QLabel(tr("Image ROI width [px]:"));
    imageROIwidthLabel->setMinimumWidth(120);
    imageROIwidthInputBox = new QSpinBox();
    imageROIwidthInputBox->setFixedWidth(60);
    imageROIwidthMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow1->addWidget(imageROIwidthLabel);
    imageROIlayoutRow1->addWidget(imageROIwidthInputBox);
    imageROIlayoutRow1->addWidget(imageROIwidthMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow1);

    QHBoxLayout *imageROIlayoutRow2 = new QHBoxLayout;
    imageROIheightLabel = new QLabel(tr("Image ROI height [px]:"));
    imageROIheightLabel->setMinimumWidth(120);
    imageROIheightInputBox = new QSpinBox();
    imageROIheightInputBox->setFixedWidth(60);
    imageROIheightMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow2->addWidget(imageROIheightLabel);
    imageROIlayoutRow2->addWidget(imageROIheightInputBox);
    imageROIlayoutRow2->addWidget(imageROIheightMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow2);

    QHBoxLayout *imageROIlayoutRow3 = new QHBoxLayout;
    imageROIoffsetXLabel = new QLabel(tr("Image ROI offsetX [px]:"));
    imageROIoffsetXLabel->setMinimumWidth(120);
    imageROIoffsetXInputBox = new QSpinBox();
    imageROIoffsetXInputBox->setFixedWidth(60);
    imageROIoffsetXMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow3->addWidget(imageROIoffsetXLabel);
    imageROIlayoutRow3->addWidget(imageROIoffsetXInputBox);
    imageROIlayoutRow3->addWidget(imageROIoffsetXMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow3);

    QHBoxLayout *imageROIlayoutRow4 = new QHBoxLayout;
    //QHBoxLayout *imageROIoffsetYInputLayout = new QHBoxLayout;
    imageROIoffsetYLabel = new QLabel(tr("Image ROI offsetY [px]:"));
    imageROIoffsetYLabel->setMinimumWidth(120);
    imageROIoffsetYInputBox = new QSpinBox();
    imageROIoffsetYInputBox->setFixedWidth(60);
    imageROIoffsetYMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow4->addWidget(imageROIoffsetYLabel);
    imageROIlayoutRow4->addWidget(imageROIoffsetYInputBox);
    imageROIlayoutRow4->addWidget(imageROIoffsetYMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow4);

    
    QVBoxLayout *imageROIlayoutNestedVBlock2 = new QVBoxLayout;
    imageROIlayoutNestedVBlock2->setMargin(0);

    camImageRegionsWidget = new CamImageRegionsWidget(this);
    camImageRegionsWidget->setFixedHeight(80);
    imageROIlayoutNestedVBlock2->addWidget(camImageRegionsWidget);

    imageROIlayoutHBlock->addLayout(imageROIlayoutNestedVBlock1);
    imageROIlayoutHBlock->addLayout(imageROIlayoutNestedVBlock2);
    acquisitionLayout->addLayout(imageROIlayoutHBlock);



    QHBoxLayout *imageROIlayoutRow5 = new QHBoxLayout;
    binningLabel = new QLabel(tr("Binning:"));
    binningLabel->setFixedWidth(70);
    binningBox = new QComboBox();
    binningBox->addItem(QString("1 (no binning)"));
    binningBox->addItem(QString("2"));
    binningBox->addItem(QString("4"));
    binningBox->setFixedWidth(100);
    imageROIlayoutRow5->addWidget(binningLabel);
    imageROIlayoutRow5->addWidget(binningBox);
    imageROIlayoutRow5->addStretch();
    acquisitionLayout->addLayout(imageROIlayoutRow5);
    // BG added end

    // BG modified begin
    frameRateLabel = new QLabel("Resulting Framerate:");
    frameRateValueLabel = new QLabel(QString::number(camera->getResultingFrameRateValue()));

    //lineSourceBox->setCurrentText(QString::fromStdString(camera->getLineSource().c_str()));
    //lineSourceBox->setEnabled(false);
    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), lineSourceBox, SLOT(setEnabled(bool)));

    // BG NOTE: migrated the checkbox and spinbox into a single line to fit better on small screens
    framerateEnabled = new QCheckBox("Limit framerate to:");
    framerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
    framerateInputBox = new QSpinBox();
    framerateInputLayout = new QHBoxLayout;
    framerateInputLayout->addWidget(frameRateLabel);
    framerateInputLayout->addWidget(frameRateValueLabel);
    QSpacerItem *sp4 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
    framerateInputLayout->addSpacerItem(sp4);
    framerateInputLayout->addWidget(framerateEnabled);
    framerateInputLayout->addWidget(framerateInputBox);
    framerateInputLayout->addStretch();
   // framerateInputLayout->addSpacerItem(sp);
    framerateInputBox->setMinimum(std::max(1, camera->getAcquisitionFPSMin()));
    framerateInputBox->setMaximum(camera->getAcquisitionFPSMax());
    framerateInputBox->setSingleStep(1);
    framerateInputBox->setValue(camera->getAcquisitionFPSValue());
    framerateInputBox->setEnabled(camera->isEnabledAcquisitionFrameRate());
    framerateInputBox->setFixedWidth(60);

    acquisitionLayout->addLayout(framerateInputLayout);




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

    // BG: added this label to warn user that a new image acq ROI or binning setting needs new calibration
    QLabel *imageROIWarningLabel = new QLabel(tr("Warning: If image acquisition ROI or Binning is altered,\na new camera calibration is necessary for proper undistortion."));
    mainLayout->addWidget(imageROIWarningLabel);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);


    // GB modified/added begin

    // BG: only reveal settings when the cameras are connected
    setLimitationsWhileUnconnected(true);

    // GB: merged here below all connect() calls of createDialog() as well as new ones, for clearer code
    connect(lineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));
    connect(binningBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBinningModeChange(int)));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));
    connect(gainInputBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));

    connect(imageROIwidthInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIwidth(int)));    connect(imageROIheightInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIheight(int)));    connect(imageROIoffsetXInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetX(int)));    connect(imageROIoffsetYInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetY(int)));

    connect(framerateEnabled, SIGNAL(toggled(bool)), framerateInputBox, SLOT(setEnabled(bool)));

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

    connect(updateDevicesButton, SIGNAL(clicked()), this, SLOT(updateDevicesBox()));
    connect(openButton, SIGNAL(clicked()), this, SLOT(onOpen()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeStereoCamera()));
    // GB modified/added end
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

    if(lstDevices.size()>1) {
        //mainCameraBox->setCurrentIndex(0);
        secondaryCameraBox->setCurrentIndex(1);
    }
}

void StereoCameraSettingsDialog::updateForms() {

    if(!camera->isOpen()) // BG: Little nicer like this
        return;

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
    
    // BG: track in a global variable
    lastUsedBinningVal = camera->getBinningVal(); 

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
    int count = (int)((runtime*60000000)/(delay*2)); // corrected by SBelgers in previous commit


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

// This method is to be used for camera warmup connection upon program start (argument command), 
// or for remote control connection invoked camera connection
// GB IMPORTANT TODO: TO BE TESTED
void StereoCameraSettingsDialog::openStereoCamera(const QString &camName1, const QString &camName2) {
    int idx1 = -1;
    for(int i=0; i<mainCameraBox->count(); i++)
        if(mainCameraBox->itemText(i) == camName1) {
            idx1 = i;
            break;
        }
    int idx2 = -1;
    for(int i=0; i<secondaryCameraBox->count(); i++)
        if(secondaryCameraBox->itemText(i) == camName1) {
            idx2 = i;
            break;
        }
    if(idx1==idx2 || idx1<0 || idx2<0)
        return;

    mainCameraBox->setCurrentIndex(idx1);
    secondaryCameraBox->setCurrentIndex(idx2);

    onOpen();
}

// Opens the stereo camera system, which means that both selected cameras are attached to the stereo camera object and opened, started to fetch images
// Beware that the opening of the stereo camera must happen BEFORE the start of the hardware trigger signal to ensure a sync image signal
// This is ensured in this form by disabling the start hardware trigger buttons until the cameras are opened
void StereoCameraSettingsDialog::onOpen() {

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

        // BG: moved widget group disabling from here

        bool enableHardwareTrigger = true;
        // For debug/testing purposes only
        if(QString::fromStdString(lstDevices[mainCameraIndex].GetFriendlyName().c_str()).contains("emulat", Qt::CaseInsensitive) ||
                QString::fromStdString(lstDevices[secondaryCameraIndex].GetFriendlyName().c_str()).contains("emulat", Qt::CaseInsensitive)) {
            enableHardwareTrigger = false;
        }
        // Its important to open the camera here not earlier, as loading config overrides the config in open
        camera->open(enableHardwareTrigger);

        loadSettings();

        updateForms();

        // BG added begin
        updateImageROISettingsMax();
        updateImageROISettingsMin(camera->getBinningVal());
        updateImageROISettingsValues();

        // BG: migrated enabling/disabling widget groups into a separate function
        // Activate all settings groups underneath
        setLimitationsWhileUnconnected(false);
        // BG added end

        //hwTriggerGroup->setDisabled(false); 
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
    // BG: migrated into a separate function
    setLimitationsWhileUnconnected(true);
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

    // BG added begin
    int lastUsedBinningVal = applicationSettings->value("StereoCameraSettingsDialog.binningVal", camera->getBinningVal()).toInt();
    camera->setBinningVal(lastUsedBinningVal);
    int tempidx = 0;
    if(lastUsedBinningVal==2 || lastUsedBinningVal==3)
        tempidx = 1;
    else if(lastUsedBinningVal==4)
        tempidx = 2;
    binningBox->setCurrentIndex(tempidx);

    imageROIwidthInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIwidth", camera->getImageROIwidthMax() ).toInt());
    imageROIheightInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIheight", camera->getImageROIheightMax()).toInt());
    imageROIoffsetXInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIoffsetX", 0).toInt()); // DEV
    imageROIoffsetYInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIoffsetY", 0).toInt()); // DEV
    // BG added end

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

    // BG added begin
    applicationSettings->setValue("StereoCameraSettingsDialog.binningVal", lastUsedBinningVal);
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIwidth", imageROIwidthInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIheight", imageROIheightInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIoffsetX", imageROIoffsetXInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIoffsetY", imageROIoffsetYInputBox->value());
    // BG added end

    applicationSettings->setValue("StereoCameraSettingsDialog.settingsDirectory", settingsDirectory.path());

    if(camera->isOpen()) {
        QString mainName = QString::fromStdString(lstDevices[mainCameraBox->currentIndex()].GetFriendlyName().c_str());
        QString configFile = settingsDirectory.filePath(mainName+".pfs");
        configFile.replace(" ", "");
        std::cout<<"Saving config to settings directory: "<< configFile.toStdString() <<std::endl;
        camera->saveMainToFile(configFile.toStdString().c_str());
    }
}


void StereoCameraSettingsDialog::onSetImageROIwidth(int val) {    
    if (camera->isEmulated())
        camera->setImageROIwidthEmu(val);
    else
        camera->setImageROIwidth(val);
    // NOTE: qt will not consider the programmatic change of the value as a user event to handle
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
}

void StereoCameraSettingsDialog::onSetImageROIheight(int val) {
    if (camera->isEmulated())
        camera->setImageROIheightEmu(val);
    else
        camera->setImageROIheight(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
}

void StereoCameraSettingsDialog::onSetImageROIoffsetX(int val) {
    if (camera->isEmulated())
        camera->setImageROIoffsetXEmu(val);
    else    
        camera->setImageROIoffsetX(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
}

void StereoCameraSettingsDialog::onSetImageROIoffsetY(int val) {    
    if (camera->isEmulated())
        camera->setImageROIoffsetYEmu(val);
    else   
    camera->setImageROIoffsetY(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
}

void StereoCameraSettingsDialog::updateImageROISettingsMin(int binningVal) {
    imageROIwidthInputBox->setMinimum(minImageSize/binningVal);
    imageROIwidthInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
    imageROIheightInputBox->setMinimum(minImageSize/binningVal);
    imageROIheightInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
    imageROIoffsetXInputBox->setMinimum(0);
    imageROIoffsetXInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
    imageROIoffsetYInputBox->setMinimum(0);
    imageROIoffsetYInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
}

void StereoCameraSettingsDialog::updateImageROISettingsMax() {
    imageROIwidthInputBox->setMaximum(camera->getImageROIwidthMax());
    imageROIheightInputBox->setMaximum(camera->getImageROIheightMax());
    imageROIoffsetXInputBox->setMaximum(camera->getImageROIwidthMax() -camera->getImageROIwidth());
    imageROIoffsetYInputBox->setMaximum(camera->getImageROIheightMax() -camera->getImageROIheight());

    imageROIwidthMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIwidthMax()));
    imageROIheightMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIheightMax()));
    imageROIoffsetXMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIwidthMax() -camera->getImageROIwidth()));
    imageROIoffsetYMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIheightMax() -camera->getImageROIheight()));
}

void StereoCameraSettingsDialog::updateImageROISettingsValues() {

    int width = camera->getImageROIwidth();
    int height = camera->getImageROIheight();
    int offsetX = camera->getImageROIoffsetX();
    int offsetY = camera->getImageROIoffsetY();

    imageROIwidthInputBox->setValue(width);
    imageROIheightInputBox->setValue(height);
    imageROIoffsetXInputBox->setValue(offsetX);
    imageROIoffsetYInputBox->setValue(offsetY);

    emit onImageROIChanged(QRect(offsetX, offsetY, width, height));
}

void StereoCameraSettingsDialog::onBinningModeChange(int index) {
    int binningVal = 1;
    if(index==1)
        binningVal = 2;
    else if(index==2)
        binningVal = 4;

    camera->setBinningVal(binningVal);

    if(lastUsedBinningVal > binningVal) {
        //qDebug() << "Inflating image ROI";
        // First set minimum and maximum values for the widgets 
        // (first setting the actual value would take no effect as the maximum does not let it happen)
        updateImageROISettingsMin(binningVal);
        updateImageROISettingsMax();
        // Then set the values on GUI according to the (already automatically changed) camera image ROI parameters 
        updateImageROISettingsValues();
    } else { 
        //qDebug() << "Shrinking image ROI";
        // First set the values on GUI according to the (already automatically changed) camera image ROI parameters
        updateImageROISettingsValues();
        updateImageROISettingsMax();
        // Then set minimum and maximum values for the widgets (e.g. first setting the maximum 
        // would auto-reset the value if that was a bigger number... and that would cause strange behaviour of the GUI)
        updateImageROISettingsMin(binningVal);
    }

    // GB NOTE: here we could tell cameraview that it should expect different image size. But it is now programmed to be adaptive
    lastUsedBinningVal = binningVal;
    updateCamImageRegionsWidget();
    updateFrameRateValue(); // only update when camera has updated too

    // Change: okay, we tell the cameraview, but only for properly letting it know where the positioning guide should be drawn
    updateSensorSize();
}

void StereoCameraSettingsDialog::updateSensorSize() {
    emit onSensorSizeChanged(QSize(camera->getImageROIwidthMax(), camera->getImageROIheightMax()));
}

void StereoCameraSettingsDialog::updateCamImageRegionsWidget() {
    camImageRegionsWidget->setImageMaxSize( QSize(
        camera->getImageROIwidthMax(), camera->getImageROIheightMax()
        ) );
    camImageRegionsWidget->setImageAcqROI1Rect( QRect(
        camera->getImageROIoffsetX(), camera->getImageROIoffsetY(),
        camera->getImageROIwidth(), camera->getImageROIheight()
        ) );
}

void StereoCameraSettingsDialog::setLimitationsWhileTracking(bool state) {
    //hwTriggerGroup->setDisabled(state);
    //analogGroup->setDisabled(state);

    //exposureLabel->setDisabled(state);
    //exposureInputBox->setDisabled(state);
    //exposureAutoOnceButton->setDisabled(state);

    imageROIwidthLabel->setDisabled(state);
    imageROIwidthInputBox->setDisabled(state);
    imageROIheightLabel->setDisabled(state);
    imageROIheightInputBox->setDisabled(state);
    imageROIoffsetXLabel->setDisabled(state);
    imageROIoffsetXInputBox->setDisabled(state);
    imageROIoffsetYLabel->setDisabled(state);
    imageROIoffsetYInputBox->setDisabled(state);

    imageROIwidthMaxLabel->setDisabled(state);
    imageROIheightMaxLabel->setDisabled(state);
    imageROIoffsetXMaxLabel->setDisabled(state);
    imageROIoffsetYMaxLabel->setDisabled(state);

    if (!camera->isEmulated()){
        binningLabel->setDisabled(state);
        binningBox->setDisabled(state);
    }
    else {
        binningLabel->setDisabled(true);
        binningBox->setDisabled(true);
    }

    loadButton->setDisabled(state);
    saveButton->setDisabled(state);
}

void StereoCameraSettingsDialog::setLimitationsWhileUnconnected(bool state) {
    if (!camera->isEmulated()){
        hwTriggerGroup->setDisabled(state);
        binningLabel->setDisabled(state);
        binningBox->setDisabled(state);
    }
    else{
        hwTriggerGroup->setDisabled(true);
        binningLabel->setDisabled(true);
        binningBox->setDisabled(true);
    }
    analogGroup->setDisabled(state);
    acquisitionGroup->setDisabled(state);    
    loadButton->setDisabled(state);
    saveButton->setDisabled(state);
}
