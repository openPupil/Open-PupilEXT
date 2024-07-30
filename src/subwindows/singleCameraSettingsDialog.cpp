
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
SingleCameraSettingsDialog::SingleCameraSettingsDialog(SingleCamera *cameraPtr, SerialSettingsDialog *serialSettings, QWidget *parent) :
        QDialog(parent), 
        serialSettings(serialSettings), 
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    camera = cameraPtr;

    settingsDirectory = QDir(applicationSettings->value("SingleCameraSettingsDialog.settingsDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    setMinimumSize(500, 610);

    setWindowTitle(QString("[%1] Camera Settings").arg(camera->getFriendlyName()));

    createForm();

    // BG: moved all connect() calls to the end of createForm() for clarity

    loadSettings();
    updateForms();
}

void SingleCameraSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    serialConnGroup = new QGroupBox("1. Camera Serial Connection (needed only for Hardware-triggered image acquisition)");
    QFormLayout *serialConnGroupLayout = new QFormLayout();

    QSpacerItem *sp10 = new QSpacerItem(70, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    serialConfigButton = new QPushButton();
//    serialConfigButton->setText("Camera Serial Connection Settings");
    serialConfigButton->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/rs232.svg"), applicationSettings));
//    serialConfigButton->setStyleSheet("text-align:left; padding-left : 10px; padding-top : 3px; padding-bottom : 3px;"); //
    serialConfigButton->setStyleSheet("text-align:left;");
    serialConfigButton->setLayout(new QGridLayout);
    QLabel* serialConfigButtonLabel = new QLabel("Camera Serial Connection Settings");
    serialConfigButtonLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    serialConfigButtonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    serialConfigButton->layout()->addWidget(serialConfigButtonLabel);
    serialConfigButton->layout()->setContentsMargins(5,0,10,0);
    serialConfigButton->setFixedWidth(230);
//    serialConfigButton->setFixedHeight(25); //
    QSpacerItem *sp9 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    serialConnDisconnButton = new QPushButton(); // Will change to disconnect when connected
    serialConnDisconnButton->setLayout(new QGridLayout);
    serialConnDisconnButtonLabel = new QLabel("Connect");
    serialConnDisconnButtonLabel->setStyleSheet("background-color:#f5ab87;"); // light red (alternative: orange: #ebbd3f)
    serialConnDisconnButtonLabel->setAlignment(Qt::AlignCenter);
    serialConnDisconnButton->layout()->addWidget(serialConnDisconnButtonLabel);
    serialConnDisconnButton->layout()->setContentsMargins(5,5,5,5);
    serialConnDisconnButton->setFixedWidth(150);
//    serialConnDisconnButton->setFixedHeight(25); //

    QHBoxLayout *serialConnRow1 = new QHBoxLayout;
    serialConnRow1->addSpacerItem(sp10);
    serialConnRow1->addWidget(serialConfigButton);
    serialConnRow1->addSpacerItem(sp9);
    serialConnRow1->addWidget(serialConnDisconnButton);
//    serialConnRow1->addStretch();
    serialConnGroupLayout->addItem(serialConnRow1);

    serialConnGroup->setLayout(serialConnGroupLayout);
    mainLayout->addWidget(serialConnGroup);

    connect(serialConnDisconnButton, SIGNAL(clicked()), this, SLOT(serialConnDisconnButtonClicked()));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    acquisitionGroup = new QGroupBox("2. Image Acquisition Control");
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

    autoExposureOnceButton = new QPushButton("Auto Exposure (Once)");
    autoExposureOnceButton->setFixedWidth(150);
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

    /////////////////////////////////////////////////
    QHBoxLayout *imageROIlayoutRow6 = new QHBoxLayout;
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

    SWTradioButton = new QRadioButton("Software triggering:", this);
    SWTradioButton->setFixedHeight(20);
    SWTradioButton->setDisabled(false);
    SWTradioButton->setChecked(!camera->isHardwareTriggerEnabled());

    // This would normally never be used in case of hardware triggering, so it is moved to the "software triggering"
    // section and kept disabled
    SWTframerateEnabled = new QCheckBox("Limit framerate to: ");
    SWTframerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
    SWTframerateEnabled->setEnabled(!camera->isHardwareTriggerEnabled()); //
    SWTframerateBox = new QSpinBox();
    SWTframerateLayout = new QHBoxLayout;
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

    HWTstartStopButton = new QPushButton();
    HWTstartStopButton->setLayout(new QGridLayout);
    HWTstartStopButtonLabel = new QLabel("Start Image Acquisition");
    HWTstartStopButtonLabel->setStyleSheet("background-color:#f5ab87;"); // light red (alternative: orange: #ebbd3f)
    HWTstartStopButtonLabel->setAlignment(Qt::AlignCenter);
    HWTstartStopButton->layout()->addWidget(HWTstartStopButtonLabel);
    HWTstartStopButton->layout()->setContentsMargins(5,5,5,5);
    HWTstartStopButton->setFixedWidth(150);
    HWTstartStopButton->setEnabled(camera->isHardwareTriggerEnabled() && serialSettings->isConnected());

    HWTframerateLayout->addSpacerItem(sp5);
    HWTframerateLayout->addWidget(HWTframerateLabel);
    HWTframerateLayout->addWidget(HWTframerateBox);
    HWTframerateLayout->addSpacerItem(sp7);
    HWTframerateLayout->addWidget(HWTstartStopButton);
//    HWTframerateLayout->addStretch();

    triggerGroupLayout->addRow(HWTradioButton, HWTframerateLayout);

    HWTgroupLayout = new QFormLayout();

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
    QHBoxLayout *gainLayout = new QHBoxLayout();

    QLabel *gainLabel = new QLabel(tr("Gain [dB]:"));
    gainLabel->setMinimumWidth(80);

    // BG modified begin
    // BG NOTE: Modified to fit on smaller screens too
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
    // BG modified end

    analogGroup->setLayout(analogLayout);
    //analogGroup->setDisabled(true); // BG
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

    // GB: added this label to warn user that a new image acq ROI or binning setting needs new calibration
    QLabel *imageROIWarningLabel = new QLabel(tr("Warning: If you are using Hardware-triggered image acquisition, please restart Image Acquisition\n    Triggering whenever Image Acquisition ROI is modified.\nWarning: if Image Acquisition ROI or Binning is altered, a new camera calibration is necessary\n    for proper undistortion."));
    mainLayout->addWidget(imageROIWarningLabel);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    // GB modified/added begin
    // GB: these are needed for binning-dependent limits for image acq ROI spinboxes
    updateImageROISettingsMax();
    updateImageROISettingsValues();

    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(updateFrameRateValue()));
    connect(exposureInputBox, SIGNAL(valueChanged(int)), this, SLOT(setExposureTimeValue(int)));
    connect(binningBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBinningModeChange(int)));
    connect(gainBox, SIGNAL(valueChanged(double)), this, SLOT(updateFrameRateValue()));
    connect(gainBox, SIGNAL(valueChanged(double)), camera, SLOT(setGainValue(double)));

    connect(imageROIwidthInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIwidth(int)));
    connect(imageROIheightInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIheight(int)));
    connect(imageROIoffsetXInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetX(int)));
    connect(imageROIoffsetYInputBox, SIGNAL(valueChanged(int)), this, SLOT(onSetImageROIoffsetY(int)));

    connect(SWTframerateEnabled, SIGNAL(toggled(bool)), SWTframerateBox, SLOT(setEnabled(bool)));
    connect(SWTframerateEnabled, SIGNAL(toggled(bool)), this, SLOT(enableAcquisitionFrameRate(bool)));
    connect(SWTframerateBox, SIGNAL(valueChanged(int)), this, SLOT(setAcquisitionFPSValue(int)));

    connect(HWTlineSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLineSourceChange(int)));
    connect(HWTradioButton, SIGNAL(toggled(bool)), this, SLOT(onHWTenabledChange(bool)));

    connect(saveButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::saveButtonClick);
    connect(loadButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::loadButtonClick);
    connect(autoGainOnceButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::autoGainOnce);
    connect(autoExposureOnceButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::autoExposureOnce);
    connect(serialConfigButton, &QPushButton::clicked, this, &SingleCameraSettingsDialog::onSerialConfig);
    connect(HWTstartStopButton, SIGNAL(clicked()), this, SLOT(HWTstartStopButtonClicked()));

    onHWTenabledChange(camera->isHardwareTriggerEnabled());
}

