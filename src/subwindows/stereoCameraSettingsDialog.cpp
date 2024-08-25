
#include <iostream>
#include <pylon/TlFactory.h>
#include <pylon/Container.h>
#include "stereoCameraSettingsDialog.h"
#include "../supportFunctions.h"

// Creates a new stereo camera settings dialog
// The dialog setups the stereo camera and starts fetching image frames through a hardware trigger
// A stereo camera consists of two physical cameras, which are chosen in this widget
// For the hardware trigger a connection to a microcontroller is necessary which is handled through the MCUSettings object
// This is ensured in this form by disabling the start hardware trigger buttons until the cameras are opened
StereoCameraSettingsDialog::StereoCameraSettingsDialog(StereoCamera *cameraPtr, MCUSettingsDialog *MCUSettings, QWidget *parent) :
        QDialog(parent),
        camera(cameraPtr),
        MCUSettings(MCUSettings),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    settingsDirectory = QDir(applicationSettings->value("StereoCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
// mkdir(".") DOES NOT WORK ON MACOS, ONLY WINDOWS. (Reported on MacOS 12.7.6 and Windows 10)
//        settingsDirectory.mkdir(".");
        QDir().mkpath(settingsDirectory.absolutePath());
    }

#ifdef Q_OS_WIN // Q_OS_MACOS
    setMinimumSize(500, 700);
#else
    setMinimumSize(500, 870);
#endif

    setWindowTitle(QString("Stereo Camera Settings"));

    createForm();

    // Update the device list from which main and secondary camera are chosen
    updateDevicesBox();
    loadSettings();
    updateForms();
}

void StereoCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    MCUConnGroup = new QGroupBox("1. Microcontroller Connection (needed for Hardware-triggered image acquisition)");
    QFormLayout *MCUConnGroupLayout = new QFormLayout();
    MCUConnGroupLayout->setMargin(10);
    MCUConnGroupLayout->setContentsMargins(5,5,5,5);

    QSpacerItem *sp10 = new QSpacerItem(90, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
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
    MCUConfigButton->setMinimumHeight(22);
    QSpacerItem *sp9 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    MCUConnDisconnButton = new QPushButton("Connect"); // Will change to disconnect when connected
    MCUConnDisconnButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    MCUConnDisconnButton->setFixedWidth(150);
    MCUConnDisconnButton->setMinimumHeight(22);

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

    QGroupBox *cameraGroup = new QGroupBox("2. Camera Selection");
    QFormLayout *cameraLayout = new QFormLayout();
    cameraLayout->setMargin(10);
    cameraLayout->setContentsMargins(5,5,5,5);

    QLabel *mainCameraLabel = new QLabel(tr("Main:"));
    mainCameraLabel->setFixedWidth(90);
    mainCameraBox = new QComboBox();
    mainCameraBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *secondaryCameraLabel = new QLabel(tr("Secondary:"));
    secondaryCameraLabel->setFixedWidth(90);
    secondaryCameraBox = new QComboBox();
    secondaryCameraBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(mainCameraBox, SIGNAL(currentIndexChanged(int)), this, SLOT(mainCameraBoxCurrentIndexChanged(int)));
    connect(secondaryCameraBox, SIGNAL(currentIndexChanged(int)), this, SLOT(secondaryCameraBoxCurrentIndexChanged(int)));

    cameraLayout->addRow(mainCameraLabel, mainCameraBox);
    cameraLayout->addRow(secondaryCameraLabel, secondaryCameraBox);

    QWidget *tmp = new QWidget();
    QHBoxLayout *cameraButtonLayout = new QHBoxLayout();
    cameraButtonLayout->setMargin(0);
    cameraButtonLayout->setContentsMargins(0,0,0,0);
    updateDevicesButton = new QPushButton(tr("Refresh Devices"));
    updateDevicesButton->setStyleSheet("QPushButton { border: 1px solid #757575; border-radius: 5px;}");
    updateDevicesButton->setMinimumWidth(120);
    updateDevicesButton->setMinimumHeight(22);
    QSpacerItem *sp11 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    cameraOpenCloseButton = new QPushButton("Open Camera Devices");
    cameraOpenCloseButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    cameraOpenCloseButton->setMinimumWidth(190);
    cameraOpenCloseButton->setMinimumHeight(22);
//    cameraOpenCloseButton->setLayout(new QGridLayout);
//    cameraOpenCloseButtonLabel = new QLabel("Open/Close");
//    cameraOpenCloseButtonLabel->setStyleSheet("background-color:#f5ab87;"); // light red (alternative: orange: #ebbd3f)
//    cameraOpenCloseButtonLabel->setAlignment(Qt::AlignCenter);
//    cameraOpenCloseButton->layout()->addWidget(cameraOpenCloseButtonLabel);
//    cameraOpenCloseButton->layout()->setContentsMargins(5,5,5,5);
    cameraButtonLayout->addWidget(updateDevicesButton);
    cameraButtonLayout->addSpacerItem(sp11);
    cameraButtonLayout->addWidget(cameraOpenCloseButton);
    tmp->setLayout(cameraButtonLayout);
    cameraLayout->addWidget(tmp);

    updateDevicesBox();

    cameraGroup->setLayout(cameraLayout);
    mainLayout->addWidget(cameraGroup);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    acquisitionGroup = new QGroupBox("3. Image Acquisition Control");
    QVBoxLayout *acquisitionLayout = new QVBoxLayout;
    acquisitionLayout->setMargin(10);
    acquisitionLayout->setContentsMargins(5,5,5,5);

    QHBoxLayout *exposureInputLayout = new QHBoxLayout;
    exposureInputLayout->setMargin(0);
    exposureInputLayout->setContentsMargins(0,0,0,0);
    exposureLabel = new QLabel(tr("Exposure [µs]:")); 
    exposureLabel->setFixedWidth(100);
    exposureInputBox = new QSpinBox();
    exposureInputBox->setMinimum(0);
    exposureInputBox->setMaximum(std::numeric_limits<short>::max());
    exposureInputBox->setSingleStep(1);
//    exposureInputBox->setValue(camera->getExposureTimeValue()); // we cannot know as we don't have an opened camera yet
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
    camImageRegionsWidget->setFixedHeight(90);
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

    triggerGroup = new QGroupBox("4. Image Acquisition Triggering and Framerate setting");
    QFormLayout *triggerGroupLayout = new QFormLayout();
    triggerGroupLayout->setMargin(10);
    triggerGroupLayout->setContentsMargins(5,5,5,5);

    SWTradioButton = new QRadioButton("Software triggering:", this);
    SWTradioButton->setFixedHeight(20);
    SWTradioButton->setDisabled(true);
    SWTradioButton->setChecked(false);

    // This would normally never be used in case of hardware triggering, so it is moved to the "software triggering"
    // section and kept disabled
    SWTframerateEnabled = new QCheckBox("Limit framerate to: ");
    SWTframerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate()); // Should normally be always false. TODO: check?
    SWTframerateEnabled->setEnabled(false); //
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
//    SWTframerateBox->setEnabled(camera->isEnabledAcquisitionFrameRate());
    SWTframerateBox->setEnabled(false); //
    SWTframerateBox->setFixedWidth(60);
    camera->enableAcquisitionFrameRate(false); //

    triggerGroupLayout->addRow(SWTradioButton, SWTframerateLayout);

    /////////////////////////////////////////////////
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Raised);
    triggerGroupLayout->addRow(line);

    HWTradioButton = new QRadioButton("Hardware triggering:", this);
    HWTradioButton->setFixedHeight(20);
    HWTradioButton->setDisabled(false);
    HWTradioButton->setChecked(true);
//    triggerGroupLayout->addRow(HWTradioButton);

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
    HWTstartStopButton->setEnabled(MCUSettings->isConnected());

    HWTframerateLayout->addSpacerItem(sp5);
    HWTframerateLayout->addWidget(HWTframerateLabel);
    HWTframerateLayout->addWidget(HWTframerateBox);
    HWTframerateLayout->addSpacerItem(sp7);
    HWTframerateLayout->addWidget(HWTstartStopButton);
