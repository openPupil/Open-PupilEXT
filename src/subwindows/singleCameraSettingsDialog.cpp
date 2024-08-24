
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include "singleCameraSettingsDialog.h"
#include "../supportFunctions.h"

#include <iostream>
#include <exception>
#include <stdexcept>


// Create a new camera settings dialog window
// Takes the camera which is configured, and the MCU settings dialog for the hardware trigger handling
// Loads camera settings if available from the application settings
SingleCameraSettingsDialog::SingleCameraSettingsDialog(SingleCamera *cameraPtr, MCUSettingsDialog *MCUSettings, QWidget *parent) :
        QDialog(parent),
        MCUSettings(MCUSettings),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    camera = cameraPtr;

    settingsDirectory = QDir(applicationSettings->value("SingleCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
// mkdir(".") DOES NOT WORK ON MACOS, ONLY WINDOWS. (Reported on MacOS 12.7.6 and Windows 10)
//        settingsDirectory.mkdir(".");
        QDir().mkpath(settingsDirectory.absolutePath());
    }

#ifdef Q_OS_WIN // Q_OS_MACOS
    setMinimumSize(500, 630);
#else
    setMinimumSize(500, 720);
#endif

    setWindowTitle(QString("[%1] Camera Settings").arg(camera->getFriendlyName()));

    createForm();

    loadSettings();
    updateForms();
}

void SingleCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    MCUConnGroup = new QGroupBox("1. Microcontroller Connection (needed only for Hardware-triggered image acquisition)");
    QFormLayout *MCUConnGroupLayout = new QFormLayout();
    MCUConnGroupLayout->setMargin(10);
    MCUConnGroupLayout->setContentsMargins(5,5,5,5);

    QSpacerItem *sp10 = new QSpacerItem(70, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    MCUConfigButton = new QPushButton();
//    MCUConfigButton->setText("Microcontroller Connection Settings");
    MCUConfigButton->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/show-gpu-effects.svg"), applicationSettings));
    MCUConfigButton->setStyleSheet("QPushButton { text-align:left; border: 1px solid #757575; border-radius: 5px;}");
    MCUConfigButton->setLayout(new QGridLayout);
    QLabel* MCUConfigButtonLabel = new QLabel("Microcontroller Connection Settings");
    MCUConfigButtonLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    MCUConfigButtonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    MCUConfigButton->layout()->addWidget(MCUConfigButtonLabel);
    MCUConfigButton->layout()->setContentsMargins(5, 0, 10, 0);
    MCUConfigButton->setMinimumWidth(260);
    MCUConfigButton->setMinimumHeight(22); //
    QSpacerItem *sp9 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    MCUConnDisconnButton = new QPushButton("Connect"); // Will change to disconnect when connected
    MCUConnDisconnButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    MCUConnDisconnButton->setFixedWidth(150);
    MCUConnDisconnButton->setMinimumHeight(22); //

    QHBoxLayout *MCUConnRow1 = new QHBoxLayout;
    MCUConnRow1->setContentsMargins(0,0,0,0);
    MCUConnRow1->addSpacerItem(sp10);
    MCUConnRow1->addWidget(MCUConfigButton);
    MCUConnRow1->addSpacerItem(sp9);
    MCUConnRow1->addWidget(MCUConnDisconnButton);
//    MCUConnRow1->addStretch();
    MCUConnGroupLayout->addItem(MCUConnRow1);

    MCUConnGroup->setLayout(MCUConnGroupLayout);
    mainLayout->addWidget(MCUConnGroup);

    connect(MCUConnDisconnButton, SIGNAL(clicked()), this, SLOT(MCUConnDisconnButtonClicked()));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    acquisitionGroup = new QGroupBox("2. Image Acquisition Control");
    QVBoxLayout *acquisitionLayout = new QVBoxLayout;

    QHBoxLayout *exposureInputLayout = new QHBoxLayout;
    exposureInputLayout->setMargin(0);
    exposureInputLayout->setContentsMargins(0,0,0,0);
    exposureLabel = new QLabel(tr("Exposure [µs]:"));
    exposureLabel->setFixedWidth(100);
    exposureInputBox = new QSpinBox();
    exposureInputBox->setMinimum(camera->getExposureTimeMin());
    exposureInputBox->setMaximum(camera->getExposureTimeMax());
    exposureInputBox->setSingleStep(1);
    exposureInputBox->setValue(camera->getExposureTimeValue());
    exposureInputBox->setFixedWidth(70);

    autoExposureOnceButton = new QPushButton("Auto Exposure (Once)");
    autoExposureOnceButton->setMinimumWidth(190);
    exposureInputLayout->addWidget(exposureLabel);
    exposureInputLayout->addWidget(exposureInputBox);
    exposureInputLayout->addWidget(autoExposureOnceButton);
    exposureInputLayout->addStretch();
    acquisitionLayout->addLayout(exposureInputLayout);

    QHBoxLayout *imageROIlayoutHBlock = new QHBoxLayout;

    QVBoxLayout *imageROIlayoutNestedVBlock1 = new QVBoxLayout;

//    QSpacerItem *sp2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
//    imageROIlayoutRow2->addSpacerItem(sp2);
    QHBoxLayout *imageROIlayoutRow1 = new QHBoxLayout;
    imageROIlayoutRow1->setMargin(0);
    imageROIlayoutRow1->setContentsMargins(0,0,0,0);
    imageROIwidthLabel = new QLabel(tr("Image ROI width [px]:"));
    imageROIwidthLabel->setMinimumWidth(120);
    imageROIwidthInputBox = new QSpinBox();
    imageROIwidthInputBox->setFixedWidth(60);
    imageROIwidthInputBox->setMinimum(16);
    imageROIwidthInputBox->setMaximum(std::numeric_limits<short>::max());
    imageROIwidthInputBox->setSingleStep(16);
    imageROIwidthInputBox->setKeyboardTracking(false); // to not invoke change upon each digit input
    imageROIwidthMaxLabel = new QLabel(tr("/ 0"));
    imageROIwidthMaxLabel->setFixedWidth(40);
    imageROIlayoutRow1->addWidget(imageROIwidthLabel);
    imageROIlayoutRow1->addWidget(imageROIwidthInputBox);
    imageROIlayoutRow1->addWidget(imageROIwidthMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow1);

    QHBoxLayout *imageROIlayoutRow2 = new QHBoxLayout;
    imageROIlayoutRow2->setMargin(0);
    imageROIlayoutRow2->setContentsMargins(0,0,0,0);
    imageROIheightLabel = new QLabel(tr("Image ROI height [px]:"));
    imageROIheightLabel->setMinimumWidth(120);
    imageROIheightInputBox = new QSpinBox();
    imageROIheightInputBox->setFixedWidth(60);
    imageROIheightInputBox->setMinimum(16);
    imageROIheightInputBox->setMaximum(std::numeric_limits<short>::max());
    imageROIheightInputBox->setSingleStep(16);
    imageROIheightInputBox->setKeyboardTracking(false); // to not invoke change upon each digit input
    imageROIheightMaxLabel = new QLabel(tr("/ 0"));
    imageROIheightMaxLabel->setFixedWidth(40);
    imageROIlayoutRow2->addWidget(imageROIheightLabel);
    imageROIlayoutRow2->addWidget(imageROIheightInputBox);
    imageROIlayoutRow2->addWidget(imageROIheightMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow2);

    QHBoxLayout *imageROIlayoutRow3 = new QHBoxLayout;
    imageROIlayoutRow3->setMargin(0);
    imageROIlayoutRow3->setContentsMargins(0,0,0,0);
    imageROIoffsetXLabel = new QLabel(tr("Image ROI offsetX [px]:"));
    imageROIoffsetXLabel->setMinimumWidth(120);
    imageROIoffsetXInputBox = new QSpinBox();
    imageROIoffsetXInputBox->setFixedWidth(60);
    imageROIoffsetXInputBox->setMinimum(0);
    imageROIoffsetXInputBox->setMaximum(std::numeric_limits<short>::max());
    imageROIoffsetXInputBox->setSingleStep(16);
    imageROIoffsetXInputBox->setKeyboardTracking(false); // to not invoke change upon each digit input
    imageROIoffsetXMaxLabel = new QLabel(tr("/ 0"));
    imageROIoffsetXMaxLabel->setFixedWidth(40);
    imageROIlayoutRow3->addWidget(imageROIoffsetXLabel);
    imageROIlayoutRow3->addWidget(imageROIoffsetXInputBox);
    imageROIlayoutRow3->addWidget(imageROIoffsetXMaxLabel);
    imageROIlayoutNestedVBlock1->addLayout(imageROIlayoutRow3);

    QHBoxLayout *imageROIlayoutRow4 = new QHBoxLayout;
    imageROIlayoutRow4->setMargin(0);
    imageROIlayoutRow4->setContentsMargins(0,0,0,0);
    //QHBoxLayout *imageROIoffsetYInputLayout = new QHBoxLayout;
    imageROIoffsetYLabel = new QLabel(tr("Image ROI offsetY [px]:"));
    imageROIoffsetYLabel->setMinimumWidth(120);
    imageROIoffsetYInputBox = new QSpinBox();
    imageROIoffsetYInputBox->setFixedWidth(60);
    imageROIoffsetYInputBox->setMinimum(0);
    imageROIoffsetYInputBox->setMaximum(std::numeric_limits<short>::max());
    imageROIoffsetYInputBox->setSingleStep(16);
    imageROIoffsetYInputBox->setKeyboardTracking(false); // to not invoke change upon each digit input
    imageROIoffsetYMaxLabel = new QLabel(tr("/ 0"));
    imageROIoffsetYMaxLabel->setFixedWidth(40);
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
    imageROIlayoutRow5->setMargin(0);
    imageROIlayoutRow5->setContentsMargins(0,0,0,0);
    binningLabel = new QLabel(tr("Binning:"));
    binningLabel->setFixedWidth(70);
    binningBox = new QComboBox();
    binningBox->addItem(QString("1 (no binning)"));
    binningBox->addItem(QString("2"));
    binningBox->addItem(QString("4"));
    binningBox->setMinimumWidth(140);
    imageROIlayoutRow5->addWidget(binningLabel);
    imageROIlayoutRow5->addWidget(binningBox);
    imageROIlayoutRow5->addStretch();
    acquisitionLayout->addLayout(imageROIlayoutRow5);

    /////////////////////////////////////////////////
    QHBoxLayout *imageROIlayoutRow6 = new QHBoxLayout;
    imageROIlayoutRow6->setMargin(0);
    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Raised);
    imageROIlayoutRow6->addWidget(line2);
    acquisitionLayout->addLayout(imageROIlayoutRow6);

    QHBoxLayout *imageROIlayoutRow7 = new QHBoxLayout;
    frameRateLabel = new QLabel("Resulting (maximum achievable) framerate:");
    frameRateValueLabel = new QLabel(QString::number(camera->getResultingFrameRateValue()));
    imageROIlayoutRow7->addWidget(frameRateLabel);
    imageROIlayoutRow7->addWidget(frameRateValueLabel);
    imageROIlayoutRow7->addStretch();
