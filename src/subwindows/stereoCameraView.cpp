
#include "stereoCameraView.h"

#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>
#include "../supportFunctions.h"
#include "../SVGIconColorAdjuster.h"

// Creates a new stereo camera view widget, taking a stereo based camera (StereoCamera or FileCamera with stero mode)
// Pupil detection process is used to get detection results and display them, communicate ROI selection
StereoCameraView::StereoCameraView(Camera *camera, PupilDetection *pupilDetection, bool playbackFrozen, QWidget *parent) :
        QWidget(parent),
        camera(camera),
        pupilDetection(pupilDetection),
        displayPupilView(false),
        plotPupilCenter(false),
        plotROIContour(true),
        showAutoParamOverlay(false),
        initPupilViewSize(false),
        playbackFrozen(playbackFrozen),
        //pupilViewSize(0, 0),
        currentCameraFPS(0.0),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {


    setWindowTitle("Stereo camera view");


    QVBoxLayout* layout = new QVBoxLayout(this);

    toolBar = new QToolBar();
    QMenu *viewportMenu = new QMenu("Viewport");
    viewportMenuAct = viewportMenu->menuAction();
    connect(viewportMenuAct, &QAction::triggered, this, &StereoCameraView::onViewportMenuClick);

    viewportMenu->addAction("Fit", this, &StereoCameraView::onFitClick);
    viewportMenu->addAction("100%", this, &StereoCameraView::on100pClick);
    viewportMenu->addSeparator();
    viewportMenu->addAction("+Zoom", this, &StereoCameraView::onZoomPlusClick);
    viewportMenu->addAction("-Zoom", this, &StereoCameraView::onZoomMinusClick);
    viewportMenu->addSeparator();
            
    freezeAct = viewportMenu->addAction("Freeze", this, &StereoCameraView::onFreezeClicked);
    freezeAct->setCheckable(true);
    freezeAct->setChecked(playbackFrozen);
    toolBar->addAction(viewportMenuAct);
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

//    showAutoParamAct = plotMenu->addAction(QChar(0x21D2) +' '+ tr("Show Automatic Parametrization Overlay"));
    showAutoParamAct = plotMenu->addAction(tr("Show Automatic Parametrization Overlay"));
    showAutoParamAct->setCheckable(true);
    showAutoParamAct->setChecked(showAutoParamOverlay && pupilDetection->isAutoParamSettingsEnabled());
    showAutoParamAct->setEnabled(pupilDetection->isAutoParamSettingsEnabled());
    showAutoParamAct->setStatusTip(tr("Display expected pupil size maximum and minimum values as currently set for Automatic Parametrization."));
    plotMenu->addAction(showAutoParamAct);
    connect(showAutoParamAct, SIGNAL(toggled(bool)), this, SLOT(onShowAutoParamOverlay(bool)));

    showPositioningGuideAct = plotMenu->addAction(tr("Show Camera Positioning Guide"));
    showPositioningGuideAct->setCheckable(true);
    showPositioningGuideAct->setChecked(showPositioningGuide);
    showPositioningGuideAct->setStatusTip(tr("Show overlay to help position the camera(s)."));
    plotMenu->addAction(showPositioningGuideAct);
    connect(showPositioningGuideAct, SIGNAL(toggled(bool)), this, SLOT(onShowPositioningGuide(bool)));

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
    pupilColorFillLabel->setFixedWidth(90);

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
    pupilColorFillThresholdLabel->setFixedWidth(90);

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

    roiMenu = pupilDetectionMenu->addMenu(tr("&Pupil Detection ROI"));
    roiMenu->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/16/highlight-pointer-spot.svg"), applicationSettings));
    roiMenu->setEnabled(pupilDetection->isROIPreProcessingEnabled());

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


    autoParamMenu = pupilDetectionMenu->addMenu(tr("&Automatic Parametrization"));
    autoParamMenu->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/adjustlevels.svg"), applicationSettings));
    autoParamMenu->setEnabled(isAutoParamModificationEnabled());
    
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


    const QIcon okIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/dialog-ok-apply.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    saveROI = new QAction(okIcon, tr("Set ROI"), this);
    connect(saveROI, &QAction::triggered, this, &StereoCameraView::onSaveROIClick);

    const QIcon discardIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/dialog-cancel.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    discardROISelection = new QAction(discardIcon, tr("Cancel ROI Selection"), this);
    connect(discardROISelection, &QAction::triggered, this, &StereoCameraView::onDiscardROISelectionClick);

    const QIcon resetIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/gtk-convert.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
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
    mainVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);
    secondaryVideoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);
    layout->addLayout(videoViewLayout);

    statusBar = new QStatusBar();
    // statusbar widget
    QWidget *statusCameraFPSWidget = new QWidget();
    QHBoxLayout *statusBarLayout = new QHBoxLayout();
    statusBarLayout->setContentsMargins(8,0,8,0);
    QLabel *cameraFPSLabel = new QLabel();
    if(camera->getType() != STEREO_IMAGE_FILE)
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

    statusProcessingFPSWidget = new QWidget();
    QHBoxLayout *statusBarProcessingLayout = new QHBoxLayout();
    statusBarProcessingLayout->setContentsMargins(8,0,8,0);

    processingConfigLabel = new QLabel();
    processingAlgorithmLabel = new QLabel();

    QLabel *processingFPSLabel = new QLabel("Processing FPS:");
    processingFPSValue = new QLabel();

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

    statusProcessingFPSWidget->setLayout(statusBarProcessingLayout);

    layout->addWidget(statusBar);


    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

    connect(pupilDetection, SIGNAL(processingStarted()), this, SLOT(onPupilDetectionStart()));
    connect(pupilDetection, SIGNAL(processingFinished()), this, SLOT(onPupilDetectionStop()));
    connect(pupilDetection, SIGNAL(fps(double)), this, SLOT(updateProcessingFPS(double)));
    connect(pupilDetection, SIGNAL (algorithmChanged()), this, SLOT (updateAlgorithmLabel()));
    connect(pupilDetection, SIGNAL (configChanged(QString)), this, SLOT (onPupilDetectionConfigChanged(QString)));

    //connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage))); // this now has never relevance because signals always go through pupilDetection
    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(camera, SIGNAL(fps(double)), this, SLOT(updateCameraFPS(double)));

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
    connect(this, SIGNAL (onChangeShowPositioningGuide(bool)), mainVideoView, SLOT (onChangeShowPositioningGuide(bool)));
    connect(this, SIGNAL (onChangeShowPositioningGuide(bool)), secondaryVideoView, SLOT (onChangeShowPositioningGuide(bool)));
    connect(pupilDetection, SIGNAL (onROIPreprocessingChanged(bool)), mainVideoView, SLOT (onChangePupilDetectionUsingROI(bool)));
    connect(pupilDetection, SIGNAL (onROIPreprocessingChanged(bool)), secondaryVideoView, SLOT (onChangePupilDetectionUsingROI(bool)));

    connect(pupilColorFillBox, SIGNAL (currentIndexChanged(int)), this, SLOT (onPupilColorFillChanged(int)));
    connect(pupilColorFillThresholdBox, SIGNAL (valueChanged(double)), this, SLOT (onPupilColorFillThresholdChanged(double)));

    connect(autoParamPupSizeBox, SIGNAL(valueChanged(int)), autoParamSlider, SLOT(setValue(int)));
    connect(autoParamSlider, SIGNAL(valueChanged(int)), autoParamPupSizeBox, SLOT(setValue(int)));
    connect(autoParamPupSizeBox, SIGNAL(valueChanged(int)), this, SLOT(onAutoParamPupSize(int)));

    // NOTE: currently it loads the settings (for loading ROI settings), so the loadSettings call at the end is not necessary
    updateForPupilDetectionProcMode();
    //loadSettings();
}

