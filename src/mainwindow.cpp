#include "mainwindow.h"

#include <QtWidgets>
#include <QtWidgets/QWidget>
#include "subwindows/graphPlot.h"
#include "subwindows/singleCameraView.h"
#include "subwindows/singleCameraCalibrationView.h"
#include "subwindows/dataTable.h"
#include "devices/fileCamera.h"
#include "subwindows/stereoCameraSettingsDialog.h"
#include "subwindows/stereoCameraView.h"
#include "subwindows/stereoCameraCalibrationView.h"
#include "subwindows/stereoFileCameraCalibrationView.h"
#include "subwindows/singleFileCameraCalibrationView.h"
#include "subwindows/RestorableQMdiSubWindow.h"
#include "subwindows/singleCameraSharpnessView.h"
#include "subwindows/gettingsStartedWizard.h"

#include "supportFunctions.h"


// Upon construction, worker objects for processing are created pupil detection and its respective thread
MainWindow::MainWindow():
                          mdiArea(new QMdiArea(this)),
                          signalPubSubHandler(new SignalPubSubHandler(this)),
                          

                          subjectSelectionDialog(new SubjectSelectionDialog(this)),
                          singleCameraSettingsDialog(nullptr),
                          stereoCameraSettingsDialog(nullptr),
                          pupilDetectionThread(new QThread()),
                          selectedCamera(nullptr),
                          cameraViewWindow(nullptr),
                          calibrationWindow(nullptr),
                          sharpnessWindow(nullptr),
                          dataWriter(nullptr), // GB set to nullptr
                          imageWriter(nullptr), // GB set to nullptr
                          
                          singleWebcamSettingsDialog(nullptr),
                          singleCameraChildWidget(nullptr),
                          stereoCameraChildWidget(nullptr),
                          recEventTracker(nullptr),
                          camTempMonitor(nullptr),
                          connPoolCOM(new ConnPoolCOM(this)),
                          dataStreamer(nullptr),
                          imagePlaybackControlDialog(nullptr),

                          /*serialSettingsDialog(new SerialSettingsDialog(connPoolCOM, this)),
                          //remoteCCDialog(new RemoteCCDialog(connPoolUDP, connPoolCOM, this)),
                          remoteCCDialog(new RemoteCCDialog(connPoolCOM,this)), //connPoolCOM, pupilDetectionWorker, dataWriter, imageWriter, dataStreamer, offlineEventLogWriter, 
                          streamingSettingsDialog(new StreamingSettingsDialog(connPoolCOM, pupilDetectionWorker, dataStreamer, this)),
*/
                          applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), this)) {

    loadIcons();
    const QByteArray alwaysOnTop = applicationSettings->value("alwaysOnTop", "0").toByteArray();
    if (!alwaysOnTop.isEmpty() && (alwaysOnTop == "1" || alwaysOnTop == "true") ) {
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
        //show();
    }

    imageMutex = new QMutex();
    imagePublished = new QWaitCondition();
    imageProcessed = new QWaitCondition();
    pupilDetectionWorker = new PupilDetection(imageMutex, imagePublished, imageProcessed);
    serialSettingsDialog = new SerialSettingsDialog(connPoolCOM, this);
    serialSettingsDialog->setWindowIcon(cameraSerialConnectionIcon);
    remoteCCDialog = new RemoteCCDialog(connPoolCOM,this); //connPoolCOM, pupilDetectionWorker, dataWriter, imageWriter, dataStreamer, offlineEventLogWriter,
    remoteCCDialog->setWindowIcon(remoteCCIcon);
    streamingSettingsDialog = new StreamingSettingsDialog(connPoolCOM, pupilDetectionWorker, dataStreamer, this);
    streamingSettingsDialog->setWindowIcon(streamingSettingsIcon);

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    qDebug() << "Application settings location: " << applicationSettings->fileName() << Qt::endl;

    connect(serialSettingsDialog, SIGNAL (onConnect()), this, SLOT (onSerialConnect()));
    connect(serialSettingsDialog, SIGNAL (onDisconnect()), this, SLOT (onSerialDisconnect()));

    pupilDetectionSettingsDialog = new PupilDetectionSettingsDialog(pupilDetectionWorker, this);
    pupilDetectionSettingsDialog->setWindowIcon(pupilDetectionSettingsIcon);

    generalSettingsDialog = new GeneralSettingsDialog(this);
    generalSettingsDialog->setWindowIcon(generalSettingsIcon);

    subjectSelectionDialog->setWindowIcon(subjectsIcon);

    connect(generalSettingsDialog, SIGNAL (onSettingsChange()), this, SLOT (onGeneralSettingsChange()));
    connect(subjectSelectionDialog, SIGNAL (onSubjectChange(QString)), this, SLOT (onSubjectsSettingsChange(QString)));
    connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), pupilDetectionSettingsDialog, SLOT (onSettingsChange()));


    // Pupil detection is conducted in another thread, move the created object to this thread and connect its finished signal for cleanup
    pupilDetectionWorker->moveToThread(pupilDetectionThread);
    connect(pupilDetectionThread, SIGNAL (finished()), pupilDetectionThread, SLOT (deleteLater()));
    pupilDetectionThread->start();
    pupilDetectionThread->setPriority(QThread::HighPriority); // highest priority


    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    QVBoxLayout *verticalLayout = new QVBoxLayout(mdiArea);
    mdiArea->setLayout(verticalLayout);
    mdiArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setCentralWidget(mdiArea);

    connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMenus);

    createActions();
    createStatusBar();
    updateMenus();

    // read user settings stored in the user directory
    readSettings();

    setWindowTitle(QCoreApplication::applicationName());
    setUnifiedTitleAndToolBarOnMac(true);

    QIcon::setThemeName( "Breeze" );

    QStringList themeSearchPaths = QIcon::themeSearchPaths();
    themeSearchPaths.prepend(":/icons");
    QIcon::setThemeSearchPaths(themeSearchPaths);

    qRegisterMetaType<Pupil>("Pupil");
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<CameraImage>("CameraImage");

    // DEV
    qRegisterMetaType<cv::Rect>("cv::Rect");
    qRegisterMetaType<std::vector<Pupil>>("std::vector<Pupil>");
    qRegisterMetaType<std::vector<cv::Rect>>("std::vector<cv::Rect>");

    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");
    qRegisterMetaType<std::vector<double>>("std::vector<double>");
    // DEV


    // Get settings key if gettings started dialog was already opened
    //bool showGettingsStartedWizard = applicationSettings->value("ShowGettingsStartedWizard", false).toBool();

    // GB begin:
    // GB note I think .toBool does not work (?), always assigns true. People say its that "false" text converts to true
    bool showGettingsStartedWizard = false;
    const QByteArray m_showGettingsStartedWizard = applicationSettings->value("ShowGettingsStartedWizard", "1").toByteArray();
    //std::cout << m_metaSnapshotsEnabled.toStdString() << std::endl; //
    if (!m_showGettingsStartedWizard.isEmpty() && (m_showGettingsStartedWizard == "1" || m_showGettingsStartedWizard == "true"))
        showGettingsStartedWizard = true;
    // GB end

    if(showGettingsStartedWizard) {
        GettingsStartedWizard* wizard = new GettingsStartedWizard(this);
        wizard->show();
        connect(wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this , SLOT(onGettingsStartedWizardFinish()));
    }
    
    // GB added begin
    connect(remoteCCDialog, SIGNAL (onConnStateChanged()), this, SLOT (onRemoteConnStateChanged()));
    //connect(streamingSettingsDialog, SIGNAL (onConnStateChanged()), this, SLOT (onStreamingConnStateChanged()));
    connect(streamingSettingsDialog, SIGNAL (onUDPConnect()), this, SLOT (onStreamingUDPConnect()));
    connect(streamingSettingsDialog, SIGNAL (onUDPDisconnect()), this, SLOT (onStreamingUDPDisconnect()));
    connect(streamingSettingsDialog, SIGNAL (onCOMConnect()), this, SLOT (onStreamingCOMConnect()));
    connect(streamingSettingsDialog, SIGNAL (onCOMDisconnect()), this, SLOT (onStreamingCOMDisconnect())); 

    // if proc mode settings are not interpretable, reset them
    ProcMode pmSingle = (ProcMode)applicationSettings->value("PupilDetectionSettingsDialog.singleCam.procMode", ProcMode::UNDETERMINED).toInt();
    ProcMode pmStereo = (ProcMode)applicationSettings->value("PupilDetectionSettingsDialog.stereoCam.procMode", ProcMode::UNDETERMINED).toInt();
    if( pmSingle != ProcMode::SINGLE_IMAGE_ONE_PUPIL ||
        pmSingle != ProcMode::SINGLE_IMAGE_TWO_PUPIL // ||
        // pmSingle != ProcMode::MIRR_IMAGE_ONE_PUPIL 
        ) {

        applicationSettings->setValue("PupilDetectionSettingsDialog.singleCam.procMode", ProcMode::SINGLE_IMAGE_ONE_PUPIL);
    }
    if( pmStereo != ProcMode::STEREO_IMAGE_ONE_PUPIL ||
        pmStereo != ProcMode::STEREO_IMAGE_TWO_PUPIL ) {

        applicationSettings->setValue("PupilDetectionSettingsDialog.stereoCam.procMode", ProcMode::STEREO_IMAGE_ONE_PUPIL);
    }

    connect(pupilDetectionSettingsDialog, SIGNAL (pupilDetectionProcModeChanged(int)), pupilDetectionWorker, SLOT (setCurrentProcMode(int)));

    setAcceptDrops(true);

    playbackSynchroniser = nullptr;
    // GB added end
}

void MainWindow::onGettingsStartedWizardFinish() {
    applicationSettings->setValue("ShowGettingsStartedWizard", false);
}

void MainWindow::loadIcons() {
    fileOpenIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/document-open.svg"), applicationSettings);
    cameraSerialConnectionIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/rs232.svg"), applicationSettings);
    pupilDetectionSettingsIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/draw-circle.svg"), applicationSettings);
    remoteCCIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/computer-connection.svg"), applicationSettings);
    generalSettingsIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/mimetypes/16/application-x-sharedlib.svg"), applicationSettings);
    singleCameraIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/devices/22/camera-video.svg"), applicationSettings);
    stereoCameraIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/camera-video-stereo.svg"), applicationSettings);
    cameraSettingsIcon1 = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/configure.svg"), applicationSettings);
    cameraSettingsIcon2 = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/configure.svg"), applicationSettings);
    calibrateIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/crosshairs.svg"), applicationSettings);
    sharpnessIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/edit-select-all.svg"), applicationSettings);
    subjectsIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/im-user.svg"), applicationSettings);
    outputDataFileIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/edit-text-frame-update.svg"), applicationSettings);
    streamingSettingsIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/view-presentation.svg"), applicationSettings);
    imagePlaybackControlIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-start.svg"), applicationSettings);
}