//    acquisitionLayout->addRow(frameRateLabel, frameRateValueLabel);
    acquisitionLayout->addLayout(imageROIlayoutRow7);

    acquisitionGroup->setLayout(acquisitionLayout);
    mainLayout->addWidget(acquisitionGroup);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    triggerGroup = new QGroupBox("3. Image Acquisition Triggering and Framerate setting");
    QFormLayout *triggerGroupLayout = new QFormLayout();
    triggerGroupLayout->setMargin(10);
    triggerGroupLayout->setContentsMargins(5,5,5,5);

    SWTradioButton = new QRadioButton("Software triggering:", this);
    SWTradioButton->setFixedHeight(20);
    SWTradioButton->setDisabled(false);
    SWTradioButton->setChecked(!camera->isHardwareTriggerEnabled());

    // This would normally never be used in case of hardware triggering, so it is moved to the "software triggering"
    // section and kept disabled
    SWTframerateEnabled = new QCheckBox("Limit framerate to: ");
    // NOTE: isEnabledAcquisitionFrameRate() HAS TO BE corresponding to the QSettings state,
    // and NOT the inherent camera state here! because loadSettings resets it like so beforehand
    // It is necessary because opening the camera once as part of stereo will wipe this internal
    // setting of the camera to false (it has to, to let it see the ResultingFramerate)
    SWTframerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
    SWTframerateEnabled->setEnabled(!camera->isHardwareTriggerEnabled()); //
    SWTframerateBox = new QSpinBox();
    SWTframerateLayout = new QHBoxLayout;
    SWTframerateLayout->setContentsMargins(0,0,0,0);
    QSpacerItem *sp4 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    SWTframerateLayout->addSpacerItem(sp4);
    SWTframerateLayout->addWidget(SWTframerateEnabled);
    SWTframerateLayout->addWidget(SWTframerateBox);
    SWTframerateLayout->addStretch();
    // SWTframerateLayout->addSpacerItem(sp);
    SWTframerateBox->setMinimum(std::max(1, camera->getAcquisitionFPSMin()));
    SWTframerateBox->setMaximum(std::numeric_limits<short>::max());
    SWTframerateBox->setSingleStep(1);
    SWTframerateBox->setValue(camera->getAcquisitionFPSValue());
    // NOTE: isEnabledAcquisitionFrameRate() HAS TO BE corresponding to the QSettings state,
    // and NOT the inherent camera state here! because loadSettings resets it like so beforehand
    // It is necessary because opening the camera once as part of stereo will wipe this internal
    // setting of the camera to false (it has to, to let it see the ResultingFramerate)
    SWTframerateBox->setEnabled(camera->isEnabledAcquisitionFrameRate()); //
    SWTframerateBox->setFixedWidth(60);
//    camera->enableAcquisitionFrameRate(false); //

    triggerGroupLayout->addRow(SWTradioButton, SWTframerateLayout);

    /////////////////////////////////////////////////
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Raised);
    triggerGroupLayout->addRow(line);

    HWTradioButton = new QRadioButton("Hardware triggering:", this);
    HWTradioButton->setFixedHeight(20);
    HWTradioButton->setDisabled(false);
    HWTradioButton->setChecked(camera->isHardwareTriggerEnabled());

    HWTframerateLabel = new QLabel(tr("Set framerate to: "));
    HWTframerateBox = new QSpinBox();
    HWTframerateLayout = new QHBoxLayout;
    QSpacerItem *sp5 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    // HWTframerateLayout->addSpacerItem(sp);
    HWTframerateBox->setMinimum(1);
    HWTframerateBox->setMaximum(5000);
    HWTframerateBox->setSingleStep(1);
    HWTframerateBox->setEnabled(false);
    HWTframerateBox->setFixedWidth(60);
    QSpacerItem *sp7 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    HWTstartStopButton = new QPushButton("Start Image Acquisition");
    HWTstartStopButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    HWTstartStopButton->setMinimumWidth(190);
    HWTstartStopButton->setMinimumHeight(22);
    HWTstartStopButton->setEnabled(camera->isHardwareTriggerEnabled() && MCUSettings->isConnected());

    HWTframerateLayout->addSpacerItem(sp5);
    HWTframerateLayout->addWidget(HWTframerateLabel);
    HWTframerateLayout->addWidget(HWTframerateBox);
    HWTframerateLayout->addSpacerItem(sp7);
    HWTframerateLayout->addWidget(HWTstartStopButton);
