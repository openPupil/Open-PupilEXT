
#include "stereoCameraView.h"

#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>
#include "../supportFunctions.h"

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
        //pupilViewSize(0, 0),
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

    // GB added/modified begin
    showAutoParamAct = plotMenu->addAction(tr("Show Automatic Parametrization Overlay"));
    showAutoParamAct->setCheckable(true);
    showAutoParamAct->setChecked(showAutoParamOverlay);
    showAutoParamAct->setStatusTip(tr("Display expected pupil size maximum and minimum values as currently set for Automatic Parametrization."));
    plotMenu->addAction(showAutoParamAct);
    connect(showAutoParamAct, SIGNAL(toggled(bool)), this, SLOT(onShowAutoParamOverlay(bool)));
    
    plotMenu->addSeparator();

    QLabel *pupilColorLabel = new QLabel("Show pupil detection confidence:");
    pupilColorLabel->setContentsMargins(8,0,8,0);
    QWidgetAction *act0 = new QWidgetAction(plotMenu);
    act0->setCheckable(false);
    act0->setDefaultWidget(pupilColorLabel);
    plotMenu->addAction(act0);


    QWidget *pupilColorFillWidget = new QWidget();
    QHBoxLayout *pupilColorFillLayout = new QHBoxLayout();
    pupilColorFillLayout->setContentsMargins(8,0,8,0); 

    QLabel *pupilColorFillLabel = new QLabel("Coloring:");
    pupilColorFillLabel->setFixedWidth(80);

    QComboBox *pupilColorFillBox = new QComboBox();
    pupilColorFillBox->addItem(QString("No fill"), QString("NO_FILL"));
    pupilColorFillBox->addItem(QString("Confidence"), QString("CONFIDENCE"));
    pupilColorFillBox->addItem(QString("Outline confidence"), QString("OUTLINE_CONFIDENCE"));
    pupilColorFillBox->setCurrentText("NO_FILL");

    pupilColorFillLayout->addWidget(pupilColorFillLabel);
    pupilColorFillLayout->addWidget(pupilColorFillBox);
    pupilColorFillWidget->setLayout(pupilColorFillLayout);

    QWidgetAction *act1 = new QWidgetAction(plotMenu);
    act1->setCheckable(false);
    act1->setDefaultWidget(pupilColorFillWidget);
    plotMenu->addAction(act1);

    
    QWidget *pupilColorFillThresholdWidget = new QWidget();
    QHBoxLayout *pupilColorFillThresholdLayout = new QHBoxLayout();
    pupilColorFillThresholdLayout->setContentsMargins(8,0,8,0); 

    QLabel *pupilColorFillThresholdLabel = new QLabel("Low threshold:");
    pupilColorFillThresholdLabel->setFixedWidth(80);

    QDoubleSpinBox *pupilColorFillThresholdBox = new QDoubleSpinBox();
    pupilColorFillThresholdBox->setMinimum(0.1);
    pupilColorFillThresholdBox->setMaximum(0.9);
    pupilColorFillThresholdBox->setSingleStep(0.1);

    pupilColorFillThresholdLayout->addWidget(pupilColorFillThresholdLabel);
    pupilColorFillThresholdLayout->addWidget(pupilColorFillThresholdBox);
    pupilColorFillThresholdWidget->setLayout(pupilColorFillThresholdLayout);

    QWidgetAction *act2 = new QWidgetAction(plotMenu);
    act2->setCheckable(false);
    act2->setDefaultWidget(pupilColorFillThresholdWidget);
    plotMenu->addAction(act2);

    toolBar->addAction(plotMenuAct);
    toolBar->addSeparator();


    QMenu *pupilDetectionMenu = new QMenu("Pupil Detection");
    pupilDetectionMenuAct = pupilDetectionMenu->menuAction();
    connect(pupilDetectionMenuAct, &QAction::triggered, this, &StereoCameraView::onPupilDetectionMenuClick);

    // GB: renamed to be pore descriptive (not to be confused with camera image acquisition ROI), also modified tooltips
    QMenu *roiMenu = pupilDetectionMenu->addMenu(tr("&Pupil Detection ROI"));

    customROIAct = roiMenu->addAction(tr("Custom"), this, [this]()
    {
        onSetROIClick(static_cast<float>(-1.0));
    });

    customROIAct->setStatusTip(tr("Select custom Pupil Detection ROI area in the image.")); 
    roiMenu->addAction(customROIAct);

    smallROIAct = roiMenu->addAction(tr("30% Image Width"), this, [this]()
    {
        onSetROIClick(0.3F);
    });
    smallROIAct->setStatusTip(tr("Pupil Detection ROI area with a size of 30% of the image width, preserving image aspect ratio.")); 
    roiMenu->addAction(smallROIAct);

    middleROIAct = roiMenu->addAction(tr("60% Image Width"), this, [this]()
    {
        onSetROIClick(0.6F);
    });
    middleROIAct->setStatusTip(tr("Pupil Detection ROI area with a size of 60% of the image width, preserving image aspect ratio."));
    roiMenu->addAction(middleROIAct);


    QMenu *autoParamMenu = pupilDetectionMenu->addMenu(tr("&Automatic Parametrization"));
    
    QWidget *autoParamPupSizeWidget = new QWidget();
    QHBoxLayout *autoParamPupSizeLayout = new QHBoxLayout();
    autoParamPupSizeLayout->setContentsMargins(8,0,8,0); 

    QLabel *autoParamPupSizeLabel = new QLabel("Expected max. pupil size [%]:");
    autoParamPupSizeLabel->setFixedWidth(150);

    autoParamPupSizeBox = new QSpinBox();
    autoParamPupSizeBox->setMinimum(20);
    autoParamPupSizeBox->setMaximum(100);
    autoParamPupSizeBox->setSingleStep(5);

    autoParamPupSizeLayout->addWidget(autoParamPupSizeLabel);
    autoParamPupSizeLayout->addWidget(autoParamPupSizeBox);
    autoParamPupSizeWidget->setLayout(autoParamPupSizeLayout);

    QWidgetAction *act3 = new QWidgetAction(autoParamMenu);
    act3->setCheckable(false);
    act3->setDefaultWidget(autoParamPupSizeWidget);
    autoParamMenu->addAction(act3);
    
    QWidget *autoParamSliderWidget = new QWidget();
    QHBoxLayout *autoParamSliderLayout = new QHBoxLayout();
    autoParamSliderLayout->setContentsMargins(8,0,8,0); 

    autoParamSlider = new QSlider();
    autoParamSlider->setOrientation(Qt::Horizontal);
    autoParamSlider->setMinimum(20);
    autoParamSlider->setMaximum(100);
    //autoParamSlider->setSingleStep(10);
    autoParamSlider->setFocusPolicy(Qt::StrongFocus);
    autoParamSlider->setTickPosition(QSlider::TicksBelow);
    autoParamSlider->setTickInterval(5);
    autoParamSlider->setSingleStep(1);

    autoParamSliderLayout->addWidget(autoParamSlider);
    autoParamSliderWidget->setLayout(autoParamSliderLayout);

    QWidgetAction *act4 = new QWidgetAction(autoParamMenu);
    act4->setCheckable(false);
    act4->setDefaultWidget(autoParamSliderWidget);
    autoParamMenu->addAction(act4);


    //autoParamPupSizeBox->setValue(50);
    //autoParamSlider->setValue(50);
    toolBar->addAction(pupilDetectionMenuAct);
    toolBar->addSeparator();

    // NOTE: added this to prevent the toolbar from popping bigger/smaller everytime saveROI and resetROI actions are revealed/hidden
    toolBar->setFixedHeight(36);

    // GB added/modified end



    const QIcon okIcon = QIcon(":/icons/Breeze/actions/22/dialog-ok-apply.svg"); //QIcon::fromTheme("camera-video");
    saveROI = new QAction(okIcon, tr("Set ROI"), this);
    connect(saveROI, &QAction::triggered, this, &StereoCameraView::onSaveROIClick);

    const QIcon discardIcon = QIcon(":/icons/Breeze/actions/22/dialog-cancel.svg"); //QIcon::fromTheme("camera-video");
    discardROISelection = new QAction(discardIcon, tr("Cancel ROI Selection"), this);
    connect(discardROISelection, &QAction::triggered, this, &StereoCameraView::onDiscardROISelectionClick);

    const QIcon resetIcon = QIcon(":/icons/Breeze/actions/22/gtk-convert.svg"); //QIcon::fromTheme("camera-video");
    resetROI = new QAction(resetIcon, tr("Reset ROI"), this);
    connect(resetROI, &QAction::triggered, this, &StereoCameraView::onResetROIClick);


    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    layout->addWidget(toolBar);

    QHBoxLayout *videoViewLayout = new QHBoxLayout();

    // GB NOTE: first just create the videoView instances like for single ROIs, and then we can change
    mainVideoView = new VideoView();
    videoViewLayout->addWidget(mainVideoView);
    secondaryVideoView = new VideoView();
    videoViewLayout->addWidget(secondaryVideoView);
    // GB added begin
    mainVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);
    secondaryVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);
    // GB added end
    layout->addLayout(videoViewLayout);

    statusBar = new QStatusBar();
    // statusbar widget
    QWidget *statusCameraFPSWidget = new QWidget();
    QHBoxLayout *statusBarLayout = new QHBoxLayout();
    statusBarLayout->setContentsMargins(8,0,8,0);
    // GB modified begin
    // GB NOTE: made it only appear if camera is not fileCamera. Also added separator
    QLabel *cameraFPSLabel = new QLabel();
    if(camera->getType() != SINGLE_IMAGE_FILE)
        cameraFPSLabel->setText("Camera FPS:");
    else
        cameraFPSLabel->setText("Image read FPS:");
    cameraFPSValue = new QLabel();
    
    QFrame* cameraFPSSep = new QFrame();
    cameraFPSSep->setFrameShape(QFrame::HLine);
    cameraFPSSep->setFrameShadow(QFrame::Sunken);
    statusBarLayout->addWidget(cameraFPSSep);

    statusBarLayout->addWidget(cameraFPSLabel);
    statusBarLayout->addWidget(cameraFPSValue);
    statusCameraFPSWidget->setLayout(statusBarLayout);
    statusBar->addPermanentWidget(statusCameraFPSWidget);
    // GB modified end

    statusProcessingFPSWidget = new QWidget();
    QHBoxLayout *statusBarProcessingLayout = new QHBoxLayout();
    statusBarProcessingLayout->setContentsMargins(8,0,8,0);

    processingConfigLabel = new QLabel();
    processingAlgorithmLabel = new QLabel();

    QLabel *processingFPSLabel = new QLabel("Processing FPS:");
    processingFPSValue = new QLabel();

    // GB modified/added begin
    // added separator vertical lines that belong to labels
    processingModeLabel = new QLabel();

    QFrame* processingAlgorithmSep = new QFrame();
    processingAlgorithmSep->setFrameShape(QFrame::VLine);
    processingAlgorithmSep->setFrameShadow(QFrame::Sunken);
    QFrame* processingConfigSep = new QFrame();
    processingConfigSep->setFrameShape(QFrame::VLine);
    processingConfigSep->setFrameShadow(QFrame::Sunken);
    QFrame* processingModeSep = new QFrame();
    processingModeSep->setFrameShape(QFrame::VLine);
    processingModeSep->setFrameShadow(QFrame::Sunken);

    //processingModeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    //processingModeLabel->setStyleSheet("border: 1px solid gray");
    statusBarProcessingLayout->addWidget(processingModeLabel);
    statusBarProcessingLayout->addWidget(processingModeSep);

    statusBarProcessingLayout->addWidget(processingConfigLabel);
    statusBarProcessingLayout->addWidget(processingConfigSep);
    statusBarProcessingLayout->addWidget(processingAlgorithmLabel);
    statusBarProcessingLayout->addWidget(processingAlgorithmSep);
    statusBarProcessingLayout->addWidget(processingFPSLabel);
    statusBarProcessingLayout->addWidget(processingFPSValue);
    // GB modified/added end

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


    // GB modified/added begin
    // GB: onShowROI() and onShowPupilCenter() not handled in pupilDetection anymore. Taken care of in camera view and videoView
    connect(this, SIGNAL (onShowROI(bool)), mainVideoView, SLOT (onShowROI(bool)));
    connect(this, SIGNAL (onShowROI(bool)), secondaryVideoView, SLOT (onShowROI(bool)));
    connect(this, SIGNAL (onShowPupilCenter(bool)), mainVideoView, SLOT (onShowPupilCenter(bool)));
    connect(this, SIGNAL (onShowPupilCenter(bool)), secondaryVideoView, SLOT (onShowPupilCenter(bool)));
    connect(this, SIGNAL (onChangePupilColorFill(int)), mainVideoView, SLOT (onChangePupilColorFill(int)));
    connect(this, SIGNAL (onChangePupilColorFill(int)), secondaryVideoView, SLOT (onChangePupilColorFill(int)));
    connect(this, SIGNAL (onChangePupilColorFillThreshold(float)), mainVideoView, SLOT (onChangePupilColorFillThreshold(float)));
    connect(this, SIGNAL (onChangePupilColorFillThreshold(float)), secondaryVideoView, SLOT (onChangePupilColorFillThreshold(float)));
    connect(this, SIGNAL (onChangeShowAutoParamOverlay(bool)), mainVideoView, SLOT (onChangeShowAutoParamOverlay(bool)));
    connect(this, SIGNAL (onChangeShowAutoParamOverlay(bool)), secondaryVideoView, SLOT (onChangeShowAutoParamOverlay(bool)));
    connect(pupilDetection, SIGNAL (onROIPreprocessingChanged(bool)), mainVideoView, SLOT (onChangePupilDetectionUsingROI(bool)));
    connect(pupilDetection, SIGNAL (onROIPreprocessingChanged(bool)), secondaryVideoView, SLOT (onChangePupilDetectionUsingROI(bool)));

    connect(pupilColorFillBox, SIGNAL (currentIndexChanged(int)), this, SLOT (onPupilColorFillChanged(int)));
    connect(pupilColorFillThresholdBox, SIGNAL (valueChanged(double)), this, SLOT (onPupilColorFillThresholdChanged(double)));

    connect(autoParamPupSizeBox, SIGNAL(valueChanged(int)), autoParamSlider, SLOT(setValue(int)));
    connect(autoParamSlider, SIGNAL(valueChanged(int)), autoParamPupSizeBox, SLOT(setValue(int)));
    connect(autoParamPupSizeBox, SIGNAL(valueChanged(int)), this, SLOT(onAutoParamPupSize(int)));

    timer.start();
    pupilViewTimer.start();

    // GB NOTE: currently it loads the settings (for loading ROI settings), so the loadSettings call at the end is not necessary
    updateForPupilDetectionProcMode();
    //loadSettings();
    
    // GB modified/added end
}

