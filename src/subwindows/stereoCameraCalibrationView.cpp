#include "stereoCameraCalibrationView.h"

#include <QtWidgets/QHBoxLayout>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QtWidgets>

// Create new widget with a given stereo camera
// Loads the cameras stereo calibration process and show its processed images in a live-view
StereoCameraCalibrationView::StereoCameraCalibrationView(StereoCamera *camera, QWidget *parent): QWidget(parent), camera(camera), calibrationHelpDialog(new CalibrationHelpDialog(this)) {

    calibrationWorker = camera->getCameraCalibration();

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout *videoViewLayout = new QHBoxLayout();

    mainVideoView = new VideoView();
    videoViewLayout->addWidget(mainVideoView);
    secondaryVideoView = new VideoView();
    videoViewLayout->addWidget(secondaryVideoView);
    layout->addLayout(videoViewLayout);

    QGroupBox *qualityGroup = new QGroupBox("Calibration Quality");
    QHBoxLayout *qualityLayout = new QHBoxLayout();
    QLabel *sharpnessThresholdLabel = new QLabel(tr("Avg. Sharpness Threshold [<a href=\"http://mock.link\">?</a>]:"));
    connect(sharpnessThresholdLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));
    sharpnessThresholdBox = new QSpinBox();
    sharpnessThresholdBox->setMinimum(1);
    sharpnessThresholdBox->setValue(calibrationWorker->getSharpnessThreshold());

    connect(sharpnessThresholdBox, SIGNAL(valueChanged(int)), this, SLOT(onSharpnessThresholdBoxChange()));

    qualityLayout->addWidget(sharpnessThresholdLabel);
    qualityLayout->addWidget(sharpnessThresholdBox);
    qualityLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    qualityGroup->setLayout(qualityLayout);
    layout->addWidget(qualityGroup);

    QGroupBox *patternGroup = new QGroupBox("Calibration Pattern");
    QHBoxLayout *patternLayout = new QHBoxLayout();

    QButtonGroup *patternCheckboxGroup = new QButtonGroup();
    QVBoxLayout *checkboxLayout = new QVBoxLayout();

    chessboardCheckbox = new QCheckBox("Chessboard");
    chessboardCheckbox->setChecked(true);
    circlesCheckbox = new QCheckBox("Circles");
    asymmetricCirclesCheckbox = new QCheckBox("Asymmetric Circles");

    patternCheckboxGroup->addButton(chessboardCheckbox);
    patternCheckboxGroup->addButton(circlesCheckbox);
    patternCheckboxGroup->addButton(asymmetricCirclesCheckbox);
    checkboxLayout->addWidget(chessboardCheckbox);
    checkboxLayout->addWidget(circlesCheckbox);
    checkboxLayout->addWidget(asymmetricCirclesCheckbox);

    connect(patternCheckboxGroup, SIGNAL(buttonClicked(int)), this, SLOT(onCheckboxClick()));

    if(calibrationWorker->getPattern() == CalibrationPattern::CHESSBOARD) {
        chessboardCheckbox->setChecked(true);
    } else if(calibrationWorker->getPattern() == CalibrationPattern::CIRCLES_GRID ) {
        circlesCheckbox->setChecked(true);
    } else {
        asymmetricCirclesCheckbox->setChecked(true);
    }

    QFormLayout *sizeLayout = new QFormLayout();
    QLabel *verticalCornerLabel = new QLabel(tr("Rows [<a href=\"http://mock.link\">?</a>]:"));
    connect(verticalCornerLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));
    verticalCornerBox = new QSpinBox();
    verticalCornerBox->setMinimum(1);
    verticalCornerBox->setValue(calibrationWorker->getBoardSize().height);

    connect(verticalCornerBox, SIGNAL(valueChanged(int)), this, SLOT(onSizeBoxChange()));

    sizeLayout->addRow(verticalCornerLabel, verticalCornerBox);

    QLabel *horizontalCornerLabel = new QLabel(tr("Columns [<a href=\"http://mock.link\">?</a>]:"));
    connect(horizontalCornerLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));

    horizontalCornerBox = new QSpinBox();
    horizontalCornerBox->setMinimum(1);
    horizontalCornerBox->setValue(calibrationWorker->getBoardSize().width);

    connect(horizontalCornerBox, SIGNAL(valueChanged(int)), this, SLOT(onSizeBoxChange()));

    sizeLayout->addRow(horizontalCornerLabel, horizontalCornerBox);

    QLabel *squareSizeLabel = new QLabel(tr("Checker/Circle Width [mm] [<a href=\"http://mock.link\">?</a>]:"));
    connect(squareSizeLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));
    squareSizeBox = new QSpinBox();
    squareSizeBox->setMinimum(1);
    squareSizeBox->setValue(calibrationWorker->getSquareSize());

    connect(squareSizeBox, SIGNAL(valueChanged(int)), this, SLOT(onSizeBoxChange()));

    sizeLayout->addRow(squareSizeLabel, squareSizeBox);

    patternLayout->addLayout(checkboxLayout);
    patternLayout->addLayout(sizeLayout);
    patternGroup->setLayout(patternLayout);
    layout->addWidget(patternGroup);


    QHBoxLayout* verifyLayout = new QHBoxLayout();

    verifyFileBox = new QCheckBox("Save verification to file.");

    connect(verifyFileBox, SIGNAL(toggled(bool)), this, SLOT(onVerifyFileChecked(bool)));

    verifyLayout->addWidget(verifyFileBox);
    QSpacerItem *spv = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    verifyLayout->addSpacerItem(spv);

    verifyLayout->setContentsMargins(10, 6, 6, 6);

    layout->addLayout(verifyLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    calibrateButton = new QPushButton("Calibrate");
    verifyButton = new QPushButton("Verify");
    verifyButton->setEnabled(calibrationWorker->isCalibrated());
    stopButton = new QPushButton("Stop");
    stopButton->setDisabled(true);
    saveButton = new QPushButton("Save");
    loadButton = new QPushButton("Load");

    QSpacerItem *sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addSpacerItem(sp);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(calibrateButton);
    buttonLayout->addWidget(verifyButton);

    layout->addLayout(buttonLayout);
    setLayout(layout);

    connect(calibrateButton, SIGNAL(clicked()), this, SLOT(onCalibrateClick()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveClick()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(onLoadClick()));
    connect(verifyButton, SIGNAL(clicked()), this, SLOT(onVerifyClick()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(onStopClick()));

    // Propagate images to the calibration process
    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), calibrationWorker, SLOT(onNewImage(CameraImage)));
    //connect(this, SIGNAL(onNewImage(CameraImage)), calibrationWorker, SLOT(onNewImage(CameraImage)));
    // Show the processed calibration images in the live-view
    connect(calibrationWorker, SIGNAL(processedImageLowFPS(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(calibrationWorker, SIGNAL (finishedCalibration()), this, SLOT (onCalibrationFinished()));

    if(!calibrationWorker->isCalibrated()) {
        // If we already opened this camera, a config file may exists
        loadConfigFile();
    }
}

StereoCameraCalibrationView::~StereoCameraCalibrationView() {

}

// Received image signals from the calibration process
void StereoCameraCalibrationView::updateView(const CameraImage &cimg) {

    if(!cimg.img.empty() && !cimg.imgSecondary.empty()) {

        mainVideoView->updateView(cimg.img);
        secondaryVideoView->updateView(cimg.imgSecondary);
    }
}

// Starts the calibration process
void StereoCameraCalibrationView::onCalibrateClick() {

    calibrationWorker->startCapturing();
    stopButton->setDisabled(false);
    calibrateButton->setDisabled(true);
}

// Starts the calibration verification process
void StereoCameraCalibrationView::onVerifyClick() {

    calibrationWorker->startVerifying();
    stopButton->setDisabled(false);
    calibrateButton->setDisabled(true);
    verifyButton->setDisabled(true);
}

// Slot event handler for the calibartion finished signal of the calibration process
void StereoCameraCalibrationView::onCalibrationFinished() {
    // This gets called on calibration finished
    verifyButton->setDisabled(false);
    stopButton->setDisabled(true);
}

void StereoCameraCalibrationView::onStopClick() {
    calibrationWorker->stop();
    stopButton->setDisabled(true);
    calibrateButton->setDisabled(false);
    verifyButton->setEnabled(calibrationWorker->isCalibrated());
}

// Selects the calibration pattern used
// Each change in the calibration config invalidates the current calibration, thus its loaded again if a file exist
void StereoCameraCalibrationView::onCheckboxClick() {

    if(chessboardCheckbox->isChecked()) {
        calibrationWorker->setPattern(CalibrationPattern::CHESSBOARD);
    } else if(circlesCheckbox->isChecked()) {
        calibrationWorker->setPattern(CalibrationPattern::CIRCLES_GRID);
    } else {
        calibrationWorker->setPattern(CalibrationPattern::ASYMMETRIC_CIRCLES_GRID);
    }

    loadConfigFile();
}

void StereoCameraCalibrationView::onSaveClick() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Calibration File"), "", tr("XML files (*.xml)"));

    if(!filename.isEmpty()) {
        // filename.replace(" ", ""); // Commented out by BG on 2024.04.22., refer to https://github.com/openPupil/Open-PupilEXT/issues/42

        QFileInfo fileInfo(filename);
        // check if filename has extension, else append xml to it
        if(fileInfo.suffix().isEmpty()) {
            filename = filename + ".xml";
        }

        std::cout<<"Saving calibration file to settings directory: "<< filename.toStdString() <<std::endl;
        calibrationWorker->saveToFile(filename.toStdString().c_str());
    }
}

void StereoCameraCalibrationView::onLoadClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Calibration File"), "", tr("XML files (*.xml)"));

    if(!filename.isEmpty()) {

        calibrationWorker->loadFromFile(filename.toStdString().c_str());
        updateSettings();
    }
}

void StereoCameraCalibrationView::closeEvent(QCloseEvent *event) {

    disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), calibrationWorker, SLOT(onNewImage(CameraImage)));

    if(calibrationWorker->isCalibrated()) {
        QString configFile = camera->getCalibrationFilename();
        //configFile.replace(" ", ""); // Commented out by BG on 2024.04.22., refer to https://github.com/openPupil/Open-PupilEXT/issues/42
        std::cout<<"Saving calibration file to settings directory: "<< configFile.toStdString() <<std::endl;
        calibrationWorker->saveToFile(configFile.toStdString().c_str());
        QWidget::closeEvent(event);
    }
}

