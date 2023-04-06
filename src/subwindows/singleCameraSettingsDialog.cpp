
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include "singleCameraSettingsDialog.h"

#include <iostream>
#include <exception>
#include <stdexcept>

// Create a new camera settings dialog window
// Takes the camera which is configured, and the serial settings dialog for the hardware trigger handling
// Loads camera settings if available from the application settings
// GB TODO: no need for single
SingleCameraSettingsDialog::SingleCameraSettingsDialog(SingleCamera *camera, SerialSettingsDialog *serialSettings, QWidget *parent) :
        QDialog(parent), 
        serialSettings(serialSettings), 
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    singleCamera = camera;

    settingsDirectory = QDir(applicationSettings->value("SingleCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    setMinimumSize(400, 400); 
    //setMinimumSize(600, 450); // BG

    setWindowTitle(QString("[%1] Camera Settings").arg(singleCamera->getFriendlyName()));

    createForm();

    // BG: moved all connect() calls to the end of createForm() for clarity

    loadSettings();
    updateForms();
}

void SingleCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    hwTriggerGroup = new QGroupBox("Hardware Trigger");
    QFormLayout *hwTriggerGroupLayout = new QFormLayout();

    hwTriggerEnabled = new QCheckBox("Enable Hw. Trigger");
    hwTriggerEnabled->setChecked(singleCamera->isHardwareTriggerEnabled());
    hwTriggerEnabled->setEnabled(false);

    
    // BG modified begin
    // BG NOTE: Modified to fit better on screen
    lineSourceBox = new QComboBox();

    // BG: shortened from text "Select line source" to fit better
    lineSourceBox->addItem(QString("Select")); 
    for(int i=1; i<7;i++) {
        lineSourceBox->addItem(QString("Line") + QString::number(i));
    }
    lineSourceBox->setFixedWidth(50); // BG
    //lineSourceBox->setCurrentText(QString::fromStdString(camera->getLineSource().c_str()));
    //lineSourceBox->setEnabled(false);
    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), lineSourceBox, SLOT(setEnabled(bool)));

    serialConfigButton = new QPushButton("Serial Connection");

    QHBoxLayout *trigConfigRow1 = new QHBoxLayout;
    trigConfigRow1->addWidget(lineSourceBox);
    trigConfigRow1->addWidget(serialConfigButton);
    hwTriggerGroupLayout->addRow(hwTriggerEnabled, trigConfigRow1); 


    QLabel *triggerFramerateLabel = new QLabel(tr("Trigger FPS:"));
    triggerFramerateInputBox = new QSpinBox();
    triggerFramerateInputBox->setMinimum(std::max(1, singleCamera->getAcquisitionFPSMin()));
    triggerFramerateInputBox->setMaximum(singleCamera->getAcquisitionFPSMax());
    triggerFramerateInputBox->setSingleStep(1);
    triggerFramerateInputBox->setValue(singleCamera->getAcquisitionFPSValue());
    triggerFramerateInputBox->setEnabled(false);
    triggerFramerateInputBox->setFixedWidth(50); // BG

    QLabel *triggerTimeSpanLabel = new QLabel(tr("Trigger Runtime [min]:"));
    triggerTimeSpanInputBox = new QDoubleSpinBox();
    triggerTimeSpanInputBox->setMinimum(0);
    triggerTimeSpanInputBox->setMaximum(std::numeric_limits<double>::max());
    triggerTimeSpanInputBox->setSingleStep(0.1);
    triggerTimeSpanInputBox->setValue(1);
    triggerTimeSpanInputBox->setEnabled(false);
    triggerTimeSpanInputBox->setFixedWidth(50); // BG

    QHBoxLayout *trigConfigRow2 = new QHBoxLayout;
    trigConfigRow2->addWidget(triggerFramerateInputBox);
    QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
    trigConfigRow2->addSpacerItem(sp1);
    trigConfigRow2->addWidget(triggerTimeSpanLabel);
    trigConfigRow2->addWidget(triggerTimeSpanInputBox);
    hwTriggerGroupLayout->addRow(triggerFramerateLabel, trigConfigRow2);


    startHWButton = new QPushButton("Start");
    stopHWButton = new QPushButton("Stop");
    bool serialConnected = serialSettings->isConnected();
    startHWButton->setEnabled(serialConnected);
    stopHWButton->setEnabled(serialConnected);

    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), startHWButton, SLOT(setEnabled(bool)));
    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), stopHWButton, SLOT(setEnabled(bool)));

    QLabel *trigButtonsLabel = new QLabel(tr("")); // workaround, todo
    trigButtonsLabel->setMinimumWidth(50);
    QHBoxLayout *trigConfigRow3 = new QHBoxLayout;
    trigConfigRow3->addWidget(startHWButton);
    trigConfigRow3->addWidget(stopHWButton);
    hwTriggerGroupLayout->addRow(trigButtonsLabel, trigConfigRow3);
    // BG modified end

    hwTriggerGroup->setLayout(hwTriggerGroupLayout);
    mainLayout->addWidget(hwTriggerGroup);


    analogGroup = new QGroupBox("Analog Control");
    QFormLayout *analogLayout = new QFormLayout();
    QHBoxLayout *gainInputLayout = new QHBoxLayout();

    QLabel *gainLabel = new QLabel(tr("Gain [dB]:"));
    gainLabel->setMinimumWidth(90); // was: 50

    // BG modified begin
    // BG NOTE: Modified to fit on smaller screens too
    gainInputBox = new QDoubleSpinBox();
    gainInputBox->setMinimum(singleCamera->getGainMin());
    gainInputBox->setMaximum(singleCamera->getGainMax());
    gainInputBox->setSingleStep(0.01);
    gainInputBox->setValue(singleCamera->getGainValue());
    gainInputBox->setFixedWidth(50); // BG

    gainAutoOnceButton = new QPushButton("Auto Gain (Once)");
    gainInputLayout->addWidget(gainInputBox);
    gainInputLayout->addWidget(gainAutoOnceButton);
    analogLayout->addRow(gainLabel, gainInputLayout);
    // BG modified end

    analogGroup->setLayout(analogLayout);
    mainLayout->addWidget(analogGroup);


    acquisitionGroup = new QGroupBox("Acquisition Control");
    QFormLayout *acquisitionLayout = new QFormLayout;
    
    QHBoxLayout *exposureInputLayout = new QHBoxLayout;
    // BG: we are in unicode, so can use greek mu sign. Previously it was written as "Exposure [us]"
    exposureLabel = new QLabel(tr("Exposure [µs]:")); 
    exposureLabel->setMinimumWidth(50);
    exposureInputBox = new QSpinBox();
    exposureInputBox->setMinimum(singleCamera->getExposureTimeMin());
    exposureInputBox->setMaximum(singleCamera->getExposureTimeMax());
    exposureInputBox->setSingleStep(1);
    exposureInputBox->setValue(singleCamera->getExposureTimeValue());
    exposureInputBox->setFixedWidth(50);

    // BG modified begin
    // BG NOTE: Modified to fit on smaller screens too
    exposureAutoOnceButton = new QPushButton("Auto Exposure (Once)");
    exposureInputLayout->addWidget(exposureInputBox);
    exposureInputLayout->addWidget(exposureAutoOnceButton);
    acquisitionLayout->addRow(exposureLabel, exposureInputLayout);
    // BG modified end

    // BG added begin
