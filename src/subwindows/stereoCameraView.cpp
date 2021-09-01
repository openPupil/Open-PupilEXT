
#include "stereoCameraView.h"

#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>

// Creates a new stereo camera view widget, taking a stereo based camera (StereoCamera or FileCamera with stero mode)
// Pupil detection process is used to get detection results and display them, communicate ROI selection
StereoCameraView::StereoCameraView(Camera *camera, PupilDetection *pupilDetection, QWidget *parent) :
        QWidget(parent),
        camera(camera),
        pupilDetection(pupilDetection),
        displayPupilView(false),
        plotPupilCenter(false),
        plotROIContour(true),
        initPupilViewSize(false),
        pupilViewSize(0, 0),
        currentCameraFPS(0.0),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {


    setWindowTitle("Stereo camera view");


    QVBoxLayout* layout = new QVBoxLayout(this);

    toolBar = new QToolBar();
    toolBar->addAction("Fit", this, &StereoCameraView::onFitClick);
    toolBar->addAction("100%", this, &StereoCameraView::on100pClick);
    toolBar->addSeparator();
    toolBar->addAction("+Zoom", this, &StereoCameraView::onZoomPlusClick);
    toolBar->addAction("-Zoom", this, &StereoCameraView::onZoomMinusClick);
    toolBar->addSeparator();

    QMenu *plotMenu = new QMenu("Show");
    plotMenuAct = plotMenu->menuAction();
    connect(plotMenuAct, &QAction::triggered, this, &StereoCameraView::onPlotMenuClick);

    displayDetailAct = plotMenu->addAction(tr("Show Pupil Detail View"));
    displayDetailAct->setCheckable(true);
    displayDetailAct->setChecked(displayPupilView);
    displayDetailAct->setStatusTip(tr("Display the detected pupil center in the camera image."));
    plotMenu->addAction(displayDetailAct);
    connect(displayDetailAct, SIGNAL(toggled(bool)), this, SLOT(onDisplayPupilViewClick(bool)));

    plotCenterAct = plotMenu->addAction(tr("Visualize Pupil Center"));
    plotCenterAct->setCheckable(true);
    plotCenterAct->setChecked(plotPupilCenter);
    plotCenterAct->setStatusTip(tr("Display the detected pupil center in the camera image."));
    plotMenu->addAction(plotCenterAct);
    connect(plotCenterAct, SIGNAL(toggled(bool)), this, SLOT(onPlotPupilCenterClick(bool)));

    plotROIAct = plotMenu->addAction(tr("Visualize ROI Contour"), this, &StereoCameraView::onPlotROIClick);
    plotROIAct->setCheckable(true);
    plotROIAct->setChecked(plotROIContour);
    plotROIAct->setStatusTip(tr("Display the region of interest applied in detection in the camera image."));
    plotMenu->addAction(plotROIAct);
    connect(plotROIAct, SIGNAL(toggled(bool)), this, SLOT(onPlotROIClick(bool)));

    toolBar->addAction(plotMenuAct);
    toolBar->addSeparator();


    QMenu *roiMenu = new QMenu("ROI");
    roiMenuAct = roiMenu->menuAction();
    connect(roiMenuAct, &QAction::triggered, this, &StereoCameraView::onROIMenuClick);

    customROIAct = roiMenu->addAction(tr("Custom"), this, [this]()
    {
        onSetROIClick(static_cast<float>(-1.0));
    });

    customROIAct->setStatusTip(tr("Select custom ROI area in the image."));
    roiMenu->addAction(customROIAct);

    smallROIAct = roiMenu->addAction(tr("30% Image Width"), this, [this]()
    {
        onSetROIClick(0.3);
    });
    smallROIAct->setStatusTip(tr("ROI area with a size of 30% of the image width, preserving image aspect ratio."));
    roiMenu->addAction(smallROIAct);

    middleROIAct = roiMenu->addAction(tr("60% Image Width"), this, [this]()
    {
        onSetROIClick(0.6);
    });
    middleROIAct->setStatusTip(tr("ROI area with a size of 60% of the image width, preserving image aspect ratio."));
    roiMenu->addAction(middleROIAct);

    toolBar->addAction(roiMenuAct);
    toolBar->addSeparator();

    const QIcon okIcon = QIcon(":/icons/Breeze/actions/22/dialog-ok-apply.svg"); //QIcon::fromTheme("camera-video");
    saveROI = new QAction(okIcon, tr("Set ROI"), this);
    connect(saveROI, &QAction::triggered, this, &StereoCameraView::onSaveROIClick);

    const QIcon discardIcon = QIcon(":/icons/Breeze/actions/22/dialog-cancel.svg"); //QIcon::fromTheme("camera-video");
    discardROI = new QAction(discardIcon, tr("Reset ROI"), this);
    connect(discardROI, &QAction::triggered, this, &StereoCameraView::onDiscardROIClick);

    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    layout->addWidget(toolBar);

    QHBoxLayout *videoViewLayout = new QHBoxLayout();

    mainVideoView = new VideoView();
    videoViewLayout->addWidget(mainVideoView);
    secondaryVideoView = new VideoView();
    videoViewLayout->addWidget(secondaryVideoView);
    layout->addLayout(videoViewLayout);

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

    statusProcessingFPSWidget = new QWidget();
    QHBoxLayout *statusBarProcessingLayout = new QHBoxLayout();
    statusBarProcessingLayout->setContentsMargins(8,0,8,0);

    processingConfigLabel = new QLabel();
    processingAlgorithmLabel = new QLabel();

    QLabel *processingFPSLabel = new QLabel("Processing FPS:");
    processingFPSValue = new QLabel();

    statusBarProcessingLayout->addWidget(processingConfigLabel);
    statusBarProcessingLayout->addWidget(processingAlgorithmLabel);
    statusBarProcessingLayout->addWidget(processingFPSLabel);
    statusBarProcessingLayout->addWidget(processingFPSValue);
    statusProcessingFPSWidget->setLayout(statusBarProcessingLayout);

    layout->addWidget(statusBar);


    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

    updateDelay = 33; // ~30fps

    connect(pupilDetection, SIGNAL(processingStarted()), this, SLOT(onPupilDetectionStart()));
    connect(pupilDetection, SIGNAL(processingFinished()), this, SLOT(onPupilDetectionStop()));
    connect(pupilDetection, SIGNAL(fps(double)), this, SLOT(updateProcessingFPS(double)));
    connect(pupilDetection, SIGNAL (algorithmChanged()), this, SLOT (updateAlgorithmLabel()));
    connect(pupilDetection, SIGNAL (configChanged(QString)), this, SLOT (updateConfigLabel(QString)));

    pupilDetection->setUpdateFPS(1000/updateDelay);

    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(camera, SIGNAL(fps(double)), this, SLOT(updateCameraFPS(double)));

    connect(mainVideoView, SIGNAL (onROISelection(QRectF)), pupilDetection, SLOT (setROI(QRectF)));
    connect(mainVideoView, SIGNAL (onROISelection(QRectF)), this, SLOT (saveROISelection(QRectF)));

    connect(secondaryVideoView, SIGNAL (onROISelection(QRectF)), pupilDetection, SLOT (setSecondaryROI(QRectF)));
    connect(secondaryVideoView, SIGNAL (onROISelection(QRectF)), this, SLOT (saveSecondaryROISelection(QRectF)));

    connect(this, SIGNAL (onROISelection(QRectF)), pupilDetection, SLOT (setROI(QRectF)));
    connect(this, SIGNAL (onShowROI(bool)), pupilDetection, SLOT (onShowROI(bool)));
    connect(this, SIGNAL (onShowPupilCenter(bool)), pupilDetection, SLOT (onShowPupilCenter(bool)));

    timer.start();
    pupilViewTimer.start();
    loadSettings();
}