// Each change in the calibration config invalidates the current calibration, thus its loaded again if a file exist
void StereoCameraCalibrationView::onSizeBoxChange() {

    if(calibrationWorker->isCalibrated())
        calibrationWorker->reset();

    calibrationWorker->setBoardSize(horizontalCornerBox->value(), verticalCornerBox->value());
    calibrationWorker->setSquareSize(squareSizeBox->value());

    loadConfigFile();
}

// Sets the sharpness threshold of the calibration
// The threshold is used to filter bad images
void StereoCameraCalibrationView::onSharpnessThresholdBoxChange() {

    calibrationWorker->setSharpnessThreshold(sharpnessThresholdBox->value());
}

// Loads a pre-specified calibration file if it exists
// Calibration filename is predefined by the configured calibration settings (pattern size etc.)
void StereoCameraCalibrationView::loadConfigFile() {
    QString configFile = camera->getCalibrationFilename();
    //configFile.replace(" ", ""); // Commented out by BG on 2024.04.22., refer to https://github.com/openPupil/Open-PupilEXT/issues/42
    if (QFile::exists(configFile)) {
        std::cout << "Found calibration file in settings directory. Loading: " << configFile.toStdString()
                  << std::endl;
        calibrationWorker->loadFromFile(configFile.toStdString().c_str());
        updateSettings();
    }
}

