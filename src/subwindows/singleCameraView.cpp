#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>

#include "singleCameraView.h"

// Create new single camera view given a single camera object and a pupil detection process
// The pupil detection is used to display the detected pupils and show detection information such as processing fps
SingleCameraView::SingleCameraView(Camera *camera, PupilDetection *pupilDetection, QWidget *parent) :
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


    setWindowTitle("Single camera view");


    QVBoxLayout* layout = new QVBoxLayout(this);

    toolBar = new QToolBar();
    toolBar->addAction("Fit", this, &SingleCameraView::onFitClick);
    toolBar->addAction("100%", this, &SingleCameraView::on100pClick);
    toolBar->addSeparator();
    toolBar->addAction("+Zoom", this, &SingleCameraView::onZoomPlusClick);
    toolBar->addAction("-Zoom", this, &SingleCameraView::onZoomMinusClick);
    toolBar->addSeparator();

    QMenu *plotMenu = new QMenu("Show");
    plotMenuAct = plotMenu->menuAction();
    connect(plotMenuAct, &QAction::triggered, this, &SingleCameraView::onPlotMenuClick);


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


    plotROIAct = plotMenu->addAction(tr("Visualize ROI Contour"));
    plotROIAct->setCheckable(true);
    plotROIAct->setChecked(plotROIContour);
    plotROIAct->setStatusTip(tr("Display the region of interest applied in detection in the camera image."));
    plotMenu->addAction(plotROIAct);
    connect(plotROIAct, SIGNAL(toggled(bool)), this, SLOT(onPlotROIClick(bool)));

    toolBar->addAction(plotMenuAct);
    toolBar->addSeparator();


    QMenu *roiMenu = new QMenu("ROI");
    roiMenuAct = roiMenu->menuAction();
    connect(roiMenuAct, &QAction::triggered, this, &SingleCameraView::onROIMenuClick);

    customROIAct = roiMenu->addAction(tr("Custom"), this, [this]()
    {
        onSetROIClick(-1.0);
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
    connect(saveROI, &QAction::triggered, this, &SingleCameraView::onSaveROIClick);

    const QIcon discardIcon = QIcon(":/icons/Breeze/actions/22/dialog-cancel.svg"); //QIcon::fromTheme("camera-video");
    discardROI = new QAction(discardIcon, tr("Reset ROI"), this);
    connect(discardROI, &QAction::triggered, this, &SingleCameraView::onDiscardROIClick);

    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    layout->addWidget(toolBar);

    videoView = new VideoView();
    layout->addWidget(videoView);

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

    processingAlgorithmLabel = new QLabel();
    processingConfigLabel = new QLabel();
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

    // Connect the pupil detection process to inform this widget of changes
    connect(pupilDetection, SIGNAL(processingStarted()), this, SLOT(onPupilDetectionStart()));
    connect(pupilDetection, SIGNAL(processingFinished()), this, SLOT(onPupilDetectionStop()));
    connect(pupilDetection, SIGNAL(fps(double)), this, SLOT(updateProcessingFPS(double)));

    pupilDetection->setUpdateFPS(1000/updateDelay);

    // In the normal state, images are shown from the camera directly, when pupil detection is activated,
    // this signal is stopped and images from the detection are displayed
    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(camera, SIGNAL(fps(double)), this, SLOT(updateCameraFPS(double)));

    connect(videoView, SIGNAL (onROISelection(QRectF)), pupilDetection, SLOT (setROI(QRectF)));
    connect(videoView, SIGNAL (onROISelection(QRectF)), this, SLOT (saveROISelection(QRectF)));

    connect(pupilDetection, SIGNAL (algorithmChanged()), this, SLOT (updateAlgorithmLabel()));
    connect(pupilDetection, SIGNAL (configChanged(QString)), this, SLOT (updateConfigLabel(QString)));

    connect(this, SIGNAL (onShowROI(bool)), pupilDetection, SLOT (onShowROI(bool)));
    connect(this, SIGNAL (onShowPupilCenter(bool)), pupilDetection, SLOT (onShowPupilCenter(bool)));

    timer.start();
    pupilViewTimer.start();

    loadSettings();
}

SingleCameraView::~SingleCameraView() {
}


void SingleCameraView::loadSettings() {

    displayPupilView = applicationSettings->value("SingleCameraView.displayPupilView", displayPupilView).toBool();
    onDisplayPupilViewClick(displayPupilView);
    displayDetailAct->setChecked(displayPupilView);

    plotPupilCenter = applicationSettings->value("SingleCameraView.plotPupilCenter", plotPupilCenter).toBool();
    onPlotPupilCenterClick(plotPupilCenter);
    plotCenterAct->setChecked(plotPupilCenter);

    plotROIContour = applicationSettings->value("SingleCameraView.plotROIContour", plotROIContour).toBool();
    plotROIAct->setChecked(plotROIContour);
    onPlotROIClick(plotROIContour);

    QRectF roi = applicationSettings->value("SingleCameraView.roiSelectionRect", QRectF()).toRectF();

    if(!roi.isEmpty()) {
        videoView->setROISelection(roi);
        videoView->saveROISelection();
    }
}

void SingleCameraView::onPlotMenuClick() {
    // Fix to open submenu in the camera menu
    plotMenuAct->menu()->exec(QCursor::pos());
}

void SingleCameraView::onROIMenuClick() {
    // Fix to open submenu in the camera menu
    roiMenuAct->menu()->exec(QCursor::pos());
}

// When pupil detection is started, signaled by the pupil detection process, the camera image signals are disconnected from this view
// and the processed pupil images from the pupil detection are shown (pupil plotted on top etc.)
void SingleCameraView::onPupilDetectionStart() {

    statusBar->insertPermanentWidget(1, statusProcessingFPSWidget);
    statusProcessingFPSWidget->show();
    processingConfigLabel->setText(pupilDetection->getCurrentConfigLabel());
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod()->title()));

    disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));
    
    connect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(pupilDetection, SIGNAL(processedPupilData(quint64, Pupil, QString)), this, SLOT(updatePupilView(quint64, Pupil, QString)));

}