//    HWTframerateLayout->addStretch();

    triggerGroupLayout->addRow(HWTradioButton, HWTframerateLayout);

    HWTgroupLayout = new QFormLayout();
    HWTgroupLayout->setMargin(0);
    HWTgroupLayout->setContentsMargins(0,0,0,0);

    QSpacerItem *sp6 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    HWTlineSourceLabel = new QLabel(tr("Source: "));
    HWTlineSourceBox = new QComboBox();
    HWTlineSourceBox->addItem(QString("Select"));
    for(int i=1; i<5;i++) {
        HWTlineSourceBox->addItem(QString("Line") + QString::number(i));
    }
    HWTlineSourceBox->setFixedWidth(80);

    HWTtimeSpanLabel = new QLabel(tr("Runtime [min] (0=inf.): "));
    QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    HWTtimeSpanBox = new QDoubleSpinBox();
    HWTtimeSpanBox->setMinimum(0);
    HWTtimeSpanBox->setMaximum(std::numeric_limits<double>::max());
    HWTtimeSpanBox->setSingleStep(0.1);
    HWTtimeSpanBox->setEnabled(false);
    HWTtimeSpanBox->setFixedWidth(70);

    QHBoxLayout *HWTrow1 = new QHBoxLayout;
    HWTrow1->addSpacerItem(sp6);
    HWTrow1->addWidget(HWTlineSourceLabel);
    HWTrow1->addWidget(HWTlineSourceBox);
    HWTrow1->addSpacerItem(sp1);
    HWTrow1->addWidget(HWTtimeSpanLabel);
    HWTrow1->addWidget(HWTtimeSpanBox);
    HWTgroupLayout->addRow(HWTrow1);

    triggerGroupLayout->addItem(HWTgroupLayout);

    HWTframerateLayout->setEnabled(camera->isHardwareTriggerEnabled());
    HWTgroupLayout->setEnabled(camera->isHardwareTriggerEnabled());

    triggerGroup->setLayout(triggerGroupLayout);
    mainLayout->addWidget(triggerGroup);
    //triggerGroup->setDisabled(true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    analogGroup = new QGroupBox("4. Image Analog Control");
    QFormLayout *analogLayout = new QFormLayout();
    analogLayout->setContentsMargins(10,5,10,5);
    QHBoxLayout *gainLayout = new QHBoxLayout();
    gainLayout->setContentsMargins(0,0,0,0);

    QLabel *gainLabel = new QLabel(tr("Gain [dB]:"));
    gainLabel->setMinimumWidth(80);

    // We do not set its value yet, because the camera may not have a valid value yet (not opened)
    gainBox = new QDoubleSpinBox();
    gainBox->setFixedWidth(60);
    gainBox->setMinimum(0.0);
    //gainBox->setMaximum(floor(camera->getGainMax()));
    gainBox->setSingleStep(0.01);
    //gainBox->setValue(camera->getGainValue());

    autoGainOnceButton = new QPushButton("Auto Gain (Once)");
    autoGainOnceButton->setFixedWidth(150);
    gainLayout->addWidget(gainBox);
    gainLayout->addWidget(autoGainOnceButton);
    analogLayout->addRow(gainLabel, gainLayout);

    analogGroup->setLayout(analogLayout);
    //analogGroup->setDisabled(true);
    mainLayout->addWidget(analogGroup);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    saveButton = new QPushButton(tr("Save to File"));
    loadButton = new QPushButton(tr("Load from File"));
    saveButton->setFixedWidth(100);
    loadButton->setFixedWidth(100);
    saveButton->setDisabled(true);
    loadButton->setDisabled(true);

    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addWidget(loadButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    QLabel *imageROIWarningLabel = new QLabel(tr("Warning: If you are using Hardware-triggered image acquisition, please restart Image Acquisition\n    Triggering whenever Image Acquisition ROI is modified.\nWarning: if Image Acquisition ROI or Binning is altered, a new camera calibration is necessary\n    for proper undistortion."));
    SupportFunctions::setSmallerLabelFontSize(imageROIWarningLabel);
    mainLayout->addWidget(imageROIWarningLabel);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    updateImageROISettingsMax();
    updateImageROISettingsValues();

    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(setExposureTimeValue(int)));
    connect(binningBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBinningModeChange(int)));
    connect(gainBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));
    connect(gainBox, SIGNAL(valueChanged(double)), this, SLOT(setGainValue(double)));

    connect(imageROIwidthInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIwidth(int)));
    connect(imageROIheightInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIheight(int)));
    connect(imageROIoffsetXInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetX(int)));
    connect(imageROIoffsetYInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetY(int)));

    connect(SWTframerateEnabled, SIGNAL(toggled(bool)), this, SLOT(SWTframerateEnabledToggled(bool)));
    connect(SWTframerateBox, SIGNAL(valueChanged(int)), this, SLOT(setAcquisitionFPSValue(int)));

    connect(HWTlineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));
    connect(HWTradioButton, SIGNAL(toggled(bool)), this, SLOT(onHWTenabledChange(bool)));

    connect(saveButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::saveButtonClick);
    connect(loadButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::loadButtonClick);
    connect(autoGainOnceButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::autoGainOnce);
    connect(autoExposureOnceButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::autoExposureOnce);
    connect(MCUConfigButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::onMCUConfig);
    connect(HWTstartStopButton, SIGNAL(clicked()), this, SLOT(HWTstartStopButtonClicked()));

    onHWTenabledChange(camera->isHardwareTriggerEnabled());
}