StereoCameraView::~StereoCameraView() {

}

// Loads the view settings from the application wide settings
// Application settings contain both ROI selection if available
void StereoCameraView::loadSettings() {

    displayPupilView = SupportFunctions::readBoolFromQSettings("StereoCameraView.displayPupilView", false, applicationSettings);
    onDisplayPupilViewClick(displayPupilView);
    displayDetailAct->setChecked(displayPupilView);

    plotPupilCenter = SupportFunctions::readBoolFromQSettings("StereoCameraView.plotPupilCenter", false, applicationSettings);
    onPlotPupilCenterClick(plotPupilCenter);
    plotCenterAct->setChecked(plotPupilCenter);

    plotROIContour = SupportFunctions::readBoolFromQSettings("StereoCameraView.plotROIContour", true, applicationSettings);
    plotROIAct->setChecked(plotROIContour);
    onPlotROIClick(plotROIContour);

    showAutoParamOverlay = SupportFunctions::readBoolFromQSettings("StereoCameraView.showAutoParamOverlay", false, applicationSettings);
    showAutoParamAct->setChecked(showAutoParamOverlay);
    onShowAutoParamOverlay(showAutoParamOverlay);

    showPositioningGuide = SupportFunctions::readBoolFromQSettings("StereoCameraView.showPositioningGuide", false, applicationSettings);
    if(camera->getType() == STEREO_IMAGE_FILE) {
        showPositioningGuideAct->setDisabled(true);
        showPositioningGuideAct->setChecked(false);
    } else {
        showPositioningGuideAct->setChecked(showPositioningGuide);
        onShowPositioningGuide(showPositioningGuide);
    }

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
        roiMain1R = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil1.rational", QRectF(VideoView::defaultROImiddleR)).toRectF();
        roiSecondary1R = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil2.rational", QRectF(VideoView::defaultROImiddleR)).toRectF();
    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        roiMain1R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilA1.rational", QRectF(VideoView::defaultROIleftHalfR)).toRectF();
        roiMain2R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB1.rational", QRectF(VideoView::defaultROIrightHalfR)).toRectF();
        roiSecondary1R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilA2.rational", QRectF(VideoView::defaultROIleftHalfR)).toRectF();
        roiSecondary2R = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB2.rational", QRectF(VideoView::defaultROIrightHalfR)).toRectF();
    }

    mainVideoView->setROI1SelectionR(roiMain1R);
    mainVideoView->setROI2SelectionR(roiMain2R);
    secondaryVideoView->setROI1SelectionR(roiSecondary1R);
    secondaryVideoView->setROI2SelectionR(roiSecondary2R);

    // these are needed in order to save the ROI even if QSettings is reset and the default roi value is set
    mainVideoView->saveROI1Selection();
    mainVideoView->saveROI2Selection();
    secondaryVideoView->saveROI1Selection();
    secondaryVideoView->saveROI2Selection();

