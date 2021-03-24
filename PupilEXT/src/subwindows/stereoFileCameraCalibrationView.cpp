
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QtWidgets>
#include "stereoFileCameraCalibrationView.h"

// Creates a new widget for loading a offline stereo calibration file given a filecamera in stereo mode
// Using the offline calibration, stereo triangulation can be performed on recorded stereo images
StereoFileCameraCalibrationView::StereoFileCameraCalibrationView(FileCamera *camera, QWidget *parent): QWidget(parent), camera(camera) {

    this->setMinimumSize(600, 330);
    this->setWindowTitle("Offline Stereo Calibration");

    calibrationWorker = camera->getStereoCameraCalibration();

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    textField = new QTextEdit();
    textField->setText("Not calibrated.");
    textField->setTextInteractionFlags(Qt::NoTextInteraction);
    layout->addWidget(textField);

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

    if(calibrationWorker->getPattern() == CalibrationPattern::CHESSBOARD) {
        chessboardCheckbox->setChecked(true);
    } else if(calibrationWorker->getPattern() == CalibrationPattern::CIRCLES_GRID ) {
        circlesCheckbox->setChecked(true);
    } else {
        asymmetricCirclesCheckbox->setChecked(true);
    }

    QFormLayout *sizeLayout = new QFormLayout();
    QLabel *verticalCornerLabel = new QLabel(tr("Rows:"));
    verticalCornerBox = new QSpinBox();
    verticalCornerBox->setMinimum(1);
    verticalCornerBox->setValue(calibrationWorker->getBoardSize().height+1);

    sizeLayout->addRow(verticalCornerLabel, verticalCornerBox);

    QLabel *horizontalCornerLabel = new QLabel(tr("Columns:"));
    horizontalCornerBox = new QSpinBox();
    horizontalCornerBox->setMinimum(1);
    horizontalCornerBox->setValue(calibrationWorker->getBoardSize().width+1);

    sizeLayout->addRow(horizontalCornerLabel, horizontalCornerBox);

    QLabel *squareSizeLabel = new QLabel(tr("Checker/Circle Width [mm]"));
    squareSizeBox = new QSpinBox();
    squareSizeBox->setMinimum(1);
    squareSizeBox->setValue(calibrationWorker->getSquareSize());

    sizeLayout->addRow(squareSizeLabel, squareSizeBox);

    patternLayout->addLayout(checkboxLayout);
    patternLayout->addLayout(sizeLayout);
    patternGroup->setLayout(patternLayout);
    layout->addWidget(patternGroup);
    patternGroup->setDisabled(true);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    loadButton = new QPushButton("Load");

    QSpacerItem *sp = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addSpacerItem(sp);

    layout->addLayout(buttonLayout);
    setLayout(layout);

    connect(loadButton, SIGNAL(clicked()), this, SLOT(onLoadClick()));

    // Calibration process sends signal when calibration is loaded successfully
    //connect(calibrationWorker, SIGNAL (finishedCalibration()), this, SLOT (onCalibrationFinished()));
}

// On load button click, user specifies a filepath which is then tried to loaded
// When successfully loaded, calibration meta information are displayed in the textfield
void StereoFileCameraCalibrationView::onLoadClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Calibration File"), "", tr("XML files (*.xml)"));

    if(!filename.isEmpty()) {

        calibrationWorker->loadFromFile(filename.toStdString().c_str());
        updateSettings();

        textField->clear();
        if(calibrationWorker->isCalibrated()) {
            textField->append(tr("Calibration Found."));
            textField->append(tr("Image Size: %1x%2px").arg(calibrationWorker->getImageSize().width).arg(calibrationWorker->getImageSize().height));

            textField->append(tr("Main RMSE: %1px, avg. MAE: %2px")
                                      .arg(calibrationWorker->getMainRMSE()).arg(calibrationWorker->getMainMAE()));
            textField->append(tr("Secondary RMSE: %1px, avg. MAE: %2px")
                                      .arg(calibrationWorker->getSecondaryRMSE()).arg(calibrationWorker->getSecondaryMAE()));

            textField->append(tr("Stereo RMSE: %1px, avg. World Pattern Distance: %2mm")
                                      .arg(calibrationWorker->getStereoRMSE()).arg(calibrationWorker->getWorldMAE()));
        } else {
            textField->append(tr("Calibration file could not been loaded."));
        }
    }
}

// Updates the form inputs based on calibration settings of the calibration process
void StereoFileCameraCalibrationView::updateSettings() {

    if(calibrationWorker->getPattern() == CalibrationPattern::CHESSBOARD) {
        chessboardCheckbox->setChecked(true);
    } else if(calibrationWorker->getPattern() == CalibrationPattern::CIRCLES_GRID ) {
        circlesCheckbox->setChecked(true);
    } else {
        asymmetricCirclesCheckbox->setChecked(true);
    }

    horizontalCornerBox->setValue(calibrationWorker->getBoardSize().width+1);
    verticalCornerBox->setValue(calibrationWorker->getBoardSize().height+1);
    squareSizeBox->setValue(calibrationWorker->getSquareSize());
}

// Slot handling the successful calibration loading
//void StereoFileCameraCalibrationView::onCalibrationFinished() {
//    // Nothing to the here yet
//}

StereoFileCameraCalibrationView::~StereoFileCameraCalibrationView() {

}