void SingleCameraSettingsDialog::updateForms() {
    if(!camera->isOpen())
        return;

    SWTframerateEnabled->setChecked(camera->isEnabledAcquisitionFrameRate());
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

    // commented out by kheki4, reason:
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
        HWTframerateBox->setEnabled(true);
        HWTtimeSpanBox->setEnabled(true);
        HWTstartStopButton->setEnabled(serialSettings->isConnected());
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
    HWTframerateBox->setEnabled(false);
    HWTlineSourceBox->setEnabled(false);
    HWTtimeSpanBox->setEnabled(false);
//    HWTstartStopButton->setText("Stop Image Acquisition");
    HWTstartStopButtonLabel->setText("Stop Image Acquisition");
    HWTstartStopButtonLabel->setStyleSheet("background-color:#c3f558;"); // light green

}

// Stops the hardware trigger signal by sending a stop signal to the microcontroller
void SingleCameraSettingsDialog::stopHardwareTrigger() {

    emit onHardwareTriggerDisable();
    emit onHardwareTriggerStop(QString("<SX>"));

    HWTrunning = false;
    HWTframerateBox->setEnabled(true);
    HWTlineSourceBox->setEnabled(true);
    HWTtimeSpanBox->setEnabled(true);
//    HWTstartStopButton->setText("Start Image Acquisition");
    HWTstartStopButtonLabel->setText("Start Image Acquisition");
    HWTstartStopButtonLabel->setStyleSheet("background-color:#f5ab87;"); // light red
}