//    videoView->setAutoParamPupSize(applicationSettings->value("autoParamPupSizePercent", 50).toInt());
}

// Opens a contextmenu on the toolbar for settings the viewport options
void StereoCameraView::onViewportMenuClick() {
    // fix to open submenu in the camera menu
    viewportMenuAct->menu()->exec(QCursor::pos());
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

    disconnect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage)), this, SLOT(updateView(CameraImage)));

    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
}

// On stop of the pupil detection, disconnect the processed-image live-stream and show the camera image again
void StereoCameraView::onPupilDetectionStop() {

    statusBar->removeWidget(statusProcessingFPSWidget);

    disconnect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    disconnect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));

    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage)), this, SLOT(updateView(CameraImage)));

    mainVideoView->clearProcessedOverlayMemory();
    secondaryVideoView->clearProcessedOverlayMemory();
}

void StereoCameraView::updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) {

    if(cimg.img.empty())
        return;

    // NOTE: Now we are using stereo camera, so here are two videoViews, and it is necessary to know
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

    if(cimg.img.empty())
        return;

    // GB: NOTE: disabled this feature, as it can occupy big space on smaller screens, 
    // and is only useful in case of fileCamera, but now that has playbackControlDialog which shows the same already
    //      QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
    // QDateTime date = QDateTime::fromMSecsSinceEpoch(cimg.timestamp);
    //      Display the date/time in the system specific locale format
    // statusBar->showMessage(QLocale::system().toString(date));
    
    mainVideoView->updateView(cimg.img);
    secondaryVideoView->updateView(cimg.imgSecondary);
}