StereoCameraView::~StereoCameraView() {

}

// Loads the view settings from the application wide settings
// Application settings contain both ROI selection if available
void StereoCameraView::loadSettings() {
    // GB TODO: surely loads the bools always? Seems to be ok, but I remember to have seen cases in other code when the read value was "false" which was not parsed as 0

    displayPupilView = applicationSettings->value("StereoCameraView.displayPupilView", displayPupilView).toBool();
    onDisplayPupilViewClick(displayPupilView);
    displayDetailAct->setChecked(displayPupilView);

    plotPupilCenter = applicationSettings->value("StereoCameraView.plotPupilCenter", plotPupilCenter).toBool();
    onPlotPupilCenterClick(plotPupilCenter);
    plotCenterAct->setChecked(plotPupilCenter);

    plotROIContour = applicationSettings->value("StereoCameraView.plotROIContour", plotROIContour).toBool();
    plotROIAct->setChecked(plotROIContour);
    onPlotROIClick(plotROIContour);

    // GB added/modified begin
    showAutoParamOverlay = applicationSettings->value("StereoCameraView.showAutoParamOverlay", showAutoParamOverlay).toBool();
    showAutoParamAct->setChecked(showAutoParamOverlay);
    onShowAutoParamOverlay(showAutoParamOverlay);

    int autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", 50).toInt();
//    // GB: workaround to set values for auto param pup. size box and slider, without causing a cascade of events due to value change
//    autoParamPupSizeBox->blockSignals(true);
//    autoParamSlider->blockSignals(true);
    autoParamPupSizeBox->setValue(autoParamPupSizePercent);
    autoParamSlider->setValue(autoParamPupSizePercent);
//    autoParamPupSizeBox->blockSignals(false);
//    autoParamSlider->blockSignals(false);

    pupilColorFill = (ColorFill)applicationSettings->value("StereoCameraView.pupilColorFill", pupilColorFill).toInt();
    pupilColorFillThreshold = applicationSettings->value("StereoCameraView.pupilColorFillThreshold", pupilColorFillThreshold).toFloat();;
    
    
    ProcMode val = pupilDetection->getCurrentProcMode();

    QRectF roiMain1R;
    QRectF roiMain2R;
    QRectF roiSecondary1R;
    QRectF roiSecondary2R;
    if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        roiMain1R = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil1.rational", QRectF()).toRectF();
        roiSecondary1R = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil2.rational", QRectF()).toRectF();
    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        roiMain1R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilA1.rational", QRectF()).toRectF();
        roiMain2R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB1.rational", QRectF()).toRectF();
        roiSecondary1R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilA2.rational", QRectF()).toRectF();
        roiSecondary2R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB2.rational", QRectF()).toRectF();
    }

    QRectF initRoi = camera->getImageROI();
    if(!roiMain1R.isEmpty()) {
        QRectF roiMain1D = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil1.discrete", QRectF()).toRectF();
        mainVideoView->setROI1SelectionR(SupportFunctions::calculateRoiR(initRoi, roiMain1D, roiMain1R));
        //mainVideoView->saveROI1Selection(); // GB: why save just after loading?
    }
    if(!roiMain2R.isEmpty()) {
        QRectF roiMain2D = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB1.discrete", QRectF()).toRectF();
        mainVideoView->setROI2SelectionR(SupportFunctions::calculateRoiR(initRoi, roiMain2D, roiMain2R));
    }

    if(!roiSecondary1R.isEmpty()) {
        QRectF roiSecondary1D = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil2.discrete", QRectF()).toRectF();
        secondaryVideoView->setROI1SelectionR(SupportFunctions::calculateRoiR(initRoi, roiSecondary1D, roiSecondary1R));
        //secondaryVideoView->saveROI1Selection(); // GB: why save just after loading?
    }
    if(!roiSecondary2R.isEmpty()) {
        QRectF roiSecondary2D = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB2.discrete", QRectF()).toRectF();
        secondaryVideoView->setROI2SelectionR(SupportFunctions::calculateRoiR(initRoi, roiSecondary2D, roiSecondary2R));
    }

    

