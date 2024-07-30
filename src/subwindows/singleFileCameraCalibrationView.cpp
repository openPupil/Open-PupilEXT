
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QtWidgets>
#include "singleFileCameraCalibrationView.h"

// Given a file camera which loaded a single camera recording of an already calibrated camera
// Loads the calibration process of the file camera and load a user specified calibration file
SingleFileCameraCalibrationView::SingleFileCameraCalibrationView(FileCamera *camera, QWidget *parent): QWidget(parent), camera(camera) {

    this->setMinimumSize(600, 330);
    this->setWindowTitle("Offline Single Calibration");

    calibrationWorker = camera->getCameraCalibration();

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    QVBoxLayout* layout = new QVBoxLayout();

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

    // Take actions in this window when calibration successful
    //connect(calibrationWorker, SIGNAL (finishedCalibration()), this, SLOT (onCalibrationFinished()));
}

// Event handler for the load button click
// User specifies file which is loaded into the calibration
// If load is successful, meta information about the calibration are shown in a textfield to the user
void SingleFileCameraCalibrationView::onLoadClick() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Calibration File"), "", tr("XML files (*.xml)"));

    if(!filename.isEmpty()) {

        calibrationWorker->loadFromFile(filename.toStdString().c_str());
        updateSettings();

        textField->clear();
        if(calibrationWorker->isCalibrated()) {
            textField->append(tr("Calibration Found."));
            textField->append(tr("Image Size: %1x%2px").arg(calibrationWorker->getImageSize().width).arg(calibrationWorker->getImageSize().height));

            textField->append(tr("Main RMSE: %1px, avg. MAE: %2px")
                                      .arg(calibrationWorker->getRMSE()).arg(calibrationWorker->getAvgMAE()));
        } else {
            textField->append(tr("Calibration file could not been loaded."));
        }
    }
}

// Update forms based on loaded calibration settings
void SingleFileCameraCalibrationView::updateSettings() {

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

//void SingleFileCameraCalibrationView::onCalibrationFinished() {
//}

SingleFileCameraCalibrationView::~SingleFileCameraCalibrationView() = default;
