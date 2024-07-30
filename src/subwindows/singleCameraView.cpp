#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>
#include "../supportFunctions.h"
#include "singleCameraView.h"
#include "../SVGIconColorAdjuster.h"

// Create new single camera view given a single camera object and a pupil detection process
// The pupil detection is used to display the detected pupils and show detection information such as processing fps
SingleCameraView::SingleCameraView(Camera *camera, PupilDetection *pupilDetection,  bool playbackFrozen, QWidget *parent) :
        QWidget(parent),
        camera(camera),
        pupilDetection(pupilDetection),
        displayPupilView(false),
        plotPupilCenter(false),
        plotROIContour(true),
        initPupilViewSize(false),
        playbackFrozen(playbackFrozen),
        //pupilViewSize(0, 0),
        currentCameraFPS(0.0),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), this)) {


    setWindowTitle("Single camera view");


    QVBoxLayout* layout = new QVBoxLayout(this);

    toolBar = new QToolBar();
    toolBar->addAction("Fit", this, &SingleCameraView::onFitClick);
    toolBar->addAction("100%", this, &SingleCameraView::on100pClick);
    toolBar->addSeparator();
    toolBar->addAction("+Zoom", this, &SingleCameraView::onZoomPlusClick);
    toolBar->addAction("-Zoom", this, &SingleCameraView::onZoomMinusClick);
    toolBar->addSeparator();
    if (playbackFrozen)
        freezeText = "Unfreeze";
    else 
        freezeText = "Freeze";

    freezeAct = toolBar->addAction(freezeText, this, &SingleCameraView::onFreezeClicked);
    toolBar->addSeparator();
    

    QMenu *plotMenu = new QMenu("Show", this);
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


    plotROIAct = plotMenu->addAction(tr("Visualize Pupil Detection ROI Contour"));
    plotROIAct->setCheckable(true);
    plotROIAct->setChecked(plotROIContour);
    plotROIAct->setEnabled(pupilDetection->isROIPreProcessingEnabled());
    plotROIAct->setStatusTip(tr("Display the region of interest applied in detection in the camera image."));
    plotMenu->addAction(plotROIAct);
    connect(plotROIAct, SIGNAL(toggled(bool)), this, SLOT(onPlotROIClick(bool)));

    // GB added/modified begin
    showAutoParamAct = plotMenu->addAction(tr("Show Automatic Parametrization Overlay"));
    showAutoParamAct->setCheckable(true);
    showAutoParamAct->setChecked(showAutoParamOverlay & plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
    showAutoParamAct->setEnabled(plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
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


    QLabel *pupilColorLabel = new QLabel("Show pupil detection confidence:", this);
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
    connect(pupilDetectionMenuAct, &QAction::triggered, this, &SingleCameraView::onPupilDetectionMenuClick);

    // GB: renamed to be more descriptive (not to be confused with camera image acquisition ROI), also modified tooltips
    roiMenu = pupilDetectionMenu->addMenu(tr("&Pupil Detection ROI"));
    roiMenu->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/16/highlight-pointer-spot.svg"), applicationSettings));
    roiMenu->setEnabled(pupilDetection->isROIPreProcessingEnabled());

    customROIAct = roiMenu->addAction(tr("Custom"), this, [this]()
    {
        onSetROIClick(-1.0);
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

    QLabel *autoParamPupSizeLabel = new QLabel("Expected max. pupil size [%]:", this);
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


    autoParamPupSizeBox->setValue(50);
    autoParamSlider->setValue(50);
    toolBar->addAction(pupilDetectionMenuAct);
    toolBar->addSeparator();

    // NOTE: added this to prevent the toolbar from popping bigger/smaller everytime saveROI and resetROI actions are revealed/hidden
    toolBar->setFixedHeight(36);
    
    // GB added/modified end


    const QIcon okIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/dialog-ok-apply.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    saveROI = new QAction(okIcon, tr("Set ROI"), this);
    connect(saveROI, &QAction::triggered, this, &SingleCameraView::onSaveROIClick);

    const QIcon discardIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/dialog-cancel.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    discardROISelection = new QAction(discardIcon, tr("Cancel ROI Selection"), this);
    connect(discardROISelection, &QAction::triggered, this, &SingleCameraView::onDiscardROISelectionClick);

    const QIcon resetIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/gtk-convert.svg"), applicationSettings);
    resetROI = new QAction(resetIcon, tr("Reset ROI"), this);
    connect(resetROI, &QAction::triggered, this, &SingleCameraView::onResetROIClick);


    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    layout->addWidget(toolBar);

    // GB NOTE: first just create the videoView instance like it is for a single ROI, and then we can change
    videoView = new VideoView();
    videoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);
    layout->addWidget(videoView);

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

    processingAlgorithmLabel = new QLabel();
    processingConfigLabel = new QLabel();
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

    // Connect the pupil detection process to inform this widget of changes
    connect(pupilDetection, SIGNAL(processingStarted()), this, SLOT(onPupilDetectionStart()));
    connect(pupilDetection, SIGNAL(processingFinished()), this, SLOT(onPupilDetectionStop()));
    connect(pupilDetection, SIGNAL(fps(double)), this, SLOT(updateProcessingFPS(double)));
    connect(pupilDetection, SIGNAL (algorithmChanged()), this, SLOT (updateAlgorithmLabel()));
    connect(pupilDetection, SIGNAL (configChanged(QString)), this, SLOT (onPupilDetectionConfigChanged(QString)));

    pupilDetection->setUpdateFPS(1000/updateDelay);

    // In the normal state, images are shown from the camera directly, when pupil detection is activated,
    // this signal is stopped and images from the detection are displayed
    //connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    connect(camera, SIGNAL(fps(double)), this, SLOT(updateCameraFPS(double)));
    

    // GB modified/added begin
    // GB: onShowROI() and onShowPupilCenter() not handled in pupilDetection anymore. Taken care of in camera view and videoView
    connect(this, SIGNAL (onShowROI(bool)), videoView, SLOT (onShowROI(bool)));
    connect(this, SIGNAL (onShowPupilCenter(bool)), videoView, SLOT (onShowPupilCenter(bool)));
    connect(this, SIGNAL (onChangePupilColorFill(int)), videoView, SLOT (onChangePupilColorFill(int)));
    connect(this, SIGNAL (onChangePupilColorFillThreshold(float)), videoView, SLOT (onChangePupilColorFillThreshold(float)));
    connect(this, SIGNAL (onChangeShowAutoParamOverlay(bool)), videoView, SLOT (onChangeShowAutoParamOverlay(bool)));
    connect(this, SIGNAL (onChangeShowPositioningGuide(bool)), videoView, SLOT (onChangeShowPositioningGuide(bool)));
    connect(pupilDetection, SIGNAL (onROIPreprocessingChanged(bool)), videoView, SLOT (onChangePupilDetectionUsingROI(bool)));
    
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

SingleCameraView::~SingleCameraView() {

}

void SingleCameraView::loadSettings() {
    // GB TODO: surely loads the bools always? Seems to be ok, but I remember to have seen cases in other code when the read value was "false" which was not parsed as 0

    displayPupilView = applicationSettings->value("SingleCameraView.displayPupilView", false).toBool();
    onDisplayPupilViewClick(displayPupilView);
    displayDetailAct->setChecked(displayPupilView);

    plotPupilCenter = applicationSettings->value("SingleCameraView.plotPupilCenter", false).toBool();
    onPlotPupilCenterClick(plotPupilCenter);
    plotCenterAct->setChecked(plotPupilCenter);

    plotROIContour = applicationSettings->value("SingleCameraView.plotROIContour", false).toBool();
    plotROIAct->setChecked(plotROIContour);
    onPlotROIClick(plotROIContour);

    // GB added/modified begin
    showAutoParamOverlay = applicationSettings->value("SingleCameraView.showAutoParamOverlay", false).toBool();
    showAutoParamAct->setChecked(showAutoParamOverlay);
    onShowAutoParamOverlay(showAutoParamOverlay);

    showPositioningGuide = applicationSettings->value("SingleCameraView.showPositioningGuide", false).toBool();
    if(camera->getType() == SINGLE_IMAGE_FILE) {
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

    pupilColorFill = (ColorFill)applicationSettings->value("SingleCameraView.pupilColorFill", pupilColorFill).toInt();
    pupilColorFillThreshold = applicationSettings->value("SingleCameraView.pupilColorFillThreshold", pupilColorFillThreshold).toFloat();;


    ProcMode val = pupilDetection->getCurrentProcMode();
    QRectF roi1;
    QRectF roi2;
    if(val == ProcMode::SINGLE_IMAGE_ONE_PUPIL) {
        roi1 = applicationSettings->value("SingleCameraView.ROIsingleImageOnePupil.rational", QRectF()).toRectF();
    } else if(val == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
        roi1 = applicationSettings->value("SingleCameraView.ROIsingleImageTwoPupilA.rational", QRectF()).toRectF();
        roi2 = applicationSettings->value("SingleCameraView.ROIsingleImageTwoPupilB.rational", QRectF()).toRectF();
    // } else if(val == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
    //     roi1 = applicationSettings->value("SingleCameraView.ROImirrImageOnePupil1.rational", QRectF()).toRectF();
    //     roi2 = applicationSettings->value("SingleCameraView.ROImirrImageOnePupil2.rational", QRectF()).toRectF();
    }
    QRectF initRoi = camera->getImageROI();
    if(!roi1.isEmpty()) {
        QRectF roi1D = applicationSettings->value("SingleCameraView.ROIsingleImageOnePupil.discrete", QRectF()).toRectF();
        
        videoView->setROI1SelectionR(SupportFunctions::calculateRoiR(initRoi, roi1D, roi1));
    }
    if(!roi2.isEmpty()) {
        QRectF roi2D = applicationSettings->value("SingleCameraView.ROIsingleImageTwoPupilA.discrete", QRectF()).toRectF();
        
        videoView->setROI1SelectionR(SupportFunctions::calculateRoiR(initRoi, roi2D, roi2));
    }
    // GB added/modified end
}

void SingleCameraView::onPlotMenuClick() {
    // Fix to open submenu in the camera menu
    plotMenuAct->menu()->exec(QCursor::pos());
}

void SingleCameraView::onPupilDetectionMenuClick() {
    // Fix to open submenu in the camera menu
    pupilDetectionMenuAct->menu()->exec(QCursor::pos());
}

// When pupil detection is started, signaled by the pupil detection process, the camera image signals are disconnected from this view
// and the processed pupil images from the pupil detection are shown (pupil plotted on top etc.)
void SingleCameraView::onPupilDetectionStart() {

    statusBar->insertPermanentWidget(1, statusProcessingFPSWidget);
    statusProcessingFPSWidget->show();
    processingConfigLabel->setText(pupilDetection->getCurrentConfigLabel());
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title())); // GB NOTE: But when do we hide it??
    
    // GB added/modified begin
    updateProcModeLabel();

    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));
    
    connect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    connect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    // GB modified end
}

// When pupil detection is stopped, signaled by the pupil detection process, the camera image signals are connected again to this view
// and the processed pupil images from the pupil detection are disconnected
void SingleCameraView::onPupilDetectionStop() {

    statusBar->removeWidget(statusProcessingFPSWidget);

    // GB added/modified begin
    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    disconnect(pupilDetection, SIGNAL(processedImage(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));

    connect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateView(CameraImage)));

    videoView->clearProcessedOverlayMemory();
    // GB added/modified end
}

