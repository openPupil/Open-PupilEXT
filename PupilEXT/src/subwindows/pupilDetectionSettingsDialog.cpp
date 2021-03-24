
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

// Create the pupil detection settings dialog
// Given a pupil detection object to communicate to the detection algorithm objects there
PupilDetectionSettingsDialog::PupilDetectionSettingsDialog(PupilDetection *pupilDetection, QWidget *parent) :
        QDialog(parent),
        pupilDetection(pupilDetection),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setMinimumSize(800, 500);
    this->setWindowTitle("Pupil Detection Settings");

    createForm();

    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyButtonClick()));
    connect(applyCloseButton, SIGNAL(clicked()), this, SLOT(applyCloseButtonClick()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClick()));

    connect(algorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgorithmSelection(int)));

    onAlgorithmSelection(algorithmBox->currentIndex());
    loadSettings();
}

PupilDetectionSettingsDialog::~PupilDetectionSettingsDialog() {

}

void PupilDetectionSettingsDialog::createForm() {

    QGridLayout *mainLayout = new QGridLayout(this);

    QGroupBox *algorithmGroup = new QGroupBox("Algorithm");
    QGroupBox *optionsGroup = new QGroupBox("Options");

    QGridLayout *algorithmLayout = new QGridLayout();
    QHBoxLayout *algoBoxLayout = new QHBoxLayout();

    QLabel *algorithmLabel = new QLabel(tr("Algorithm:"));
    algorithmBox = new QComboBox();

    for(auto pm: pupilDetection->getMethods()) {
        algorithmBox->addItem(QString::fromStdString(pm->title()));
    }
    algorithmBox->setCurrentText(QString::fromStdString(pupilDetection->getCurrentMethod()->title()));

    algoBoxLayout->addWidget(algorithmLabel, 0);
    algoBoxLayout->addWidget(algorithmBox, 1);

    algorithmLayout->addLayout(algoBoxLayout, 0, 0, 1, 1);

    algorithmGroup->setLayout(algorithmLayout);
    mainLayout->addWidget(algorithmGroup);

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


    QLabel *imageUndistortionLabel = new QLabel(tr("Undistort complete image (slow) [<a href=\"http://mock.link\">?</a>]:"));
    connect(pupilSizeUndistortionLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));

    imageUndistortionBox = new QCheckBox();
    imageUndistortionBox->setChecked(pupilDetection->isImageUndistortionEnabled());
    checkboxGroup->addButton(imageUndistortionBox);
    optionsLayout->addRow(imageUndistortionLabel, imageUndistortionBox);
    connect(imageUndistortionBox, SIGNAL(stateChanged(int)), this, SLOT(onImageUndistortionClick(int)));

    optionsGroup->setLayout(optionsLayout);
    mainLayout->addWidget(optionsGroup);

    // For each algorithm, a special widget is implemented that contains all algorithm specific parameters
    for(PupilDetectionMethod *pm: pupilDetection->getMethods()) {
        if(pm->title() == "PuRe") {
            PuReSettings *settings = new PuReSettings(dynamic_cast<PuRe*>(pm));
            // If the current pupil detection is stereo, two algorithm instances exist, which should be confiured the same way
            settings->addSecondary(dynamic_cast<PuRe*>(pupilDetection->getSecondaryMethod("PuRe")));
            connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));

            // We add all algorithm specific widget to the same point in the layout and hide them, only the current selected algorithm is shown
            mainLayout->addWidget(settings, 0, 1, 2, 1);
            settings->hide();
            pupilMethodSettings.push_back(settings);

            settings->infoBox->hide();
            algorithmLayout->addWidget(settings->infoBox, 1, 0);
        } else if(pm->title() == "PuReST") {
            PuReSTSettings *settings = new PuReSTSettings(dynamic_cast<PuReST*>(pm));
            settings->addSecondary(dynamic_cast<PuReST*>(pupilDetection->getSecondaryMethod("PuReST")));
            connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));

            mainLayout->addWidget(settings, 0, 1, 2, 1);
            settings->hide();
            pupilMethodSettings.push_back(settings);

            settings->infoBox->hide();
            algorithmLayout->addWidget(settings->infoBox, 1, 0);
        } else if(pm->title() == "ElSe") {
            ElSeSettings *settings = new ElSeSettings(dynamic_cast<ElSe*>(pm));
            settings->addSecondary(dynamic_cast<ElSe*>(pupilDetection->getSecondaryMethod("ElSe")));
            connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));

            mainLayout->addWidget(settings, 0, 1, 2, 1);
            settings->hide();
            pupilMethodSettings.push_back(settings);

            settings->infoBox->hide();
            algorithmLayout->addWidget(settings->infoBox, 1, 0);
        } else if(pm->title() == "ExCuSe") {
            ExCuSeSettings *settings = new ExCuSeSettings(dynamic_cast<ExCuSe*>(pm));
            settings->addSecondary(dynamic_cast<ExCuSe*>(pupilDetection->getSecondaryMethod("ExCuSe")));
            connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));

            mainLayout->addWidget(settings, 0, 1, 2, 1);
            settings->hide();
            pupilMethodSettings.push_back(settings);

            settings->infoBox->hide();
            algorithmLayout->addWidget(settings->infoBox, 1, 0);
        } else if(pm->title() == "Starburst") {
            StarburstSettings *settings = new StarburstSettings(dynamic_cast<Starburst*>(pm));
            settings->addSecondary(dynamic_cast<Starburst*>(pupilDetection->getSecondaryMethod("Starburst")));
            connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));

            mainLayout->addWidget(settings, 0, 1, 2, 1);
            settings->hide();
            pupilMethodSettings.push_back(settings);

            settings->infoBox->hide();
            algorithmLayout->addWidget(settings->infoBox, 1, 0);
        } else if(pm->title() == "Swirski2D") {
            Swirski2DSettings *settings = new Swirski2DSettings(dynamic_cast<Swirski2D*>(pm));
            settings->addSecondary(dynamic_cast<Swirski2D*>(pupilDetection->getSecondaryMethod("Swirski2D")));
            connect(settings, SIGNAL(onConfigChange(QString)), pupilDetection, SLOT(setConfigLabel(QString)));

            mainLayout->addWidget(settings, 0, 1, 2, 1);
            settings->hide();
            pupilMethodSettings.push_back(settings);

            settings->infoBox->hide();
            algorithmLayout->addWidget(settings->infoBox, 1, 0);
        } else {
            PupilMethodSetting *tmp = new PupilMethodSetting();
            mainLayout->addWidget(tmp, 0, 1, 2, 1);

            pupilMethodSettings.push_back(tmp);

            tmp->infoBox->hide();
            algorithmLayout->addWidget(tmp->infoBox, 1, 0);
        }
