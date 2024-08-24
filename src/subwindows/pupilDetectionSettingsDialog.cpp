
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/QLabel>
#include <QtWidgets/QtWidgets>
#include "pupilDetectionSettingsDialog.h"
#include "../pupil-detection-methods/PuReST.h"
#include "pupil-detection-methods/PuReSettings.h"
#include "pupil-detection-methods/ElSeSettings.h"
#include "pupil-detection-methods/ExCuSeSettings.h"
#include "pupil-detection-methods/StarburstSettings.h"
#include "pupil-detection-methods/Swirski2DSettings.h"
#include "pupil-detection-methods/PuReSTSettings.h"
#include "../SVGIconColorAdjuster.h"

// Create the pupil detection settings dialog
// Given a pupil detection object to communicate to the detection algorithm objects there
PupilDetectionSettingsDialog::PupilDetectionSettingsDialog(PupilDetection *pupilDetection, QWidget *parent) :
        QDialog(parent),
        pupilDetection(pupilDetection),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    //this->setMinimumSize(800, 500);
    this->setMinimumSize(980, 600);
    this->setWindowTitle("Pupil Detection Settings");

    createForm();

    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyButtonClick()));
    connect(applyCloseButton, SIGNAL(clicked()), this, SLOT(applyCloseButtonClick()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClick()));

    connect(procModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onProcModeSelection(int)));
    connect(algorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgorithmSelection(int)));

    onProcModeSelection(procModeBox->currentIndex());
    onAlgorithmSelection(algorithmBox->currentIndex());
    loadSettings();

    updateProcModeEnabled();
}

PupilDetectionSettingsDialog::~PupilDetectionSettingsDialog() {

}

void PupilDetectionSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(10);
    mainLayout->setContentsMargins(10,10,10,10);
    QHBoxLayout *mainLayoutInner = new QHBoxLayout();
    mainLayoutInner->setMargin(0);
    mainLayoutInner->setContentsMargins(0,0,0,0);
    QVBoxLayout *mainLayoutInnerCol1 = new QVBoxLayout();
    mainLayoutInnerCol1->setMargin(0);
    mainLayoutInnerCol1->setContentsMargins(0,5,0,5);
    QVBoxLayout *mainLayoutInnerCol2 = new QVBoxLayout();
    mainLayoutInnerCol2->setMargin(0);
    mainLayoutInnerCol2->setContentsMargins(0,5,0,5);

    procModeGroup = new QGroupBox("Pupil detection via image processing");
    QGroupBox *algorithmGroup = new QGroupBox("Algorithm");
    QGroupBox *optionsGroup = new QGroupBox("General Options");

    QFormLayout *procModeBoxLayout = new QFormLayout();
    QVBoxLayout *algorithmLayout = new QVBoxLayout();
    QHBoxLayout *algoBoxLayout = new QHBoxLayout();

    QLabel *procModeLabel = new QLabel(tr("Image processing mode:"));
    procModeBox = new QComboBox();

    procModeBox->addItem(QString::fromStdString("(Undetermined)"));
    procModeBox->addItem(QString::fromStdString("Single camera image for one pupil"));
    procModeBox->addItem(QString::fromStdString("Single camera image for two pupils"));
    procModeBox->addItem(QString::fromStdString("Stereo camera images for one pupil (stereoscopic)"));
    procModeBox->addItem(QString::fromStdString("Stereo camera images for two pupils (stereoscopic)"));
    // procModeBox->addItem(QString::fromStdString("Mirrored single camera image for one pupil"));
    // TODO: add monoscopic processing of stereo camera
    // TODO: add mirrored stereo and make it work
    
    procModeBox->setCurrentIndex(pupilDetection->getCurrentProcMode());
    //procModeBox->setCurrentIndex(0);

    procModeBoxLayout->addWidget(procModeLabel);
    procModeBoxLayout->addWidget(procModeBox);

    procModeIcon_undetermined = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/status/22/dialog-information.svg"), applicationSettings);
    procModeIcon_1cam1pup = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/1cam1pup.svg"), applicationSettings);
    procModeIcon_1cam2pup = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/1cam2pup.svg"), applicationSettings);
    procModeIcon_1Mcam1pup = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/1Mcam1pup.svg"), applicationSettings);
    procModeIcon_2cam1pup = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/2cam1pup.svg"), applicationSettings);
    procModeIcon_2cam2pup = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/2cam2pup.svg"), applicationSettings);

    // Note: this is a workaround to let the icons be repainted
    // (Pixmaps, which could have been used for real QLabels, do not get repainted, they would need to be read again
    // from the svg resource and rendered to pixmap, then set for the labels pixmaps. This is better now)
    iLabelFakeButton = new QPushButton();
    iLabelFakeButton->setFlat(true);
    iLabelFakeButton->setContentsMargins(0,0,0,0);
    iLabelFakeButton->setAttribute(Qt::WA_NoSystemBackground, true);
    iLabelFakeButton->setAttribute(Qt::WA_TranslucentBackground, true);
    iLabelFakeButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px }");
    iLabelFakeButton->setIcon(procModeIcon_undetermined);
    iLabelFakeButton->setFixedSize(QSize(50, 50));
    iLabelFakeButton->setIconSize(QSize(50, 50));

    procModeInfoLabel = new QLabel(tr(""));
    //procModeBoxLayout->addWidget(procModeInfoLabel);


    QGridLayout *layoutRow1 = new QGridLayout;
    layoutRow1->addWidget(iLabelFakeButton, 0, 0, 0, 1); // row, column, rowSpan, columnSpan
    layoutRow1->addWidget(procModeInfoLabel, 0, 1, 0, 5); // row, column, rowSpan, columnSpan
    //layoutRow1->addSpacerItem(sp);
    procModeBoxLayout->addRow(layoutRow1);


    procModeGroup->setLayout(procModeBoxLayout);
    mainLayoutInnerCol1->addWidget(procModeGroup);

    QLabel *algorithmLabel = new QLabel(tr("Algorithm:"));
    algorithmBox = new QComboBox();

    for(auto pm: pupilDetection->getMethods()) {
        algorithmBox->addItem(QString::fromStdString(pm->title()));
    }
    algorithmBox->setCurrentText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));

    algoBoxLayout->addWidget(algorithmLabel, 0);
    algoBoxLayout->addWidget(algorithmBox, 1);

    algorithmLayout->addLayout(algoBoxLayout);

    algorithmGroup->setLayout(algorithmLayout);
    mainLayoutInnerCol1->addWidget(algorithmGroup);

    QFormLayout *optionsLayout = new QFormLayout();

    QLabel *roiPreprocessingLabel = new QLabel(tr("Use ROI Area Selection:"));
    roiPreprocessingBox = new QCheckBox();
    roiPreprocessingBox->setChecked(pupilDetection->isROIPreProcessingEnabled());
    optionsLayout->addRow(roiPreprocessingLabel, roiPreprocessingBox);

    QLabel *outlineConfidenceLabel = new QLabel(tr("Compute Additional Outline Confidence:"));
    outlineConfidenceBox = new QCheckBox();
    outlineConfidenceBox->setChecked(pupilDetection->isOutlineConfidenceEnabled());
    optionsLayout->addRow(outlineConfidenceLabel, outlineConfidenceBox);


    QLabel *pupilSizeUndistortionLabel = new QLabel(tr("Undistort individual pupil size (fast) [<a href=\"http://mock.link\">?</a>]:"));
    connect(pupilSizeUndistortionLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));

    QButtonGroup *checkboxGroup = new QButtonGroup();
    checkboxGroup->setExclusive(false);

    pupilUndistortionBox = new QCheckBox();
    pupilUndistortionBox->setChecked(pupilDetection->isPupilUndistortionEnabled());
    checkboxGroup->addButton(pupilUndistortionBox);
    optionsLayout->addRow(pupilSizeUndistortionLabel, pupilUndistortionBox);
    connect(pupilUndistortionBox, SIGNAL(stateChanged(int)), this, SLOT(onPupilUndistortionClick(int)));


    QLabel *imageUndistortionLabel = new QLabel(tr("Undistort complete image (slow) (single camera only) [<a href=\"http://mock.link\">?</a>]:"));
    connect(imageUndistortionLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));

    imageUndistortionBox = new QCheckBox();
    imageUndistortionBox->setChecked(pupilDetection->isImageUndistortionEnabled());
    checkboxGroup->addButton(imageUndistortionBox);
    optionsLayout->addRow(imageUndistortionLabel, imageUndistortionBox);
    connect(imageUndistortionBox, SIGNAL(stateChanged(int)), this, SLOT(onImageUndistortionClick(int)));

    optionsGroup->setLayout(optionsLayout);
    mainLayoutInnerCol1->addWidget(optionsGroup);

    // TODO: make code fit in 1/6th of the actual line count...

    // For each algorithm, a special widget is implemented that contains all algorithm specific parameters
    for(PupilDetectionMethod *pm: pupilDetection->getMethods()) {
        PupilMethodSetting *settings;
        if(pm->title() == "PuRe") {
            settings = new PuReSettings(pupilDetection, dynamic_cast<PuRe*>(pm));
            // If the current pupil detection is stereo, two algorithm instances exist, which should be confiured the same way
            dynamic_cast<PuReSettings*>(settings)->add2(dynamic_cast<PuRe*>(pupilDetection->getMethod2("PuRe")));
            dynamic_cast<PuReSettings*>(settings)->add3(dynamic_cast<PuRe*>(pupilDetection->getMethod3("PuRe")));
            dynamic_cast<PuReSettings*>(settings)->add4(dynamic_cast<PuRe*>(pupilDetection->getMethod4("PuRe")));
        } else if(pm->title() == "PuReST") {
            settings = new PuReSTSettings(pupilDetection, dynamic_cast<PuReST*>(pm));
            dynamic_cast<PuReSTSettings*>(settings)->add2(dynamic_cast<PuReST*>(pupilDetection->getMethod2("PuReST")));
            dynamic_cast<PuReSTSettings*>(settings)->add3(dynamic_cast<PuReST*>(pupilDetection->getMethod3("PuReST")));
            dynamic_cast<PuReSTSettings*>(settings)->add4(dynamic_cast<PuReST*>(pupilDetection->getMethod4("PuReST")));
        } else if(pm->title() == "ElSe") {
            settings = new ElSeSettings(pupilDetection, dynamic_cast<ElSe*>(pm));
            dynamic_cast<ElSeSettings*>(settings)->add2(dynamic_cast<ElSe*>(pupilDetection->getMethod2("ElSe")));
            dynamic_cast<ElSeSettings*>(settings)->add3(dynamic_cast<ElSe*>(pupilDetection->getMethod3("ElSe")));
            dynamic_cast<ElSeSettings*>(settings)->add4(dynamic_cast<ElSe*>(pupilDetection->getMethod4("ElSe")));
        } else if(pm->title() == "ExCuSe") {
            settings = new ExCuSeSettings(pupilDetection, dynamic_cast<ExCuSe*>(pm));
            dynamic_cast<ExCuSeSettings*>(settings)->add2(dynamic_cast<ExCuSe*>(pupilDetection->getMethod2("ExCuSe")));
            dynamic_cast<ExCuSeSettings*>(settings)->add3(dynamic_cast<ExCuSe*>(pupilDetection->getMethod3("ExCuSe")));
            dynamic_cast<ExCuSeSettings*>(settings)->add4(dynamic_cast<ExCuSe*>(pupilDetection->getMethod4("ExCuSe")));
        } else if(pm->title() == "Starburst") {
            settings = new StarburstSettings(pupilDetection, dynamic_cast<Starburst*>(pm));
            dynamic_cast<StarburstSettings*>(settings)->add2(dynamic_cast<Starburst*>(pupilDetection->getMethod2("Starburst")));
            dynamic_cast<StarburstSettings*>(settings)->add3(dynamic_cast<Starburst*>(pupilDetection->getMethod3("Starburst")));
            dynamic_cast<StarburstSettings*>(settings)->add4(dynamic_cast<Starburst*>(pupilDetection->getMethod4("Starburst")));
        } else if(pm->title() == "Swirski2D") {
            settings = new Swirski2DSettings(pupilDetection, dynamic_cast<Swirski2D*>(pm));
            dynamic_cast<Swirski2DSettings*>(settings)->add2(dynamic_cast<Swirski2D*>(pupilDetection->getMethod2("Swirski2D")));
            dynamic_cast<Swirski2DSettings*>(settings)->add3(dynamic_cast<Swirski2D*>(pupilDetection->getMethod3("Swirski2D")));
            dynamic_cast<Swirski2DSettings*>(settings)->add4(dynamic_cast<Swirski2D*>(pupilDetection->getMethod4("Swirski2D")));
        } else {
            // TODO: why is this here anyway?
            settings = new PupilMethodSetting();
        }
//        else if(pm->title() == "APPD") {
//            APPDSettings *settings = new APPDSettings(dynamic_cast<APPD*>(pm));
//            settings->addSecondary(dynamic_cast<APPD*>(pupilDetection->getSecondaryMethod("APPD")));
//            mainLayoutInnerCol2->addWidget(settings);
//            settings->hide();
//            pupilMethodSettings.push_back(settings);
//
//            settings->infoBox->hide();
//            algorithmLayout->addWidget(settings->infoBox, 1, 0);
//        }
        connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));
        mainLayoutInnerCol2->addWidget(settings);
        settings->hide();
        pupilMethodSettings.push_back(settings);
        settings->infoBox->hide();
        algorithmLayout->addWidget(settings->infoBox);
        
    }

    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    applyButton = new QPushButton(tr("Apply"));
    applyCloseButton = new QPushButton(tr("Apply and Close"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    buttonsLayout->addWidget(applyCloseButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayoutInner->addLayout(mainLayoutInnerCol1);
    mainLayoutInner->addLayout(mainLayoutInnerCol2);
    mainLayout->addLayout(mainLayoutInner);
    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    updateProcModeCompatibility();
}

// Update the dialog form from the settings configured in the pupil detection process
void PupilDetectionSettingsDialog::updateForm() {

    algorithmBox->setCurrentText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
    roiPreprocessingBox->setChecked(pupilDetection->isROIPreProcessingEnabled());
    outlineConfidenceBox->setChecked(pupilDetection->isOutlineConfidenceEnabled());

    pupilUndistortionBox->setChecked(pupilDetection->isPupilUndistortionEnabled());
    imageUndistortionBox->setChecked(pupilDetection->isImageUndistortionEnabled());

    pupilMethodSettings[algorithmBox->currentIndex()]->applyAndSaveSpecificSettings();

    procModeBox->setCurrentIndex(pupilDetection->getCurrentProcMode());
    if(pupilDetection->isTrackingOn()) {
        procModeGroup->setDisabled(true);
    } else {
        procModeGroup->setDisabled(false);
    }
}

void PupilDetectionSettingsDialog::reject() {
    QDialog::reject();
}

// Information dialog containing infos to camera lens distortion
// Could be expanded
void PupilDetectionSettingsDialog::onShowHelpDialog() {
    QMessageBox msgBox;
    msgBox.setText("Lens distortion can influence the pupil size measurement. If a camera calibration is available, the image can be undistorted.\nDue to real-time requirements, not the whole image but only contour points of the detected pupil ellipse are undistorted. From these contour points a undistorted pupil size is then derived.\n\nFor stereo cameras, this option will calculate the undistorted pupil size for each camera individually.\n\nThe undistorted pupil size is available under the undistortedDiameter_px value.");
    msgBox.exec();
}

// Load pupil detection settings from the application wide settings (which are stored in file)
// Default values if no application settings are available are the current values from the pupil detection process
void PupilDetectionSettingsDialog::loadSettings() {
    if(!pupilDetection->isTrackingOn() && pupilDetection->hasOpenCamera()) {
        // Note: workaround: these should not stay "undetermined" at any time, so use meaningful default values, even though they could be changed in GUI
        if(!pupilDetection->isStereo()) {
            pupilDetection->setCurrentProcMode(applicationSettings->value("PupilDetectionSettingsDialog.singleCam.procMode", ProcMode::SINGLE_IMAGE_ONE_PUPIL).toInt());
            //std::cout << "LOADED SINGLE CAM PROC MODE" << std::endl;
        } else {
            pupilDetection->setCurrentProcMode(applicationSettings->value("PupilDetectionSettingsDialog.stereoCam.procMode", ProcMode::STEREO_IMAGE_ONE_PUPIL).toInt());
            //std::cout << "LOADED STEREO CAM PROC MODE" << std::endl;
        }
    }

    pupilDetection->setAlgorithm(applicationSettings->value("PupilDetectionSettingsDialog.algorithm", algorithmBox->currentText()).toString());
//    pupilDetection->enableOutlineConfidence(SupportFunctions::readBoolFromQSettings("PupilDetectionSettingsDialog.outlineConfidence", outlineConfidenceBox->isChecked(), applicationSettings));
//    pupilDetection->enableROIPreProcessing(SupportFunctions::readBoolFromQSettings("PupilDetectionSettingsDialog.processROI", roiPreprocessingBox->isChecked(), applicationSettings));
    pupilDetection->enableOutlineConfidence(SupportFunctions::readBoolFromQSettings("PupilDetectionSettingsDialog.outlineConfidence", true, applicationSettings));
    pupilDetection->enableROIPreProcessing(SupportFunctions::readBoolFromQSettings("PupilDetectionSettingsDialog.processROI", true, applicationSettings));
    pupilDetection->enablePupilUndistortion(SupportFunctions::readBoolFromQSettings("PupilDetectionSettingsDialog.undistortPupilSize", pupilUndistortionBox->isChecked(), applicationSettings));
    pupilDetection->enableImageUndistortion(SupportFunctions::readBoolFromQSettings("PupilDetectionSettingsDialog.undistortImage", imageUndistortionBox->isChecked(), applicationSettings));

    pupilMethodSettings[algorithmBox->currentIndex()]->loadSettings();

    lastKnownProcMode = pupilDetection->getCurrentProcMode();

    updateForm();
}

// Save the detection settings to the application settings which are persisted on disk
void PupilDetectionSettingsDialog::saveUniversalSettings() {
    if(!pupilDetection->isTrackingOn() && procModeBox->currentIndex()!=0) {
        if(!pupilDetection->isStereo()) {
            applicationSettings->setValue("PupilDetectionSettingsDialog.singleCam.procMode", procModeBox->currentIndex());
        } else {
            applicationSettings->setValue("PupilDetectionSettingsDialog.stereoCam.procMode", procModeBox->currentIndex());
        }
    }

    applicationSettings->setValue("PupilDetectionSettingsDialog.algorithm", algorithmBox->currentText());
    applicationSettings->setValue("PupilDetectionSettingsDialog.outlineConfidence", outlineConfidenceBox->isChecked());
    applicationSettings->setValue("PupilDetectionSettingsDialog.processROI", roiPreprocessingBox->isChecked());
    applicationSettings->setValue("PupilDetectionSettingsDialog.undistortPupilSize", pupilUndistortionBox->isChecked());
    applicationSettings->setValue("PupilDetectionSettingsDialog.undistortImage", imageUndistortionBox->isChecked());
}

// Show and hide the image processing mode specific info depending on the current selection
void PupilDetectionSettingsDialog::onProcModeSelection(int idx) {

    if(idx == 0) {
        iLabelFakeButton->setIcon(procModeIcon_undetermined);
        procModeInfoLabel->setText("Here you can specify how many eyes the program should look for \nwhen performing pupil detection, depending on physical arrangement \nof the camera(s) and eye(s).");
    } else if(idx == 1) {
        iLabelFakeButton->setIcon(procModeIcon_1cam1pup);
        procModeInfoLabel->setText("Detecting one pupil from a single camera viewpoint. \n(Low CPU load)"); // This way no eye distance can be measured optically.
    } else if(idx == 2) {
        iLabelFakeButton->setIcon(procModeIcon_1cam2pup);
        procModeInfoLabel->setText("Detecting both pupils from a single camera viewpoint. \n(Medium CPU load)"); // This way no eye distance can be measured optically.
    } else if(idx == 3) {
        iLabelFakeButton->setIcon(procModeIcon_2cam1pup);
        procModeInfoLabel->setText("Detecting one pupil from two cameras, producing a stereoscopic \npair of viewpoints. (Medium CPU load)");
    } else if(idx == 4) {
        iLabelFakeButton->setIcon(procModeIcon_2cam2pup);
        procModeInfoLabel->setText("Detecting both pupils from two cameras, producing stereoscopic \npairs of viewpoints. (High CPU load)");
    } 
    // else if(idx == 5) {
    //     iLabel->setPixmap(procModePixmap_1Mcam1pup);
    //     procModeInfoLabel->setText("Detecting one pupil from a single camera, but through two different \nviewpoints via an image splitter arrangement of a knife edge prism and \ntwo mirrors. (Medium CPU load)");
    // } //else {}
}

// Show and hide the algorithm specific settings depending on the current algorithm selection
void PupilDetectionSettingsDialog::onAlgorithmSelection(int idx) {

    int i = 0;
    for(auto pms: pupilMethodSettings) {
        if(i==idx) {
            pms->show();
            pms->infoBox->show();
        } else {
            pms->hide();
            pms->infoBox->hide();
        }
        i++;
    }
//    this->update();
}

// Apply the settings to the pupil detection process and save them to the application settings
void PupilDetectionSettingsDialog::applyButtonClick() {

    bool procModeChanged = (lastKnownProcMode != procModeBox->currentIndex());

    if(pupilDetection->hasOpenCamera() && !pupilDetection->isTrackingOn() && procModeChanged) {
        pupilDetection->setCurrentProcMode(procModeBox->currentIndex());
        lastKnownProcMode = (ProcMode)procModeBox->currentIndex();
        emit pupilDetectionProcModeChanged(procModeBox->currentIndex());
    } else {
        procModeBox->setCurrentIndex(pupilDetection->getCurrentProcMode());
    }

    pupilDetection->setAlgorithm(algorithmBox->currentText());
    pupilDetection->enableOutlineConfidence(outlineConfidenceBox->isChecked());
    pupilDetection->enableROIPreProcessing(roiPreprocessingBox->isChecked());
    pupilDetection->enablePupilUndistortion(pupilUndistortionBox->isChecked());
    pupilDetection->enableImageUndistortion(imageUndistortionBox->isChecked());

    pupilMethodSettings[algorithmBox->currentIndex()]->applyAndSaveSpecificSettings();

    if(pupilMethodSettings[algorithmBox->currentIndex()]->isAutoParamEnabled()) {
        pupilDetection->setAutoParamEnabled(true);
    } else {
        pupilDetection->setAutoParamEnabled(false);
    }

    saveUniversalSettings();
}

void PupilDetectionSettingsDialog::cancelButtonClick() {
    close();
}

void PupilDetectionSettingsDialog::applyCloseButtonClick() {

    applyButtonClick();
    close();
}

// This slot is called by a settings changed signal, used to propagate that at some other point a setting changed
// If a subject configuration is loaded, the application settings are changed, thus the need to be loaded again here
// GB: also, this is called whenever a new camera is connected, etc to actualize proc mode selection
void PupilDetectionSettingsDialog::onSettingsChange() {
    loadSettings();

    updateProcModeEnabled();
    updateProcModeCompatibility();
}

void PupilDetectionSettingsDialog::onPupilUndistortionClick(int state) {

    if(state == Qt::Checked) {
        imageUndistortionBox->setChecked(false);
    }
}

void PupilDetectionSettingsDialog::onImageUndistortionClick(int state) {

    if(state == Qt::Checked) {
        pupilUndistortionBox->setChecked(false);
    }
}

void PupilDetectionSettingsDialog::updateProcModeEnabled() {
    // neither can it be set if there is no pupilDetection instance in memory
    if(pupilDetection == nullptr)
        return;
    // nor when there is no camera selected yet
    //if(!pupilDetection->hasCamera()) { // GB: this still returns true when we have opened and then disconnected from a device
    if(!pupilDetection->hasOpenCamera()) { 
        procModeGroup->setEnabled(false);
    } else {
        // NOTE: now proc mode can not be changed while tracking is on, just because there may be a dataWriter or dataStreamer instance depending on it
        procModeGroup->setEnabled( !pupilDetection->isTrackingOn() );
    }
}

void PupilDetectionSettingsDialog::updateProcModeCompatibility() {
    if(!pupilDetection->isStereo()) {
        /*
        procModeBox->addItem(QString::fromStdString("(Undetermined)"));
        procModeBox->addItem(QString::fromStdString("Single camera image for one pupil"));
        procModeBox->addItem(QString::fromStdString("Single camera image for two pupils"));
        procModeBox->addItem(QString::fromStdString("Stereo camera images for one pupil (stereoscopic)"));
        procModeBox->addItem(QString::fromStdString("Stereo camera images for two pupils (stereoscopic)"));
        procModeBox->addItem(QString::fromStdString("Mirrored single camera image for one pupil"));
        */
        
        std::vector<bool> stateArr = {false, true, true, false, false}; // TODO: bug: valamiért az első elem disabled lesz mindig. miért?
        
        QStandardItem *item;
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(procModeBox->model());

        for(int idx = 0; idx < stateArr.size(); idx++) {
            item = model->item(idx);
            //std::cout << item->text().toStdString() << std::endl;
            item->setFlags(stateArr[idx] ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled);
        }

    } else {

        std::vector<bool> stateArr = {false, false, false, true, true};
        
        QStandardItem *item;
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(procModeBox->model());

        for(int idx = 0; idx < stateArr.size(); idx++) {
            item = model->item(idx);
            //std::cout << item->text().toStdString() << std::endl;
            item->setFlags(stateArr[idx] ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled);
        }

    }
    //procModeGroup->setEnabled(state);
}

void PupilDetectionSettingsDialog::onDisableProcModeSelector(bool state) {
    procModeGroup->setEnabled(!state);
}