void MainWindow::createActions() {

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));

    // Note: made global to let is get disabled/enabled, whether there is already an opened directory or not
    fileOpenAct = fileMenu->addAction(tr("Open Images Directory"), this, &MainWindow::onOpenImageDirectory);
    fileOpenAct->setIcon(fileOpenIcon);
    fileOpenAct->setStatusTip(tr("Open Image Directory for Playback. Single and Stereo Mode supported."));
    fileMenu->addAction(fileOpenAct);
    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), qApp, &QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);

    QMenu *viewMenu = menuBar()->addMenu(tr("View"));
    QMenu *addWindowsMenu = viewMenu->addMenu(tr("Windows"));

    addWindowsMenu->addAction(tr("Camera View"), this, &MainWindow::cameraViewClick);
    addWindowsMenu->addAction(tr("Data Table"), this, &MainWindow::dataTableClick);

    viewMenu->addSeparator();
    viewMenu->addAction(tr("Toggle Fullscreen"));
    //viewMenu->addAction(tr("Switch layout direction"), this, &MainWindow::switchLayoutDirection);

    QMenu *settingsMenu = menuBar()->addMenu(tr("Settings"));
    settingsMenu->addAction(cameraSerialConnectionIcon, tr("Camera Serial Connection"), serialSettingsDialog, &SerialSettingsDialog::show);
    settingsMenu->addAction(pupilDetectionSettingsIcon, tr("Pupil Detection"), pupilDetectionSettingsDialog, &PupilDetectionSettingsDialog::show);
    settingsMenu->addAction(remoteCCIcon, tr("Remote Control Connection"), remoteCCDialog, &RemoteCCDialog::show);
    settingsMenu->addAction(generalSettingsIcon, tr("General Settings"), generalSettingsDialog, &GeneralSettingsDialog::show);

    windowMenu = menuBar()->addMenu(tr("Windows"));
    connect(windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

    menuBar()->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    QAction *openSourceAct = helpMenu->addAction(tr("Open Source Licenses"), this, &MainWindow::openSourceDialog);
    openSourceAct->setStatusTip(tr("Show the application's open source usages."));

    QAction *aboutAct = helpMenu->addAction(tr("About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));


    toolBar = new QToolBar(); // addToolBar(tr("Toolbar"));
    toolBar->setStyleSheet("QToolBar{spacing:10px;}");
    toolBar->setMovable(true); // Makes the toolbar moveable by the user, default is set ot left side
    toolBar->setFloatable(false); // Sets the toolbar as its own window
    toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    menuBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    addToolBar(Qt::LeftToolBarArea, toolBar); // Add the toolbar to the window, on the left side initially

    cameraAct = new QAction(singleCameraIcon, tr("Camera"), this);
    cameraAct->setStatusTip(tr("Connect to camera(s)."));

    QMenu* cameraMenu = new QMenu(this);
    singleCameraDevicesMenu = cameraMenu->addMenu(singleCameraIcon,tr("&Single Camera"));
    updateCameraMenu();
    connect(singleCameraDevicesMenu, SIGNAL(triggered(QAction*)), this, SLOT(singleCameraSelected(QAction*)));
    cameraMenu->addAction(stereoCameraIcon, tr("Stereo Camera"), this, &MainWindow::stereoCameraSelected);
    cameraMenu->addSeparator();

    // updateCameraMenu
    cameraMenu->addAction(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/refactor.svg"), applicationSettings), tr("Refresh Devices"), this, &MainWindow::updateCameraMenu);

    // GB added begin
    // GB: added webcams option, as webcams/opencv devices are now supported
    cameraMenu->addSeparator();

    QWidget *webcamInfoWidget = new QWidget();
    QHBoxLayout *webcamInfoLayout = new QHBoxLayout();
    webcamInfoLayout->setContentsMargins(8,4,8,4); 
    QLabel *webcamInfoLabel = new QLabel("Connect single OpenCV webcam:");
    webcamInfoLayout->addWidget(webcamInfoLabel);
    webcamInfoWidget->setLayout(webcamInfoLayout);
    
    QWidgetAction *wact1 = new QWidgetAction(cameraMenu);
    wact1->setCheckable(false);
    wact1->setDefaultWidget(webcamInfoWidget);
    cameraMenu->addAction(wact1);

    QWidget *webcamDeviceWidget = new QWidget();
    QHBoxLayout *webcamDeviceLayout = new QHBoxLayout();
    webcamDeviceLayout->setContentsMargins(8,0,8,4); 
    QLabel *webcamDeviceLabel = new QLabel("Device ID:");
    webcamDeviceLabel->setFixedWidth(60);
    webcamDeviceBox = new QSpinBox();
    webcamDeviceBox->setMinimum(0);
    webcamDeviceBox->setMaximum(64);
    webcamDeviceBox->setSingleStep(1);
    webcamDeviceBox->setValue(0);
    QPushButton *webcamDeviceButton = new QPushButton("Connect");
    connect(webcamDeviceButton, SIGNAL(clicked()), this, SLOT(singleWebcamSelected()));

    webcamDeviceLayout->addWidget(webcamDeviceLabel);
    webcamDeviceLayout->addWidget(webcamDeviceBox);
    webcamDeviceLayout->addWidget(webcamDeviceButton);
    webcamDeviceWidget->setLayout(webcamDeviceLayout);

    QWidgetAction *wact2 = new QWidgetAction(cameraMenu);
    wact2->setCheckable(false);
    wact2->setDefaultWidget(webcamDeviceWidget);
    cameraMenu->addAction(wact2);
    // GB added end


    cameraAct->setMenu(cameraMenu);
    connect(cameraAct, &QAction::triggered, this, &MainWindow::onCameraClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(cameraAct);

    const QIcon disconnectIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/network-disconnect.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    cameraActDisconnectAct = new QAction(disconnectIcon, tr("Disconnect"), this);
    //trackAct->setShortcuts(QKeySequence::New);
    cameraActDisconnectAct->setStatusTip(tr("Disconnect camera."));
    connect(cameraActDisconnectAct, &QAction::triggered, this, &MainWindow::onCameraDisconnectClick);
    cameraActDisconnectAct->setDisabled(true);
    //fileMenu->addAction(newAct);
    toolBar->addAction(cameraActDisconnectAct);

    cameraSettingsAct = new QAction(cameraSettingsIcon1, tr("Camera Settings"), this);
    //trackAct->setShortcuts(QKeySequence::New);
    cameraSettingsAct->setStatusTip(tr("Camera settings."));
    connect(cameraSettingsAct, &QAction::triggered, this, &MainWindow::onCameraSettingsClick);
    cameraSettingsAct->setDisabled(true);
    settingsMenu->addAction(cameraSettingsAct);
    toolBar->addAction(cameraSettingsAct);

    // GB added begin
    // NOTE: these should have come before, when the menu actions are defined, but as these whould come after cameraSettingsAct, I put them down here
    settingsMenu->addSeparator();

    const QIcon forceResetTrialIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/equals1b.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    forceResetTrialAct = new QAction(forceResetTrialIcon, tr("Force reset trial counter"), this);
    forceResetTrialAct->setEnabled(false);
    //connect(forceResetTrialAct, &QAction::triggered, this, &MainWindow::forceResetTrialCounter);
    connect(forceResetTrialAct, SIGNAL(triggered()), this, SLOT(forceResetTrialCounter()));
    settingsMenu->addAction(forceResetTrialAct);

    const QIcon manualIncTrialIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/plus1b.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    manualIncTrialAct = new QAction(manualIncTrialIcon, tr("Manually increment trial counter"), this);
    manualIncTrialAct->setEnabled(false);
    connect(manualIncTrialAct, SIGNAL(triggered()), this, SLOT(incrementTrialCounter()));
    settingsMenu->addAction(manualIncTrialAct);
    // GB added end

    // GB: play and stop buttons were here, but have been moved to ImagePlaybackControlDialog

    toolBar->addSeparator();

    const QIcon trackOffIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/view-visible.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    trackAct = new QAction(trackOffIcon, tr("Track"), this);
    trackAct->setCheckable(true);
    //trackAct->setShortcuts(QKeySequence::New);
    trackAct->setStatusTip(tr("Start/Stop pupil tracking."));
    connect(trackAct, &QAction::triggered, this, &MainWindow::onTrackActClick);
    trackAct->setDisabled(true);
    //fileMenu->addAction(newAct);
    toolBar->addAction(trackAct);

    toolBar->addSeparator();

    calibrateAct = new QAction(calibrateIcon, tr("Camera calibration to compensate lens distortion"), this);
    calibrateAct->setStatusTip(tr("Start camera calibration to compensate lens distortion."));
    connect(calibrateAct, &QAction::triggered, this, &MainWindow::onCalibrateClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(calibrateAct);
    calibrateAct->setDisabled(true);

    sharpnessAct = new QAction(sharpnessIcon, tr("Sharpness"), this);
    sharpnessAct->setStatusTip(tr("Validate sharpness."));
    connect(sharpnessAct, &QAction::triggered, this, &MainWindow::onSharpnessClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(sharpnessAct);
    sharpnessAct->setDisabled(true);

    subjectsAct = new QAction(subjectsIcon, tr("Subjects"), this);
    subjectsAct->setStatusTip(tr("Load subject specific detection configuration."));
    connect(subjectsAct, &QAction::triggered, this, &MainWindow::onSubjectsClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(subjectsAct);
    subjectsAct->setDisabled(true);

    toolBar->addSeparator();

    logFileAct = new QAction(outputDataFileIcon, tr("Output data file"), this);
    logFileAct->setStatusTip(tr("Set output data file path and name."));
    connect(logFileAct, &QAction::triggered, this, &MainWindow::setLogFile);
    //fileMenu->addAction(newAct);
    toolBar->addAction(logFileAct);
    logFileAct->setDisabled(true);


    const QIcon recordIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-record.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    recordAct = new QAction(recordIcon, tr("Record"), this);
    recordAct->setStatusTip(tr("Start pupil recording."));
    connect(recordAct, &QAction::triggered, this, &MainWindow::onRecordClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(recordAct);
    recordAct->setDisabled(true);


    toolBar->addSeparator();

    outputDirectoryAct = new QAction(fileOpenIcon, tr("Output Directory"), this);
    outputDirectoryAct->setStatusTip(tr("Set output directory."));
    connect(outputDirectoryAct, &QAction::triggered, this, &MainWindow::setOutputDirectory);
    //fileMenu->addAction(newAct);
    toolBar->addAction(outputDirectoryAct);
    outputDirectoryAct->setDisabled(true);

    const QIcon recordImagesIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-record-blue.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    recordImagesAct = new QAction(recordImagesIcon, tr("Record Images"), this);
    recordImagesAct->setStatusTip(tr("Start image recording."));
    connect(recordImagesAct, &QAction::triggered, this, &MainWindow::onRecordImageClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(recordImagesAct);
    recordImagesAct->setDisabled(true);

    // GB added begin
    toolBar->addSeparator();

    streamingSettingsAct = new QAction(streamingSettingsIcon, tr("Streaming settings"), this);
    streamingSettingsAct->setStatusTip(tr("Settings for data streaming."));
    connect(streamingSettingsAct, &QAction::triggered, this, &MainWindow::onStreamingSettingsClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(streamingSettingsAct);
    //streamingSettingsAct->setDisabled(true);
    streamingSettingsAct->setDisabled(true);

    const QIcon streamIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/media-record-green.svg"), applicationSettings);
    streamAct = new QAction(streamIcon, tr("Stream"), this);
    streamAct->setStatusTip(tr("Stream pupil detection output."));
    connect(streamAct, &QAction::triggered, this, &MainWindow::onStreamClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(streamAct);
    streamAct->setDisabled(true);
    // GB added end


    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, &QAction::triggered, mdiArea, &QMdiArea::closeActiveSubWindow);

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, &QAction::triggered, mdiArea, &QMdiArea::closeAllSubWindows);

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, &QAction::triggered, mdiArea, &QMdiArea::tileSubWindows);

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, &QAction::triggered, mdiArea, &QMdiArea::cascadeSubWindows);

    resetGeometryAct = new QAction(tr("&Reset Interface"), this);
    resetGeometryAct->setStatusTip(tr("Reset the position and size of the windows"));
    connect(resetGeometryAct, &QAction::triggered, this, &MainWindow::resetGeometry);

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, &QAction::triggered, mdiArea, &QMdiArea::activateNextSubWindow);

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous window"));
    connect(previousAct, &QAction::triggered, mdiArea, &QMdiArea::activatePreviousSubWindow);

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);

    updateWindowMenu();



    
}

void MainWindow::createStatusBar() {

    //statusBar()->setMaximumHeight(20);
    //statusBar()->layout()->setContentsMargins(0,0,0,0);
    //statusBar()->layout()->setSpacing(0);
    //statusBar()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QWidget *widget = new QWidget();
    QHBoxLayout *statusBarLayout = new QHBoxLayout(widget);
    statusBarLayout->setContentsMargins(8,0,8,0);

    // GB added begin
    QLabel *remoteLabel = new QLabel("Remote Control Conn.");
    const QIcon remoteIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    remoteStatusIcon = new QLabel();
    remoteStatusIcon->setPixmap(remoteIcon.pixmap(16, 16));

    statusBarLayout->addWidget(remoteLabel);
    statusBarLayout->addWidget(remoteStatusIcon);
    // GB added end

    QLabel *calibrationLabel = new QLabel("Camera Calibration");
    const QIcon calibrationIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    calibrationStatusIcon = new QLabel();
    calibrationStatusIcon->setPixmap(calibrationIcon.pixmap(16, 16));

    statusBarLayout->addWidget(calibrationLabel);
    statusBarLayout->addWidget(calibrationStatusIcon);

    // GB: renamed to be better descriptive, as now there are other purposes for serial connection too
    QLabel *serialLabel = new QLabel("Camera Serial Conn.");
    const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    serialStatusIcon = new QLabel();
    serialStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));

    statusBarLayout->addWidget(serialLabel);
    statusBarLayout->addWidget(serialStatusIcon);

    QLabel *hwTriggerLabel = new QLabel("Hardware Trigger");
    hwTriggerStatusIcon = new QLabel();
    hwTriggerStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));

    statusBarLayout->addWidget(hwTriggerLabel);
    statusBarLayout->addWidget(hwTriggerStatusIcon);

    // GB added begin
    trialWidget = new QWidget();

    QHBoxLayout *trialWidgetLayout = new QHBoxLayout(trialWidget);
    trialWidgetLayout->setContentsMargins(8,0,8,0);

    QLabel *trialLabel = new QLabel("Trial: ");
    currentTrialLabel = new QLabel();
    trialWidgetLayout->addWidget(trialLabel);
    trialWidgetLayout->addWidget(currentTrialLabel);
    //QFrame* sep = new QFrame();
    //sep->setFrameShape(QFrame::VLine);
    //sep->setFrameShadow(QFrame::Sunken);
    //statusBarLayout->addWidget(sep);
    updateCurrentTrialLabel();

    statusBar()->addPermanentWidget(trialWidget);
    trialWidget->setVisible(false);
    // GB added end

    statusBar()->addPermanentWidget(widget);

    QLabel *versionLabel = new QLabel(QCoreApplication::applicationVersion());
    statusBar()->addPermanentWidget(versionLabel);

    subjectConfigurationLabel = new QLabel("");
    statusBar()->addWidget(subjectConfigurationLabel);

    currentDirectoryLabel = new QLabel("");
    statusBar()->addWidget(currentDirectoryLabel);
}

void MainWindow::closeEvent(QCloseEvent *event) {

    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F){
        Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
        if (modifiers == Qt::ShiftModifier){
            onCameraFreezePressed();
        }
        else
            QWidget::keyPressEvent(event);
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::onCameraFreezePressed()
{
    emit cameraPlaybackChanged();
}

void MainWindow::onCameraPlaybackChanged()
{
    if (cameraPlaying){
        stopCamera();
    }
    else{ 
        startCamera();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == singleCameraSettingsDialog || obj == stereoCameraSettingsDialog){
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_F){
                keyPressEvent(keyEvent);
                return true;
            }
            else
                return false;
        }
        else 
            return false;
    }
    else
        return QObject::eventFilter(obj, event);
}

void MainWindow::about() {
    GettingsStartedWizard* wizard = new GettingsStartedWizard(this);
    wizard->show();
    connect(wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this , SLOT(onGettingsStartedWizardFinish()));

//    QMessageBox::about(this, tr("About ") + QCoreApplication::applicationName(),
//            tr("%1 is an open source application for pupillometry.<br><br>"
//               "Babak Zandi, Moritz Lode, Alexander Herzog, Georgios Sakas and Tran Quoc Khanh. (2021)."
//               " PupilEXT: Flexible Open-Source Platform for High-Resolution Pupil Measurement in Vision Research. "
//               "Frontiers in Neuroscience. doi:10.3389/fnins.2021.676220."
//               "<br><br>"
//               "Github: <a href=\"https://github.com/openPupil/Open-PupilEXT\">https://github.com/openPupil/Open-PupilEXT</a><br>"
//               "Developer-Team: Moritz Lode, Babak Zandi<br>Version: %2<br><br>"
//               "The software PupilEXT is licensed under <a href=\"https://github.com/openPupil/Open-PupilEXT/blob/main/PupilEXT/LICENSE\">GNU General Public License v.3.0.</a>"
//               ", Copyright (c) 2021 Technical University of Darmstadt. PupilEXT is for academic and non-commercial use only."
//               " Third-party libraries may be distributed under other open-source licenses (see GitHub repository).<br><br>"
//               "Application settings: %3<br>"
//    ).arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), applicationSettings->fileName()));
}