void SingleCameraView::updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) {

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
    // As we are using a single camera right now, we can just pass the ROIs and Pupils vectors
    // But in case of stereo cameras, where there are two videoViews, it is necessary to know
    // which videoView gets which two ROIs and pupils (see stereoCameraView for details)
    videoView->updateViewProcessed(cimg.img, ROIs, Pupils);
}

void SingleCameraView::updateView(const CameraImage &cimg) {

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
        
    videoView->updateView(cimg.img);
}


// GB: adaptively rounded value for better visibility and comparability
void SingleCameraView::updateCameraFPS(double fps) {
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

// If the processing fps is sign. slower (10%) than the camera fps, color it red
// GB: adaptively rounded value for better visibility and comparability. Also it gets hidden if we are watching playback
void SingleCameraView::updateProcessingFPS(double fps) {
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

void SingleCameraView::updateAlgorithmLabel() {
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
}

void SingleCameraView::onPupilDetectionConfigChanged(QString config) {
    processingConfigLabel->setText(config);
    autoParamMenu->setEnabled(isAutoParamModificationEnabled());
    roiMenu->setEnabled(pupilDetection->isROIPreProcessingEnabled());
    showAutoParamAct->setEnabled(plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
    plotROIAct->setEnabled(pupilDetection->isROIPreProcessingEnabled());
    emit onChangeShowAutoParamOverlay(showAutoParamOverlay & plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
    emit onShowROI(plotROIContour & pupilDetection->isROIPreProcessingEnabled());
}


// Updates the position and size of the small pupil view based on the latest pupil detection
// GB: updated for 2 pupil version, and also reformed to use vector of Pupils
void SingleCameraView::updatePupilView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) {

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

        std::vector<QRect> targets;

        for(std::size_t z=0; z<Pupils.size(); z++)
            targets.push_back( QRect( QPoint(
                    static_cast<int>(Pupils[z].center.x - (0.5 * pupilViewSize[z].width())),
                    static_cast<int>(Pupils[z].center.y - (0.5 * pupilViewSize[z].width()))), pupilViewSize[z]) );

        // Create a ROI around the pupil big enough to make changes visible
        videoView->updatePupilViews(targets);
    }
}

/*
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
        videoView->updatePupil1View(QRect(tl, pupilViewSize));
    }

}
*/

// Click event handler
// Fits the camera view to the size of the window
void SingleCameraView::onFitClick() {
    videoView->fitView();
}

// begin edit of kheki4 on 2022.10.24, NOTE: to properly re-fit view when image acquisition ROI is changed
/*
void SingleCameraView::onAcqImageROIchanged() {
    videoView->resetInitialFit();
    //videoView->fitView();
    //videoView->drawBlank();
}
*/
// end of edit by kheki4

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
    toolBar->addAction(resetROI);
    toolBar->addAction(discardROISelection);
    toolBar->addAction(saveROI);
    
    tempROIRect1 = videoView->getROI1SelectionR();
    videoView->setROI1SelectionR(roiSize);
    if(videoView->getDoubleROI()) { // GB
        tempROIRect2 = videoView->getROI2SelectionR();
        videoView->setROI2SelectionR(roiSize);
    }
    //videoView->resetROISelection();
    videoView->showROISelection(true);
}

void SingleCameraView::onSaveROIClick() {
    // GB: modified
    bool s1, s2;
    s1=s2=false;
    s1 = videoView->saveROI1Selection();
    if(videoView->getDoubleROI())
        s2 = videoView->saveROI2Selection();

    if( s1 || s2 ) { 
        // GB: if any is ok to set, we proceed
        videoView->showROISelection(false);
        toolBar->removeAction(resetROI);
        toolBar->removeAction(saveROI);
        toolBar->removeAction(discardROISelection);
    }
}

// One method for discarding one or both ROIs
void SingleCameraView::onResetROIClick() {
    videoView->resetROISelection();
}

// Changes pupil color fill of the videoView
void SingleCameraView::onPupilColorFillChanged(int itemIndex) {
    pupilColorFill = (ColorFill)itemIndex;
    applicationSettings->setValue("SingleCameraView.pupilColorFill", pupilColorFill);
    
    emit onChangePupilColorFill(pupilColorFill);
}

// Changes pupil color fill lower-end threshold of the videoView
void SingleCameraView::onPupilColorFillThresholdChanged(double value) {
    pupilColorFillThreshold = value;
    applicationSettings->setValue("SingleCameraView.pupilColorFillThreshold", pupilColorFillThreshold);

    emit onChangePupilColorFillThreshold(pupilColorFillThreshold);
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
    showAutoParamAct->setEnabled(plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
    emit onShowAutoParamOverlay(showAutoParamOverlay & plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
    emit onShowROI(plotROIContour & pupilDetection->isROIPreProcessingEnabled());
}

void SingleCameraView::saveROI1Selection(QRectF roiR) { 
    qDebug() << "Saving ROI 1 selection" << Qt::endl;
    //applicationSettings->setValue("SingleCameraView.roi1SelectionRect", roi);

    // GB modified begin
    QRectF imageSize = videoView->getImageSize();
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());

    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::SINGLE_IMAGE_ONE_PUPIL) {
        applicationSettings->setValue("SingleCameraView.ROIsingleImageOnePupil.rational", roiR);
        applicationSettings->setValue("SingleCameraView.ROIsingleImageOnePupil.discrete", roiD);
    } else if(val == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
        applicationSettings->setValue("SingleCameraView.ROIsingleImageTwoPupilA.rational", roiR);
        applicationSettings->setValue("SingleCameraView.ROIsingleImageTwoPupilA.discrete", roiD);
    // } else if(val == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
    //     applicationSettings->setValue("SingleCameraView.ROImirrImageOnePupil1.rational", roiR);
    //     applicationSettings->setValue("SingleCameraView.ROImirrImageOnePupil1.discrete", roiD);
    } 
    // GB modified end
}

void SingleCameraView::saveROI2Selection(QRectF roiR) {
    qDebug() << "Saving ROI 2 selection" << Qt::endl;

    QRectF imageSize = videoView->getImageSize();
    QRectF roiD = QRectF(roiR.x()*imageSize.width(), roiR.y()*imageSize.height(), roiR.width()*imageSize.width(), roiR.height()*imageSize.height());

    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
        applicationSettings->setValue("SingleCameraView.ROIsingleImageTwoPupilB.rational", roiR);
        applicationSettings->setValue("SingleCameraView.ROIsingleImageTwoPupilB.discrete", roiD);
    // } else if(val == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
    //     applicationSettings->setValue("SingleCameraView.ROImirrImageOnePupil2.rational", roiR);
    //     applicationSettings->setValue("SingleCameraView.ROImirrImageOnePupil2.discrete", roiD);
    } 
}