// When pupil detection is stopped, signaled by the pupil detection process, the camera image signals are connected again to this view
// and the processed pupil images from the pupil detection are disconnected
void SingleCameraView::onPupilDetectionStop() {

    statusBar->removeWidget(statusProcessingFPSWidget);

    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    disconnect(pupilDetection, SIGNAL(processedPupilData(quint64, Pupil, QString)), this, SLOT(updatePupilView(quint64, Pupil, QString)));

    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));
}

void SingleCameraView::updateView(const CameraImage &cimg) {

    if(timer.elapsed() > updateDelay && !cimg.img.empty()) {
        timer.restart();

        // QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
        QDateTime date = QDateTime::fromMSecsSinceEpoch(cimg.timestamp);
        // Display the date/time in the system specific locale format
        statusBar->showMessage(QLocale::system().toString(date));
        videoView->updateView(cimg.img);
    }
}

void SingleCameraView::updateCameraFPS(double fps) {
    currentCameraFPS = fps;
    cameraFPSValue->setText(QString::number(fps));
}

// If the processing fps is sign. slower (10%) than the camera fps, color it red
void SingleCameraView::updateProcessingFPS(double fps) {
    if(fps < currentCameraFPS*0.9) {
        processingFPSValue->setStyleSheet("color: red;");
    } else {
        processingFPSValue->setStyleSheet("color: black;");
    }
    processingFPSValue->setText(QString::number(fps));
}

void SingleCameraView::updateAlgorithmLabel() {
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod()->title()));
}

void SingleCameraView::updateConfigLabel(QString config) {
    processingConfigLabel->setText(config);
}

// Updates the position and size of the small pupil view based on the latest pupil detection
void SingleCameraView::updatePupilView(quint64 timestamp, const Pupil &pupil, const QString &filename) {

    // If the view is not yet initialized, set a fixed size for it
    // This is done only once after activation to not switch sizes at each pupil update which makes the pupil view to jitterish
    if(displayPupilView && !initPupilViewSize && pupil.valid(-2)) {
        initPupilViewSize = true;

        pupilViewSize = QSize(static_cast<int>(pupil.size.width * 1.6), static_cast<int>(pupil.size.height * 1.6));
    }

    // Update the view not too often, ~30fps
    if(pupilViewTimer.elapsed() > updateDelay && pupil.valid(-2)) {
        pupilViewTimer.restart();

        //std::cout<<pupil.center.x<<", "<<pupil.center.y<<"; "<<pupil.size.width<<", "<<pupil.size.height<<std::endl;
        // Create a ROI around the pupil big enough to make changes visible
        QPoint tl = QPoint(static_cast<int>(pupil.center.x - (0.5 * pupilViewSize.width())),
                           static_cast<int>(pupil.center.y - (0.5 * pupilViewSize.width())));
        videoView->updatePupilView(QRect(tl, pupilViewSize));
    }

}

// Click event handler
// Fits the camera view to the size of the window
void SingleCameraView::onFitClick() {
    videoView->fitView();
}

// Shows the camera view in the original resolution, no scaling
void SingleCameraView::on100pClick() {
    videoView->showFullView();
}

// Zooms in
void SingleCameraView::onZoomPlusClick() {
    videoView->zoomInView();
}

// Zooms out
void SingleCameraView::onZoomMinusClick() {
    videoView->zoomOutView();
}

// Displays a small pupil lens view on top of the camera view
void SingleCameraView::onDisplayPupilViewClick(bool value) {
    displayPupilView = value;
    applicationSettings->setValue("SingleCameraView.displayPupilView", displayPupilView);

    if(displayPupilView)
        initPupilViewSize = false;
    videoView->enablePupilView(displayPupilView);
}

void SingleCameraView::onSetROIClick(float roiSize) {

    toolBar->addAction(discardROI);
    toolBar->addAction(saveROI);
    videoView->setROISelection(roiSize);
    videoView->showROISelection(true);
}

void SingleCameraView::onSaveROIClick() {

    if(videoView->saveROISelection()) {
        videoView->showROISelection(false);
        toolBar->removeAction(discardROI);
        toolBar->removeAction(saveROI);
    }
}

void SingleCameraView::onDiscardROIClick() {
    videoView->discardROISelection();
}

// Plots the pupil center point, plotting is done in the pupil detection thread thus a signal communicates this change
void SingleCameraView::onPlotPupilCenterClick(bool value) {
    plotPupilCenter = value;
    applicationSettings->setValue("SingleCameraView.plotPupilCenter", plotPupilCenter);

    emit onShowPupilCenter(plotPupilCenter);
}

// Plots the region of interest, plotting is done in the pupil detection thread thus a signal communicates this change
void SingleCameraView::onPlotROIClick(bool value) {
    plotROIContour = value;
    applicationSettings->setValue("SingleCameraView.plotROIContour", plotROIContour);

    emit onShowROI(plotROIContour);
}

void SingleCameraView::saveROISelection(QRectF roi) {

    std::cout<<"saveing ROI selection"<<std::endl;

    applicationSettings->setValue("SingleCameraView.roiSelectionRect", roi);
}

void SingleCameraView::onSettingsChange() {
    loadSettings();
}