void MainWindow::openSourceDialog() {

    QDialog *dialog = new QDialog(this);
    dialog->resize(500, 300);
    dialog->setWindowTitle("Open Source Contributions and Licenses");

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);

    QVBoxLayout *l = new QVBoxLayout();

    QScrollArea *scroll = new QScrollArea();
    l->addWidget(scroll);
    l->addWidget(buttonBox);

    QLabel *label = new QLabel();
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    label->setWordWrap(true);

    label->setText(
            "Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci, \"ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments.\", 2016<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini / University of Tübingen<br><br>"
            "Wolfgang Fuhl, Thomas Kübler, Katrin Sippel, Wolfgang Rosenstiel, Enkelejda Kasneci, \"ExCuSe: Robust Pupil Detection in Real-World Scenarios.\", 2015<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini / University of Tübingen<br><br>"
            "Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, \"PuRe: Robust pupil detection for real-time pervasive eye tracking.\", 2018<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini<br><br>"
            "Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, \"PuReST: Robust Pupil Tracking for Real-Time Pervasive Eye.\", 2018<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini<br><br>"
            "Li, Dongheng & Winfield, D. & Parkhurst, D.J., \"Starburst: A hybrid algorithm for video-based eye tracking combining feature-based and model-based approaches.\", 2005<br/>Part of the <a href=\"http://thirtysixthspan.com/openEyes/software.html\">cvEyeTracker</a> software License: <a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">GPL 3</a><br><br>"
            "Lech Swirski, Andreas Bulling, Neil A. Dodgson, \"Robust real-time pupil tracking in highly off-axis images\", 2012 <a href=\"http://www.cl.cam.ac.uk/research/rainbow/projects/pupiltracking\">Website</a><br/>License: <a href=\"https://opensource.org/licenses/MIT\">MIT</a><br><br>"
            "Boost libraries, License: <a href=\"https://www.boost.org/LICENSE_1_0.txt\">Boost License</a><br><br>"
            "Qt framework, License: <a href=\"https://www.gnu.org/licenses/lgpl-3.0.txt\">LGPL v3</a><br><br>"
            "OpenCV library, License: <a href=\"https://opencv.org/license/\">BSD 3-Clause</a><br><br>"
            "TBB library, License: <a href=\"https://www.apache.org/licenses/LICENSE-2.0.txt\">Apache 2.0</a><br><br>"
            "Gflags, License: <a href=\"https://opencv.org/license/\">BSD 3-Clause</a><br><br>"
            "QCustomPLot Library, License: <a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">GPL 3</a><br><br>"
            "Breeze Icon Theme, License: <a href=\"https://www.gnu.org/licenses/lgpl-3.0.txt\">LGPL v3</a><br><br>");

    scroll->setWidget(label);
    label->setOpenExternalLinks(true);
    dialog->setLayout(l);
    dialog->show();
}


void MainWindow::setLogFile() {

    QString tempFile = QFileDialog::getSaveFileName(this, tr("Save Log File"), recentPath, tr("CSV files (*.csv)"), nullptr, QFileDialog::DontConfirmOverwrite);

    // GB: cleaner code without long indentation
    if(tempFile.isEmpty())
        return;

    if(!QFileInfo(tempFile).dir().exists())
        return;

    pupilDetectionDataFile = tempFile;
    QFileInfo fileInfo(pupilDetectionDataFile);
    recentPath = fileInfo.dir().path();

    // check if filename has extension
    if(fileInfo.suffix().isEmpty()) {
        pupilDetectionDataFile = pupilDetectionDataFile + ".csv";
    }

    //QFile file(pupilDetectionDataFile);
    //file.open(QIODevice::WriteOnly); // Or QIODevice::ReadWrite
    //file.close();

    // GB: I think its better like this 
    if(trackingOn)
        recordAct->setDisabled(false);
}

void MainWindow::setOutputDirectory() {

    outputDirectory = QFileDialog::getExistingDirectory(this, tr("Output Directory"), recentPath);
    

    // GB: cleaner code without long indentation
    if(outputDirectory.isEmpty()) 
        return;

    // GB modified begin
    // GB NOTE: caused crash when path depth was less than 4, so this was a solution 
    recentPath = outputDirectory;
    std::cout << recentPath.toStdString() << std::endl;
    QStringList lst = recentPath.split('/');
    if(lst.count() > 3)
        currentDirectoryLabel->setText("Current directory: .../"  + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    else if(lst.count() == 3)
        currentDirectoryLabel->setText("Current directory: " + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    else if(lst.count() == 2)
        currentDirectoryLabel->setText("Current directory: " + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    else if(lst.count() == 1)
        currentDirectoryLabel->setText("Current directory: " + lst[lst.count()-1]);
    currentDirectoryLabel->setToolTip(recentPath);
    // GB modified end

    recordImagesAct->setDisabled(false);
}

void MainWindow::updateMenus() {
    bool hasMdiChild = (activeMdiChild() != nullptr);

    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    windowMenuSeparatorAct->setVisible(hasMdiChild);
}

void MainWindow::updateWindowMenu() {

    //std::cout<<"updateWindowMenu"<<std::endl;

    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(resetGeometryAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(windowMenuSeparatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    windowMenuSeparatorAct->setVisible(!windows.isEmpty());

    for(auto mdiSubWindow : windows) {
        QWidget *child = mdiSubWindow->widget();

        QString text = child->windowTitle();
        QAction *action = windowMenu->addAction(text, mdiSubWindow, [this, mdiSubWindow]() {
            mdiArea->setActiveSubWindow(mdiSubWindow);
        });
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
    }
}

void MainWindow::readSettings() {

    const QByteArray geometry = applicationSettings->value("MainWindow.geometry", QByteArray()).toByteArray();

    if (geometry.isEmpty()) {
        this->showMaximized();
    } else {
        restoreGeometry(geometry);
    }

    recentPath = applicationSettings->value("RecentOutputPath", "").toString();

    Qt::ToolBarArea toolBarPosition = static_cast<Qt::ToolBarArea>(applicationSettings->value("MainWindow.ToolbarPosition", Qt::LeftToolBarArea).toUInt());
    addToolBar(toolBarPosition, toolBar); // As toolbar is already attached to the window, it is only moved to this position by addToolBar
}

void MainWindow::writeSettings() {
    applicationSettings->setValue("MainWindow.geometry", saveGeometry());
    applicationSettings->setValue("RecentOutputPath", recentPath);

    applicationSettings->setValue("MainWindow.ToolbarPosition", static_cast<uint>(toolBarArea(toolBar)));
}

QWidget* MainWindow::activeMdiChild() const {
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return activeSubWindow->widget();
    return nullptr;
}

void MainWindow::updateCameraMenu() {

    //singleCameraDevicesMenu->clear();
    Pylon::DeviceInfoList_t lstDevices = enumerateCameraDevices();

    if (!lstDevices.empty()) {
        Pylon::DeviceInfoList_t::const_iterator deviceIt;
        QList<QAction*>::iterator actionIt; // need to use iterator as we modify the list while iterating
        QList<QAction*> actions = singleCameraDevicesMenu->actions();

        // first remove all devices not found anymore
        for(actionIt = actions.begin(); actionIt != actions.end(); ++actionIt ) {
            if((*actionIt)->text() == "No devices.") {
                singleCameraDevicesMenu->removeAction(*actionIt);
            }
            bool found = false;
            for(deviceIt = lstDevices.begin(); deviceIt != lstDevices.end(); ++deviceIt ) {
                if((*actionIt)->data() == deviceIt->GetFullName().c_str()) {
                    found = true;
                }
            }
            if(!found) {
                singleCameraDevicesMenu->removeAction(*actionIt);
            }
        }

        // then add all new devices
        for(deviceIt = lstDevices.begin(); deviceIt != lstDevices.end(); ++deviceIt ) {
            QList<QAction*> new_actions = singleCameraDevicesMenu->actions();
            bool found = false;
            for(QAction *action: new_actions) {
                if(action->data() == deviceIt->GetFullName().c_str()) {
                    found = true;
                }
            }
            if(!found) {
                QAction *cameraAction = singleCameraDevicesMenu->addAction(deviceIt->GetFriendlyName().c_str());
                cameraAction->setData(QString(deviceIt->GetFullName()));
                if(QString(deviceIt->GetFriendlyName().c_str()).toLower().contains("emulat")) {
                    cameraAction->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/composite-track-preview.svg"), applicationSettings));
                }
            }
        }

    }
}

void MainWindow::onTrackActClick() {

    if(trackingOn) {
        // Deactivate tracking
        pupilDetectionWorker->stopDetection();

        // GB added begin
        if(pupilDetectionSettingsDialog) {
            //pupilDetectionSettingsDialog->updateProcModeEnabled();
            pupilDetectionSettingsDialog->onSettingsChange();
        }
        // GB added end

        const QIcon trackOffIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/view-visible.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
        trackAct->setIcon(trackOffIcon);
        trackingOn = false;

        // GB added begin
        if(recordOn)
            onStreamClick();
        if(streamOn)
            onStreamClick();

        if(singleCameraSettingsDialog && !recordImagesOn)
            singleCameraSettingsDialog->setLimitationsWhileTracking(false);
        if(stereoCameraSettingsDialog && !recordImagesOn)
            stereoCameraSettingsDialog->setLimitationsWhileTracking(false);
        if(singleWebcamSettingsDialog && !recordImagesOn)
            singleWebcamSettingsDialog->setLimitationsWhileTracking(false);
        
        recordAct->setDisabled(true);
        //streamAct->setDisabled(true);
        // GB added end
    } else {
        // Activate tracking

        // Even though its already set in the selection of the camera, camera calibration may changed till now so we need to load it again
        // TODO better way of doing this without setting the camera two times (or loading the config at selection differently)
        pupilDetectionWorker->setCamera(selectedCamera);

        // GB added begin
        // GB TODO: ?does fileCamera have a default procMode set?
        if(singleCameraChildWidget)
            singleCameraChildWidget->updateForPupilDetectionProcMode();
        if(stereoCameraChildWidget)
            stereoCameraChildWidget->updateForPupilDetectionProcMode();

        if(singleCameraSettingsDialog)
            singleCameraSettingsDialog->setLimitationsWhileTracking(true);
        if(stereoCameraSettingsDialog)
            stereoCameraSettingsDialog->setLimitationsWhileTracking(true);
        if(singleWebcamSettingsDialog)
            singleWebcamSettingsDialog->setLimitationsWhileTracking(true);
        // GB added end

        // GB: moved from here
        //pupilDetectionWorker->startDetection();

        // GB added begin
        if(pupilDetectionSettingsDialog) {
            //pupilDetectionSettingsDialog->updateProcModeEnabled();
            pupilDetectionSettingsDialog->onSettingsChange();
        }
    
        int val = pupilDetectionWorker->getCurrentProcMode();
        // BREAKPOINT
        // GB: this needs to happen, because if we just open a camera, and start tracking, no ROI has been set for pupilDetection before
        if(val == ProcMode::SINGLE_IMAGE_ONE_PUPIL) {
            QRectF roi1D = applicationSettings->value("SingleCameraView.ROIsingleImageOnePupil.discrete", QRectF()).toRectF();
            if(!roi1D.isEmpty()){
                QRectF initRoi = selectedCamera->getImageROI();
                QRectF roi1R = applicationSettings->value("SingleCameraView.ROIsingleImageOnePupil.rational", QRectF()).toRectF();
                pupilDetectionWorker->setROIsingleImageOnePupil(SupportFunctions::calculateRoiD(initRoi, roi1D, roi1R));
                }
        } else if(val == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
            QRectF roiA = applicationSettings->value("SingleCameraView.ROIsingleImageTwoPupilA.discrete", QRectF()).toRectF();
            QRectF roiB = applicationSettings->value("SingleCameraView.ROIsingleImageTwoPupilB.discrete", QRectF()).toRectF();
            if(!roiA.isEmpty())
                pupilDetectionWorker->setROIsingleImageTwoPupilA(roiA);
            if(!roiB.isEmpty())
                pupilDetectionWorker->setROIsingleImageTwoPupilB(roiB);
        } else if(val == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
            QRectF roiMain1 = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil1.discrete", QRectF()).toRectF();
            QRectF roiSecondary1 = applicationSettings->value("StereoCameraView.ROIstereoImageOnePupil2.discrete", QRectF()).toRectF();
            if(!roiMain1.isEmpty())
                pupilDetectionWorker->setROIstereoImageOnePupil1(roiMain1);
            if(!roiSecondary1.isEmpty())
                pupilDetectionWorker->setROIstereoImageOnePupil2(roiSecondary1);
        } else if(val == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
            QRectF roiMain1 = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilA1.discrete", QRectF()).toRectF();
            QRectF roiMain2 = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB1.discrete", QRectF()).toRectF();
            QRectF roiSecondary1 = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilA2.discrete", QRectF()).toRectF();
            QRectF roiSecondary2 = applicationSettings->value("StereoCameraView.ROIstereoImageTwoPupilB2.discrete", QRectF()).toRectF();
            if(!roiMain1.isEmpty())
                pupilDetectionWorker->setROIstereoImageTwoPupilA1(roiMain1);
            if(!roiMain2.isEmpty())
                pupilDetectionWorker->setROIstereoImageTwoPupilB1(roiMain2);
            if(!roiSecondary1.isEmpty())
                pupilDetectionWorker->setROIstereoImageTwoPupilA2(roiSecondary1);
            if(!roiSecondary2.isEmpty())
                pupilDetectionWorker->setROIstereoImageTwoPupilB2(roiSecondary2);
        // } else if(val == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
        //     QRectF roi1 = applicationSettings->value("SingleCameraView.ROImirrImageOnePupil1.discrete", QRectF()).toRectF();
        //     QRectF roi2 = applicationSettings->value("SingleCameraView.ROImirrImageOnePupil2.discrete", QRectF()).toRectF();
        //     if(!roi1.isEmpty())
        //         pupilDetectionWorker->setROImirrImageOnePupil1(roi1);
        //     if(!roi2.isEmpty())
        //         pupilDetectionWorker->setROImirrImageOnePupil2(roi2);
        }
        // BG: NOTE: This needs to be called AFTER all pupil detection ROIs are loaded and set in the current
        // pupilDetection instance, otherwise autoParam will not be done
        pupilDetectionWorker->startDetection();
        // GB added end

        const QIcon trackOnIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/redeyes.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
        trackAct->setIcon(trackOnIcon);
        trackingOn = true;

        if(!pupilDetectionDataFile.isEmpty())
            recordAct->setDisabled(false); // GB added here
    }
}

void MainWindow::onStreamingSettingsClick() {
    if(streamingSettingsDialog)
        streamingSettingsDialog->show();
}

void MainWindow::onStreamClick() {

    if(dataStreamer) { // if streaming is on, deactivate streaming

        disconnect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, int, std::vector<Pupil>, QString)), dataStreamer, SLOT (newPupilData(quint64, int, std::vector<Pupil>, QString)));

        streamingSettingsDialog->setLimitationsWhileStreamingUDP(false);
        streamingSettingsDialog->setLimitationsWhileStreamingCOM(false);

        dataStreamer->close(); // TODO check if may terminate writing to early? because of the lag of the event queue in pupildetection
        dataStreamer->deleteLater();
        // GB: this way we can safely check like if(var!=nullptr) or if(var)
        dataStreamer = nullptr;

        const QIcon streamIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/media-record-green.svg"), applicationSettings);
        streamAct->setIcon(streamIcon);
        streamOn = false;

    } else { // Activate streaming

        if(!streamingSettingsDialog->isAnyConnected())
            return;

        safelyResetTrialCounter();

        dataStreamer = new DataStreamer(
            connPoolCOM,
            recEventTracker,
            this
            );
        
        if(streamingSettingsDialog->isUDPConnected()) {
            dataStreamer->startUDPStreamer(streamingSettingsDialog->getUDPsocket(), streamingSettingsDialog->getUDPip(), streamingSettingsDialog->getUDPport(), streamingSettingsDialog->getDataContainerUDP());
            streamingSettingsDialog->setLimitationsWhileStreamingUDP(true);
        } if(streamingSettingsDialog->isCOMConnected()) {
            dataStreamer->startCOMStreamer(streamingSettingsDialog->getConnPoolCOMIndex(), streamingSettingsDialog->getDataContainerCOM());
            streamingSettingsDialog->setLimitationsWhileStreamingCOM(true);
        }
//        streamingSettingsDialog->setLimitationsWhileStreaming(true);

        connect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, int, std::vector<Pupil>, QString)), dataStreamer, SLOT (newPupilData(quint64, int, std::vector<Pupil>, QString)));

        const QIcon streamIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/kt-stop-all.svg"), applicationSettings);
        streamAct->setIcon(streamIcon);
        streamOn = true;
    }
}