void StereoCameraView::updateCameraFPS(double fps) {
    currentCameraFPS = fps;
    //cameraFPSValue->setText(QString::number(fps));

    if(fps == 0)
        cameraFPSValue->setText("-");
    else if(fps > 0 && fps < 1)
        cameraFPSValue->setText(QString::number(fps,'f',4));
    else
        cameraFPSValue->setText(QString::number(round(fps)));
}

// Updates the label showing the current pupil detection processing fps
void StereoCameraView::updateProcessingFPS(double fps) {

    if(fps < currentCameraFPS*0.9) {
        processingFPSValue->setStyleSheet("color: red;");
    } else {
        processingFPSValue->setStyleSheet("color: black;");
    }
    //processingFPSValue->setText(QString::number(fps));

    if(fps == 0)
        processingFPSValue->setText("-");
    else if(fps > 0 && fps < 1)
        processingFPSValue->setText(QString::number(fps,'f',4));
    else
        processingFPSValue->setText(QString::number(round(fps)));
}

// Updates the label displaying the current pupil detection algorithm used
void StereoCameraView::updateAlgorithmLabel() {
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
}

// Updates the position and size of the small pupil view based on the latest pupil detection
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

    if(pupilViewSize.size() > 0) {

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

    if(roiSize == -1.0) {// "Custom"
        emit doingPupilDetectionROIediting(true);

        toolBar->addAction(resetROI);
        toolBar->addAction(discardROISelection);
        toolBar->addAction(saveROI);

        mainVideoView->showROISelection(true);
        secondaryVideoView->showROISelection(true);

        smallROIAct->setEnabled(false);
        middleROIAct->setEnabled(false);
        customROIAct->setEnabled(false);
    } else {
        mainVideoView->saveROI1Selection();
        secondaryVideoView->saveROI1Selection();
        if(mainVideoView->getDoubleROI()){
            mainVideoView->saveROI2Selection();
        }
        if(secondaryVideoView->getDoubleROI()){
            secondaryVideoView->saveROI2Selection();
        }
    }
    this->update();
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
    
    if( s1 || s2 || s3 || s4 ) {
        mainVideoView->showROISelection(false);
        secondaryVideoView->showROISelection(false);
        toolBar->removeAction(resetROI);
        toolBar->removeAction(saveROI);
        toolBar->removeAction(discardROISelection);
    }

    smallROIAct->setEnabled(true);
    middleROIAct->setEnabled(true);
    customROIAct->setEnabled(true);

    mainVideoView->drawOverlay();
    secondaryVideoView->drawOverlay();

    emit doingPupilDetectionROIediting(false);
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
    showAutoParamAct->setEnabled(pupilDetection->isAutoParamSettingsEnabled());
    emit onShowAutoParamOverlay(showAutoParamOverlay && pupilDetection->isAutoParamSettingsEnabled());
    emit onShowROI(plotROIContour && pupilDetection->isROIPreProcessingEnabled());
}


// Saves the ROI selection to the application wide settings (which are persisted to file)
// saves main view, roi nr 1
void StereoCameraView::saveMainROI1Selection(QRectF roiR) {
    qDebug() << "Saving Main ROI 1 selection" << Qt::endl;

    QRectF imageSize = mainVideoView->getImageSize();
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
}

// Saves the ROI selection to the application wide settings (which are persisted to file)
// saves secondary view, roi nr 1
void StereoCameraView::saveSecondaryROI1Selection(QRectF roiR) {
    qDebug() << "Saving Secondary ROI 1 selection" << Qt::endl;

    QRectF imageSize = mainVideoView->getImageSize();
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
}