void SingleCameraSettingsDialog::updateForms() {
    if(!camera->isOpen())
        return;

    updateHWTStartStopRelatedWidgets();
    updateMCUConnDisconnButtonState();

    SWTframerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
    applicationSettings->setValue("SingleCameraSettingsDialog.SWTframerateEnabled", camera->isEnabledAcquisitionFrameRate());
    if(!HWTrunning) {
        SWTframerateBox->setMinimum(std::max(1, camera->getAcquisitionFPSMin()));
        SWTframerateBox->setMaximum(camera->getAcquisitionFPSMax());
        SWTframerateBox->setValue(camera->getAcquisitionFPSValue());
    }

    gainBox->setMinimum(camera->getGainMin());
    gainBox->setMaximum(camera->getGainMax());
    gainBox->setValue(camera->getGainValue());

    exposureInputBox->setMinimum(camera->getExposureTimeMin());
    exposureInputBox->setMaximum(camera->getExposureTimeMax());
    exposureInputBox->setValue(camera->getExposureTimeValue());

    SWTradioButton->setChecked(!camera->isHardwareTriggerEnabled());
    HWTradioButton->setChecked(camera->isHardwareTriggerEnabled());

    // Note: is this surely good here?
    if(camera->isHardwareTriggerEnabled()) {
        HWTlineSourceBox->setCurrentText(QString::fromStdString(camera->getLineSource().c_str()));
    }

    lastUsedBinningVal = camera->getBinningVal();
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

        camera->saveToFile(filename.toStdString().c_str());
    }
}

// Loading a Basler pfs camera settings file
void SingleCameraSettingsDialog::loadButtonClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature File"), "", tr("PFS files (*.pfs)"));

    if(!filename.isEmpty()) {

        camera->loadFromFile(filename.toStdString().c_str());
        updateForms();
    }
}

void SingleCameraSettingsDialog::autoGainOnce() {

    camera->autoGainOnce();
    gainBox->setValue(camera->getGainValue());
}

void SingleCameraSettingsDialog::autoExposureOnce() {

    camera->autoExposureOnce();
    exposureInputBox->setValue(camera->getExposureTimeValue());
}

// Instead of rejecting the dialog, thus closing it, we only hide it and show it again, so that all settings of the current camera are still in the forms
void SingleCameraSettingsDialog::reject() {
    saveSettings();
    //QDialog::reject();
    hide();
}

// TODO: This should be equal to camera closing in case of a stereo camera.. but this is a single camera right now
void SingleCameraSettingsDialog::accept() {
    if(HWTrunning) {
        stopHardwareTrigger();
    }

    saveSettings();
    QDialog::accept();
}

void SingleCameraSettingsDialog::updateFrameRateValue() {
    frameRateValueLabel->setText(QString::number(camera->getResultingFrameRateValue()));

    std::cout << "resulting framerate " << camera->getResultingFrameRateValue() << std::endl;

    // commented out, reason:
    // TODO: problematic, as resulting framerate is affected by framerate limit, which creates a "loop" of events,
    // setting the max on gui as the current fps
    //HWTframerateBox->setMaximum(static_cast<int>(floor(camera->getResultingFrameRateValue())));
    //SWTframerateBox->setMaximum(static_cast<int>(floor(camera->getResultingFrameRateValue())));

    // instead:
    int supposedMaxFPS = static_cast<int>(floor(camera->getAcquisitionFPSMax()));
    if(/*!camera->isOpen() ||*/ supposedMaxFPS <= 0) {
        supposedMaxFPS = std::numeric_limits<short>::max();
    }
    HWTframerateBox->setMaximum(supposedMaxFPS);
    SWTframerateBox->setMaximum(supposedMaxFPS);
}

void SingleCameraSettingsDialog::onLineSourceChange(int index) {
    if(index!=0) {
        camera->setLineSource(HWTlineSourceBox->itemText(index).toStdString().c_str());
        HWTframerateBox->setEnabled(camera->isHardwareTriggerEnabled());
        HWTtimeSpanBox->setEnabled(camera->isHardwareTriggerEnabled());
        HWTstartStopButton->setEnabled(MCUSettings->isConnected());
    } else {
        HWTframerateBox->setEnabled(false);
        HWTtimeSpanBox->setEnabled(false);
        HWTstartStopButton->setEnabled(false);
    }
}

void SingleCameraSettingsDialog::HWTstartStopButtonClicked() {
    if(!HWTrunning) {
        startHardwareTrigger();
    } else {
        stopHardwareTrigger();
    }
}