void MainWindow::onRecordClick() {

    if(pupilDetectionDataFile.isEmpty())
        return;

    if(recordOn && dataWriter) {
        // Deactivate recording

        dataWriter->close(); // TODO check if may terminate writing to early? because of the lag of the event queue in pupildetection
        dataWriter->deleteLater();
        // GB added: this way we can safely check like if(var!=nullptr) or if(var)
        dataWriter = nullptr;

        if(generalSettingsDialog)
            generalSettingsDialog->setLimitationsWhileDataWriting(false);

        const QIcon recordOffIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-record.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
        recordAct->setIcon(recordOffIcon);
        recordOn = false;

    } else {
        // Activate recording

        if(generalSettingsDialog)
            generalSettingsDialog->setLimitationsWhileDataWriting(true);

        // TODO: this version is imperfect yet, as it permanently overwrites pupilDetectionDataFile name
        bool changedGiven = false; // unused yet
        pupilDetectionDataFile = SupportFunctions::prepareOutputFileForDataWriter(pupilDetectionDataFile, applicationSettings, changedGiven, this);

        dataWriter = 
            new DataWriter(
                pupilDetectionDataFile, 
                (ProcMode)pupilDetectionWorker->getCurrentProcMode(),
                recEventTracker,
                this);
        if(!dataWriter->isReady()) { // in case we failed to open csv file for writing
            dataWriter->deleteLater();
            dataWriter = nullptr;
            return;
        }

        safelyResetTrialCounter();
        
        QFileInfo fi(pupilDetectionDataFile);
        QDir pupilDetectionDir = fi.dir();
        QString metadataFileName = fi.baseName() + QString::fromStdString("_datarec_meta.xml");

        int currentProcMode = pupilDetectionWorker->getCurrentProcMode();

        if( (applicationSettings->value("metaSnapshotsEnabled", "1") == "1" || 
            applicationSettings->value("metaSnapshotsEnabled", "1") == "true" ))
            MetaSnapshotOrganizer::writeMetaSnapshot(
                pupilDetectionDir.filePath(metadataFileName),
                selectedCamera, imageWriter, pupilDetectionWorker, dataWriter, MetaSnapshotOrganizer::Purpose::DATA_REC, applicationSettings);
        
        // GB new kind of signals
        connect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, int, std::vector<Pupil>, QString)), dataWriter, SLOT (newPupilData(quint64, int, std::vector<Pupil>, QString)));

        const QIcon recordOnIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/kt-stop-all.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
        recordAct->setIcon(recordOnIcon);
        recordOn = true;
    }
}

void MainWindow::onRecordImageClick() {

    if(recordImagesOn) {
        // Deactivate recording

        // disconnect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), imageWriter, SLOT (onNewImage(CameraImage)));
        disconnect(signalPubSubHandler, SIGNAL(onNewGrabResult(CameraImage)), imageWriter, SLOT (onNewImage(CameraImage)));

        const QIcon recordOffIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-record-blue.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
        recordImagesAct->setIcon(recordOffIcon);
        recordImagesOn = false;

        if (imageWriter != nullptr){
            imageWriter->deleteLater();
            imageWriter = nullptr;
        }


        // GB added begin
        if( applicationSettings->value("saveOfflineEventLog", "1") == "1" || 
            applicationSettings->value("saveOfflineEventLog", "1") == "true" ) {
            recEventTracker->saveOfflineEventLog(
                imageRecStartTimestamp,
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
                outputDirectory + "/" + QString::fromStdString("offline_event_log.xml") );
                //outputDirectory + "/" + QString::fromStdString("offline_event_log.csv") );
        }
        
        if(singleCameraSettingsDialog && !trackingOn)
            singleCameraSettingsDialog->setLimitationsWhileTracking(false);
        if(stereoCameraSettingsDialog && !trackingOn)
            stereoCameraSettingsDialog->setLimitationsWhileTracking(false);
        if(singleWebcamSettingsDialog && !trackingOn)
            singleWebcamSettingsDialog->setLimitationsWhileTracking(false);

        if(generalSettingsDialog)
            generalSettingsDialog->setLimitationsWhileImageWriting(false);
        // GB added end
    } else {
        // Activate recording
        
        // GB added begin
        safelyResetTrialCounter();
        imageRecStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        if(outputDirectory.isEmpty())
            return;
        
        if(singleCameraSettingsDialog)
            singleCameraSettingsDialog->setLimitationsWhileTracking(true);
        if(stereoCameraSettingsDialog)
            stereoCameraSettingsDialog->setLimitationsWhileTracking(true);
        if(singleWebcamSettingsDialog)
            singleWebcamSettingsDialog->setLimitationsWhileTracking(true);

        if(generalSettingsDialog)
            generalSettingsDialog->setLimitationsWhileImageWriting(true);

        bool stereo = selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE;

        // TODO: why use string everywhere for directory? Use QDir instead, or clarify naming ("directory" variables should all be QString or QDir type)
        // TODO: this version is imperfect yet, as it permanently overwrites outputDirectory (image output directory) name
        bool changedGiven = false; // unused yet
        outputDirectory = SupportFunctions::prepareOutputDirForImageWriter(outputDirectory, applicationSettings, changedGiven, this);

        imageWriter = new ImageWriter(outputDirectory, stereo, this);

        // this should come here as the "directory already exists" dialog is only answered before, upon creation of imageWriter, and meta snapshot creation relies on that response
        if( applicationSettings->value("metaSnapshotsEnabled", "1") == "1" ||
            applicationSettings->value("metaSnapshotsEnabled", "1") == "true" ) {

            MetaSnapshotOrganizer::writeMetaSnapshot(
                    outputDirectory + "/" + QString::fromStdString("imagerec_meta.xml"),
                    selectedCamera, imageWriter, pupilDetectionWorker, dataWriter, MetaSnapshotOrganizer::Purpose::IMAGE_REC, applicationSettings);
        }
        // GB: maybe write unix timestamp too in the name of meta snapshot file?
        // GB added end

        // connect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), signalPubSubHandler, SLOT (onNewImage(CameraImage)));
        connect(signalPubSubHandler, SIGNAL(onNewGrabResult(CameraImage)), imageWriter, SLOT (onNewImage(CameraImage)));

        const QIcon recordOnIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/kt-stop-all.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
        recordImagesAct->setIcon(recordOnIcon);
        recordImagesOn = true;
    }
}

void MainWindow::onCameraClick() {
    // fix to open submenu in the camera menu
    cameraAct->menu()->exec(QCursor::pos());
}

void MainWindow::onCameraDisconnectClick() {

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();

    for(auto mdiSubWindow : windows) {
        mdiSubWindow->close();
        mdiSubWindow->deleteLater();
    }

    // GB added begin
    if(imagePlaybackControlDialog) {
//        disconnect(selectedCamera, SIGNAL(finished()), imagePlaybackControlDialog, SLOT(onPlaybackFinished()));
        //disconnect(selectedCamera, SIGNAL(endReached()), imagePlaybackControlDialog, SLOT(onAutomaticFinish()));
        disconnect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyStarted()), this, SLOT(onPlaybackSafelyStarted()));
        disconnect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyPaused()), this, SLOT(onPlaybackSafelyPaused()));
        disconnect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyStopped()), this, SLOT(onPlaybackSafelyStopped()));
        imagePlaybackControlDialog = nullptr;
    }
    if(camTempMonitor) {
        if(recEventTracker)
            disconnect(camTempMonitor, SIGNAL(camTempChecked(std::vector<double>)), recEventTracker, SLOT(addTemperatureCheck(std::vector<double>)));
        
        camTempMonitor->setRunning(false);
        //camTempMonitor->thread()->deleteLater();
        camTempMonitor->deleteLater();
        camTempMonitor = nullptr; 
    }

    if (cameraViewWindow != nullptr) {
        cameraViewWindow->deleteLater();
        cameraViewWindow = nullptr;

    }

    if (singleCameraChildWidget != nullptr) {
        singleCameraChildWidget->deleteLater();
        singleCameraChildWidget = nullptr;
    }
    if (stereoCameraChildWidget != nullptr) {
        stereoCameraChildWidget->deleteLater();
        stereoCameraChildWidget = nullptr;
    }
    if(recEventTracker) {
        disconnect(this, SIGNAL(commitTrialCounterIncrement(quint64)), recEventTracker, SLOT(addTrialIncrement(quint64)));
        disconnect(this, SIGNAL(commitTrialCounterReset(quint64)), recEventTracker, SLOT(resetBufferTrialCounter(quint64)));
        disconnect(this, SIGNAL(commitRemoteMessage(quint64, QString)), recEventTracker, SLOT(addMessage(quint64, QString)));

        recEventTracker->close();
        recEventTracker->deleteLater();
        //disconnect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), recEventTracker, SLOT(updateGrabTimestamp(CameraImage)));
        recEventTracker = nullptr;
    }
    // GB added/modified end
    if (calibrationWindow  != nullptr){
        calibrationWindow->deleteLater();
        calibrationWindow = nullptr;
    }


    

    // NOTE: valamiért itt volt eredetileg a kódban, de nem jó így, hiszen később kell a trackinget abbahagyni, mint a csv és image recordingot abbahagyni
    /*
    if(trackingOn) {
        trackAct->setChecked(false);
        onTrackActClick();
    }
    */

    if(streamOn) {
        onStreamClick();
    }

    if(recordOn) {
        onRecordClick();
    }

    if(recordImagesOn) {
        onRecordImageClick();
    }

    // NOTE: ide tettem át
    if(trackingOn) {
        trackAct->setChecked(false);
        onTrackActClick();
    }

    if(selectedCamera  != nullptr) {
        // only if not "file camera":
        if(singleCameraSettingsDialog && (selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA ||
            selectedCamera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM) ) {
            //onSingleCameraSettingsClick();
            singleCameraSettingsDialog->accept();
        } else if(stereoCameraSettingsDialog && selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA) {
            //stereoCameraSelected();
            stereoCameraSettingsDialog->accept();
        } else if(singleWebcamSettingsDialog && selectedCamera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM) {
            singleWebcamSettingsDialog->accept();
        }

        // for all occasions:
        selectedCamera->close();
        // GB added: this way we can safely check like if(var!=nullptr) or if(var)
        selectedCamera->deleteLater();
        selectedCamera = nullptr;
        // GB added end
    }

    if (playbackSynchroniser != nullptr){
        playbackSynchroniser->deleteLater();
        playbackSynchroniser = nullptr;
    }

    // GB: take care of fileOpenAct
    fileOpenAct->setEnabled(true);

    if (selectedCamera && signalPubSubHandler) {
        disconnect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), signalPubSubHandler,
                   SIGNAL(onNewGrabResult(CameraImage)));
        disconnect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
        disconnect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

        disconnect(selectedCamera, SIGNAL (imagesSkipped()), this, SLOT (onImagesSkipped()));
        disconnect(selectedCamera, SIGNAL (cameraDeviceRemoved()), this, SLOT (onCameraUnexpectedlyDisconnected()));
    }

    // GB begin
    pupilDetectionSettingsDialog->onSettingsChange();
    if (pupilDetectionSettingsDialog && (singleCameraChildWidget || stereoCameraChildWidget)) {
        disconnect(pupilDetectionSettingsDialog, SIGNAL(pupilDetectionProcModeChanged(int)), singleCameraChildWidget,
                   SLOT(updateForPupilDetectionProcMode()));
        disconnect(pupilDetectionSettingsDialog, SIGNAL(pupilDetectionProcModeChanged(int)), stereoCameraChildWidget,
                   SLOT(updateForPupilDetectionProcMode()));
    }
    // GB end

    if(hwTriggerOn) {
        serialSettingsDialog->sendCommand(QString("<SX>"));
        onHwTriggerDisable();
    }

    if(serialSettingsDialog->isConnected()) {
        serialSettingsDialog->disconnectSerialPort();
    }
    if(serialSettingsDialog->isVisible()) {
        serialSettingsDialog->close();
    }
    if(pupilDetectionSettingsDialog->isVisible()) {
        pupilDetectionSettingsDialog->close();
    }
    if(subjectSelectionDialog->isVisible()) {
        subjectSelectionDialog->close();
    }

    onCameraCalibrationDisabled();

    subjectConfigurationLabel->setText("");
    currentDirectoryLabel->setText("");
    currentDirectoryLabel->setToolTip("");


    resetStatus(false);
    // GB added end

    
}