//    HWTframerateLayout->addStretch();

    triggerGroupLayout->addRow(HWTradioButton, HWTframerateLayout);

    QFormLayout *HWTgroupLayout = new QFormLayout();
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

    triggerGroup->setLayout(triggerGroupLayout);
    mainLayout->addWidget(triggerGroup);
    //triggerGroup->setDisabled(true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    analogGroup = new QGroupBox("5. Image Analog Control");
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


    // BG: only reveal settings when the cameras are connected
    setLimitationsWhileCameraNotOpen(true);

    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(setExposureTimeValue(int)));
    connect(binningBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBinningModeChange(int)));
    connect(gainBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));
    connect(gainBox, SIGNAL(valueChanged(double)), this, SLOT(setGainValue(double)));

    connect(imageROIwidthInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIwidth(int)));
    connect(imageROIheightInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIheight(int)));
    connect(imageROIoffsetXInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetX(int)));
    connect(imageROIoffsetYInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetY(int)));

//    connect(SWTframerateEnabled, SIGNAL(toggled(bool)), SWTframerateBox, SLOT(setEnabled(bool)));
//    connect(SWTframerateEnabled, SIGNAL(toggled(bool)), camera, SLOT(enableAcquisitionFrameRate(bool)));
//    connect(SWTframerateBox, SIGNAL(valueChanged(int)), camera, SLOT(setAcquisitionFPSValue(int)));

    connect(HWTlineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));
//    connect(HWTradioButton, ...

    connect(saveButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::saveButtonClick);
    connect(loadButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::loadButtonClick);
    connect(autoGainOnceButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::autoGainOnce);
    connect(autoExposureOnceButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::autoExposureOnce);
    connect(MCUConfigButton, &QPushButton::clicked, this, &StereoCameraSettingsDialog::onMCUConfig);
    connect(HWTstartStopButton, SIGNAL(clicked()), this, SLOT(HWTstartStopButtonClicked()));

    connect(updateDevicesButton, SIGNAL(clicked()), this, SLOT(updateDevicesBox()));
    connect(cameraOpenCloseButton, SIGNAL(clicked()), this, SLOT(cameraOpenCloseButtonClicked()));

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
    if(HWTrunning) {
        stopHardwareTrigger();
    }

//    saveUniversalSettings(); // closing stereo camera already saves settings
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
    if(!camera->isOpen())
        return;

    updateHWTStartStopRelatedWidgets();
    updateMCUConnDisconnButtonState();

    SWTframerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
//    SWTframerateBox->setMinimum(std::max(1, camera->getAcquisitionFPSMin()));
//    SWTframerateBox->setMaximum(camera->getAcquisitionFPSMax());
//    SWTframerateBox->setValue(camera->getAcquisitionFPSValue());

    gainBox->setMinimum(floor(camera->getGainMin()));
    gainBox->setMaximum(floor(camera->getGainMax()));
    gainBox->setValue(camera->getGainValue());

    exposureInputBox->setMinimum(camera->getExposureTimeMin());
    exposureInputBox->setMaximum(camera->getExposureTimeMax());
    exposureInputBox->setValue(camera->getExposureTimeValue());

    // Note: is this surely good here?
    HWTlineSourceBox->setCurrentText(QString::fromStdString(camera->getLineSource().c_str()));

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
    gainBox->setValue(camera->getGainValue());
}

void StereoCameraSettingsDialog::autoExposureOnce() {
    camera->autoExposureOnce();
    exposureInputBox->setValue(camera->getExposureTimeValue());
}

// TODO: EZ NEM JÓ
void StereoCameraSettingsDialog::updateFrameRateValue() {
    frameRateValueLabel->setText(QString::number(camera->getResultingFrameRateValue()));

    // commented out, reason:
    // TODO: problematic, as resulting framerate is affected by framerate limit, which creates a "loop" of events,
    // setting the max on gui as the current fps
//    //// if(HWTstartStopButton->isEnabled()) {
//        HWTframerateBox->setMaximum(static_cast<int>(floor(camera->getResultingFrameRateValue())));
//        SWTframerateBox->setMaximum(static_cast<int>(floor(camera->getResultingFrameRateValue())));
//    //// }

    // instead:
    int supposedMaxFPS = static_cast<int>(floor(camera->getResultingFrameRateValue()));
    if(!camera->isOpen() || supposedMaxFPS <= 0) {
        supposedMaxFPS = std::numeric_limits<short>::max();
    }
    HWTframerateBox->setMaximum(supposedMaxFPS);
    SWTframerateBox->setMaximum(supposedMaxFPS);
}