void StereoCameraCalibrationView::updateSettings() {

    if(calibrationWorker->getPattern() == CalibrationPattern::CHESSBOARD) {
        chessboardCheckbox->setChecked(true);
    } else if(calibrationWorker->getPattern() == CalibrationPattern::CIRCLES_GRID ) {
        circlesCheckbox->setChecked(true);
    } else {
        asymmetricCirclesCheckbox->setChecked(true);
    }

    horizontalCornerBox->setValue(calibrationWorker->getBoardSize().width);
    verticalCornerBox->setValue(calibrationWorker->getBoardSize().height);

    squareSizeBox->setValue(calibrationWorker->getSquareSize());

    verifyButton->setEnabled(calibrationWorker->isCalibrated());
}

void StereoCameraCalibrationView::onShowHelpDialog() {

    calibrationHelpDialog->show();
}

// Sets the output filepath of the verification file containing calibration errors of the verify phase
void StereoCameraCalibrationView::onVerifyFileChecked(bool value) {

    if(value) {
        QString verifyFileName = QFileDialog::getSaveFileName(this, tr("Save verification history to file"), "", tr("CSV files (*.csv)"), nullptr, QFileDialog::DontConfirmOverwrite);

        if(!verifyFileName.isEmpty()) {

            calibrationWorker->setVerifyOutputPath(verifyFileName);
        } else {
            verifyFileBox->setChecked(false);
            calibrationWorker->setVerifyOutputPath("");
        }
    }
}