void MainWindow::singleCameraSelected(QAction *action) {

    QString deviceFullname = action->data().toString();

    // BASLER SINGLE CAMERA
    try {
        selectedCamera = new SingleCamera(deviceFullname.toStdString().c_str(), this);
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;

        QMessageBox err(this);
        err.critical(this, "Device Error", e.GetDescription());

        return;
    }

    // GB added
    //safelyResetTrialCounter();
    // GB added end

    if(dynamic_cast<SingleCamera*>(selectedCamera)->getCameraCalibration()->isCalibrated())
        onCameraCalibrationEnabled();

    connect(selectedCamera, SIGNAL (imagesSkipped()), this, SLOT (onImagesSkipped()));
    connect(selectedCamera, SIGNAL (cameraDeviceRemoved()), this, SLOT (onCameraUnexpectedlyDisconnected()));

    connect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL (onNewGrabResult(CameraImage)));
    connect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
    connect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

    connect(dynamic_cast<SingleCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
    connect(dynamic_cast<SingleCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled())); 

    cameraViewClick();
    onSingleCameraSettingsClick();

    cameraSettingsAct->setDisabled(false);

    //pupilDetectionSettingsDialog->onSettingsChange(); // must come in this order, to set proc mode first
    pupilDetectionWorker->setCamera(selectedCamera);
    pupilDetectionSettingsDialog->onSettingsChange();

    // GB added begin
    recEventTracker = new RecEventTracker();
    //connect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), recEventTracker, SLOT(updateGrabTimestamp(CameraImage)));
    connect(this, SIGNAL(commitTrialCounterIncrement(quint64)), recEventTracker, SLOT(addTrialIncrement(quint64)));
    connect(this, SIGNAL(commitTrialCounterReset(quint64)), recEventTracker, SLOT(resetBufferTrialCounter(quint64)));
    connect(this, SIGNAL(commitRemoteMessage(quint64, QString)), recEventTracker, SLOT(addMessage(quint64, QString)));
    safelyResetTrialCounter();
    
    QThread *tempMonitorThread = new QThread();
    camTempMonitor = new CamTempMonitor(dynamic_cast<SingleCamera*>(selectedCamera));
    connect(tempMonitorThread, &QThread::started, camTempMonitor, &CamTempMonitor::run);
    camTempMonitor->moveToThread(tempMonitorThread);    
    //connect(tempMonitorThread, SIGNAL (finished()), tempMonitorThread, SLOT (deleteLater()));
    tempMonitorThread->start();
    tempMonitorThread->setPriority(QThread::LowPriority);
    connect(camTempMonitor, SIGNAL(camTempChecked(std::vector<double>)), recEventTracker, SLOT(addTemperatureCheck(std::vector<double>)));
    

    connect(pupilDetectionSettingsDialog, SIGNAL (pupilDetectionProcModeChanged(int)), singleCameraChildWidget, SLOT (updateForPupilDetectionProcMode()));
    // GB added end
    resetStatus(true);
    // GB added end

}



void MainWindow::singleWebcamSelected() {

    int deviceID = webcamDeviceBox->value();

    

    try {
        selectedCamera = new SingleWebcam(deviceID, "Webcam", this);
    } catch (const QException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.what() << std::endl;

        QMessageBox err(this);
        err.critical(this, "Device Error", e.what());

        return;
    }

    //safelyResetTrialCounter();

    if(dynamic_cast<SingleWebcam*>(selectedCamera)->getCameraCalibration()->isCalibrated())
        onCameraCalibrationEnabled();

    // TODO: handle unexpected device removal
    connect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL (onNewGrabResult(CameraImage)));
    connect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
    connect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

    connect(dynamic_cast<SingleWebcam*>(selectedCamera)->getCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
    connect(dynamic_cast<SingleWebcam*>(selectedCamera)->getCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));        

    cameraViewClick();
    onSingleWebcamSettingsClick();


    pupilDetectionWorker->setCamera(selectedCamera);
    pupilDetectionSettingsDialog->onSettingsChange();

    // GB added begin
    recEventTracker = new RecEventTracker();
    //connect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), recEventTracker, SLOT(updateGrabTimestamp(CameraImage)));
    connect(this, SIGNAL(commitTrialCounterIncrement(quint64)), recEventTracker, SLOT(addTrialIncrement(quint64)));
    connect(this, SIGNAL(commitTrialCounterReset(quint64)), recEventTracker, SLOT(resetBufferTrialCounter(quint64)));
    connect(this, SIGNAL(commitRemoteMessage(quint64, QString)), recEventTracker, SLOT(addMessage(quint64, QString)));
    safelyResetTrialCounter();
    // GB added end

    connect(pupilDetectionSettingsDialog, SIGNAL (pupilDetectionProcModeChanged(int)), singleCameraChildWidget, SLOT (updateForPupilDetectionProcMode()));

    resetStatus(true);
    // GB added end
}

void MainWindow::stereoCameraSelected() {

    try {
        selectedCamera = new StereoCamera(this);
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;

        QMessageBox err(this);
        err.critical(this, "Device Error", e.GetDescription());

        return;
    }

    // GB added
    //safelyResetTrialCounter();
    // GB added end

    stereoCameraSettingsDialog = new StereoCameraSettingsDialog(dynamic_cast<StereoCamera*>(selectedCamera), serialSettingsDialog, this);
    stereoCameraSettingsDialog->setWindowIcon(cameraSettingsIcon1);
    stereoCameraSettingsDialog->installEventFilter(this);
    //auto *child = new RestorableQMdiSubWindow(childWidget, "StereoCameraSettingsDialog", this);
    stereoCameraSettingsDialog->show();

    connect(selectedCamera, SIGNAL (imagesSkipped()), this, SLOT (onImagesSkipped()));
    connect(selectedCamera, SIGNAL (cameraDeviceRemoved()), this, SLOT (onCameraUnexpectedlyDisconnected()));

    connect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL(onNewGrabResult(CameraImage)));
    connect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
    connect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

    connect(stereoCameraSettingsDialog, &StereoCameraSettingsDialog::onSerialConfig, serialSettingsDialog, &SerialSettingsDialog::show);
    connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), stereoCameraSettingsDialog, SLOT (onSettingsChange()));

    connect(serialSettingsDialog, SIGNAL (onConnect()), stereoCameraSettingsDialog, SLOT (onSerialConnect()));
    connect(serialSettingsDialog, SIGNAL (onDisconnect()), stereoCameraSettingsDialog, SLOT (onSerialDisconnect()));

    connect(stereoCameraSettingsDialog, SIGNAL (onHardwareTriggerStart(QString)), serialSettingsDialog, SLOT (sendCommand(QString)));
    connect(stereoCameraSettingsDialog, SIGNAL (onHardwareTriggerStop(QString)), serialSettingsDialog, SLOT (sendCommand(QString)));

    connect(stereoCameraSettingsDialog, SIGNAL (onHardwareTriggerEnable()), this, SLOT (onHwTriggerEnable()));
    connect(stereoCameraSettingsDialog, SIGNAL (onHardwareTriggerDisable()), this, SLOT (onHwTriggerDisable()));

    connect(dynamic_cast<StereoCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
    connect(dynamic_cast<StereoCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));


    cameraViewClick();

    pupilDetectionWorker->setCamera(selectedCamera);
    pupilDetectionSettingsDialog->onSettingsChange();

    recEventTracker = new RecEventTracker();
    //connect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), recEventTracker, SLOT(updateGrabTimestamp(CameraImage)));
    connect(this, SIGNAL(commitTrialCounterIncrement(quint64)), recEventTracker, SLOT(addTrialIncrement(quint64)));
    connect(this, SIGNAL(commitTrialCounterReset(quint64)), recEventTracker, SLOT(resetBufferTrialCounter(quint64)));
    connect(this, SIGNAL(commitRemoteMessage(quint64, QString)), recEventTracker, SLOT(addMessage(quint64, QString)));
    safelyResetTrialCounter();
    
    QThread *tempMonitorThread = new QThread();
    camTempMonitor = new CamTempMonitor(dynamic_cast<StereoCamera*>(selectedCamera));
    connect(tempMonitorThread, &QThread::started, camTempMonitor, &CamTempMonitor::run);
    camTempMonitor->moveToThread(tempMonitorThread);    
    //connect(tempMonitorThread, SIGNAL (finished()), tempMonitorThread, SLOT (deleteLater()));
    tempMonitorThread->start();
    tempMonitorThread->setPriority(QThread::LowPriority);
    connect(camTempMonitor, SIGNAL(camTempChecked(std::vector<double>)), recEventTracker, SLOT(addTemperatureCheck(std::vector<double>)));

    connect(pupilDetectionSettingsDialog, SIGNAL (pupilDetectionProcModeChanged(int)), stereoCameraChildWidget, SLOT (updateForPupilDetectionProcMode()));

    connect(stereoCameraSettingsDialog, SIGNAL(onImageROIChanged(QRect)), stereoCameraChildWidget, SLOT(onImageROIChanged(QRect)));
    connect(stereoCameraSettingsDialog, SIGNAL(onSensorSizeChanged(QSize)), stereoCameraChildWidget, SLOT(onSensorSizeChanged(QSize)));

    // call these once again, to evoke a signal that will tell the camera view window where the image ROI is, upon window creation
    stereoCameraSettingsDialog->updateImageROISettingsValues();
    stereoCameraSettingsDialog->updateCamImageRegionsWidget();
    stereoCameraSettingsDialog->updateSensorSize();

    resetStatus(true);
}

void MainWindow::cameraViewClick() {

    if(cameraViewWindow && cameraViewWindow->isVisible()) {
        cameraViewWindow->close();
        // GB added begin
        singleCameraChildWidget->deleteLater();
        singleCameraChildWidget = nullptr;
        stereoCameraChildWidget->deleteLater();
        stereoCameraChildWidget = nullptr;
        // GB added end
        cameraViewWindow->deleteLater();
    }

    if(selectedCamera && (
        selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA || 
        selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE ||
        selectedCamera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM
        ) ) {
        //SingleCameraView *childWidget = new SingleCameraView(selectedCamera, pupilDetectionWorker, this);
        singleCameraChildWidget = new SingleCameraView(selectedCamera, pupilDetectionWorker, !cameraPlaying, this); // changed by kheki4 on 2022.10.24, NOTE: to be able to pass singlecameraview instance pointer to sindglecamerasettingsdialog constructor
        connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), singleCameraChildWidget, SLOT (onSettingsChange()));

        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(singleCameraChildWidget, "SingleCameraView", this);
        //SingleCameraView *child = new SingleCameraView(selectedCamera, pupilDetectionWorker, this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        cameraViewWindow = child;
        cameraViewWindow->setWindowIcon(singleCameraIcon);


    } else if(selectedCamera && (
        selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || 
        selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE
        ) ) {
        //StereoCameraView *childWidget = new StereoCameraView(selectedCamera, pupilDetectionWorker, this); // GB modified: now it is in a global pointer
        stereoCameraChildWidget = new StereoCameraView(selectedCamera, pupilDetectionWorker, !cameraPlaying, this);
        connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), stereoCameraChildWidget, SLOT (onSettingsChange()));

        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(stereoCameraChildWidget, "StereoCameraView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        cameraViewWindow = child;
        cameraViewWindow->setWindowIcon(stereoCameraIcon);
    }
    connectCameraPlaybackChangedSlots();
}

void MainWindow::onCameraSettingsClick() {

    if(selectedCamera && singleCameraSettingsDialog && (
        selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) ) {
        //onSingleCameraSettingsClick();
        singleCameraSettingsDialog->show();
    } else if(selectedCamera && stereoCameraSettingsDialog && 
        selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA ) {
        //stereoCameraSelected();
        stereoCameraSettingsDialog->show();
    } else if(selectedCamera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM) {
        singleWebcamSettingsDialog->show();
    }
}