// Starts the hardware trigger
// To start a hardware trigger signal for the camera, the command string is build and send over the serial port to the microcontroller
// The command structure is: <TX...X...> with the number of images and their delay specified i.e. <TX1000X33000> for thousand frame with ~30fps
// Corrected by Gabor Benyei: the second parameter is expected in microseconds on the microcontroller side (".attach_us" needs microseconds)
void SingleCameraSettingsDialog::startHardwareTrigger() {

    double runtime = HWTtimeSpanBox->value();
    double fps = HWTframerateBox->value();

    // Calculate the delay and number of frames from the input values
    int delay = (int) (((1000.0f / fps) * 1000.0f) / 2.0f);
    int count = (int) ((runtime * 60000000) / (delay * 2)); // corrected following SBelgers in previous commit

    QString cmd = "<TX" + QString::number(count) + "X" + QString::number(delay) + ">";
    std::cout << "Sending hardware trigger command: " << cmd.toStdString() << std::endl;

    emit onHardwareTriggerEnable();
    emit onHardwareTriggerStart(cmd);

    HWTrunning = true;
    updateHWTStartStopRelatedWidgets();
}

// Stops the hardware trigger signal by sending a stop signal to the microcontroller
void SingleCameraSettingsDialog::stopHardwareTrigger() {

    emit onHardwareTriggerDisable();
    emit onHardwareTriggerStop(QString("<SX>"));

    HWTrunning = false;
    updateHWTStartStopRelatedWidgets();
}