//    QSpacerItem *sp2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
//    imageROIlayoutRow2->addSpacerItem(sp2);
    QHBoxLayout *imageROIlayoutRow1 = new QHBoxLayout;
    imageROIwidthLabel = new QLabel(tr("Image ROI width [px]:"));
    imageROIwidthLabel->setMinimumWidth(50);
    imageROIwidthInputBox = new QSpinBox();
    imageROIwidthInputBox->setFixedWidth(50);
    imageROIwidthMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow1->addWidget(imageROIwidthInputBox);
    imageROIlayoutRow1->addWidget(imageROIwidthMaxLabel);
    acquisitionLayout->addRow(imageROIwidthLabel, imageROIlayoutRow1);

    QHBoxLayout *imageROIlayoutRow2 = new QHBoxLayout;
    imageROIheightLabel = new QLabel(tr("Image ROI height [px]:"));
    imageROIheightLabel->setMinimumWidth(50);
    imageROIheightInputBox = new QSpinBox();
    imageROIheightInputBox->setFixedWidth(50);
    imageROIheightMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow2->addWidget(imageROIheightInputBox);
    imageROIlayoutRow2->addWidget(imageROIheightMaxLabel);
    acquisitionLayout->addRow(imageROIheightLabel, imageROIlayoutRow2);

    QHBoxLayout *imageROIlayoutRow3 = new QHBoxLayout;
    imageROIoffsetXLabel = new QLabel(tr("Image ROI offsetX [px]:"));
    imageROIoffsetXLabel->setMinimumWidth(50);
    imageROIoffsetXInputBox = new QSpinBox();
    imageROIoffsetXInputBox->setFixedWidth(50);
    imageROIoffsetXMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow3->addWidget(imageROIoffsetXInputBox);
    imageROIlayoutRow3->addWidget(imageROIoffsetXMaxLabel);
    acquisitionLayout->addRow(imageROIoffsetXLabel, imageROIlayoutRow3);

    QHBoxLayout *imageROIlayoutRow4 = new QHBoxLayout;
    QHBoxLayout *imageROIoffsetYInputLayout = new QHBoxLayout;
    imageROIoffsetYLabel = new QLabel(tr("Image ROI offsetY [px]:"));
    imageROIoffsetYLabel->setMinimumWidth(50);
    imageROIoffsetYInputBox = new QSpinBox();
    imageROIoffsetYInputBox->setFixedWidth(50);
    imageROIoffsetYMaxLabel = new QLabel(tr("/ 0"));
    imageROIlayoutRow4->addWidget(imageROIoffsetYInputBox);
    imageROIlayoutRow4->addWidget(imageROIoffsetYMaxLabel);
    acquisitionLayout->addRow(imageROIoffsetYLabel, imageROIlayoutRow4);

    binningLabel = new QLabel(tr("Binning:"));
    binningLabel->setMinimumWidth(50);
    binningBox = new QComboBox();
    binningBox->addItem(QString("1 (no binning)"));
    binningBox->addItem(QString("2"));
    binningBox->addItem(QString("4"));
    binningBox->setFixedWidth(100);
    acquisitionLayout->addRow(binningLabel, binningBox);
    // BG added end

    // BG modified begin
    frameRateLabel = new QLabel("Resulting Framerate:");
    frameRateValueLabel = new QLabel(QString::number(singleCamera->getResultingFrameRateValue()));

    //lineSourceBox->setCurrentText(QString::fromStdString(camera->getLineSource().c_str()));
    //lineSourceBox->setEnabled(false);
    //connect(hwTriggerEnabled, SIGNAL(toggled(bool)), lineSourceBox, SLOT(setEnabled(bool)));

    // BG NOTE: migrated the checkbox and spinbox into a single line to fit better on small screens
    framerateEnabled = new QCheckBox("Limit framerate to:");
    framerateEnabled->setChecked(singleCamera->isEnabledAcquisitionFrameRate());
    framerateInputBox = new QSpinBox();
    framerateInputLayout = new QHBoxLayout;
    framerateInputLayout->addWidget(frameRateValueLabel);
    QSpacerItem *sp4 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
    framerateInputLayout->addSpacerItem(sp4);
    framerateInputLayout->addWidget(framerateEnabled);
    framerateInputLayout->addWidget(framerateInputBox);
   // framerateInputLayout->addSpacerItem(sp);
    framerateInputBox->setMinimum(std::max(1, singleCamera->getAcquisitionFPSMin()));
    framerateInputBox->setMaximum(singleCamera->getAcquisitionFPSMax());
    framerateInputBox->setSingleStep(1);
    framerateInputBox->setValue(singleCamera->getAcquisitionFPSValue());
    framerateInputBox->setEnabled(singleCamera->isEnabledAcquisitionFrameRate());
    framerateInputBox->setFixedWidth(50); // BG
    // GB modified end

    acquisitionLayout->addRow(frameRateLabel, framerateInputLayout);

    acquisitionGroup->setLayout(acquisitionLayout);
    mainLayout->addWidget(acquisitionGroup);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    saveButton = new QPushButton(tr("Save to File"));
    loadButton = new QPushButton(tr("Load from File"));

    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addWidget(loadButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    // GB: added this label to warn user that a new image acq ROI or binning setting needs new calibration
    QLabel *imageROIWarningLabel = new QLabel(tr("Warning: If image acquisition ROI or Binning is altered,\na new camera calibration is necessary for proper undistortion."));
    mainLayout->addWidget(imageROIWarningLabel);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    // GB modified/added begin
    // GB: these are needed for binning-dependent limits for image acq ROI spinboxes
    updateImageROISettingsMax();
    updateImageROISettingsMin(singleCamera->getBinningVal());
    updateImageROISettingsValues();
    
    // GB: merged here below all connect() calls of createDialog() as well as new ones, for clearer code
    connect(lineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));
    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), this, SLOT(onHardwareTriggerCheckbox(bool)));
    connect(gainInputBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));

    connect(binningBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBinningModeChange(int)));
    connect(imageROIwidthInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIwidth(int)));
    connect(imageROIheightInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIheight(int)));
    connect(imageROIoffsetXInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetX(int)));
    connect(imageROIoffsetYInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetY(int)));
    
    //connect(binningBox,... // no need to do this, because acq. image ROI changes will update the camera view already

    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), triggerTimeSpanInputBox, SLOT(setEnabled(bool)));
    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), triggerFramerateInputBox, SLOT(setEnabled(bool)));
    connect(framerateEnabled, SIGNAL(toggled(bool)), framerateInputBox, SLOT(setEnabled(bool)));
    
    connect(hwTriggerEnabled, SIGNAL(toggled(bool)), singleCamera, SLOT(enableHardwareTrigger(bool)));

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
    // GB modified/added end
}