void MainWindow::onSingleCameraSettingsClick() {

    // GB NOTE: to be able to pass singlecameraview instance pointer to sindglecamerasettingsdialog constructor
    singleCameraSettingsDialog = new SingleCameraSettingsDialog(dynamic_cast<SingleCamera*>(selectedCamera), serialSettingsDialog, this);
    singleCameraSettingsDialog->setWindowIcon(cameraSettingsIcon2);
    singleCameraSettingsDialog->installEventFilter(this);
    //auto *child = new RestorableQMdiSubWindow(childWidget, "SingleCameraSettingsDialog", this);
    singleCameraSettingsDialog->show();

    connect(singleCameraSettingsDialog, &SingleCameraSettingsDialog::onSerialConfig, serialSettingsDialog, &SerialSettingsDialog::show);

    connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), singleCameraSettingsDialog, SLOT (onSettingsChange()));

    connect(serialSettingsDialog, SIGNAL (onConnect()), singleCameraSettingsDialog, SLOT (onSerialConnect()));
    connect(serialSettingsDialog, SIGNAL (onDisconnect()), singleCameraSettingsDialog, SLOT (onSerialDisconnect()));

    connect(singleCameraSettingsDialog, SIGNAL (onHardwareTriggerStart(QString)), serialSettingsDialog, SLOT (sendCommand(QString)));
    connect(singleCameraSettingsDialog, SIGNAL (onHardwareTriggerStop(QString)), serialSettingsDialog, SLOT (sendCommand(QString)));

    connect(singleCameraSettingsDialog, SIGNAL (onHardwareTriggerEnable()), this, SLOT (onHwTriggerEnable()));
    connect(singleCameraSettingsDialog, SIGNAL (onHardwareTriggerDisable()), this, SLOT (onHwTriggerDisable()));

    connect(singleCameraSettingsDialog, SIGNAL(onImageROIChanged(QRect)), singleCameraChildWidget, SLOT(onImageROIChanged(QRect)));
    connect(singleCameraSettingsDialog, SIGNAL(onSensorSizeChanged(QSize)), singleCameraChildWidget, SLOT(onSensorSizeChanged(QSize)));

    // call these once again, to evoke a signal that will tell the camera view window where the image ROI is, upon window creation
    singleCameraSettingsDialog->updateImageROISettingsValues();
    singleCameraSettingsDialog->updateCamImageRegionsWidget();
    singleCameraSettingsDialog->updateSensorSize();
}

Pylon::DeviceInfoList_t MainWindow::enumerateCameraDevices() {

    Pylon::CTlFactory& TlFactory = Pylon::CTlFactory::GetInstance();
    Pylon::DeviceInfoList_t lstDevices;

    TlFactory.EnumerateDevices(lstDevices);

    return lstDevices;
}

void MainWindow::onSingleWebcamSettingsClick() {
    singleWebcamSettingsDialog = new SingleWebcamSettingsDialog(dynamic_cast<SingleWebcam*>(selectedCamera), this); 
    //auto *child = new RestorableQMdiSubWindow(childWidget, "SingleWebcamSettingsDialog", this);
    singleWebcamSettingsDialog->show();

    connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), singleWebcamSettingsDialog, SLOT (onSettingsChange()));
}


MainWindow::~MainWindow() {
    pupilDetectionThread->quit();
    pupilDetectionThread->wait();
}

void MainWindow::onCalibrateClick() {
    // GB: this method caused crash, cannot set itself to null when closed, and thus cannot be checked properly for existence
    // GB: I still could not make it work properly, but yet I shortened the code
    // GB: problem is that, when the window is closed by someone using the X button, it closes itself, but does not set to nullptr
    // I think the same can happen to Sharpness window too

    // GB modified begin
    
    if(mdiArea->subWindowList().contains(calibrationWindow)) {
        calibrationWindow->show();
        calibrationWindow->raise();
        calibrationWindow->activateWindow();
        calibrationWindow->setFocus();
        if(calibrationWindow->isMinimized() || calibrationWindow->isShaded())
            calibrationWindow->showNormal();
    }
    else {
        loadCalibrationWindow();
    }
    // GB modified end
}

void MainWindow::onSharpnessClick() {

    if (mdiArea->subWindowList().contains(sharpnessWindow)){
        sharpnessWindow->show();
        sharpnessWindow->raise();
        sharpnessWindow->activateWindow();
        sharpnessWindow->setFocus();
        if(sharpnessWindow->isMinimized() || sharpnessWindow->isShaded())
            sharpnessWindow->showNormal();
    }
    else {
        loadSharpnessWindow();
    }

}

void MainWindow::dataTableClick() {

    // GB TODO: close if procMode has changed ?
    // GB NOTE: removed code that decides if mode is stereo or single, as not necessary anymore, due to procMode

    DataTable *childWidget = new DataTable(pupilDetectionWorker->getCurrentProcMode(), this);
    //RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(childWidget, "DataTable", this);
    RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(childWidget, "DataTable"+QString::number(pupilDetectionWorker->getCurrentProcMode()), this);
    /*QMdiSubWindow *child = new QMdiSubWindow(this);
    child->setWidget(childWidget);
    child->setAttribute(Qt::WA_DeleteOnClose);
    QSize hint = childWidget->sizeHint();
    child->setGeometry(QRect(QPoint(0, 0), hint));*/

    if(selectedCamera) {
        connect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, int, std::vector<Pupil>, QString)), childWidget, SLOT (onPupilData(quint64, int, std::vector<Pupil>, QString)));
        //std::cout << "dataTableClick()" << std::endl;
        
        connect(signalPubSubHandler, SIGNAL(cameraFPS(double)), childWidget, SLOT(onCameraFPS(double)));
        connect(signalPubSubHandler, SIGNAL(cameraFramecount(int)), childWidget, SLOT(onCameraFramecount(int)));
    }

    connect(pupilDetectionWorker, SIGNAL(fps(double)), childWidget, SLOT(onProcessingFPS(double)));
    connect(childWidget, SIGNAL(createGraphPlot(QString)), this, SLOT(onCreateGraphPlot(QString)));

    mdiArea->addSubWindow(child);
    child->show();
    child->restoreGeometry();
    connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
}

void MainWindow::onCreateGraphPlot(const QString &value) {

    std::cout<<"Created GraphPlot slot: " << value.toStdString()<<std::endl;

    QWidget *childWidget = new GraphPlot(value, pupilDetectionWorker->getCurrentProcMode(), false, this);
    auto *child = new RestorableQMdiSubWindow(childWidget, "GraphPlot_"+value, this);


    // switch values to connect different slots/signals to GraphPlot and the data
    if(value == DataTable::FRAME_NUMBER) {

        connect(signalPubSubHandler, SIGNAL (cameraFramecount(int)), childWidget, SLOT (appendData(int)));
    } else if(value == DataTable::CAMERA_FPS) {

        connect(signalPubSubHandler, SIGNAL (cameraFPS(double)), childWidget, SLOT (appendData(double)));
    } else if(value == DataTable::PUPIL_FPS) {

        connect(pupilDetectionWorker, SIGNAL (fps(double)), childWidget, SLOT (appendData(double)));
    } else {
        connect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, int, std::vector<Pupil>, QString)), childWidget, SLOT (appendData(quint64, int, std::vector<Pupil>, QString)));
        /*
        if(pupilDetectionWorker->isStereo()) {
            connect(pupilDetectionWorker, SIGNAL (processedStereoImageForOnePupilData(quint64, Pupil, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, Pupil, QString))); //REFACTORED
            connect(pupilDetectionWorker, SIGNAL (processedStereoImageForTwoPupilData(quint64, Pupil, Pupil, Pupil, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, Pupil, Pupil, Pupil, QString)));
        } else {
            connect(pupilDetectionWorker, SIGNAL (processedSingleImageForOnePupilData(quint64, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, QString))); // REFACTORED
            connect(pupilDetectionWorker, SIGNAL (processedSingleImageForTwoPupilData(quint64, Pupil, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, Pupil, QString)));
            connect(pupilDetectionWorker, SIGNAL (processedMirrImageForOnePupilData(quint64, Pupil, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, Pupil, QString)));
        }
        */
    }
    mdiArea->addSubWindow(child);
    child->show();
    child->restoreGeometry();
    connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
}

// GB: Repaired bug leading to crash when path was too short
void MainWindow::onOpenImageDirectory() {
    QFileDialog dialog(this, tr("Image Directory"), recentPath,tr("Image Files (*.png *.jpg *.bmp *.tiff *.jpeg *.webp)"));
    dialog.setOptions(QFileDialog::DontResolveSymlinks); // BG: tried QFileDialog::DontUseNativeDialog flag too but it is slow. TODO: make own dialog

    dialog.setFileMode(QFileDialog::Directory);

    if(!dialog.exec())
        return;
    QString tempDir = dialog.directory().absolutePath();
    // GB: negated check logic and removed indentation accordingly
    if(tempDir.isEmpty())
        return;

    QDir imageDir(tempDir);
    if (imageDir.isEmpty())
        return;

    QStringList nameFilter = QStringList() << "*.png" << "*.jpg" << "*.bmp" << "*.tiff" << "*.jpeg" <<  "*.webp";
    QStringList fileNames = imageDir.entryList(nameFilter, QDir::Files);
    QStringList folderNames = imageDir.entryList(QStringList() << "0" << "1", QDir::Dirs);
    if (fileNames.isEmpty() && folderNames.size() < 2)
        return;

    if (folderNames.size() == 2){
        QDir stereo0Dir(imageDir.filePath("0"));
        if (stereo0Dir.isEmpty() || stereo0Dir.entryList(nameFilter, QDir::Files).isEmpty())
            return;
        QDir stereo1Dir(imageDir.filePath("1"));
        if (stereo1Dir.isEmpty() || stereo1Dir.entryList(nameFilter, QDir::Files).isEmpty())
            return;
    }
    openImageDirectory(tempDir);
}