StereoCameraView::~StereoCameraView() {

}

// Loads the view settings from the application wide settings
// Application settings contain both ROI selection if available
void StereoCameraView::loadSettings() {

    displayPupilView = applicationSettings->value("StereoCameraView.displayPupilView", displayPupilView).toBool();
    onDisplayPupilViewClick(displayPupilView);
    displayDetailAct->setChecked(displayPupilView);

    plotPupilCenter = applicationSettings->value("StereoCameraView.plotPupilCenter", plotPupilCenter).toBool();
    onPlotPupilCenterClick(plotPupilCenter);
    plotCenterAct->setChecked(plotPupilCenter);

    plotROIContour = applicationSettings->value("StereoCameraView.plotROIContour", plotROIContour).toBool();
    plotROIAct->setChecked(plotROIContour);
    onPlotROIClick(plotROIContour);

    QRectF roi = applicationSettings->value("StereoCameraView.roiSelectionRect", QRectF()).toRectF();
    if(!roi.isNull()) {
        mainVideoView->setROISelection(roi);
        mainVideoView->saveROISelection();
    }

    QRectF secRoi = applicationSettings->value("StereoCameraView.secondaryRoiSelectionRect", QRectF()).toRectF();
    if(!secRoi.isNull()) {
        secondaryVideoView->setROISelection(secRoi);
        secondaryVideoView->saveROISelection();
    }
}