void SingleCameraSettingsDialog::updateForms() {

    if(!singleCamera->isOpen()) // BG: Little nicer like this
        return;

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

    // BG: track in a global variable
    lastUsedBinningVal = singleCamera->getBinningVal();
    
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

    std::cout << "resulting framerate " << singleCamera->getResultingFrameRateValue() << std::endl;

    // commented out by kheki4: TODO: problematic, as resulting framerate is affected by framerate limit, which creates a "loop" of events, ű
    // setting the max on gui as the current fps
    //triggerFramerateInputBox->setMaximum(static_cast<int>(floor(singleCamera->getResultingFrameRateValue())));
    //framerateInputBox->setMaximum(static_cast<int>(floor(singleCamera->getResultingFrameRateValue())));

    // instead:
    triggerFramerateInputBox->setMaximum(static_cast<int>(floor(singleCamera->getAcquisitionFPSMax())));
    framerateInputBox->setMaximum(static_cast<int>(floor(singleCamera->getAcquisitionFPSMax())));
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
        int count = (int)((runtime*60000000)/(delay*2)); // corrected following SBelgers in previous commit

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

    // BG added begin
    int lastUsedBinningVal = applicationSettings->value("SingleCameraSettingsDialog.binningVal", singleCamera->getBinningVal()).toInt();
    singleCamera->setBinningVal(lastUsedBinningVal);
    int tempidx = 0;
    if(lastUsedBinningVal==2 || lastUsedBinningVal==3)
        tempidx = 1;
    else if(lastUsedBinningVal==4)
        tempidx = 2;
    binningBox->setCurrentIndex(tempidx);

    imageROIwidthInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIwidth", singleCamera->getImageROIwidthMax() ).toInt());
    imageROIheightInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIheight", singleCamera->getImageROIheightMax()).toInt());
    imageROIoffsetXInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIoffsetX", 0).toInt()); // DEV
    imageROIoffsetYInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIoffsetY", 0).toInt()); // DEV
    // BG added end

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

    // BG added begin
    applicationSettings->setValue("SingleCameraSettingsDialog.binningVal", lastUsedBinningVal);
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIwidth", imageROIwidthInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIheight", imageROIheightInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIoffsetX", imageROIoffsetXInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIoffsetY", imageROIoffsetYInputBox->value());
    // BG added end

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