void MainWindow::openImageDirectory(QString imageDirectory) {

    if(imageDirectory[imageDirectory.length()-1]=='/')
        imageDirectory.chop(1);

    // GB added begin
    // GB: added to disable/enable opening of a new directory when there is already one opened
    fileOpenAct->setEnabled(false);
    // GB added end

    recentPath = imageDirectory;
    /*
    QStringList lst = recentPath.split('/');
    currentDirectoryLabel->setText("Current directory: .../" + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    currentDirectoryLabel->setToolTip(recentPath);
    */
    std::cout << recentPath.toStdString() << std::endl;
    QStringList lst = recentPath.split('/');
    if(lst.count() > 3)
        currentDirectoryLabel->setText("Current directory: .../"  + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    else if(lst.count() == 3)
        currentDirectoryLabel->setText("Current directory: " + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    else if(lst.count() == 2)
        currentDirectoryLabel->setText("Current directory: " + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
    else if(lst.count() == 1)
        currentDirectoryLabel->setText("Current directory: " + lst[lst.count()-1]);
    currentDirectoryLabel->setToolTip(recentPath);

    if(lst.count() > 1) {
        QString suggestedCSVLoc = imageDirectory.chopped(lst[lst.count()-1].length());
        PRGsetCsvPathAndName(suggestedCSVLoc + '/' + lst[lst.count()-1] + ".csv");
    }

    if(selectedCamera) {
        selectedCamera->close();
        // GB added: this way we can safely check like if(var!=nullptr) or if(var)
        selectedCamera = nullptr;
        // GB added end
    }
    onCameraCalibrationDisabled();
    resetStatus(true);
    // GB added end


    // GB modified begin
    //const int playbackSpeed = applicationSettings->value("playbackSpeed", generalSettingsDialog->getPlaybackSpeed()).toInt();
    //const bool playbackLoop = (bool) applicationSettings->value("playbackLoop", (int) generalSettingsDialog->getPlaybackLoop()).toInt();
    const int playbackSpeed = applicationSettings->value("playbackSpeed", 30).toInt();
    bool playbackLoop = false;
    if( applicationSettings->value("playbackLoop", "1") == "1" ||
        applicationSettings->value("playbackLoop", "1") == "true" )
        playbackLoop = true;
    // GB modified end

    // create simulated FileCamera

    // GB added begin
    QString offlineEventLogFileName = imageDirectory + '/' + "offline_event_log.xml";
    std::cout << "expected offlineEventLogFileName = " << offlineEventLogFileName.toStdString() << std::endl;
    if(QFileInfo(offlineEventLogFileName).exists() == true) {
        recEventTracker = new RecEventTracker(offlineEventLogFileName);
        if(recEventTracker->isReady()) {
            //connect( ...
        } else {
            recEventTracker->deleteLater();
            recEventTracker = nullptr;
        }
    }
    safelyResetTrialCounter();

    selectedCamera = new FileCamera(imageDirectory, imageMutex, imagePublished, imageProcessed, playbackSpeed, playbackLoop, this);
    std::cout<<"FileCamera created using playbackspeed [fps]: "<<playbackSpeed <<std::endl;

    connect(selectedCamera, SIGNAL(onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL (onNewGrabResult(CameraImage)));
    connect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
    connect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

    if(selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
        connect(dynamic_cast<FileCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
        connect(dynamic_cast<FileCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));

        int pmSingle = applicationSettings->value("PupilDetectionSettingsDialog.singleCam.procMode", ProcMode::SINGLE_IMAGE_ONE_PUPIL).toInt();
        if( pmSingle != ProcMode::SINGLE_IMAGE_ONE_PUPIL ||
            pmSingle != ProcMode::SINGLE_IMAGE_TWO_PUPIL // ||
            // pmSingle != ProcMode::MIRR_IMAGE_ONE_PUPIL
                )

            pmSingle = ProcMode::SINGLE_IMAGE_ONE_PUPIL;
        pupilDetectionWorker->setCurrentProcMode(pmSingle);
        // this line below is to ensure if an erroneous value was found in the QSettings ini, a good one gets in place
        applicationSettings->setValue("PupilDetectionSettingsDialog.singleCam.procMode", pmSingle);

    } else if(selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
        connect(dynamic_cast<FileCamera*>(selectedCamera)->getStereoCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
        connect(dynamic_cast<FileCamera*>(selectedCamera)->getStereoCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));

        int pmStereo = applicationSettings->value("PupilDetectionSettingsDialog.stereoCam.procMode", ProcMode::STEREO_IMAGE_ONE_PUPIL).toInt();
        if( pmStereo != ProcMode::STEREO_IMAGE_ONE_PUPIL ||
            pmStereo != ProcMode::STEREO_IMAGE_TWO_PUPIL )
            pmStereo = ProcMode::STEREO_IMAGE_ONE_PUPIL;
        pupilDetectionWorker->setCurrentProcMode(pmStereo);
        // this line below is to ensure if an erroneous value was found in the QSettings ini, a good one gets in place
        applicationSettings->setValue("PupilDetectionSettingsDialog.stereoCam.procMode", pmStereo);
    }
    this->cameraPlaying = false;
    cameraViewClick(); // GB: moved here. Had to ensure that proc mode is correctly set before creating camera view (as not it relies on pupilDetection instance too)
    onCalibrateClick();

    // Basically only that pupilDetectionSettingsDialog knows which type of camera is connected
    pupilDetectionWorker->setCamera(selectedCamera);
    // GB NOTE:
    // this line below calls loadSettings too
    // NOTE: importantly, this call must lead to calls in pupilDetectionSettingsDialog for
    // updateProcModeEnabled() and updateProcModeCompatibility()
    pupilDetectionSettingsDialog->onSettingsChange();

    // GB NOTE:
    // This must happen here, after cameraViewClick() call, because only then will a
    // singleCameraChildWidget or stereoCameraChildWidget exist in memory
    if(selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
        connect(pupilDetectionSettingsDialog, SIGNAL (pupilDetectionProcModeChanged(int)), singleCameraChildWidget, SLOT (updateForPupilDetectionProcMode()));
    } else if(selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
        connect(pupilDetectionSettingsDialog, SIGNAL (pupilDetectionProcModeChanged(int)), stereoCameraChildWidget, SLOT (updateForPupilDetectionProcMode()));
    }


    /*
    // Dirty fix to display the first frame preview in the camera view window before pushing play (i.e. for setting ROI etc.)
    // Problem is that Filecamera only sends signals after play click/start
    dynamic_cast<FileCamera*>(selectedCamera)->start();
    QThread::msleep(5);
    dynamic_cast<FileCamera*>(selectedCamera)->stop();
    // GB NOTE: does not work if play fps is set to longer intervals than 5ms. Possible solution below:
    */
    //QStringList images = QDir(imageDirectory).entryList(QStringList() << "*.jpg" << "*.JPG",QDir::Files);

    if(selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE && singleCameraChildWidget) {
        //cv::Mat temp1 = dynamic_cast<FileCamera*>(selectedCamera)->getStillImageSingle(0);
        //singleCameraChildWidget->displayStillImage(temp1);
        singleCameraChildWidget->displayFileCameraFrame(0);
    } else if(selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE && stereoCameraChildWidget) {
        //std::vector<cv::Mat> temp2 = dynamic_cast<FileCamera*>(selectedCamera)->getStillImageStereo(0);
        //stereoCameraChildWidget->displayStillImage(temp2);
        stereoCameraChildWidget->displayFileCameraFrame(0);
    }

    imagePlaybackControlDialog = new ImagePlaybackControlDialog(dynamic_cast<FileCamera*>(selectedCamera), pupilDetectionWorker, recEventTracker, this);
    RestorableQMdiSubWindow *imagePlaybackControlWindow = new RestorableQMdiSubWindow(imagePlaybackControlDialog, "ImagePlaybackControlDialog", this);
    imagePlaybackControlWindow->setWindowIcon(imagePlaybackControlIcon);
    mdiArea->addSubWindow(imagePlaybackControlWindow);
    //imagePlaybackControlWindow->resize(650, 230); // Min. size will set automatically anyways
    // No "X" button on this window
    imagePlaybackControlWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowStaysOnTopHint);
    //imagePlaybackControlWindow->setWindowFlags(imagePlaybackControlWindow->windowFlags() & ~Qt::WindowCloseButtonHint);
    //imagePlaybackControlWindow->setWindowFlags( (Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint) & ~Qt::WindowCloseButtonHint );
    imagePlaybackControlWindow->show();
    //imagePlaybackControlWindow->restoreGeometry();

    //connect(selectedCamera, SIGNAL(finished()), imagePlaybackControlDialog, SLOT(onPlaybackFinished()));
    // GB: right now, this only gets called when playbackLoop is false, and we need to finish playing (with possible overhead)
    //connect(selectedCamera, SIGNAL(endReached()), imagePlaybackControlDialog, SLOT(onAutomaticFinish()));
    connect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyStarted()), this, SLOT(onPlaybackSafelyStarted()));
    connect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyPaused()), this, SLOT(onPlaybackSafelyPaused()));
    connect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyStopped()), this, SLOT(onPlaybackSafelyStopped()));

    connectCameraPlaybackChangedSlots();


    // GB NOTE:
    // I could have done this in a way that the camera child widgets only receive a cv::Mat to display..
    // but we are actually not displaying anything else in the views, just fileCamera frames,
    // so I dedicated separate functions for them, which only take the frameNumber,
    // implemented for both single and stereo camera views. This is ok too
    if(selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE && singleCameraChildWidget) {
        connect(imagePlaybackControlDialog, SIGNAL(stillImageChange(int)), singleCameraChildWidget, SLOT(displayFileCameraFrame(int)));
    } else if(selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE && stereoCameraChildWidget) {
        connect(imagePlaybackControlDialog, SIGNAL(stillImageChange(int)), stereoCameraChildWidget, SLOT(displayFileCameraFrame(int)));
    }
    // GB modified/added end

    playbackSynchroniser = new PlaybackSynchroniser();
    playbackSynchroniser->setCamera(selectedCamera);
    playbackSynchroniser->setPupilDetection(pupilDetectionWorker);


    connect(pupilDetectionWorker, SIGNAL(processingStarted()), playbackSynchroniser, SLOT(onPupilDetectionStarted()));
    connect(pupilDetectionWorker, SIGNAL(processingFinished()), playbackSynchroniser, SLOT(onPupilDetectionStopped()));
    connect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyStarted()), playbackSynchroniser, SLOT(onPlaybackStarted()));
    connect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyStopped()), playbackSynchroniser, SLOT(onPlaybackStopped()));
    connect(imagePlaybackControlDialog, SIGNAL(onPlaybackSafelyPaused()), playbackSynchroniser, SLOT(onPlaybackStopped()));



}

void MainWindow::onPlaybackSafelyStarted() {
    const QByteArray m_syncRecordCsv = applicationSettings->value("syncRecordCsv", imagePlaybackControlDialog->getSyncRecordCsv()).toByteArray();
    bool syncRecordCsv = true;
    if (!m_syncRecordCsv.isEmpty() && (m_syncRecordCsv == "0" || m_syncRecordCsv == "false"))
        syncRecordCsv = false;

    const QByteArray m_syncStream = applicationSettings->value("syncStream", imagePlaybackControlDialog->getSyncStream()).toByteArray();
    bool syncStream = true;
    if (!m_syncStream.isEmpty() && (m_syncStream == "0" || m_syncStream == "false"))
        syncStream = false;
    
    if(syncRecordCsv && trackingOn && !pupilDetectionDataFile.isEmpty() && !recordOn) {
        onRecordClick();
    }

    if(syncStream && trackingOn && !streamOn) {
        onStreamClick();
    }
}

void MainWindow::onPlaybackSafelyPaused() {
    const QByteArray m_syncRecordCsv = applicationSettings->value("syncRecordCsv", imagePlaybackControlDialog->getSyncRecordCsv()).toByteArray();
    bool syncRecordCsv = true;
    if (!m_syncRecordCsv.isEmpty() && (m_syncRecordCsv == "0" || m_syncRecordCsv == "false"))
        syncRecordCsv = false;

    const QByteArray m_syncStream = applicationSettings->value("syncStream", imagePlaybackControlDialog->getSyncStream()).toByteArray();
    bool syncStream = true;
    if (!m_syncStream.isEmpty() && (m_syncStream == "0" || m_syncStream == "false"))
        syncStream = false;
    
    if(syncRecordCsv && trackingOn && recordOn) {
        onRecordClick();
    }

    if(syncStream && trackingOn && streamOn) {
        onStreamClick();
    }
}

void MainWindow::onPlaybackSafelyStopped() {
    // yet this is the same as onPlaybackSafelyPaused() but this can change 
    // if future features would behave differently, so I leave it here like this
    onPlaybackSafelyPaused();
}

// GB: onPlayImageDirectoryClick() and onStopImageDirectoryClick() and onPlayImageDirectoryFinished()
// were here, but their functionality has been moved to ImagePlaybackControlDialog

void MainWindow::onSerialConnect() {
    const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/emblems/22/vcs-normal.svg"), applicationSettings);
    serialStatusIcon->setPixmap(offlineIcon.pixmap(12, 12));
}

void MainWindow::onSerialDisconnect() {
    const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    serialStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));
}

void MainWindow::onRemoteEnable() {
    remoteOn = true;
    const QIcon remoteIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/emblems/22/vcs-normal.svg"), applicationSettings);
    remoteStatusIcon->setPixmap(remoteIcon.pixmap(12, 12));
}

void MainWindow::onRemoteDisable() {
    remoteOn = false;
    const QIcon remoteIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    remoteStatusIcon->setPixmap(remoteIcon.pixmap(16, 16));
}

void MainWindow::onHwTriggerEnable() {
    hwTriggerOn = true;
    const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/emblems/22/vcs-normal.svg"), applicationSettings);
    hwTriggerStatusIcon->setPixmap(offlineIcon.pixmap(12, 12));
}

void MainWindow::onHwTriggerDisable() {
    hwTriggerOn = false;
    const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    hwTriggerStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));
}

void MainWindow::onCameraCalibrationEnabled() {
    const QIcon calibIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/emblems/22/vcs-normal.svg"), applicationSettings);
    calibrationStatusIcon->setPixmap(calibIcon.pixmap(12, 12));
}

void MainWindow::onCameraCalibrationDisabled() {
    const QIcon calibIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
    calibrationStatusIcon->setPixmap(calibIcon.pixmap(16, 16));
}


void MainWindow::onGeneralSettingsChange() {

    this->repaint();

    if(pupilDetectionSettingsDialog)
        pupilDetectionSettingsDialog->repaint();

    if(singleCameraSettingsDialog)
        singleCameraSettingsDialog->repaint();

    if(stereoCameraSettingsDialog)
        stereoCameraSettingsDialog->repaint();
}

void MainWindow::onSubjectsSettingsChange(QString subject) {

    subjectConfigurationLabel->setText("Current configuration: " + subject);
}

void MainWindow::onSubjectsClick() {
    const QIcon subjectsSelectedIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/im-user-online.svg"), applicationSettings); //QIcon::fromTheme("camera-video");
    //subjectsAct->setIcon(subjectsSelectedIcon);

    subjectSelectionDialog->show();
}

void MainWindow::resetGeometry() {
    this->showMaximized();

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();

    for(auto mdiSubWindow : windows) {
        auto *child = dynamic_cast<RestorableQMdiSubWindow *>(mdiSubWindow);

        child->resetGeometry();
    }

}

// only to be called from GUI
void MainWindow::incrementTrialCounter() {
    // This function is to be called only from GUI interaction.
    quint64 timestamp  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    incrementTrialCounter(timestamp);
}

void MainWindow::incrementTrialCounter(const quint64 &timestamp) {
    if(!selectedCamera || (selectedCamera && (selectedCamera->getType()==SINGLE_IMAGE_FILE || selectedCamera->getType()==STEREO_IMAGE_FILE)) || !recEventTracker )
        return;
//    trialCounter->incrementTrial();
//    if(offlineEventLogWriter)
//        offlineEventLogWriter->incrementTrial();
    // if a camera connected exists, there shoudl always be a non-nullptr recEventTracker
    //recEventTracker->addTrialIncrement(timestamp);
    emit commitTrialCounterIncrement(timestamp);
    updateCurrentTrialLabel();
}

// TODO: let the user place a message from GUI too
void MainWindow::logRemoteMessage(const quint64 &timestamp, const QString &str) {
    if(!selectedCamera || (selectedCamera && (selectedCamera->getType()==SINGLE_IMAGE_FILE || selectedCamera->getType()==STEREO_IMAGE_FILE)) || !recEventTracker )
        return;
    emit commitRemoteMessage(timestamp, str);
}

void MainWindow::updateCurrentTrialLabel() {
    if(recEventTracker) {
        unsigned int num = recEventTracker->getLastCommissionedTrialNumber();
        //currentTrialLabel->setText(QString::number(trialCounter->value()));
        currentTrialLabel->setText(QString::number(num));
        // It is important to emphasize the trial counter value by colour, as they can instantly catch the attention of
        // the experimenter if trigger signals are not arriving from the experiment computer
        if(num%2==0) {
            trialWidget->setStyleSheet("background-color: #ebd234; color: #000000; border: 1px #000000;");
        } else {
            trialWidget->setStyleSheet("background-color: #19fac2; color: #000000; border: 1px #000000;");
        }
        // NOTE: currently the label doe not show the trial number associated with the displayed image,
        // (can be lagging due to high fps) but the trial number that will be associated with the 
        // currently grabbed frames
    } else {
        currentTrialLabel->setText("-");
        trialWidget->setStyleSheet(styleSheet());
    }
}


void MainWindow::safelyResetTrialCounter() {
    // Only called when someone wants to start a new recording.
    // This function is to be called only from GUI interaction.
    quint64 timestamp  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    safelyResetTrialCounter(timestamp);
}        

void MainWindow::safelyResetTrialCounter(const quint64 &timestamp) {
    // Only called when someone wants to start a new recording.
    if(streamOn || recordOn || !recEventTracker)
        return;
    //recEventTracker->resetBufferTrialCounter(timestamp);
    emit commitTrialCounterReset(timestamp);
    updateCurrentTrialLabel();
}

void MainWindow::forceResetTrialCounter() {
    quint64 timestamp  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    forceResetTrialCounter(timestamp);
}

void MainWindow::forceResetTrialCounter(const quint64 &timestamp) {
    // Can be used when e.g. there is a streaming going on, but the experiment PC sends a remote control command 
    // to start a new csv recording. As streaming is on, trial counter would not automatically reset, 
    // But if we call this method, it will.
    // This also can be called from general settings
    if(!recEventTracker)
        return;
    //recEventTracker->resetBufferTrialCounter(timestamp);
    emit commitTrialCounterReset(timestamp);
    updateCurrentTrialLabel();
}