void SingleCameraSettingsDialog::updateHWTStartStopRelatedWidgets() {
    if(HWTrunning) {
        HWTframerateBox->setEnabled(false);
        HWTlineSourceBox->setEnabled(false);
        HWTtimeSpanBox->setEnabled(false);
        HWTstartStopButton->setText("Stop Image Acquisition");
        HWTstartStopButton->setStyleSheet(
                "QPushButton { background-color: #c3f558; border: 1px solid #757575; border-radius: 5px;}");
    } else {
        HWTframerateBox->setEnabled(camera->isHardwareTriggerEnabled());
        HWTlineSourceBox->setEnabled(camera->isHardwareTriggerEnabled());
        HWTtimeSpanBox->setEnabled(camera->isHardwareTriggerEnabled());
        HWTstartStopButton->setText("Start Image Acquisition");
        HWTstartStopButton->setStyleSheet(
                "QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    }
}

void SingleCameraSettingsDialog::connectMCU() {
    if(MCUSettings->isConnected())
        return;
    MCUConnDisconnButtonClicked();
}

void SingleCameraSettingsDialog::startHWT() {
    if(HWTrunning)
        return;
    HWTstartStopButtonClicked();
}

void SingleCameraSettingsDialog::onHWTenabledChange(bool state) {

//    if(state == camera->isHardwareTriggerEnabled())
//        return;

    // if set programmatically, we need to change radio button states here
    SWTradioButton->blockSignals(true);
    SWTradioButton->blockSignals(true);
    SWTradioButton->setChecked(!state);
    HWTradioButton->setChecked(state);
    SWTradioButton->blockSignals(false);
    SWTradioButton->blockSignals(false);

    if(HWTrunning) {
        stopHardwareTrigger();
    }

    SWTframerateEnabled->setEnabled(!state);
    SWTframerateBox->setEnabled(!state && camera->isEnabledAcquisitionFrameRate());
    if(state || !SWTframerateEnabled->isChecked()) {
        //camera->setAcquisitionFPSValue(camera->getAcquisitionFPSMax());
        setAcquisitionFPSValue(camera->getAcquisitionFPSMax());
    } else {
        setAcquisitionFPSValue(SWTframerateBox->value());
    }

    HWTframerateLayout->setEnabled(state);
    HWTgroupLayout->setEnabled(state); // Note: not sure if this does anything
    HWTframerateLabel->setEnabled(state);
    HWTlineSourceLabel->setEnabled(state);
    HWTtimeSpanLabel->setEnabled(state);
    HWTlineSourceBox->setEnabled(state);
    HWTstartStopButton->setEnabled(state && MCUSettings->isConnected());

    HWTtimeSpanBox->setEnabled(state);
    HWTframerateBox->setEnabled(state);
    camera->enableHardwareTrigger(state);

    if(state) {
        emit onHardwareTriggerEnable();
    } else {
        emit onHardwareTriggerDisable();
    }
    this->update();
}

void SingleCameraSettingsDialog::loadSettings() {

    SWTradioButton->setChecked(!SupportFunctions::readBoolFromQSettings("SingleCameraSettingsDialog.hwTriggerEnabled", camera->isHardwareTriggerEnabled(), applicationSettings));
    HWTradioButton->setChecked(SupportFunctions::readBoolFromQSettings("SingleCameraSettingsDialog.hwTriggerEnabled", camera->isHardwareTriggerEnabled(), applicationSettings));
    camera->enableHardwareTrigger(HWTradioButton->isChecked());

    HWTlineSourceBox->setCurrentText(applicationSettings->value("SingleCameraSettingsDialog.lineSource", QString::fromStdString(camera->getLineSource().c_str())).toString());
    camera->setLineSource(HWTlineSourceBox->currentText().toStdString().c_str());

    HWTframerateBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerFramerate", HWTframerateBox->value()).toInt());
    HWTtimeSpanBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerTime", HWTtimeSpanBox->value()).toDouble());

    gainBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.analogGain", camera->getGainValue()).toDouble());
    camera->setGainValue(gainBox->value());

    exposureInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.analogExposure", camera->getExposureTimeValue()).toInt());
    camera->setExposureTimeValue(exposureInputBox->value());

    int lastUsedBinningVal = applicationSettings->value("SingleCameraSettingsDialog.binningVal", camera->getBinningVal()).toInt();
    camera->setBinningVal(lastUsedBinningVal);
    int tempidx = 0;
    if(lastUsedBinningVal==2 || lastUsedBinningVal==3)
        tempidx = 1;
    else if(lastUsedBinningVal==4)
        tempidx = 2;
    binningBox->setCurrentIndex(tempidx);

    imageROIwidthInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIwidth", camera->getImageROIwidthMax() ).toInt());
    imageROIheightInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIheight", camera->getImageROIheightMax()).toInt());
    imageROIoffsetXInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIoffsetX", 0).toInt());
    imageROIoffsetYInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIoffsetY", 0).toInt());

    // The safest is to enable limiting by default, as first opening a high speed hi-res camera can just freeze the computer
    bool m_SWTframerateEnabled = SupportFunctions::readBoolFromQSettings("SingleCameraSettingsDialog.SWTframerateEnabled", true, applicationSettings);
    SWTframerateEnabled->setChecked(m_SWTframerateEnabled);
    camera->enableAcquisitionFrameRate(m_SWTframerateEnabled);

    // 50 FPS is good for a first start, for the same reasons
    SWTframerateBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.acquisitionFramerate", "50").toInt());
    camera->setAcquisitionFPSValue(SWTframerateBox->value());

    // TODO load the pfs file as an backup if no appication settings are available?
}

// Save camera settings to the application settings
// Also saves camera settings as pfs file into the application settings directory
void SingleCameraSettingsDialog::saveSettings() {

    applicationSettings->setValue("SingleCameraSettingsDialog.hwTriggerEnabled", HWTradioButton->isChecked());
    applicationSettings->setValue("SingleCameraSettingsDialog.lineSource", HWTlineSourceBox->currentText());
    applicationSettings->setValue("SingleCameraSettingsDialog.hwTriggerFramerate", HWTframerateBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.hwTriggerTime", HWTtimeSpanBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.analogGain", gainBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.analogExposure", exposureInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.SWTframerateEnabled", SWTframerateEnabled->isChecked());
    applicationSettings->setValue("SingleCameraSettingsDialog.acquisitionFramerate", SWTframerateBox->value());

    applicationSettings->setValue("SingleCameraSettingsDialog.binningVal", lastUsedBinningVal);
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIwidth", imageROIwidthInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIheight", imageROIheightInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIoffsetX", imageROIoffsetXInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIoffsetY", imageROIoffsetYInputBox->value());

    applicationSettings->setValue("SingleCameraSettingsDialog.settingsDirectory", settingsDirectory.path());

    QString configFile = settingsDirectory.filePath(camera->getFriendlyName() + ".pfs");
    configFile.replace(" ", "");
    std::cout<<"Saving config to settings directory: "<< configFile.toStdString() <<std::endl;
    camera->saveToFile(configFile.toStdString().c_str());
}

void SingleCameraSettingsDialog::onSettingsChange() {
    loadSettings();
}

SingleCameraSettingsDialog::~SingleCameraSettingsDialog() = default;


void SingleCameraSettingsDialog::onSetImageROIwidth(int val) {
//    imageROIwidthInputBox->editingFinished();
    camera->setImageROIwidth(val);
    // NOTE: qt will not consider the programmatic change of the value as a user event to handle
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
    updateFrameRateValue(); // important
}

void SingleCameraSettingsDialog::onSetImageROIheight(int val) {
    camera->setImageROIheight(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
    updateFrameRateValue(); // important
}

void SingleCameraSettingsDialog::onSetImageROIoffsetX(int val) {
    camera->setImageROIoffsetX(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
}

void SingleCameraSettingsDialog::onSetImageROIoffsetY(int val) {
    camera->setImageROIoffsetY(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
}

void SingleCameraSettingsDialog::updateImageROISettingsMax() {

    imageROIwidthInputBox->setMaximum(camera->getImageROIwidthMax());
    imageROIheightInputBox->setMaximum(camera->getImageROIheightMax());
    imageROIoffsetXInputBox->setMaximum(camera->getImageROIwidthMax() - camera->getImageROIwidth());
    imageROIoffsetYInputBox->setMaximum(camera->getImageROIheightMax() - camera->getImageROIheight());

    imageROIwidthMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIwidthMax()));
    imageROIheightMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIheightMax()));
    imageROIoffsetXMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIwidthMax() - camera->getImageROIwidth()));
    imageROIoffsetYMaxLabel->setText(QString("/ ") + QString::number(camera->getImageROIheightMax() - camera->getImageROIheight()));
}

