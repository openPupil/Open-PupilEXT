#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "singleCameraSharpnessView.h"

// Creates a new view given a single camera object
// Shows the live feed of the given camera and detects a calibration pattern in its images for which a sharpness value is calculated.
SingleCameraSharpnessView::SingleCameraSharpnessView(SingleCamera *camera, QWidget *parent) :
        QWidget(parent),
        camera(camera),
        calibrationHelpDialog(new CalibrationHelpDialog(this)),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    setWindowTitle("Single camera sharpness");

    cameraCalibration = camera->getCameraCalibration();

    sharpnessWorker = new SharpnessCalculation();

    sharpnessThread = new QThread();
    sharpnessWorker->moveToThread(sharpnessThread);
    connect(sharpnessThread, SIGNAL (finished()), sharpnessThread, SLOT (deleteLater()));
    sharpnessThread->start();


    QVBoxLayout* layout = new QVBoxLayout(this);

    toolBar = new QToolBar();
    toolBar->addAction("Fit", this, &SingleCameraSharpnessView::onFitClick);
    toolBar->addAction("100%", this, &SingleCameraSharpnessView::on100pClick);
    toolBar->addSeparator();
    toolBar->addAction("+Zoom", this, &SingleCameraSharpnessView::onZoomPlusClick);
    toolBar->addAction("-Zoom", this, &SingleCameraSharpnessView::onZoomMinusClick);
    toolBar->addSeparator();

    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    layout->addWidget(toolBar);

    videoView = new VideoView();
    layout->addWidget(videoView);

    QGroupBox *patternGroup = new QGroupBox("Calibration Pattern");
    QHBoxLayout *patternLayout = new QHBoxLayout();

    QFormLayout *sizeLayout = new QFormLayout();
    QLabel *verticalCornerLabel = new QLabel(tr("Rows  [<a href=\"http://mock.link\">?</a>]:"));
    connect(verticalCornerLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));

    cv::Size boardSize = cameraCalibration->getBoardSize();

    verticalCornerBox = new QSpinBox();
    verticalCornerBox->setMinimum(1);
    verticalCornerBox->setValue(boardSize.height);

    connect(verticalCornerBox, SIGNAL(valueChanged(int)), this, SLOT(onSizeBoxChange()));

    sizeLayout->addRow(verticalCornerLabel, verticalCornerBox);

    QLabel *horizontalCornerLabel = new QLabel(tr("Columns [<a href=\"http://mock.link\">?</a>]:"));
    connect(horizontalCornerLabel, SIGNAL(linkActivated(QString)), this, SLOT(onShowHelpDialog()));

    horizontalCornerBox = new QSpinBox();
    horizontalCornerBox->setMinimum(1);
    horizontalCornerBox->setValue(boardSize.width);

    sharpnessWorker->setBoardSize(9, 7);

    connect(horizontalCornerBox, SIGNAL(valueChanged(int)), this, SLOT(onSizeBoxChange()));

    sizeLayout->addRow(horizontalCornerLabel, horizontalCornerBox);
    patternLayout->addLayout(sizeLayout);
    patternLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    patternLayout->setContentsMargins(11,11,11,11);
    patternLayout->setSpacing(8);

    patternGroup->setLayout(patternLayout);
    layout->addWidget(patternGroup);

    statusBar = new QStatusBar();
    // statusbar widget
    QWidget *statusCameraFPSWidget = new QWidget();
    QHBoxLayout *statusBarLayout = new QHBoxLayout();
    statusBarLayout->setContentsMargins(8,0,8,0);
    QLabel *cameraFPSLabel = new QLabel("Camera FPS:");
    cameraFPSValue = new QLabel();

    statusBarLayout->addWidget(cameraFPSLabel);
    statusBarLayout->addWidget(cameraFPSValue);
    statusCameraFPSWidget->setLayout(statusBarLayout);
    statusBar->addPermanentWidget(statusCameraFPSWidget);

    layout->addWidget(statusBar);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

    connect(camera, SIGNAL(fps(double)), this, SLOT(updateCameraFPS(double)));

    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), sharpnessWorker, SLOT(onNewImage(CameraImage)));
    connect(sharpnessWorker, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
}

SingleCameraSharpnessView::~SingleCameraSharpnessView() {

}

void SingleCameraSharpnessView::onShowHelpDialog() {

    calibrationHelpDialog->show();
}

void SingleCameraSharpnessView::onSizeBoxChange() {

    cv::Size boardSize = cv::Size(horizontalCornerBox->value(), verticalCornerBox->value());

    sharpnessWorker->setBoardSize(boardSize);

    std::cout<<"boardSize: "<<boardSize<<std::endl;
}

void SingleCameraSharpnessView::updateView(CameraImage *cimg) {

    if(!cimg->img.empty()) {
        videoView->updateView(cimg->img);
        //delete &cimg;
    }
}

void SingleCameraSharpnessView::updateCameraFPS(double fps) {
    cameraFPSValue->setText(QString::number(fps));
}

void SingleCameraSharpnessView::onFitClick() {
    videoView->fitView();
}

void SingleCameraSharpnessView::on100pClick() {
    videoView->showFullView();
}

void SingleCameraSharpnessView::onZoomPlusClick() {
    videoView->zoomInView();
}

void SingleCameraSharpnessView::onZoomMinusClick() {
    videoView->zoomOutView();
}