void SingleCameraView::onSettingsChange() {
    loadSettings();
}


void SingleCameraView::onAutoParamPupSize(int value) {

    videoView->setAutoParamPupSize(value);

    pupilDetection->setAutoParamPupSizePercent((float)value);
    pupilDetection->setAutoParamScheduled(true);

    applicationSettings->setValue("autoParamPupSizePercent", value); 
    videoView->drawOverlay();
}

void SingleCameraView::onFreezeClicked()
{
    emit cameraPlaybackChanged();
}

void SingleCameraView::onCameraPlaybackChanged()
{
    if (playbackFrozen)
        freezeText = "Freeze";
    else 
        freezeText = "Unfreeze";

    freezeAct->setText(freezeText);

    playbackFrozen = !playbackFrozen;
}

void SingleCameraView::updateForPupilDetectionProcMode() {

    disconnect(videoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIsingleImageOnePupil(QRectF)));
    disconnect(videoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveROI1Selection(QRectF)));
    
    disconnect(videoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIsingleImageTwoPupilA(QRectF)));
    disconnect(videoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveROI1Selection(QRectF)));
    disconnect(videoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROIsingleImageTwoPupilB(QRectF)));
    disconnect(videoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveROI2Selection(QRectF)));

    disconnect(videoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROImirrImageOnePupil1(QRectF)));
    disconnect(videoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveROI1Selection(QRectF)));
    disconnect(videoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROImirrImageOnePupil2(QRectF)));
    disconnect(videoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveROI2Selection(QRectF)));


    ProcMode val = pupilDetection->getCurrentProcMode();
    // BREAKPOINT
    if(val == ProcMode::SINGLE_IMAGE_ONE_PUPIL) {
        //qDebug() << "SINGLE_IMAGE_ONE_PUPIL" << Qt::endl;
        videoView->setDoubleROI(false);
        videoView->setROI1AllowedArea(VideoView::ROIAllowedArea::ALL);

        connect(videoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIsingleImageOnePupil(QRectF)));
        connect(videoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveROI1Selection(QRectF)));
        
    } else if(val == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
        //qDebug() << "SINGLE_IMAGE_TWO_PUPIL" << Qt::endl;
        videoView->setDoubleROI(true);
        videoView->setROI1AllowedArea(VideoView::ROIAllowedArea::LEFT_HALF);
        videoView->setROI2AllowedArea(VideoView::ROIAllowedArea::RIGHT_HALF);

        connect(videoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROIsingleImageTwoPupilA(QRectF)));
        connect(videoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveROI1Selection(QRectF)));
        connect(videoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROIsingleImageTwoPupilB(QRectF)));
        connect(videoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveROI2Selection(QRectF)));
        
    // } else if(val == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
    //     //qDebug() << "MIRR_IMAGE_ONE_PUPIL" << Qt::endl;
    //     videoView->setDoubleROI(true);
    //     videoView->setROI1AllowedArea(VideoView::ROIAllowedArea::LEFT_HALF);
    //     videoView->setROI2AllowedArea(VideoView::ROIAllowedArea::RIGHT_HALF);

    //     connect(videoView, SIGNAL (onROI1SelectionD(QRectF)), pupilDetection, SLOT (setROImirrImageOnePupil1(QRectF)));
    //     connect(videoView, SIGNAL (onROI1SelectionR(QRectF)), this, SLOT (saveROI1Selection(QRectF)));
    //     connect(videoView, SIGNAL (onROI2SelectionD(QRectF)), pupilDetection, SLOT (setROImirrImageOnePupil2(QRectF)));
    //     connect(videoView, SIGNAL (onROI2SelectionR(QRectF)), this, SLOT (saveROI2Selection(QRectF)));
        
    } else {
        //qDebug() << "Processing mode is undetermined" << Qt::endl;
    }

    videoView->setImageSize(camera->getImageROIwidth(), camera->getImageROIheight());
    loadSettings(); // same as onSettingsChange()

    updateProcModeLabel();

    // at last, we update the videoView to redraw the ROI overlay
    videoView->drawOverlay();
}