//    videoView->setAutoParamPupSize(applicationSettings->value("autoParamPupSizePercent", 50).toInt());   
    // GB added/modified end

}



// Opens a contextmenu on the toolbar for settings the plottable options
void StereoCameraView::onPlotMenuClick() {
    // fix to open submenu in the camera menu
    plotMenuAct->menu()->exec(QCursor::pos());
}

// Opens a contextmenu on the toolbar for settings the ROI options
void StereoCameraView::onPupilDetectionMenuClick() {
    // Fix to open submenu in the camera menu
    pupilDetectionMenuAct->menu()->exec(QCursor::pos());
}

// When pupil detection is started, labels are displayed and the camera live-stream is switched to the processed-image live-stream from the pupil detection
void StereoCameraView::onPupilDetectionStart() {

    statusBar->insertPermanentWidget(1, statusProcessingFPSWidget);
    statusProcessingFPSWidget->show();
    processingConfigLabel->setText(pupilDetection->getCurrentConfigLabel());
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));

    disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));

    // GB modified begin
    connect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    connect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    // GB modified end
}

// On stop of the pupil detection, disconnect the processed-image live-stream and show the camera image again
void StereoCameraView::onPupilDetectionStop() {

    statusBar->removeWidget(statusProcessingFPSWidget);

    // GB modified/added begin
    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));

    connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));

    mainVideoView->clearProcessedOverlayMemory();
    secondaryVideoView->clearProcessedOverlayMemory();
    // GB modified/added end
}