// Opens a contextmenu on the toolbar for settings the plottable options
void StereoCameraView::onPlotMenuClick() {
    // fix to open submenu in the camera menu
    plotMenuAct->menu()->exec(QCursor::pos());
}

// Opens a contextmenu on the toolbar for settings the ROI options
void StereoCameraView::onROIMenuClick() {
    // fix to open submenu in the camera menu
    roiMenuAct->menu()->exec(QCursor::pos());
}

// When pupil detection is started, labels are displayed and the camera live-stream is switched to the processed-image live-stream from the pupil detection
void StereoCameraView::onPupilDetectionStart() {

    statusBar->insertPermanentWidget(1, statusProcessingFPSWidget);
    statusProcessingFPSWidget->show();
    processingConfigLabel->setText(pupilDetection->getCurrentConfigLabel());
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod()->title()));

    disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));

    connect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(pupilDetection, SIGNAL(processedStereoPupilData(quint64, Pupil, Pupil, QString)), this, SLOT(updatePupilView(quint64, Pupil, Pupil, QString)));
}

// On stop of the pupil detection, disconnect the processed-image live-stream and show the camera image again
void StereoCameraView::onPupilDetectionStop() {

    statusBar->removeWidget(statusProcessingFPSWidget);

    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    disconnect(pupilDetection, SIGNAL(processedStereoPupilData(quint64, Pupil, Pupil, QString)), this, SLOT(updatePupilView(quint64, Pupil, Pupil, QString)));

    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));
}

// Update the live-view with images from the current update stream (either processed images or camera images)
void StereoCameraView::updateView(const CameraImage &cimg) {

    if(timer.elapsed() > updateDelay && !cimg.img.empty()) {
        timer.restart();

        // QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
        QDateTime date = QDateTime::fromMSecsSinceEpoch(cimg.timestamp);
        // Display the date/time in the system specific locale format
        statusBar->showMessage(QLocale::system().toString(date));
        mainVideoView->updateView(cimg.img);
        secondaryVideoView->updateView(cimg.imgSecondary);
    }
}

void StereoCameraView::updateCameraFPS(double fps) {
    currentCameraFPS = fps;
    cameraFPSValue->setText(QString::number(fps));
}

// Updates the label showing the current pupil detection processing fps
// To signal slow processing, the label is colored red when a certain threshold is broke
void StereoCameraView::updateProcessingFPS(double fps) {

    if(fps < currentCameraFPS*0.9) {
        processingFPSValue->setStyleSheet("color: red;");
    } else {
        processingFPSValue->setStyleSheet("color: black;");
    }
    processingFPSValue->setText(QString::number(fps));
}

// Updates the label displaying the current pupil detection algorithm used
void StereoCameraView::updateAlgorithmLabel() {
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod()->title()));
}