void SingleCameraView::onShowAutoParamOverlay(bool state) {
    showAutoParamOverlay = state;
    applicationSettings->setValue("SingleCameraView.showAutoParamOverlay", showAutoParamOverlay);
    emit onChangeShowAutoParamOverlay(showAutoParamOverlay & plotROIContour & pupilDetection->isAutoParamSettingsEnabled());
}

void SingleCameraView::onShowPositioningGuide(bool state) {
    showPositioningGuide = state;
    applicationSettings->setValue("SingleCameraView.showPositioningGuide", showAutoParamOverlay);
    emit onChangeShowPositioningGuide(showPositioningGuide);
}

void SingleCameraView::onImageROIChanged(const QRect& ROI) {
    videoView->setImageROI(ROI);
}

void SingleCameraView::onSensorSizeChanged(const QSize& size) {
    videoView->setSensorSize(size);
}

// GB TODO: STRINGS
void SingleCameraView::updateProcModeLabel() {
    ProcMode val = pupilDetection->getCurrentProcMode();
    if(val == ProcMode::UNDETERMINED) {
        processingModeLabel->setText("Undetermined");
    } else if(val == ProcMode::SINGLE_IMAGE_ONE_PUPIL) {
        processingModeLabel->setText("Single image one pupil");
    } else if(val == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
        processingModeLabel->setText("Single image two pupil");
    // } else if(val == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
    //     processingModeLabel->setText("Mirrored image one pupil");
    } else {
        processingModeLabel->setText("Error");
    }
}

void SingleCameraView::displayFileCameraFrame(int frameNumber) {
    if(camera->getType() != CameraImageType::SINGLE_IMAGE_FILE) 
        return; 
      
    cv::Mat temp1 = dynamic_cast<FileCamera*>(camera)->getStillImageSingle(frameNumber);
    videoView->updateView(temp1);
}

void SingleCameraView::onDiscardROISelectionClick(){
    videoView->setROI1SelectionR(tempROIRect1);
    if(videoView->getDoubleROI())
        videoView->setROI2SelectionR(tempROIRect2);
    onSaveROIClick();
}

bool SingleCameraView::isAutoParamModificationEnabled(){
    return pupilDetection->isAutoParamSettingsEnabled();
}