void StereoCameraView::updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) {

    if(timer.elapsed() <= updateDelay || cimg.img.empty()) 
        return;
    
    timer.restart();
        
    // GB: NOTE: disabled this feature, as it can occupy big space on smaller screens, 
    // and is only useful in case of fileCamera, but now that has playbackControlDialog which shows the same already
    //
    //      QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
    // QDateTime date = QDateTime::fromMSecsSinceEpoch(cimg.timestamp);
    //      Display the date/time in the system specific locale format
    // statusBar->showMessage(QLocale::system().toString(date));

    // GB: NOTE:
    // Now we are using stereo camera, so here are two videoViews, and it is necessary to know
    // which videoView gets which two ROIs and pupils
    
    std::vector<cv::Rect> mainViewROIs;
    std::vector<cv::Rect> secondaryViewROIs;
    std::vector<Pupil> mainViewPupils;
    std::vector<Pupil> secondaryViewPupils;
    if(procMode==ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        mainViewROIs.push_back(ROIs[0]);
        secondaryViewROIs.push_back(ROIs[1]);
        mainViewPupils.push_back(Pupils[0]);
        secondaryViewPupils.push_back(Pupils[1]);
    } else if(procMode==ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        mainViewROIs.push_back(ROIs[0]); //A1
        mainViewROIs.push_back(ROIs[2]); //B1
        secondaryViewROIs.push_back(ROIs[1]); //A2
        secondaryViewROIs.push_back(ROIs[3]); //B2
        mainViewPupils.push_back(Pupils[0]); //A1
        mainViewPupils.push_back(Pupils[2]); //B1
        secondaryViewPupils.push_back(Pupils[1]); //A2
        secondaryViewPupils.push_back(Pupils[3]); //B2
    }

    mainVideoView->updateViewProcessed(cimg.img, mainViewROIs, mainViewPupils);
    secondaryVideoView->updateViewProcessed(cimg.imgSecondary, secondaryViewROIs, secondaryViewPupils);
}