// Updates position and size of the small lens view of the pupil on receiving of a new pupil detection result
void StereoCameraView::updatePupilView(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename) {

    // If the view is not yet initialized, set a fixed size for it
    // This is done only once after activation to not switch sizes at each pupil update which makes the pupil view to jitterish
    if(displayPupilView && !initPupilViewSize && pupil.valid(-2) && pupilSec.valid(-2)) {
        initPupilViewSize = true;
        pupilViewSize = QSize(static_cast<int>(pupil.size.width * 1.6), static_cast<int>(pupil.size.height * 1.6));
        pupilViewSizeSec = QSize(static_cast<int>(pupilSec.size.width * 1.6),
                                 static_cast<int>(pupilSec.size.height * 1.6));
    }

    if(pupilViewTimer.elapsed() > updateDelay && pupil.valid(-2)) {
        pupilViewTimer.restart();

        // create a ROI around the pupil big enough to make changes visible
        QPoint tl = QPoint(static_cast<int>(pupil.center.x - (0.5 * pupilViewSize.width())),
                           static_cast<int>(pupil.center.y - (0.5 * pupilViewSize.width())));
        QPoint tlSec = QPoint(static_cast<int>(pupilSec.center.x - (0.5 * pupilViewSizeSec.width())),
                              static_cast<int>(pupilSec.center.y - (0.5 * pupilViewSizeSec.width())));

        mainVideoView->updatePupilView(QRect(tl, pupilViewSize));
        secondaryVideoView->updatePupilView(QRect(tlSec, pupilViewSizeSec));
    }
}

// Fit button click, adjusts the camera-view to the current window size
void StereoCameraView::onFitClick() {

    mainVideoView->fitView();
    secondaryVideoView->fitView();
}

// Show the camera-view with its real resolution without scaling to the window size
void StereoCameraView::on100pClick() {

    mainVideoView->showFullView();
    secondaryVideoView->showFullView();
}

// Zoom in
void StereoCameraView::onZoomPlusClick() {
    mainVideoView->zoomInView();
    secondaryVideoView->zoomInView();
}

// Zoom out
void StereoCameraView::onZoomMinusClick() {
    mainVideoView->zoomOutView();
    secondaryVideoView->zoomOutView();
}

// Activates the pupil lens view
void StereoCameraView::onDisplayPupilViewClick(bool value) {
    displayPupilView = value;
    if(displayPupilView)
        initPupilViewSize = false;
    mainVideoView->enablePupilView(displayPupilView);
    secondaryVideoView->enablePupilView(displayPupilView);
}

// Opens the ROI selection
void StereoCameraView::onSetROIClick(float roiSize) {

    toolBar->addAction(discardROI);
    toolBar->addAction(saveROI);

    mainVideoView->setROISelection(roiSize);
    secondaryVideoView->setROISelection(roiSize);
    mainVideoView->showROISelection(true);
    secondaryVideoView->showROISelection(true);
}

// Saves the current ROI selection
// This is propagated to the pupil detection dialog which then uses the new ROI selection
void StereoCameraView::onSaveROIClick() {

    if(mainVideoView->saveROISelection() && secondaryVideoView->saveROISelection()) {
        mainVideoView->showROISelection(false);
        secondaryVideoView->showROISelection(false);
        toolBar->removeAction(discardROI);
        toolBar->removeAction(saveROI);
    }
}

// Reset/Discard the current ROI selection dialog
void StereoCameraView::onDiscardROIClick() {
    mainVideoView->discardROISelection();
    secondaryVideoView->discardROISelection();
}

// Plots the pupil center on the pupil image
// Propagated to the pupil detection process which does the plotting
void StereoCameraView::onPlotPupilCenterClick(bool value) {
    plotPupilCenter = value;
    applicationSettings->setValue("StereoCameraView.plotPupilCenter", plotPupilCenter);

    emit onShowPupilCenter(plotPupilCenter);
}

// Plots the ROI on the pupil image
// Propagated to the pupil detection process which does the plotting
void StereoCameraView::onPlotROIClick(bool value) {
    plotROIContour = value;
    applicationSettings->setValue("StereoCameraView.plotROIContour", plotROIContour);

    emit onShowROI(plotROIContour);
}

// Saves the ROI selection to the application wide settings (which are persisted to file)
void StereoCameraView::saveROISelection(QRectF roi) {

    applicationSettings->setValue("StereoCameraView.roiSelectionRect", roi);
}

// Saves the ROI selection to the application wide settings (which are persisted to file)
void StereoCameraView::saveSecondaryROISelection(QRectF roi) {

    applicationSettings->setValue("StereoCameraView.secondaryRoiSelectionRect", roi);
}

void StereoCameraView::onSettingsChange() {
    loadSettings();
}

// Updates a label which shows the current config
// A config a i.e. the optimized detection parameter for a specific ROI size
void StereoCameraView::updateConfigLabel(QString config) {
    processingConfigLabel->setText(config);
}


