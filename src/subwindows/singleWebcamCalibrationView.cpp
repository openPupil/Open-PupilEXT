
#include "singleWebcamCalibrationView.h"

SingleWebcamCalibrationView::SingleWebcamCalibrationView(SingleWebcam *camera, QWidget *parent): QWidget(parent), camera(camera), calibrationHelpDialog(new CalibrationHelpDialog(this)) {

    calibrationWorker = camera->getCameraCalibration();

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    videoView = new VideoView();
    layout->addWidget(videoView);

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

    QFormLayout *sizeLayout = new QFormLayout();
    QLabel *verticalCornerLabel = new QLabel(tr("Rows  [<a href=\"http://mock.link\">?</a>]:"));
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
    verifyButton->setDisabled(true);
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
    //layout->setContentsMargins(0,0,0,0);
    //layout->setSpacing(0);
    setLayout(layout);

    connect(calibrateButton, SIGNAL(clicked()), this, SLOT(onCalibrateClick()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveClick()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(onLoadClick()));
    connect(verifyButton, SIGNAL(clicked()), this, SLOT(onVerifyClick()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(onStopClick()));

    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), calibrationWorker, SLOT(onNewImage(CameraImage)));
    //connect(this, SIGNAL(onNewImage(CameraImage)), calibrationWorker, SLOT(onNewImage(CameraImage)));
    connect(calibrationWorker, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(calibrationWorker, SIGNAL (finishedCalibration()), this, SLOT (onCalibrationFinished()));

    if(!calibrationWorker->isCalibrated()) {
        // If we already calibrated this camera, a file may exists
        loadConfigFile();
    }
}

SingleWebcamCalibrationView::~SingleWebcamCalibrationView() {

}

// Update the cameraview widget in the window
// This slot is connected to the processedImage signal which is send at a slowed down rate than the camera rate ie. 30fps
void SingleWebcamCalibrationView::updateView(const CameraImage &cimg) {

    if(!cimg.img.empty()) {
        videoView->updateView(cimg.img);
    }
}

void SingleWebcamCalibrationView::onCalibrateClick() {

    calibrationWorker->startCapturing();
    stopButton->setDisabled(false);
    calibrateButton->setDisabled(true);
}

void SingleWebcamCalibrationView::onVerifyClick() {

    calibrationWorker->startVerifying();
    stopButton->setDisabled(false);
    calibrateButton->setDisabled(true);
    verifyButton->setDisabled(true);
}

// Slot that is called from the camera calibration upon successfull calibration
void SingleWebcamCalibrationView::onCalibrationFinished() {
    // this gets called on calibration and verify
    verifyButton->setDisabled(false);
    stopButton->setDisabled(true);
}

void SingleWebcamCalibrationView::onStopClick() {
    calibrationWorker->stop();
    stopButton->setDisabled(true);
    calibrateButton->setDisabled(false);
    verifyButton->setEnabled(calibrationWorker->isCalibrated());
}

void SingleWebcamCalibrationView::onCheckboxClick() {

    if(chessboardCheckbox->isChecked()) {
        calibrationWorker->setPattern(CalibrationPattern::CHESSBOARD);
    } else if(circlesCheckbox->isChecked()) {
        calibrationWorker->setPattern(CalibrationPattern::CIRCLES_GRID);
    } else {
        calibrationWorker->setPattern(CalibrationPattern::ASYMMETRIC_CIRCLES_GRID);
    }

    loadConfigFile();
}

// Save the calibration file to a selected filename
// File is in the OpenCV XML filestorage fileformat
void SingleWebcamCalibrationView::onSaveClick() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Calibration File"), "", tr("XML files (*.xml)"));

    if(!filename.isEmpty()) {
        QFileInfo fileInfo(filename);

        // check if filename has extension
        if(fileInfo.suffix().isEmpty()) {
            filename = filename + ".xml";
        }

        calibrationWorker->saveToFile(filename.toStdString().c_str());
    }
}

// Load a calibration file from a selected filename
// File is in the OpenCV XML filestorage fileformat
void SingleWebcamCalibrationView::onLoadClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Calibration File"), "", tr("XML files (*.xml)"));

    if(!filename.isEmpty()) {

        calibrationWorker->loadFromFile(filename.toStdString().c_str());
        updateSettings();
    }
}

// On window close, save the current calibration to the application settings directory
// This file is again loaded upon reopening of the window through loadConfigFile()
void SingleWebcamCalibrationView::closeEvent(QCloseEvent *event) {

    disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), calibrationWorker, SLOT(onNewImage(CameraImage)));

    if(calibrationWorker->isCalibrated()) {
        QString configFile = camera->getCalibrationFilename();
        //configFile.replace(" ", ""); // Commented out by BG on 2024.04.22., refer to https://github.com/openPupil/Open-PupilEXT/issues/42
        std::cout<<"Saving calibration file to settings directory: "<< configFile.toStdString() <<std::endl;
        calibrationWorker->saveToFile(configFile.toStdString().c_str());
        QWidget::closeEvent(event);
    }
}

// On change of the calibration pattern square size, calibration gets invalid is one exists
void SingleWebcamCalibrationView::onSizeBoxChange() {

    if(calibrationWorker->isCalibrated())
        calibrationWorker->reset();

    calibrationWorker->setBoardSize(horizontalCornerBox->value(), verticalCornerBox->value());
    calibrationWorker->setSquareSize(squareSizeBox->value());

    std::cout<<"Squaresize changed: "<<calibrationWorker->getSquareSize()<<std::endl;

    loadConfigFile();
}

// Sharpness threshold to filter out bad calibration images based on the sharpness of the pattern contrast
void SingleWebcamCalibrationView::onSharpnessThresholdBoxChange() {

    calibrationWorker->setSharpnessThreshold(sharpnessThresholdBox->value());
}

void SingleWebcamCalibrationView::updateSettings() {

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

// Load the calibration file from predefined path
// File is in the OpenCV XML filestorage fileformat
void SingleWebcamCalibrationView::loadConfigFile() {
    QString configFile = camera->getCalibrationFilename();
    //configFile.replace(" ", ""); // Commented out by BG on 2024.04.22., refer to https://github.com/openPupil/Open-PupilEXT/issues/42
    if (QFile::exists(configFile)) {
        std::cout << "Found calibration file in settings directory. Loading: " << configFile.toStdString()
                  << std::endl;
        calibrationWorker->loadFromFile(configFile.toStdString().c_str());
        updateSettings();
    }
}

void SingleWebcamCalibrationView::onShowHelpDialog() {

    calibrationHelpDialog->show();
}

// Set an output path for saving the verification file in the csv file format
void SingleWebcamCalibrationView::onVerifyFileChecked(bool value) {

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