//        else if(pm->title() == "APPD") {
//            APPDSettings *settings = new APPDSettings(dynamic_cast<APPD*>(pm));
//            settings->addSecondary(dynamic_cast<APPD*>(pupilDetection->getSecondaryMethod("APPD")));
//            mainLayout->addWidget(settings, 0, 1, 2, 1);
//            settings->hide();
//            pupilMethodSettings.push_back(settings);
//
//            settings->infoBox->hide();
//            algorithmLayout->addWidget(settings->infoBox, 1, 0);
//        }
    }


    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    applyButton = new QPushButton(tr("Apply"));
    applyCloseButton = new QPushButton(tr("Apply and Close"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    buttonsLayout->addWidget(applyCloseButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout, 2, 0, 1, 2);

    setLayout(mainLayout);
}

// Update the dialog form from the settings configured in the pupil detection process
void PupilDetectionSettingsDialog::updateForm() {

    algorithmBox->setCurrentText(QString::fromStdString(pupilDetection->getCurrentMethod()->title()));
    roiPreprocessingBox->setChecked(pupilDetection->isROIPreProcessingEnabled());
    outlineConfidenceBox->setChecked(pupilDetection->isOutlineConfidenceEnabled());

    pupilUndistortionBox->setChecked(pupilDetection->isPupilUndistortionEnabled());
    imageUndistortionBox->setChecked(pupilDetection->isImageUndistortionEnabled());

    pupilMethodSettings[algorithmBox->currentIndex()]->updateSettings();
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

    pupilDetection->setAlgorithm(applicationSettings->value("PupilDetectionSettingsDialog.algorithm", algorithmBox->currentText()).toString());
    pupilDetection->enableOutlineConfidence(applicationSettings->value("PupilDetectionSettingsDialog.outlineConfidence", outlineConfidenceBox->isChecked()).toBool());
    pupilDetection->enableROIPreProcessing(applicationSettings->value("PupilDetectionSettingsDialog.processROI", roiPreprocessingBox->isChecked()).toBool());
    pupilDetection->enablePupilUndistortion(applicationSettings->value("PupilDetectionSettingsDialog.undistortPupilSize", pupilUndistortionBox->isChecked()).toBool());
    pupilDetection->enableImageUndistortion(applicationSettings->value("PupilDetectionSettingsDialog.undistortImage", imageUndistortionBox->isChecked()).toBool());

    pupilMethodSettings[algorithmBox->currentIndex()]->loadSettings();

    updateForm();
}

// Save the detection settings to the application settings which are persisted on disk
void PupilDetectionSettingsDialog::saveSettings() {

    applicationSettings->setValue("PupilDetectionSettingsDialog.algorithm", algorithmBox->currentText());
    applicationSettings->setValue("PupilDetectionSettingsDialog.outlineConfidence", outlineConfidenceBox->isChecked());
    applicationSettings->setValue("PupilDetectionSettingsDialog.processROI", roiPreprocessingBox->isChecked());
    applicationSettings->setValue("PupilDetectionSettingsDialog.undistortPupilSize", pupilUndistortionBox->isChecked());
    applicationSettings->setValue("PupilDetectionSettingsDialog.undistortImage", imageUndistortionBox->isChecked());
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
}

// Apply the settings to the pupil detection process and save them to the application settings
void PupilDetectionSettingsDialog::applyButtonClick() {

    pupilDetection->setAlgorithm(algorithmBox->currentText());
    pupilDetection->enableOutlineConfidence(outlineConfidenceBox->isChecked());
    pupilDetection->enableROIPreProcessing(roiPreprocessingBox->isChecked());
    pupilDetection->enablePupilUndistortion(pupilUndistortionBox->isChecked());
    pupilDetection->enableImageUndistortion(imageUndistortionBox->isChecked());

    pupilMethodSettings[algorithmBox->currentIndex()]->updateSettings();

    saveSettings();
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
void PupilDetectionSettingsDialog::onSettingsChange() {
    loadSettings();
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