///////////////////////////////////////////////////////////////////


void MainWindow::onStreamingUDPConnect() {
    if(dataStreamer) {
        dataStreamer->startUDPStreamer(streamingSettingsDialog->getUDPsocket(), streamingSettingsDialog->getUDPip(), streamingSettingsDialog->getUDPport(), streamingSettingsDialog->getDataContainerUDP());
        streamingSettingsDialog->setLimitationsWhileStreamingUDP(true);
    }
    streamAct->setDisabled(false);  
}

void MainWindow::onStreamingUDPDisconnect() {
    if(dataStreamer) {
        dataStreamer->stopUDPStreamer();
        streamingSettingsDialog->setLimitationsWhileStreamingUDP(false);
        if(dataStreamer->getNumActiveStreamers() == 0) {
            onStreamClick(); // close streamer
            streamAct->setDisabled(true); 
        }
    }
    
    if(!streamingSettingsDialog->isAnyConnected())
        streamAct->setDisabled(true); 
}

void MainWindow::onStreamingCOMConnect() {
    if(dataStreamer) {
        dataStreamer->startCOMStreamer(streamingSettingsDialog->getConnPoolCOMIndex(), streamingSettingsDialog->getDataContainerCOM());
        streamingSettingsDialog->setLimitationsWhileStreamingCOM(true);
    }
    streamAct->setDisabled(false);  
}

void MainWindow::onStreamingCOMDisconnect() {
    if(dataStreamer) {
        dataStreamer->stopCOMStreamer();
        streamingSettingsDialog->setLimitationsWhileStreamingCOM(false);
        if(dataStreamer->getNumActiveStreamers() == 0) {
            onStreamClick(); // close streamer
            streamAct->setDisabled(true); 
        }
    }
    
    if(!streamingSettingsDialog->isAnyConnected())
        streamAct->setDisabled(true); 
}

void MainWindow::onRemoteConnStateChanged() {
    if(remoteCCDialog->isAnyConnected()) {
        const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/emblems/22/vcs-normal.svg"), applicationSettings);
        remoteStatusIcon->setPixmap(offlineIcon.pixmap(12, 12));
    } else {
        const QIcon offlineIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":icons/Breeze/actions/22/media-record.svg"), applicationSettings);
        remoteStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));
    }
}

void MainWindow::loadCalibrationWindow(){
    QWidget *calibrationDialog;
    QString calibrationDialogName;
    if(selectedCamera && selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) {
        calibrationDialog = new SingleCameraCalibrationView(dynamic_cast<SingleCamera*>(selectedCamera), this);
        calibrationDialogName = "SingleCameraCalibrationView";
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM) {
        calibrationDialog = new SingleWebcamCalibrationView(dynamic_cast<SingleWebcam*>(selectedCamera), this);
        calibrationDialogName = "SingleWebcamCalibrationView";
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA) {
        calibrationDialog = new StereoCameraCalibrationView(dynamic_cast<StereoCamera*>(selectedCamera), this);
        calibrationDialogName = "StereoCameraCalibrationView";
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
        calibrationDialog = new SingleFileCameraCalibrationView(dynamic_cast<FileCamera*>(selectedCamera), this);
        calibrationDialogName = "SingleFileCameraCalibrationView";
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
        calibrationDialog = new StereoFileCameraCalibrationView(dynamic_cast<FileCamera*>(selectedCamera), this);
        calibrationDialogName = "StereoFileCameraCalibrationView";
    }

    RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(calibrationDialog, calibrationDialogName, this);
    mdiArea->addSubWindow(child);
    child->setWindowTitle("Calibration view");
    child->show();
    child->restoreGeometry();
    connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
    calibrationWindow = child;
    calibrationWindow->setWindowIcon(calibrateIcon);
}

void MainWindow::loadSharpnessWindow(){
        if(selectedCamera && (
        selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA  // ||
        // selectedCamera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM
    )) {
        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new SingleCameraSharpnessView(dynamic_cast<SingleCamera*>(selectedCamera), this), "SingleCameraSharpnessView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        sharpnessWindow = child;
        sharpnessWindow->setWindowIcon(sharpnessIcon);
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
    }
//    else if(selectedCamera && selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
//        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new SingleCameraSharpnessView(dynamic_cast<FileCamera*>(selectedCamera), this), "SingleCameraSharpnessView", this);
//        mdiArea->addSubWindow(child);
//        child->show();
//        child->restoreGeometry();
//        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
//    }
}

void MainWindow::stopCamera()
{
    if (selectedCamera){
        selectedCamera->stopGrabbing();
        cameraPlaying = false;
    }
}

void MainWindow::startCamera()
{
    if (selectedCamera){
        selectedCamera->startGrabbing();
        cameraPlaying = true;
    }
}

void MainWindow::resetStatus(bool isConnect)
{
    bool imageRecordingEnabled = selectedCamera && selectedCamera->getType() != CameraImageType::SINGLE_IMAGE_FILE && selectedCamera->getType() != CameraImageType::STEREO_IMAGE_FILE;

    if (isConnect){
        cameraAct->setEnabled(false);
        cameraSettingsAct->setEnabled(true);
        cameraActDisconnectAct->setEnabled(true);
        calibrateAct->setEnabled(true);
        sharpnessAct->setEnabled(true);
        subjectsAct->setEnabled(true);
        trackAct->setEnabled(true);
        logFileAct->setEnabled(true);
        //streamAct->setEnabled(true);

        outputDirectoryAct->setEnabled(imageRecordingEnabled);
        recordImagesAct->setEnabled(imageRecordingEnabled && !outputDirectory.isEmpty());
        forceResetTrialAct->setEnabled(imageRecordingEnabled);
        manualIncTrialAct->setEnabled(imageRecordingEnabled);

        streamingSettingsAct->setEnabled(true);
        trialWidget->setVisible(true);
    }
    else {
        cameraAct->setEnabled(true);
        cameraSettingsAct->setEnabled(false);
        cameraActDisconnectAct->setEnabled(false);
        calibrateAct->setEnabled(false);
        sharpnessAct->setEnabled(false);
        subjectsAct->setEnabled(false);
        trackAct->setEnabled(false);
        logFileAct->setEnabled(false);
        //streamAct->setEnabled(false);

        outputDirectoryAct->setEnabled(false);
        recordImagesAct->setEnabled(false);
        forceResetTrialAct->setEnabled(false);
        manualIncTrialAct->setEnabled(false);

        streamingSettingsAct->setEnabled(false);
        trialWidget->setVisible(false);

        recordAct->setEnabled(false); //
        cameraPlaying = true; //
        }
}

void MainWindow::onImagesSkipped() {
    if(imagesSkippedMsgBox != nullptr) {
        return;
    }
    imagesSkippedMsgBox = new QMessageBox(this);
    imagesSkippedMsgBox->setWindowTitle("Camera image frames skipped");
    imagesSkippedMsgBox->setText("At least one image frame was skipped due to unstable connection or interface failure.\n\nPlease check camera connection. Be sure to use a power-supply backed (active) cable for long distances, and clean electrical contacts with appropriate materials if necessary.\n\nCameras can consume considerable power during frame grabbing, thus should you also ensure that your power supply has compatible amperage rating for your camera device.");
    imagesSkippedMsgBox->setMinimumSize(330,240);
    imagesSkippedMsgBox->setIcon(QMessageBox::Warning);
    imagesSkippedMsgBox->setModal(false);
    connect(imagesSkippedMsgBox, SIGNAL(accepted()), this, SLOT(onImagesSkippedMsgClose()));
    //connect(imagesSkippedMsgBox,SIGNAL(accepted()),this,SLOT(onImagesSkippedMsgClose()));
    //connect(imagesSkippedMsgBox,SIGNAL(rejected()),this,SLOT(onImagesSkippedMsgClose()));
    imagesSkippedMsgBox->show();
}

void MainWindow::onImagesSkippedMsgClose() {
    disconnect(imagesSkippedMsgBox, SIGNAL(accepted()), this, SLOT(onImagesSkippedMsgClose()));
    imagesSkippedMsgBox->deleteLater();
    imagesSkippedMsgBox = nullptr;
}

void MainWindow::onCameraUnexpectedlyDisconnected() {

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("Camera unexpectedly disconnected");
    msgBox->setText("At least one camera in use was unexpectedly disconnected. Recordings are stopped for saving.\n\nPlease check camera connection. Be sure to use a power-supply backed (active) cable for long distances, and clean electrical contacts with appropriate materials if necessary.\n\nCameras can consume considerable power during frame grabbing, thus should you also ensure that your power supply has compatible amperage rating for your camera device.");
    msgBox->setMinimumSize(330,240);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setModal(false);
    msgBox->show();

    onCameraDisconnectClick();

}

void MainWindow::connectCameraPlaybackChangedSlots()
{
    /*    if (imagePlaybackControlDialog && cameraViewWindow){
        connect(imagePlaybackControlDialog, &ImagePlaybackControlDialog::cameraPlaybackChanged, cameraViewWindow, &SingleCameraView::onCameraPlaybackChanged);
        connect(cameraViewWindow, &SingleCameraView::cameraPlaybackChanged, imagePlaybackControlDialog, &ImagePlaybackControlDialog::onCameraPlaybackChanged);        
    }
    else if (imagePlaybackControlDialog){
        connect(this, cameraPlaybackChanged, imagePlaybackControlDialog, &ImagePlaybackControlDialog::onCameraPlaybackChanged);
        connect(imagePlaybackControlDialog, &ImagePlaybackControlDialog::cameraPlaybackChanged, this, onCameraPlaybackChanged);
        connect(imagePlaybackControlDialog, &ImagePlaybackControlDialog::cameraPlaybackChanged, imagePlaybackControlDialog, &ImagePlaybackControlDialog::onCameraPlaybackChanged);
    }
    else if (cameraViewWindow){
        connect(this, cameraPlaybackChanged, cameraViewWindow, &SingleCameraView::onCameraPlaybackChanged);
        connect(cameraViewWindow, &SingleCameraView::cameraPlaybackChanged, this, onCameraPlaybackChanged);
        connect(cameraViewWindow, &SingleCameraView::cameraPlaybackChanged, cameraViewWindow, &SingleCameraView::onCameraPlaybackChanged);
    }
    connect(this, SIGNAL(cameraPlaybackChanged()), this, SLOT(onCameraPlaybackChanged()));*/

    if (imagePlaybackControlDialog != nullptr && singleCameraChildWidget != nullptr){
        connect(imagePlaybackControlDialog, SIGNAL(cameraPlaybackChanged()), singleCameraChildWidget, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(singleCameraChildWidget, SIGNAL(cameraPlaybackChanged()), imagePlaybackControlDialog, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);        
    }
    if (imagePlaybackControlDialog != nullptr && stereoCameraChildWidget != nullptr){
        connect(imagePlaybackControlDialog, SIGNAL(cameraPlaybackChanged()), stereoCameraChildWidget, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(stereoCameraChildWidget, SIGNAL(cameraPlaybackChanged()), imagePlaybackControlDialog, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);        
    }

    if (imagePlaybackControlDialog != nullptr){
        connect(this, SIGNAL(cameraPlaybackChanged()), imagePlaybackControlDialog, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(imagePlaybackControlDialog, SIGNAL(cameraPlaybackChanged()), this, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(imagePlaybackControlDialog, SIGNAL(cameraPlaybackChanged()), imagePlaybackControlDialog, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
    }

    if (singleCameraChildWidget != nullptr){
        connect(this, SIGNAL(cameraPlaybackChanged()), singleCameraChildWidget, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(singleCameraChildWidget, SIGNAL(cameraPlaybackChanged()), this, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(singleCameraChildWidget, SIGNAL(cameraPlaybackChanged()), singleCameraChildWidget, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
    }

    if (stereoCameraChildWidget != nullptr){
        connect(this, SIGNAL(cameraPlaybackChanged()), stereoCameraChildWidget, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(stereoCameraChildWidget, SIGNAL(cameraPlaybackChanged()), this, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
        connect(stereoCameraChildWidget, SIGNAL(cameraPlaybackChanged()), stereoCameraChildWidget, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
    }
    connect(this, SIGNAL(cameraPlaybackChanged()), this, SLOT(onCameraPlaybackChanged()), Qt::UniqueConnection);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* e)
{
    QStringList pathList;
    QList<QUrl> urlList = e->mimeData()->urls();

    QString thingToOpen;
    if(!urlList.empty())
        thingToOpen = urlList.at(0).toLocalFile();

    QFileInfo fileInfo(thingToOpen);
    if(!fileInfo.isReadable()) {
        QMessageBox MsgBox;
        MsgBox.setText(QString::fromStdString("The folder you are trying to open is not readable. Please ensure sufficient permission of your user account and/or PupilEXT, and the availability of the location to be read."));
        MsgBox.exec();
    }

    if(fileInfo.isDir()) {
        if(selectedCamera && selectedCamera->isOpen()) {
            onCameraDisconnectClick();
        }
        qDebug() << "Attempting to open: " << fileInfo.filePath();
        openImageDirectory(fileInfo.filePath());
    } else if(fileInfo.isFile()) {
        // TODO: do this with a QMap that is stored in persistence/QSettings
        if(fileInfo.completeSuffix() == "tiff" || fileInfo.completeSuffix() == "jpg" || fileInfo.completeSuffix() == "bmp" || fileInfo.completeSuffix() == "png" ||
            fileInfo.fileName() == "imagerec_meta.xml" || fileInfo.fileName() == "offline_event_log.xml" ||
            fileInfo.fileName() == "imagerec-meta.xml" || fileInfo.fileName() == "offline-event-log.xml" ) {

            if(selectedCamera && selectedCamera->isOpen()) {
                onCameraDisconnectClick();
            }
            qDebug() << "Attempting to open: " << fileInfo.filePath().chopped(fileInfo.fileName().length());
            openImageDirectory(fileInfo.filePath().chopped(fileInfo.fileName().length()));
        }
    }

    // TODO: add further checks and event handling
    e->acceptProposedAction();
}