// Update the live-view with images from the current update stream (either processed images or camera images)
void StereoCameraView::updateView(const CameraImage &cimg) {

    if(timer.elapsed() <= updateDelay || cimg.img.empty()) 
        return;
    
    timer.restart();

    // GB: NOTE: disabled this feature, as it can occupy big space on smaller screens, 
    // and is only useful in case of fileCamera, but now that has playbackControlDialog which shows the same already
    //      QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
    // QDateTime date = QDateTime::fromMSecsSinceEpoch(cimg.timestamp);
    //      Display the date/time in the system specific locale format
    // statusBar->showMessage(QLocale::system().toString(date));
    
    mainVideoView->updateView(cimg.img);
    secondaryVideoView->updateView(cimg.imgSecondary);
}

// GB: adaptively rounded value for better visibility and comparability. Also it gets hidden if we are watching playback
void StereoCameraView::updateCameraFPS(double fps) {
    currentCameraFPS = fps;
    //cameraFPSValue->setText(QString::number(fps));

    // GB begin
    if(fps == 0)
        cameraFPSValue->setText("-");
    else if(fps > 0 && fps < 1)
        cameraFPSValue->setText(QString::number(fps,'f',4));
    else
        cameraFPSValue->setText(QString::number(round(fps)));
    // GB end
}

// Updates the label showing the current pupil detection processing fps
// To signal slow processing, the label is colored red when a certain threshold is broke
// GB: adaptively rounded value for better visibility and comparability
void StereoCameraView::updateProcessingFPS(double fps) {

    if(fps < currentCameraFPS*0.9) {
        processingFPSValue->setStyleSheet("color: red;");
    } else {
        processingFPSValue->setStyleSheet("color: black;");
    }
    //processingFPSValue->setText(QString::number(fps));

    // GB begin
    if(fps == 0)
        processingFPSValue->setText("-");
    else if(fps > 0 && fps < 1)
        processingFPSValue->setText(QString::number(fps,'f',4));
    else
        processingFPSValue->setText(QString::number(round(fps)));
    // GB end
}