void StereoCameraView::saveMainROI2Selection(QRectF roiR) {
    qDebug() << "Saving Main ROI 2 selection" << Qt::endl;
    
    QRectF imageSize = mainVideoView->getImageSize();
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());
    // STEREO_IMAGE_TWO_PUPIL
    applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilB1.rational", roiR);
    applicationSettings->setValue("StereoCameraView.ROIstereoImageTwoPupilB1.discrete", roiD);
    //qDebug() << "Pupil B, viewpoint 1" << Qt::endl;
}

void StereoCameraView::saveSecondaryROI2Selection(QRectF roiR) {
    qDebug() << "Saving Secondary ROI 2 selection" << Qt::endl;

    QRectF imageSize = mainVideoView->getImageSize();
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
void StereoCameraView::onPupilDetectionConfigChanged(QString config) {
    processingConfigLabel->setText(config);
    autoParamMenu->setEnabled(isAutoParamModificationEnabled());
    roiMenu->setEnabled(pupilDetection->isROIPreProcessingEnabled());
    showAutoParamAct->setEnabled(pupilDetection->isAutoParamSettingsEnabled());
    plotROIAct->setEnabled(pupilDetection->isROIPreProcessingEnabled());
    emit onChangeShowAutoParamOverlay(showAutoParamOverlay && pupilDetection->isAutoParamSettingsEnabled());
    emit onShowROI(plotROIContour && pupilDetection->isROIPreProcessingEnabled());
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

void StereoCameraView::onFreezeClicked() {
    emit cameraPlaybackChanged();
}

void StereoCameraView::onCameraPlaybackChanged() {
    playbackFrozen = !playbackFrozen;
    freezeAct->setChecked(playbackFrozen);
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
        mainVideoView->setSelectionColor1(QColor(0,0,255,76)); // Qt::blue
        secondaryVideoView->setSelectionColor1(QColor(0,255,0,76)); // Qt::green
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
        mainVideoView->setSelectionColor1(QColor(0,0,255,76)); // Qt::blue
        mainVideoView->setSelectionColor2(QColor(0,255,0,76)); // Qt::green
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

    // The following ones are NEEDED HERE, because they need to happen after we loaded new ROIs using loadSettings();
    // But they are also be needed for setting the right colour of the ROI rectangles, as these calls also do that
    if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
        mainVideoView->onROI1Change();
        secondaryVideoView->onROI1Change();
    } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
        mainVideoView->onROI1Change();
        mainVideoView->onROI2Change();
        secondaryVideoView->onROI1Change();
        secondaryVideoView->onROI2Change();
    }  else {
        //qDebug() << "Processing mode is undetermined" << Qt::endl;
    }

    updateProcModeLabel();

//    mainVideoView->update();
//    secondaryVideoView->update();

    // at last, we update the videoView to redraw the ROI overlay
    mainVideoView->drawOverlay();
    secondaryVideoView->drawOverlay();

    mainVideoView->refitPupilDetailViews();
    secondaryVideoView->refitPupilDetailViews();
}

void StereoCameraView::onShowAutoParamOverlay(bool state) {
    showAutoParamOverlay = state;
    applicationSettings->setValue("StereoCameraView.showAutoParamOverlay", showAutoParamOverlay);
    emit onChangeShowAutoParamOverlay(showAutoParamOverlay && pupilDetection->isAutoParamSettingsEnabled());
}

void StereoCameraView::onShowPositioningGuide(bool state) {
    showPositioningGuide = state;
    applicationSettings->setValue("StereoCameraView.showPositioningGuide", showPositioningGuide);
    emit onChangeShowPositioningGuide(showPositioningGuide);
}

void StereoCameraView::onImageROIChanged(const QRect& ROI) {
    mainVideoView->setImageROI(ROI);
    secondaryVideoView->setImageROI(ROI);
}

void StereoCameraView::onSensorSizeChanged(const QSize& size) {
    mainVideoView->setSensorSize(size);
    secondaryVideoView->setSensorSize(size);
}

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

bool StereoCameraView::isAutoParamModificationEnabled(){
    return pupilDetection->isAutoParamSettingsEnabled();
}