void SingleCameraSettingsDialog::onHWTenabledChange(bool state) {

    if(HWTrunning) {
        stopHardwareTrigger();
    }

    SWTframerateEnabled->setEnabled(!state);
    SWTframerateBox->setEnabled(!state && camera->isEnabledAcquisitionFrameRate());
    if(!state)
        camera->setAcquisitionFPSValue(camera->getAcquisitionFPSMax());
    else
        camera->setAcquisitionFPSValue(SWTframerateBox->value());

    HWTframerateLayout->setEnabled(state);
    HWTgroupLayout->setEnabled(state); // Note: not sure if this does anything
    HWTframerateLabel->setEnabled(state);
    HWTlineSourceLabel->setEnabled(state);
    HWTtimeSpanLabel->setEnabled(state);
    HWTlineSourceBox->setEnabled(state);
    HWTstartStopButton->setEnabled(state && serialSettings->isConnected());

    HWTtimeSpanBox->setEnabled(state);
    HWTframerateBox->setEnabled(state);
    camera->enableHardwareTrigger(state);

    if(state) {
        emit onHardwareTriggerEnable();
    } else {
        emit onHardwareTriggerDisable();
    }
}

void SingleCameraSettingsDialog::loadSettings() {

    SWTradioButton->setChecked(!applicationSettings->value("SingleCameraSettingsDialog.hwTriggerEnabled", camera->isHardwareTriggerEnabled()).toBool());
    HWTradioButton->setChecked(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerEnabled", camera->isHardwareTriggerEnabled()).toBool());
    camera->enableHardwareTrigger(HWTradioButton->isChecked());

    HWTlineSourceBox->setCurrentText(applicationSettings->value("SingleCameraSettingsDialog.lineSource", QString::fromStdString(camera->getLineSource().c_str())).toString());
    camera->setLineSource(HWTlineSourceBox->currentText().toStdString().c_str());

    HWTframerateBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerFramerate", HWTframerateBox->value()).toInt());
    HWTtimeSpanBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.hwTriggerTime", HWTtimeSpanBox->value()).toDouble());

    gainBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.analogGain", camera->getGainValue()).toDouble());
    camera->setGainValue(gainBox->value());

    exposureInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.analogExposure", camera->getExposureTimeValue()).toInt());
    camera->setExposureTimeValue(exposureInputBox->value());

    // BG added begin
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
    imageROIoffsetXInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIoffsetX", 0).toInt()); // DEV
    imageROIoffsetYInputBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.imageROIoffsetY", 0).toInt()); // DEV
    // BG added end

    SWTframerateEnabled->setChecked(applicationSettings->value("SingleCameraSettingsDialog.SWTframerateEnabled", camera->isEnabledAcquisitionFrameRate()).toBool());
    camera->enableAcquisitionFrameRate(SWTframerateEnabled->isChecked());

    SWTframerateBox->setValue(applicationSettings->value("SingleCameraSettingsDialog.acquisitionFramerate", camera->getAcquisitionFPSValue()).toInt());
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

    // BG added begin
    applicationSettings->setValue("SingleCameraSettingsDialog.binningVal", lastUsedBinningVal);
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIwidth", imageROIwidthInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIheight", imageROIheightInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIoffsetX", imageROIoffsetXInputBox->value());
    applicationSettings->setValue("SingleCameraSettingsDialog.imageROIoffsetY", imageROIoffsetYInputBox->value());
    // BG added end

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
}

void SingleCameraSettingsDialog::onSetImageROIheight(int val) {
    camera->setImageROIheight(val);
    updateImageROISettingsMax();
    updateImageROISettingsValues();
    updateCamImageRegionsWidget();
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
    camera->setExposureTimeValue(value);
    updateFrameRateValue();
}

void SingleCameraSettingsDialog::setAcquisitionFPSValue(int value) {
    camera->setAcquisitionFPSValue(value);
    updateFrameRateValue();
}

void SingleCameraSettingsDialog::enableAcquisitionFrameRate(bool state) {
    camera->enableAcquisitionFrameRate(state);
    updateFrameRateValue();
}

void SingleCameraSettingsDialog::serialConnDisconnButtonClicked() {
    if(serialSettings->isConnected()) {
        stopHardwareTrigger();

        serialSettings->disconnectSerialPort();
        serialConnDisconnButtonLabel->setText("Connect");
        serialConnDisconnButtonLabel->setStyleSheet("background-color:#f5ab87;"); // light red
        HWTstartStopButton->setEnabled(false);
    } else {
        serialSettings->connectSerialPort();
        serialConnDisconnButtonLabel->setText("Disconnect");
        serialConnDisconnButtonLabel->setStyleSheet("background-color:#c3f558;"); // light green
        HWTstartStopButton->setEnabled(camera->isHardwareTriggerEnabled());
    }
}