// Updates the label displaying the current pupil detection algorithm used
void StereoCameraView::updateAlgorithmLabel() {
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
}


// Updates the position and size of the small pupil view based on the latest pupil detection
// GB: updated for 2 pupil version, and also reformed to use vector of Pupils
void StereoCameraView::updatePupilView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) {
    /*
    STEREO_IMAGE_TWO_PUPIL :
        Pupils[0] = pupilA1 (eye A view 1)
        Pupils[1] = pupilA2 (eye A view 2)
        Pupils[2] = pupilB1 (eye B view 1)
        Pupils[3] = pupilB2 (eye B view 2)
    */

    // If the view is not yet initialized, set a fixed size for it
    // This is done only once after activation to not switch sizes at each pupil update which makes the pupil view to jitterish
    if(displayPupilView && !initPupilViewSize) {
        initPupilViewSize = true;

        for(std::size_t z=0; z<Pupils.size(); z++)
            if(Pupils[z].valid(-2))
                pupilViewSize.push_back(QSize(static_cast<int>(Pupils[z].size.width * 1.6), static_cast<int>(Pupils[z].size.height * 1.6)));
            else
                pupilViewSize.push_back(QSize(0,0));
    }

    // Update the view not too often, ~30fps
    if(pupilViewTimer.elapsed() > updateDelay && pupilViewSize.size()>0) {
        pupilViewTimer.restart();

        std::vector<QRect> targetsMain;
        std::vector<QRect> targetsSecondary;

        if(procMode == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
            targetsMain.push_back( QRect( QPoint(
                        static_cast<int>(Pupils[0].center.x - (0.5 * pupilViewSize[0].width())),
                        static_cast<int>(Pupils[0].center.y - (0.5 * pupilViewSize[0].width()))), pupilViewSize[0]) );
            targetsSecondary.push_back( QRect( QPoint(
                        static_cast<int>(Pupils[1].center.x - (0.5 * pupilViewSize[1].width())),
                        static_cast<int>(Pupils[1].center.y - (0.5 * pupilViewSize[1].width()))), pupilViewSize[1]) );
        } else if(procMode == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
            targetsMain.push_back( QRect( QPoint(
                        static_cast<int>(Pupils[0].center.x - (0.5 * pupilViewSize[0].width())),
                        static_cast<int>(Pupils[0].center.y - (0.5 * pupilViewSize[0].width()))), pupilViewSize[0]) );
            targetsMain.push_back( QRect( QPoint(
                        static_cast<int>(Pupils[2].center.x - (0.5 * pupilViewSize[2].width())),
                        static_cast<int>(Pupils[2].center.y - (0.5 * pupilViewSize[2].width()))), pupilViewSize[2]) );
            targetsSecondary.push_back( QRect( QPoint(
                        static_cast<int>(Pupils[1].center.x - (0.5 * pupilViewSize[1].width())),
                        static_cast<int>(Pupils[1].center.y - (0.5 * pupilViewSize[1].width()))), pupilViewSize[1]) );
            targetsSecondary.push_back( QRect( QPoint(
                        static_cast<int>(Pupils[3].center.x - (0.5 * pupilViewSize[3].width())),
                        static_cast<int>(Pupils[3].center.y - (0.5 * pupilViewSize[3].width()))), pupilViewSize[3]) );
        }

        // Create a ROI around the pupil big enough to make changes visible
        mainVideoView->updatePupilViews(targetsMain);
        secondaryVideoView->updatePupilViews(targetsSecondary);
    }
}

/*
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

        mainVideoView->updatePupil1View(QRect(tl, pupilViewSize));
        secondaryVideoView->updatePupil1View(QRect(tlSec, pupilViewSizeSec));
    }
    
}
*/

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

    toolBar->addAction(resetROI);
    toolBar->addAction(discardROISelection);
    toolBar->addAction(saveROI);

    // GB modified/added begin
    tempROIs[0] = mainVideoView->getROI1SelectionR();
    tempROIs[1] = secondaryVideoView->getROI1SelectionR();
    mainVideoView->setROI1SelectionR(roiSize);
    secondaryVideoView->setROI1SelectionR(roiSize);
    if(mainVideoView->getDoubleROI()){
        tempROIs[2] = mainVideoView->getROI2SelectionR();
        mainVideoView->setROI2SelectionR(roiSize);
    }
    if(secondaryVideoView->getDoubleROI()){
        tempROIs[3] = secondaryVideoView->getROI2SelectionR();
        secondaryVideoView->setROI2SelectionR(roiSize);
    }
    // GB modified/added end
    mainVideoView->showROISelection(true);
    secondaryVideoView->showROISelection(true);
}