void SingleCameraSettingsDialog::updateImageROISettingsValues() {
    if(!camera->isOpen())
        return;

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

void SingleCameraSettingsDialog::onBinningModeChange(int index) {
    int binningVal = 1;
    if(index==1)
        binningVal = 2;
    else if(index==2)
        binningVal = 4;

    camera->setBinningVal(binningVal);

    if(lastUsedBinningVal > binningVal) {
        //qDebug() << "Inflating image ROI";
        // First set maximum values for the widgets
        // (first setting the actual value would take no effect as the maximum does not let it happen)
        updateImageROISettingsMax();
        // Then set the values on GUI according to the (already automatically changed) camera image ROI parameters 
        updateImageROISettingsValues();
    } else { 
        //qDebug() << "Shrinking image ROI";
        // First set the values on GUI according to the (already automatically changed) camera image ROI parameters
        updateImageROISettingsValues();
        updateImageROISettingsMax();
        // Then set maximum values for the widgets (e.g. first setting the maximum
        // would auto-reset the value if that was a bigger number... and that would cause strange behaviour of the GUI)
    }

    // GB NOTE: here we could tell cameraview that it should expect different image size. But it is now programmed to be adaptive
    lastUsedBinningVal = binningVal;
    updateCamImageRegionsWidget();
    updateFrameRateValue(); // only update when camera has updated too

    // Change: okay, we tell the cameraview, but only for properly letting it know where the positioning guide should be drawn
    updateSensorSize();
}

void SingleCameraSettingsDialog::updateSensorSize() {
    if(!camera->isOpen())
        return;

    emit onSensorSizeChanged(QSize(camera->getImageROIwidthMax(), camera->getImageROIheightMax()));
}

void SingleCameraSettingsDialog::updateCamImageRegionsWidget() {
    if(!camera->isOpen())
        return;

    const QSize sensorSize = QSize(camera->getImageROIwidthMax(), camera->getImageROIheightMax() );
    const QRect imageAcqROI1Rect = QRect(camera->getImageROIoffsetX(), camera->getImageROIoffsetY(),
                                         camera->getImageROIwidth(), camera->getImageROIheight() );
    camImageRegionsWidget->setImageMaxSize(sensorSize);
    camImageRegionsWidget->recalculateDrawingArea();
    camImageRegionsWidget->setImageAcqROI1Rect(imageAcqROI1Rect);
}

void SingleCameraSettingsDialog::setLimitationsWhileTracking(bool state) {
    //hwTriggerGroup->setDisabled(state);
    //analogGroup->setDisabled(state);

    //exposureLabel->setDisabled(state);
    //exposureInputBox->setDisabled(state);
    //autoExposureOnceButton->setDisabled(state);

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

void SingleCameraSettingsDialog::setExposureTimeValue(int value) {

    // this is necessary if we programmatically set it
    exposureInputBox->blockSignals(true);
    exposureInputBox->setValue(value);
    exposureInputBox->blockSignals(false);

    camera->setExposureTimeValue(value);
    updateFrameRateValue();
}

void SingleCameraSettingsDialog::setGainValue(double value) {

    // this is necessary if we programmatically set it
    gainBox->blockSignals(true);
    gainBox->setValue(value);
    gainBox->blockSignals(false);

    camera->setGainValue(value);
    updateFrameRateValue();
}

void SingleCameraSettingsDialog::setAcquisitionFPSValue(int value) {

    // this is necessary if we programmatically set it
    SWTframerateBox->blockSignals(true);
    SWTframerateBox->setValue(value);
    SWTframerateBox->blockSignals(false);

    camera->setAcquisitionFPSValue(value);
    updateFrameRateValue();
}

void SingleCameraSettingsDialog::MCUConnDisconnButtonClicked() {
    if(MCUSettings->isConnected()) {
        stopHardwareTrigger();
        MCUSettings->doDisconnect();
    } else {
        MCUSettings->doConnect();
    }
    updateMCUConnDisconnButtonState();
}

void SingleCameraSettingsDialog::updateMCUConnDisconnButtonState() {
    if(MCUSettings->isConnected()) {
        MCUConnDisconnButton->setText("Disconnect");
        MCUConnDisconnButton->setStyleSheet("QPushButton { background-color: #c3f558; border: 1px solid #757575; border-radius: 5px;}");
        HWTstartStopButton->setEnabled(camera->isHardwareTriggerEnabled());
    } else {
        MCUConnDisconnButton->setText("Connect");
        MCUConnDisconnButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
        HWTstartStopButton->setEnabled(false);
    }
}

void SingleCameraSettingsDialog::setHWTlineSource(int lineSourceNum) {
    if(HWTrunning)
        return;
    HWTlineSourceBox->setCurrentIndex(lineSourceNum);
}

void SingleCameraSettingsDialog::setHWTruntime(double runtimeMinutes) {
    if(HWTrunning)
        return;
    HWTtimeSpanBox->setValue(runtimeMinutes);
}

void SingleCameraSettingsDialog::setHWTframerate(int fps) {
    if(HWTrunning)
        return;
    HWTframerateBox->setValue(fps);
}

void SingleCameraSettingsDialog::SWTframerateEnabledToggled(bool state) {

    // this is necessary if we programmatically set it
    SWTframerateEnabled->blockSignals(true);
    SWTframerateEnabled->setChecked(state);
    SWTframerateEnabled->blockSignals(false);

    camera->enableAcquisitionFrameRate(state);
    updateFrameRateValue();

    applicationSettings->setValue("SingleCameraSettingsDialog.SWTframerateEnabled", state);

    SWTframerateBox->setEnabled(state);
    if(state)
        setAcquisitionFPSValue(SWTframerateBox->value());
}