// TODO: ez miért ilyen?
void StereoCameraSettingsDialog::onLineSourceChange(int index) {
    if(index!=0) {
        camera->setLineSource(HWTlineSourceBox->itemText(index).toStdString().c_str());
        HWTframerateBox->setEnabled(true);
        HWTtimeSpanBox->setEnabled(true);
        HWTstartStopButton->setEnabled(MCUSettings->isConnected());
    } else {
        HWTframerateBox->setEnabled(false);
        HWTtimeSpanBox->setEnabled(false);
        HWTstartStopButton->setEnabled(false);
    }
}

void StereoCameraSettingsDialog::HWTstartStopButtonClicked() {
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
void StereoCameraSettingsDialog::startHardwareTrigger() {

    double runtime = HWTtimeSpanBox->value();
    double fps = HWTframerateBox->value();

    // Calculate the delay and number of frames from the input values
    int delay = (int)(((1000.0f/fps)*1000.0f) / 2.0f);
    int count = (int)((runtime*60000000)/(delay*2)); // corrected by SBelgers in previous commit

    QString cmd = "<TX"+ QString::number(count) +"X"+ QString::number(delay) +">";
    std::cout<<"Sending hardware trigger command: "<<cmd.toStdString()<<std::endl;

    emit onHardwareTriggerEnable();
    emit onHardwareTriggerStart(cmd);

    HWTrunning = true;
    updateHWTStartStopRelatedWidgets();
}

// Stops the hardware trigger signal by sending a stop signal to the microcontroller
void StereoCameraSettingsDialog::stopHardwareTrigger() {

    emit onHardwareTriggerDisable();
    emit onHardwareTriggerStop(QString("<SX>"));

    HWTrunning = false;
    updateHWTStartStopRelatedWidgets();
}

void StereoCameraSettingsDialog::updateHWTStartStopRelatedWidgets() {
    if(HWTrunning) {
        HWTframerateBox->setEnabled(false);
        HWTlineSourceBox->setEnabled(false);
        HWTtimeSpanBox->setEnabled(false);
        HWTstartStopButton->setText("Stop Image Acquisition");
        HWTstartStopButton->setStyleSheet(
                "QPushButton { background-color: #c3f558; border: 1px solid #757575; border-radius: 5px;}");
    } else {
        HWTframerateBox->setEnabled(true);
        HWTlineSourceBox->setEnabled(true);
        HWTtimeSpanBox->setEnabled(true);
        HWTstartStopButton->setText("Start Image Acquisition");
        HWTstartStopButton->setStyleSheet(
                "QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    }
}

// This method is to be used for camera warmup connection upon program start (argument command), 
// or for remote control connection invoked camera connection
// GB IMPORTANT TODO: TO BE TESTED
void StereoCameraSettingsDialog::openStereoCamera(const QString &camName1, const QString &camName2) {
    int idx1 = -1;
    for(int i=0; i<mainCameraBox->count(); i++)
        if(mainCameraBox->itemText(i).toLower() == camName1) {
            idx1 = i;
            break;
        }
    int idx2 = -1;
    for(int i=0; i<secondaryCameraBox->count(); i++)
        if(secondaryCameraBox->itemText(i).toLower() == camName2) {
            idx2 = i;
            break;
        }
    if(idx1==idx2 || idx1<0 || idx2<0)
        return;

    mainCameraBox->setCurrentIndex(idx1);
    secondaryCameraBox->setCurrentIndex(idx2);

    openStereoCamera();
}

void StereoCameraSettingsDialog::connectMCU() {
    if(MCUSettings->isConnected())
        return;
    MCUConnDisconnButtonClicked();
}

void StereoCameraSettingsDialog::startHWT() {
    if(HWTrunning)
        return;
    HWTstartStopButtonClicked();
}

void StereoCameraSettingsDialog::cameraOpenCloseButtonClicked() {
    if(!camera->isOpen()) {
        openStereoCamera();
    } else {
        closeStereoCamera();
    }
}

// Opens the stereo camera system, which means that both selected cameras are attached to the stereo camera object and opened, started to fetch images
// Beware that the opening of the stereo camera must happen BEFORE the start of the hardware trigger signal to ensure a sync image signal
// This is ensured in this form by disabling the start hardware trigger buttons until the cameras are opened
void StereoCameraSettingsDialog::openStereoCamera() {

    int mainCameraIndex = mainCameraBox->currentIndex();
    int secondaryCameraIndex = secondaryCameraBox->currentIndex();

    if(mainCameraIndex == secondaryCameraIndex) {
        std::cout<<"Error: cannot use one physical camera as a stereo system."<<std::endl;
        return;
    }

    try {
        camera->attachCameras(lstDevices[mainCameraIndex], lstDevices[secondaryCameraIndex]);
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
        QMessageBox::critical(this, "Device Error", e.GetDescription());
        return;
    }

    bool enableHardwareTrigger = true;
    // For debug/testing purposes only
    if(QString::fromStdString(lstDevices[mainCameraIndex].GetFriendlyName().c_str()).contains("emulat", Qt::CaseInsensitive) ||
            QString::fromStdString(lstDevices[secondaryCameraIndex].GetFriendlyName().c_str()).contains("emulat", Qt::CaseInsensitive)) {
        enableHardwareTrigger = false;
    }
    // Its important to open the camera here not earlier, as loading config overrides the config in open
    camera->open(enableHardwareTrigger);

    loadSettings();

    // This is very important here, this line causes signal-slot connections to be made in pupilDetection class to receive the images grabbed
    emit stereoCamerasOpened();

    updateForms();

    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateFrameRateValue();

    // BG: migrated enabling/disabling widget groups into a separate function
    // Activate all settings groups underneath
    setLimitationsWhileCameraNotOpen(false);

    //triggerGroup->setDisabled(false);

}

// Closes the stereo camera, stops the hardware trigger (by sending signal over serial port)
void StereoCameraSettingsDialog::closeStereoCamera() {
    if(HWTrunning) {
        stopHardwareTrigger();
    }

    saveSettings();

    camera->close();
    // Also stop the HW trigger signals
    stopHardwareTrigger();
    // Disable all camera settings groups underneath
    setLimitationsWhileCameraNotOpen(true);

    emit stereoCamerasClosed();
}

// Slot receiving signal that the settings changed, reload settings
void StereoCameraSettingsDialog::onSettingsChange() {
    loadSettings();
}

// Loads settings from the application setting
// Update the camera from these settings (Note: we cannot be sure that the camera is open and its getters provide valid values anyway)
void StereoCameraSettingsDialog::loadSettings() {

    mainCameraBox->setCurrentText(applicationSettings->value("StereoCameraSettingsDialog.mainCamera", mainCameraBox->currentText()).toString());
    secondaryCameraBox->setCurrentText(applicationSettings->value("StereoCameraSettingsDialog.secondaryCamera", secondaryCameraBox->currentText()).toString());

    HWTlineSourceBox->setCurrentText(applicationSettings->value("StereoCameraSettingsDialog.lineSource", QString::fromStdString(camera->getLineSource().c_str())).toString());
    camera->setLineSource(HWTlineSourceBox->currentText().toStdString().c_str());

    HWTframerateBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.hwTriggerFramerate", 30).toInt());
    HWTtimeSpanBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.hwTriggerTime", 0).toDouble());

    gainBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.analogGain", 0).toDouble());
    camera->setGainValue(gainBox->value());

    int testval = applicationSettings->value("StereoCameraSettingsDialog.analogExposure", 1000).toInt();

    exposureInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.analogExposure", 1000).toInt());
    camera->setExposureTimeValue(exposureInputBox->value());

    int lastUsedBinningVal = applicationSettings->value("StereoCameraSettingsDialog.binningVal", 1).toInt();
    camera->setBinningVal(lastUsedBinningVal);
    int tempidx = 0;
    if(lastUsedBinningVal==2 || lastUsedBinningVal==3)
        tempidx = 1;
    else if(lastUsedBinningVal==4)
        tempidx = 2;
    binningBox->setCurrentIndex(tempidx);

    imageROIwidthInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIwidth", 0 ).toInt());
    imageROIheightInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIheight", 0).toInt());
    imageROIoffsetXInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIoffsetX", 0).toInt()); // DEV
    imageROIoffsetYInputBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.imageROIoffsetY", 0).toInt()); // DEV

//    SWTframerateEnabled->setChecked(...
//    camera->enableAcquisitionFrameRate(SWTframerateEnabled->isChecked());

//    SWTframerateBox->setValue(applicationSettings->value("StereoCameraSettingsDialog.acquisitionFramerate", camera->getAcquisitionFPSValue()).toInt());
//    camera->setAcquisitionFPSValue(SWTframerateBox->value());

    qDebug() << imageROIwidthInputBox->value();

    // TODO load the pfs file as an backup if no appication settings are available?
}

// Saves settings to application settings
// As a backup the main camera settings are also saved to file
void StereoCameraSettingsDialog::saveSettings() {

    if(!camera->isOpen()) {
        applicationSettings->setValue("StereoCameraSettingsDialog.mainCamera", mainCameraBox->currentText());
        applicationSettings->setValue("StereoCameraSettingsDialog.secondaryCamera", secondaryCameraBox->currentText());
        return;
    }

    applicationSettings->setValue("StereoCameraSettingsDialog.mainCamera", mainCameraBox->currentText());
    applicationSettings->setValue("StereoCameraSettingsDialog.secondaryCamera", secondaryCameraBox->currentText());

    applicationSettings->setValue("StereoCameraSettingsDialog.lineSource", HWTlineSourceBox->currentText());
    applicationSettings->setValue("StereoCameraSettingsDialog.hwTriggerFramerate", HWTframerateBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.hwTriggerTime", HWTtimeSpanBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.analogGain", gainBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.analogExposure", exposureInputBox->value());
//    applicationSettings->setValue("StereoCameraSettingsDialog.SWTframerateEnabled", SWTframerateEnabled->isChecked());
//    applicationSettings->setValue("StereoCameraSettingsDialog.acquisitionFramerate", SWTframerateBox->value());

    applicationSettings->setValue("StereoCameraSettingsDialog.binningVal", lastUsedBinningVal);
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIwidth", imageROIwidthInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIheight", imageROIheightInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIoffsetX", imageROIoffsetXInputBox->value());
    applicationSettings->setValue("StereoCameraSettingsDialog.imageROIoffsetY", imageROIoffsetYInputBox->value());

    applicationSettings->setValue("StereoCameraSettingsDialog.settingsDirectory", settingsDirectory.path());

    QString mainName = QString::fromStdString(lstDevices[mainCameraBox->currentIndex()].GetFriendlyName().c_str());
    QString configFile = settingsDirectory.filePath(mainName+".pfs");
    configFile.replace(" ", "");
    std::cout<<"Saving config to settings directory: "<< configFile.toStdString() <<std::endl;
    camera->saveMainToFile(configFile.toStdString().c_str());

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
    updateFrameRateValue(); // important
}

void StereoCameraSettingsDialog::onSetImageROIheight(int val) {
    if (camera->isEmulated())
        camera->setImageROIheightEmu(val);
    else
        camera->setImageROIheight(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
    updateFrameRateValue(); // important
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

void StereoCameraSettingsDialog::updateImageROISettingsMax() {
    if(!camera->isOpen()) {
        imageROIwidthInputBox->setMaximum(std::numeric_limits<short>::max());
        imageROIheightInputBox->setMaximum(std::numeric_limits<short>::max());
        imageROIoffsetXInputBox->setMaximum(std::numeric_limits<short>::max());
        imageROIoffsetYInputBox->setMaximum(std::numeric_limits<short>::max());
        return;
    }

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

void StereoCameraSettingsDialog::onBinningModeChange(int index) {
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

void StereoCameraSettingsDialog::updateSensorSize() {
    if(!camera->isOpen())
        return;

    emit onSensorSizeChanged(QSize(camera->getImageROIwidthMax(), camera->getImageROIheightMax()));
}

void StereoCameraSettingsDialog::updateCamImageRegionsWidget() {
    if(!camera->isOpen())
        return;

    const QSize sensorSize = QSize( camera->getImageROIwidthMax(), camera->getImageROIheightMax() );
    const QRect imageAcqROI1Rect = QRect( camera->getImageROIoffsetX(), camera->getImageROIoffsetY(),
                                          camera->getImageROIwidth(), camera->getImageROIheight() );
    camImageRegionsWidget->setImageMaxSize(sensorSize);
    camImageRegionsWidget->recalculateDrawingArea();
    camImageRegionsWidget->setImageAcqROI1Rect(imageAcqROI1Rect);
}

void StereoCameraSettingsDialog::setLimitationsWhileTracking(bool state) {
    //triggerGroup->setDisabled(state);
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

void StereoCameraSettingsDialog::setLimitationsWhileCameraNotOpen(bool state) {
    mainCameraBox->setDisabled(!state);
    secondaryCameraBox->setDisabled(!state);
    updateDevicesButton->setDisabled(!state);

    if (!camera->isEmulated()){
        triggerGroup->setDisabled(state);
        binningLabel->setDisabled(state);
        binningBox->setDisabled(state);
    }
    else{
        triggerGroup->setDisabled(true);
        binningLabel->setDisabled(true);
        binningBox->setDisabled(true);
    }
    analogGroup->setDisabled(state);
    acquisitionGroup->setDisabled(state);    
    loadButton->setDisabled(state);
    saveButton->setDisabled(state);

    if(state) {
        cameraOpenCloseButton->setText("Open Camera Devices");
        cameraOpenCloseButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
    } else {
        cameraOpenCloseButton->setText("Close Camera Devices");
        cameraOpenCloseButton->setStyleSheet("QPushButton { background-color: #c3f558; border: 1px solid #757575; border-radius: 5px;}");
    }
}

void StereoCameraSettingsDialog::setExposureTimeValue(int value) {

    // this is necessary if we programmatically set it
    exposureInputBox->blockSignals(true);
    exposureInputBox->setValue(value);
    exposureInputBox->blockSignals(false);

    camera->setExposureTimeValue(value);
    updateFrameRateValue();
}

void StereoCameraSettingsDialog::setGainValue(double value) {

    // this is necessary if we programmatically set it
    gainBox->blockSignals(true);
    gainBox->setValue(value);
    gainBox->blockSignals(false);

    camera->setGainValue(value);
    updateFrameRateValue();
}

void StereoCameraSettingsDialog::mainCameraBoxCurrentIndexChanged(int) {
    applicationSettings->setValue("StereoCameraSettingsDialog.mainCamera", mainCameraBox->currentText());
}

void StereoCameraSettingsDialog::secondaryCameraBoxCurrentIndexChanged(int) {
    applicationSettings->setValue("StereoCameraSettingsDialog.secondaryCamera", secondaryCameraBox->currentText());
}

void StereoCameraSettingsDialog::MCUConnDisconnButtonClicked() {
    if(MCUSettings->isConnected()) {
        stopHardwareTrigger();
        MCUSettings->doDisconnect();
    } else {
        MCUSettings->doConnect();
    }
    updateMCUConnDisconnButtonState();
}

void StereoCameraSettingsDialog::updateMCUConnDisconnButtonState() {
    if(MCUSettings->isConnected()) {
        MCUConnDisconnButton->setText("Disconnect");
        MCUConnDisconnButton->setStyleSheet("QPushButton { background-color: #c3f558; border: 1px solid #757575; border-radius: 5px;}");
        HWTstartStopButton->setEnabled(true);
    } else {
        MCUConnDisconnButton->setText("Connect");
        MCUConnDisconnButton->setStyleSheet("QPushButton { background-color: #f5ab87; border: 1px solid #757575; border-radius: 5px;}");
        HWTstartStopButton->setEnabled(false);
    }
}

void StereoCameraSettingsDialog::setHWTlineSource(int lineSourceNum) {
    if(!camera->isOpen() || HWTrunning)
        return;
    HWTlineSourceBox->setCurrentIndex(lineSourceNum);
}

void StereoCameraSettingsDialog::setHWTruntime(double runtimeMinutes) {
    if(!camera->isOpen() || HWTrunning)
        return;
    HWTtimeSpanBox->setValue(runtimeMinutes);
}

void StereoCameraSettingsDialog::setHWTframerate(int fps) {
    if(!camera->isOpen() || HWTrunning)
        return;
    HWTframerateBox->setValue(fps);
}