// Saves the current ROI selection
// This is propagated to the pupil detection dialog which then uses the new ROI selection
void StereoCameraView::onSaveROIClick() {
    bool s1, s2, s3, s4;
    s1=s2=s3=s4=false;
    s1 = mainVideoView->saveROI1Selection();
    s2 = secondaryVideoView->saveROI1Selection();
    if(mainVideoView->getDoubleROI())
        s3 = mainVideoView->saveROI2Selection();
    if(secondaryVideoView->getDoubleROI())
        s4 = secondaryVideoView->saveROI2Selection();
    
    if( s1 || s2 || s3 || s4 ) { // GB: if any is ok to set, we proceed
        mainVideoView->showROISelection(false);
        secondaryVideoView->showROISelection(false);
        toolBar->removeAction(resetROI);
        toolBar->removeAction(saveROI);
        toolBar->removeAction(discardROISelection);
    }
}

// Reset/Discard the current ROI selection dialog
void StereoCameraView::onResetROIClick() {
    mainVideoView->resetROISelection();
    secondaryVideoView->resetROISelection();
}

// Changes pupil color fill of the videoView
void StereoCameraView::onPupilColorFillChanged(int itemIndex) {
    pupilColorFill = (ColorFill)itemIndex;
    applicationSettings->setValue("StereoCameraView.pupilColorFill", pupilColorFill);
    
    emit onChangePupilColorFill(pupilColorFill);
}

// Changes pupil color fill lower-end threshold of the videoView
void StereoCameraView::onPupilColorFillThresholdChanged(double value) {
    pupilColorFillThreshold = value;
    applicationSettings->setValue("StereoCameraView.pupilColorFillThreshold", pupilColorFillThreshold);

    emit onChangePupilColorFillThreshold(pupilColorFillThreshold);
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
// GB modified and renamed: now it saves main view, roi nr 1
void StereoCameraView::saveMainROI1Selection(QRectF roiR) {
    qDebug() << "Saving Main ROI 1 selection" << Qt::endl;
    
    // GB modified begin
    QRectF imageSize = mainVideoView->getImageSize(); // GB NOTE: we assume that both camera images are the same size
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());

    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        applicationSettings->setValue("StereoCameraView.ROIstereoImageOnePupil1.rational", roiR);
        applicationSettings->setValue("StereoCameraView.ROIstereoImageOnePupil1.discrete", roiD);
        //qDebug() << "Pupil, viewpoint 1" << Qt::endl;
    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilA1.rational", roiR);
        applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilA1.discrete", roiD);
        //qDebug() << "Pupil A, viewpoint 1" << Qt::endl;
    } 
    // GB modified end
}

// Saves the ROI selection to the application wide settings (which are persisted to file)
// GB modified: now it saves secondary view, roi nr 1
void StereoCameraView::saveSecondaryROI1Selection(QRectF roiR) {
    qDebug() << "Saving Secondary ROI 1 selection" << Qt::endl;
    
    // GB modified begin
    QRectF imageSize = mainVideoView->getImageSize(); // GB NOTE: we assume that both camera images are the same size
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());

    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        applicationSettings->setValue("StereoCameraView.ROIstereoImageOnePupil2.rational", roiR);
        applicationSettings->setValue("StereoCameraView.ROIstereoImageOnePupil2.discrete", roiD);
        //qDebug() << "Pupil, viewpoint 2" << Qt::endl;
    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilA2.rational", roiR);
        applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilA2.discrete", roiD);
        //qDebug() << "Pupil A, viewpoint 2" << Qt::endl;
    } 
    // GB modified end
}

void StereoCameraView::saveMainROI2Selection(QRectF roiR) {
    qDebug() << "Saving Main ROI 2 selection" << Qt::endl;
    
    QRectF imageSize = mainVideoView->getImageSize(); // GB NOTE: we assume that both camera images are the same size
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());
    // STEREO_IMAGE_TWO_PUPIL
    applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilB1.rational", roiR);
    applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilB1.discrete", roiD);
    //qDebug() << "Pupil B, viewpoint 1" << Qt::endl;
}

void StereoCameraView::saveSecondaryROI2Selection(QRectF roiR) {
    qDebug() << "Saving Secondary ROI 2 selection" << Qt::endl;

    QRectF imageSize = mainVideoView->getImageSize(); // GB NOTE: we assume that both camera images are the same size
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());
    // STEREO_IMAGE_TWO_PUPIL
    applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilB2.rational", roiR);
    applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilB2.discrete", roiD);
    //qDebug() << "Pupil B, viewpoint 2" << Qt::endl; 
}

void StereoCameraView::onSettingsChange() {
    loadSettings();
}

// Updates a label which shows the current config
// A config a i.e. the optimized detection parameter for a specific ROI size
void StereoCameraView::updateConfigLabel(QString config) {
    processingConfigLabel->setText(config);
}



void StereoCameraView::onAutoParamPupSize(int value) {

    mainVideoView->setAutoParamPupSize(value);
    secondaryVideoView->setAutoParamPupSize(value);

    pupilDetection->setAutoParamPupSizePercent((float)value);
    pupilDetection->setAutoParamScheduled(true);

    applicationSettings->setValue("autoParamPupSizePercent", value); 
    mainVideoView->drawOverlay();
    secondaryVideoView->drawOverlay();
}