void SingleCameraSettingsDialog::onSetImageROIwidth(int val) {
    singleCamera->setImageROIwidth(val);
    // NOTE: qt will not consider the programmatic change of the value as a user event to handle
    updateImageROISettingsMax();
    updateImageROISettingsValues();
}

void SingleCameraSettingsDialog::onSetImageROIheight(int val) {
    singleCamera->setImageROIheight(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
}

void SingleCameraSettingsDialog::onSetImageROIoffsetX(int val) {
    singleCamera->setImageROIoffsetX(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
}

void SingleCameraSettingsDialog::onSetImageROIoffsetY(int val) {
    singleCamera->setImageROIoffsetY(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
}

void SingleCameraSettingsDialog::updateImageROISettingsMin(int binningVal) {
    imageROIwidthInputBox->setMinimum(minImageSize/binningVal);
    imageROIwidthInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
    imageROIheightInputBox->setMinimum(minImageSize/binningVal);
    imageROIheightInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
    imageROIoffsetXInputBox->setMinimum(0);
    imageROIoffsetXInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
    imageROIoffsetYInputBox->setMinimum(0);
    imageROIoffsetYInputBox->setSingleStep(imageSizeChangeSingleStep/binningVal);
}

void SingleCameraSettingsDialog::updateImageROISettingsMax() {
    imageROIwidthInputBox->setMaximum(singleCamera->getImageROIwidthMax());
    imageROIheightInputBox->setMaximum(singleCamera->getImageROIheightMax());
    imageROIoffsetXInputBox->setMaximum(singleCamera->getImageROIwidthMax() -singleCamera->getImageROIwidth());
    imageROIoffsetYInputBox->setMaximum(singleCamera->getImageROIheightMax() -singleCamera->getImageROIheight());

    imageROIwidthMaxLabel->setText(QString("/ ") + QString::number(singleCamera->getImageROIwidthMax()));
    imageROIheightMaxLabel->setText(QString("/ ") + QString::number(singleCamera->getImageROIheightMax()));
    imageROIoffsetXMaxLabel->setText(QString("/ ") + QString::number(singleCamera->getImageROIwidthMax() -singleCamera->getImageROIwidth()));
    imageROIoffsetYMaxLabel->setText(QString("/ ") + QString::number(singleCamera->getImageROIheightMax() -singleCamera->getImageROIheight()));
}

void SingleCameraSettingsDialog::updateImageROISettingsValues() {
    imageROIwidthInputBox->setValue(singleCamera->getImageROIwidth());
    imageROIheightInputBox->setValue(singleCamera->getImageROIheight());
    imageROIoffsetXInputBox->setValue(singleCamera->getImageROIoffsetX());
    imageROIoffsetYInputBox->setValue(singleCamera->getImageROIoffsetY());
}

void SingleCameraSettingsDialog::onBinningModeChange(int index) {
    int binningVal = 1;
    if(index==1)
        binningVal = 2;
    else if(index==2)
        binningVal = 4;

    singleCamera->setBinningVal(binningVal);

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
    updateFrameRateValue(); // only update when camera has updated too
}

void SingleCameraSettingsDialog::setLimitationsWhileTracking(bool state) {
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

    binningLabel->setDisabled(state);
    binningBox->setDisabled(state);

    loadButton->setDisabled(state);
    saveButton->setDisabled(state);
}