void StereoCameraView::updateForPupilDetectionProcMode() {

    disconnect(mainVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageOnePupil1(QRectF)));
    disconnect(mainVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveMainROI1Selection(QRectF)));
    disconnect(secondaryVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageOnePupil2(QRectF)));
    disconnect(secondaryVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveSecondaryROI1Selection(QRectF)));

    disconnect(mainVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilA1(QRectF))); 
    disconnect(mainVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveMainROI1Selection(QRectF)));
    disconnect(mainVideoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilB1(QRectF)));
    disconnect(mainVideoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveMainROI2Selection(QRectF)));
    disconnect(secondaryVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilA2(QRectF)));
    disconnect(secondaryVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveSecondaryROI1Selection(QRectF)));
    disconnect(secondaryVideoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilB2(QRectF)));
    disconnect(secondaryVideoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveSecondaryROI2Selection(QRectF)));

    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        //qDebug() << "STEREO_IMAGE_ONE_PUPIL" << Qt::endl;
        mainVideoView->setDoubleROI(false);
        secondaryVideoView->setDoubleROI(false);
        secondaryVideoView->setSelectionColor1(Qt::blue);
        mainVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);
        secondaryVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);

        connect(mainVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageOnePupil1(QRectF)));
        connect(mainVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveMainROI1Selection(QRectF)));
        connect(secondaryVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageOnePupil2(QRectF)));
        connect(secondaryVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveSecondaryROI1Selection(QRectF)));

    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        //qDebug() << "STEREO_IMAGE_TWO_PUPIL" << Qt::endl;
        mainVideoView->setDoubleROI(true);
        secondaryVideoView->setDoubleROI(true);
        secondaryVideoView->setSelectionColor1(QColor(144, 55, 212, 76)); // purple
        secondaryVideoView->setSelectionColor2(QColor(214, 140, 49,76)); // orange
        mainVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::LEFT_HALF);
        mainVideoView->setROI2AllowedArea(VideoView::ROIAllowedArea::RIGHT_HALF);
        secondaryVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::LEFT_HALF);
        secondaryVideoView->setROI2AllowedArea(VideoView::ROIAllowedArea::RIGHT_HALF);

        connect(mainVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilA1(QRectF))); 
        connect(mainVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveMainROI1Selection(QRectF)));
        connect(mainVideoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilB1(QRectF)));
        connect(mainVideoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveMainROI2Selection(QRectF)));
        connect(secondaryVideoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilA2(QRectF)));
        connect(secondaryVideoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveSecondaryROI1Selection(QRectF)));
        connect(secondaryVideoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROIstereoImageTwoPupilB2(QRectF)));
        connect(secondaryVideoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveSecondaryROI2Selection(QRectF)));

    }  else {
        //qDebug() << "Processing mode is undetermined" << Qt::endl;
    }

    mainVideoView->setImageSize(camera->getImageROIwidth(), camera->getImageROIheight());
    secondaryVideoView->setImageSize(camera->getImageROIwidth(), camera->getImageROIheight());
    loadSettings(); // same as onSettingsChange()

    updateProcModeLabel();

    // at last, we update the videoView to redraw the ROI overlay
    mainVideoView->drawOverlay();
    secondaryVideoView->drawOverlay();
}

void StereoCameraView::onShowAutoParamOverlay(bool state) {
    showAutoParamOverlay = state;
    applicationSettings->setValue("StereoCameraView.showAutoParamOverlay", showAutoParamOverlay);

    emit onChangeShowAutoParamOverlay(showAutoParamOverlay);
}

// GB TODO: STRINGS
void StereoCameraView::updateProcModeLabel() {
    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::UNDETERMINED) {
        processingModeLabel->setText("Undetermined");
    } else if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        processingModeLabel->setText("Stereo image one pupil");
    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        processingModeLabel->setText("Stereo image two pupil");
    } else {
        processingModeLabel->setText("Error");
    }
}

void StereoCameraView::displayFileCameraFrame(int frameNumber) {
    if(camera->getType() != CameraImageType::STEREO_IMAGE_FILE) 
        return;
      
    std::vector<cv::Mat> temp2 = dynamic_cast<FileCamera*>(camera)->getStillImageStereo(frameNumber);
    mainVideoView->updateView(temp2[0]);
    secondaryVideoView->updateView(temp2[1]);
}

void StereoCameraView::onDiscardROISelectionClick(){
    mainVideoView->setROI1SelectionR(tempROIs[0]);
    secondaryVideoView->setROI1SelectionR(tempROIs[1]);
    if(mainVideoView->getDoubleROI())
        mainVideoView->setROI2SelectionR(tempROIs[2]);
    if(secondaryVideoView->getDoubleROI())
        secondaryVideoView->setROI2SelectionR(tempROIs[3]);
    onSaveROIClick();
}