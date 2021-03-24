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


// Upon construction, worker objects for processing are created pupil detection and its respective thread
MainWindow::MainWindow(): mdiArea(new QMdiArea(this)),
                          signalPubSubHandler(new SignalPubSubHandler(this)),
                          serialSettingsDialog(new SerialSettingsDialog(this)),
                          pupilDetectionWorker(new PupilDetection()),
                          subjectSelectionDialog(new SubjectSelectionDialog(this)),
                          singleCameraSettingsDialog(nullptr),
                          stereoCameraSettingsDialog(nullptr),
                          pupilDetectionThread(new QThread()),
                          selectedCamera(nullptr),
                          cameraViewWindow(nullptr),
                          calibrationWindow(nullptr),
                          applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), this)) {

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }


    connect(serialSettingsDialog, SIGNAL (onConnect()), this, SLOT (onSerialConnect()));
    connect(serialSettingsDialog, SIGNAL (onDisconnect()), this, SLOT (onSerialDisconnect()));

    pupilDetectionSettingsDialog = new PupilDetectionSettingsDialog(pupilDetectionWorker, this);

    generalSettingsDialog = new GeneralSettingsDialog(this);

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


    // Get settings key if gettings started dialog was already opened
    bool showGettingsStartedWizard = applicationSettings->value("ShowGettingsStartedWizard", true).toBool();

    if(showGettingsStartedWizard) {
        GettingsStartedWizard* wizard = new GettingsStartedWizard(this);
        wizard->show();
        connect(wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this , SLOT(onGettingsStartedWizardFinish()));
    }
}

void MainWindow::onGettingsStartedWizardFinish() {
    applicationSettings->setValue("ShowGettingsStartedWizard", false);
}

void MainWindow::createActions() {

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));

    QAction *fileOpenAct = fileMenu->addAction(tr("Open Images Directory"), this, &MainWindow::onOpenImageDirectory);
    fileOpenAct->setStatusTip(tr("Open Image Directory for Playback. Single and Stereo Mode supported."));
    fileMenu->addAction(fileOpenAct);

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

    QAction *serialPortSettingsAct = settingsMenu->addAction(tr("Serial Connection"));
    connect(serialPortSettingsAct, &QAction::triggered, serialSettingsDialog, &SerialSettingsDialog::show);

    QAction *pupilDetectionSettingsAct = settingsMenu->addAction(tr("Pupil Detection"));
    connect(pupilDetectionSettingsAct, &QAction::triggered, pupilDetectionSettingsDialog, &PupilDetectionSettingsDialog::show);

    QAction *generalSettingsAct = settingsMenu->addAction(tr("Settings"));
    connect(generalSettingsAct, &QAction::triggered, generalSettingsDialog, &GeneralSettingsDialog::show);

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

    const QIcon cameraIcon = QIcon(":/icons/Breeze/devices/22/camera-video.svg"); //QIcon::fromTheme("camera-video");
    cameraAct = new QAction(cameraIcon, tr("Camera"), this);
    cameraAct->setStatusTip(tr("Connect to camera(s)."));

    QMenu* cameraMenu = new QMenu(this);
    singleCameraDevicesMenu = cameraMenu->addMenu(tr("&Single Camera"));

    updateCameraMenu();
    connect(singleCameraDevicesMenu, SIGNAL(triggered(QAction*)), this, SLOT(singleCameraSelected(QAction*)));

    cameraMenu->addAction(tr("Stereo Camera"), this, &MainWindow::stereoCameraSelected);

    cameraMenu->addSeparator();

    // updateCameraMenu
    cameraMenu->addAction(tr("Refresh Devices"), this, &MainWindow::updateCameraMenu);

    cameraAct->setMenu(cameraMenu);
    connect(cameraAct, &QAction::triggered, this, &MainWindow::onCameraClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(cameraAct);

    const QIcon disconnectIcon = QIcon(":/icons/Breeze/actions/22/network-disconnect.svg"); //QIcon::fromTheme("camera-video");
    cameraActDisconnectAct = new QAction(disconnectIcon, tr("Disconnect"), this);
    //trackAct->setShortcuts(QKeySequence::New);
    cameraActDisconnectAct->setStatusTip(tr("Disconnect camera."));
    connect(cameraActDisconnectAct, &QAction::triggered, this, &MainWindow::onCameraDisconnectClick);
    cameraActDisconnectAct->setDisabled(true);
    //fileMenu->addAction(newAct);
    toolBar->addAction(cameraActDisconnectAct);

    const QIcon cameraSettingsIcon = QIcon(":/icons/Breeze/actions/22/configure.svg"); //QIcon::fromTheme("camera-video");
    cameraSettingsAct = new QAction(cameraSettingsIcon, tr("Camera Settings"), this);
    //trackAct->setShortcuts(QKeySequence::New);
    cameraSettingsAct->setStatusTip(tr("Disconnect camera."));
    connect(cameraSettingsAct, &QAction::triggered, this, &MainWindow::onCameraSettingsClick);
    cameraSettingsAct->setDisabled(true);
    settingsMenu->addAction(cameraSettingsAct);
    toolBar->addAction(cameraSettingsAct);

    const QIcon playIcon = QIcon(":/icons/Breeze/actions/22/media-playback-start.svg"); //QIcon::fromTheme("camera-video");
    playImageDirectoryAct = new QAction(playIcon, tr("Play Directory"), this);
    //trackAct->setShortcuts(QKeySequence::New);
    playImageDirectoryAct->setStatusTip(tr("Play Image Directory."));
    connect(playImageDirectoryAct, &QAction::triggered, this, &MainWindow::onPlayImageDirectoryClick);
    playImageDirectoryAct->setDisabled(true);
    toolBar->addAction(playImageDirectoryAct);

    const QIcon stopIcon = QIcon(":/icons/Breeze/actions/22/media-playback-stop.svg");
    stopImageDirectoryAct = new QAction(stopIcon, tr("Stop offline directory Playback."), this);
    stopImageDirectoryAct->setStatusTip(tr("Stop Image Directory Playback."));
    connect(stopImageDirectoryAct, &QAction::triggered, this, &MainWindow::onStopImageDirectoryClick);
    stopImageDirectoryAct->setDisabled(true);
    toolBar->addAction(stopImageDirectoryAct);

    toolBar->addSeparator();

    const QIcon trackOffIcon = QIcon(":/icons/Breeze/actions/22/view-visible.svg"); //QIcon::fromTheme("camera-video");
    trackAct = new QAction(trackOffIcon, tr("Track"), this);
    trackAct->setCheckable(true);
    //trackAct->setShortcuts(QKeySequence::New);
    trackAct->setStatusTip(tr("Start/Stop pupil tracking."));
    connect(trackAct, &QAction::triggered, this, &MainWindow::onTrackActClick);
    trackAct->setDisabled(true);
    //fileMenu->addAction(newAct);
    toolBar->addAction(trackAct);

    toolBar->addSeparator();

    const QIcon calibrateIcon = QIcon(":/icons/Breeze/actions/22/crosshairs.svg"); //QIcon::fromTheme("camera-video");
    calibrateAct = new QAction(calibrateIcon, tr("Calibrate"), this);
    calibrateAct->setStatusTip(tr("Start calibrating."));
    connect(calibrateAct, &QAction::triggered, this, &MainWindow::onCalibrateClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(calibrateAct);
    calibrateAct->setDisabled(true);

    const QIcon sharpnessIcon = QIcon(":/icons/Breeze/actions/22/edit-select-all.svg"); //QIcon::fromTheme("camera-video");
    sharpnessAct = new QAction(sharpnessIcon, tr("Sharpness"), this);
    sharpnessAct->setStatusTip(tr("Validate sharpness."));
    connect(sharpnessAct, &QAction::triggered, this, &MainWindow::onSharpnessClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(sharpnessAct);
    sharpnessAct->setDisabled(true);

    const QIcon subjectsIcon = QIcon(":/icons/Breeze/actions/22/im-user.svg"); //QIcon::fromTheme("camera-video");
    subjectsAct = new QAction(subjectsIcon, tr("Subjects"), this);
    subjectsAct->setStatusTip(tr("Load subject specific detection configuration."));
    connect(subjectsAct, &QAction::triggered, this, &MainWindow::onSubjectsClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(subjectsAct);

    toolBar->addSeparator();

    const QIcon logFileIcon = QIcon(":icons/Breeze/actions/22/edit-text-frame-update.svg"); //QIcon::fromTheme("camera-video");
    logFileAct = new QAction(logFileIcon, tr("Logfile"), this);
    logFileAct->setStatusTip(tr("Set log file."));
    connect(logFileAct, &QAction::triggered, this, &MainWindow::setLogFile);
    //fileMenu->addAction(newAct);
    toolBar->addAction(logFileAct);
    logFileAct->setDisabled(true);


    const QIcon recordIcon = QIcon(":/icons/Breeze/actions/22/media-record.svg"); //QIcon::fromTheme("camera-video");
    recordAct = new QAction(recordIcon, tr("Record"), this);
    recordAct->setStatusTip(tr("Start pupil recording."));
    connect(recordAct, &QAction::triggered, this, &MainWindow::onRecordClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(recordAct);
    recordAct->setDisabled(true);


    toolBar->addSeparator();

    const QIcon outputDirIcon = QIcon(":icons/Breeze/actions/22/document-open.svg"); //QIcon::fromTheme("camera-video");
    outputDirectoryAct = new QAction(outputDirIcon, tr("Output Directory"), this);
    outputDirectoryAct->setStatusTip(tr("Set output directory."));
    connect(outputDirectoryAct, &QAction::triggered, this, &MainWindow::setOutputDirectory);
    //fileMenu->addAction(newAct);
    toolBar->addAction(outputDirectoryAct);
    outputDirectoryAct->setDisabled(true);

    const QIcon recordImagesIcon = QIcon(":/icons/Breeze/actions/22/media-record-blue.svg"); //QIcon::fromTheme("camera-video");
    recordImagesAct = new QAction(recordImagesIcon, tr("Record Images"), this);
    recordImagesAct->setStatusTip(tr("Start image recording."));
    connect(recordImagesAct, &QAction::triggered, this, &MainWindow::onRecordImageClick);
    //fileMenu->addAction(newAct);
    toolBar->addAction(recordImagesAct);
    recordImagesAct->setDisabled(true);


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

    QLabel *calibrationLabel = new QLabel("Camera Calibration");
    const QIcon calibrationIcon = QIcon(":icons/Breeze/actions/22/media-record.svg");
    calibrationStatusIcon = new QLabel();
    calibrationStatusIcon->setPixmap(calibrationIcon.pixmap(16, 16));

    statusBarLayout->addWidget(calibrationLabel);
    statusBarLayout->addWidget(calibrationStatusIcon);


    QLabel *serialLabel = new QLabel("Serial Connection");
    const QIcon offlineIcon = QIcon(":icons/Breeze/actions/22/media-record.svg");
    serialStatusIcon = new QLabel();
    serialStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));

    statusBarLayout->addWidget(serialLabel);
    statusBarLayout->addWidget(serialStatusIcon);

    QLabel *hwTriggerLabel = new QLabel("Hardware Trigger");
    hwTriggerStatusIcon = new QLabel();
    hwTriggerStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));

    statusBarLayout->addWidget(hwTriggerLabel);
    statusBarLayout->addWidget(hwTriggerStatusIcon);

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

void MainWindow::about() {
    QMessageBox::about(this, tr("About ") + QCoreApplication::applicationName(),
            tr("<b>%1</b> is an open source application for pupillometry.<br><br>Publication:<br>Babak Zandi, Moritz Lode, Alexander Herzog, Georgios Sakas and Tran Quoc Khanh, <b>PupilEXT: flexible open-source platform for high resolution pupil measurement in vision research</b>"
               "<br><br>"
               "Github: <a href=\"https://github.com/openPupil/Open-PupilEXT\">https://github.com/openPupil/Open-PupilEXT</a><br>"
               "OpenPupil Project: <a href=\"https://openpupil.io\">www.openpupil.io</a><br><br>"
               "Developer-Team: Moritz Lode, Babak Zandi<br>Version: %2<br>"
                                                      "<br>Application settings: %3<br><br><br>"
                                                      "Powered by <b>Open Source</b>. Licensed under <a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">GPL 3</a>"
    ).arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), applicationSettings->fileName()));
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

    dialog->setLayout(l);
    dialog->show();
}


void MainWindow::setLogFile() {

    logFileName = QFileDialog::getSaveFileName(this, tr("Save Log File"), recentPath, tr("CSV files (*.csv)"), nullptr, QFileDialog::DontConfirmOverwrite);

    if(!logFileName.isEmpty()) {
        QFileInfo fileInfo(logFileName);

        recentPath = fileInfo.dir().path();

        // check if filename has extension
        if(fileInfo.suffix().isEmpty()) {
            logFileName = logFileName + ".csv";
        }

        //QFile file(logFileName);
        //file.open(QIODevice::WriteOnly); // Or QIODevice::ReadWrite
        //file.close();

        recordAct->setDisabled(false);
    }
}

void MainWindow::setOutputDirectory() {

    outputDirectory = QFileDialog::getExistingDirectory(this, tr("Output Directory"), recentPath);

    if(!outputDirectory.isEmpty()) {

        recentPath = outputDirectory;
        QStringList lst = recentPath.split('/');
        currentDirectoryLabel->setText("Current directory: .../" + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
        currentDirectoryLabel->setToolTip(recentPath);

        recordImagesAct->setDisabled(false);
    }
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

    std::cout<<"updateWindowMenu"<<std::endl;


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
            }
        }
    }

    if(singleCameraDevicesMenu->actions().empty()) {
        QAction *noAction = singleCameraDevicesMenu->addAction("No devices.");
        noAction->setDisabled(true);
    }
}

void MainWindow::onTrackActClick() {

    if(trackingOn) {
        // Deactivate tracking
        pupilDetectionWorker->stopDetection();

        const QIcon trackOffIcon = QIcon(":/icons/Breeze/actions/22/view-visible.svg"); //QIcon::fromTheme("camera-video");
        trackAct->setIcon(trackOffIcon);
        trackingOn = false;
    } else {
        // Activate tracking

        // Even though its already set in the selection of the camera, camera calibration may changed till now so we need to load it again
        // TODO better way of doing this without setting the camera two times (or loading the config at selection differently)
        pupilDetectionWorker->setCamera(selectedCamera);

        pupilDetectionWorker->startDetection();

        const QIcon trackOnIcon = QIcon(":/icons/Breeze/actions/22/redeyes.svg"); //QIcon::fromTheme("camera-video");
        trackAct->setIcon(trackOnIcon);
        trackingOn = true;
    }
}

void MainWindow::onRecordClick() {

    if(recordOn) {
        // Deactivate recording
        dataWriter->close(); // TODO check if may terminate writing to early? because of the lag of the event queue in pupildetection

        const QIcon recordOffIcon = QIcon(":/icons/Breeze/actions/22/media-record.svg"); //QIcon::fromTheme("camera-video");
        recordAct->setIcon(recordOffIcon);
        recordOn = false;
    } else {
        // Activate recording
        bool stereo = selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE;
        dataWriter = new DataWriter(logFileName, stereo ? WriteMode::STEREO : WriteMode::SINGLE, this);

        if(stereo) {
            connect(pupilDetectionWorker, SIGNAL (processedStereoPupilData(quint64, Pupil, Pupil, QString)), dataWriter, SLOT (newStereoPupilData(quint64, Pupil, Pupil, QString)));
        } else {
            connect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, Pupil, QString)), dataWriter, SLOT (newPupilData(quint64, Pupil, QString)));
        }

        const QIcon recordOnIcon = QIcon(":/icons/Breeze/actions/22/kt-stop-all.svg"); //QIcon::fromTheme("camera-video");
        recordAct->setIcon(recordOnIcon);
        recordOn = true;
    }
}

void MainWindow::onRecordImageClick() {

    if(recordImagesOn) {
        // Deactivate recording

        // disconnect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), imageWriter, SLOT (onNewImage(CameraImage)));
        disconnect(signalPubSubHandler, SIGNAL(onNewGrabResult(CameraImage)), imageWriter, SLOT (onNewImage(CameraImage)));

        const QIcon recordOffIcon = QIcon(":/icons/Breeze/actions/22/media-record-blue.svg"); //QIcon::fromTheme("camera-video");
        recordImagesAct->setIcon(recordOffIcon);
        recordImagesOn = false;
    } else {
        // Activate recording
        const QString imageFormat = applicationSettings->value("writerFormat", generalSettingsDialog->getWriterFormat()).toString();

        bool stereo = selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE;
        imageWriter = new ImageWriter(outputDirectory, imageFormat, stereo, this);

        // connect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), signalPubSubHandler, SLOT (onNewImage(CameraImage)));
        connect(signalPubSubHandler, SIGNAL(onNewGrabResult(CameraImage)), imageWriter, SLOT (onNewImage(CameraImage)));

        const QIcon recordOnIcon = QIcon(":/icons/Breeze/actions/22/kt-stop-all.svg"); //QIcon::fromTheme("camera-video");
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
    }

    cameraViewWindow = nullptr;
    calibrationWindow = nullptr;

    if(trackingOn) {
        trackAct->setChecked(false);
        onTrackActClick();
    }

    if(recordOn) {
        onRecordClick();
    }

    if(recordImagesOn) {
        onRecordImageClick();
    }

    if(selectedCamera) {
        if(singleCameraSettingsDialog && (selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) ) {
            //onSingleCameraSettingsClick();
            singleCameraSettingsDialog->accept();
        } else if(selectedCamera && stereoCameraSettingsDialog && selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA) {
            //stereoCameraSelected();
            stereoCameraSettingsDialog->accept();
        }

        selectedCamera->close();
    }

    disconnect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL (onNewGrabResult(CameraImage)));
    disconnect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
    disconnect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

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

    cameraAct->setDisabled(false);
    cameraSettingsAct->setDisabled(true);
    cameraActDisconnectAct->setDisabled(true);
    calibrateAct->setDisabled(true);
    sharpnessAct->setDisabled(true);
    trackAct->setDisabled(true);
    playImageDirectoryAct->setDisabled(true);
    stopImageDirectoryAct->setDisabled(true);
    outputDirectoryAct->setDisabled(true);
    recordImagesAct->setDisabled(true);
    recordAct->setDisabled(true);
}

void MainWindow::singleCameraSelected(QAction *action) {

    QString deviceFullname = action->data().toString();

    try {
        selectedCamera = new SingleCamera(deviceFullname.toStdString().c_str(), this);
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;

        QMessageBox err(this);
        err.critical(this, "Device Error", e.GetDescription());

        return;
    }

    if(dynamic_cast<SingleCamera*>(selectedCamera)->getCameraCalibration()->isCalibrated())
        onCameraCalibrationEnabled();

    connect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL (onNewGrabResult(CameraImage)));
    connect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
    connect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

    connect(dynamic_cast<SingleCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
    connect(dynamic_cast<SingleCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));

    cameraViewClick();
    onSingleCameraSettingsClick();

    pupilDetectionWorker->setCamera(selectedCamera);
    pupilDetectionSettingsDialog->onSettingsChange();

    cameraAct->setDisabled(true);
    playImageDirectoryAct->setDisabled(true);
    stopImageDirectoryAct->setDisabled(true);

    cameraActDisconnectAct->setDisabled(false);
    cameraSettingsAct->setDisabled(false);
    calibrateAct->setDisabled(false);
    sharpnessAct->setDisabled(false);
    trackAct->setDisabled(false);
    logFileAct->setDisabled(false);
    outputDirectoryAct->setDisabled(false);
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

    stereoCameraSettingsDialog = new StereoCameraSettingsDialog(dynamic_cast<StereoCamera*>(selectedCamera), serialSettingsDialog, this);
    //auto *child = new RestorableQMdiSubWindow(childWidget, "StereoCameraSettingsDialog", this);
    stereoCameraSettingsDialog->show();

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

    cameraAct->setDisabled(true);
    playImageDirectoryAct->setDisabled(true);
    stopImageDirectoryAct->setDisabled(true);

    cameraActDisconnectAct->setDisabled(false);
    cameraSettingsAct->setDisabled(false);
    calibrateAct->setDisabled(false);
    trackAct->setDisabled(false);
    outputDirectoryAct->setDisabled(false);
    logFileAct->setDisabled(false);
}

void MainWindow::cameraViewClick() {

    if(cameraViewWindow && cameraViewWindow->isVisible()) {
        cameraViewWindow->close();
        delete cameraViewWindow;
    }

    if(selectedCamera && (selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA || selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) ) {
        SingleCameraView *childWidget = new SingleCameraView(selectedCamera, pupilDetectionWorker, this);
        connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), childWidget, SLOT (onSettingsChange()));

        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(childWidget, "SingleCameraView", this);
        //SingleCameraView *child = new SingleCameraView(selectedCamera, pupilDetectionWorker, this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        cameraViewWindow = child;
    } else if(selectedCamera && (selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE)) {
        StereoCameraView *childWidget = new StereoCameraView(selectedCamera, pupilDetectionWorker, this);
        connect(subjectSelectionDialog, SIGNAL (onSettingsChange()), childWidget, SLOT (onSettingsChange()));

        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(childWidget, "StereoCameraView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        cameraViewWindow = child;
    }
}

void MainWindow::onCameraSettingsClick() {

    if(selectedCamera && singleCameraSettingsDialog && (selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) ) {
        //onSingleCameraSettingsClick();
        singleCameraSettingsDialog->show();
    } else if(selectedCamera && stereoCameraSettingsDialog && selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA) {
        //stereoCameraSelected();
        stereoCameraSettingsDialog->show();
    }
}

void MainWindow::onSingleCameraSettingsClick() {

    singleCameraSettingsDialog = new SingleCameraSettingsDialog(dynamic_cast<SingleCamera*>(selectedCamera), serialSettingsDialog, this);
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

    ///mdiArea->addSubWindow(child);
    //child->show();
    //child->restoreGeometry();
    //connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
}

Pylon::DeviceInfoList_t MainWindow::enumerateCameraDevices() {

    Pylon::CTlFactory& TlFactory = Pylon::CTlFactory::GetInstance();
    Pylon::DeviceInfoList_t lstDevices;

    TlFactory.EnumerateDevices(lstDevices);

    return lstDevices;
}

MainWindow::~MainWindow() {
    pupilDetectionThread->quit();
    pupilDetectionThread->wait();
}

void MainWindow::onCalibrateClick() {

    if(calibrationWindow && calibrationWindow->isVisible()) {
        calibrationWindow->close();
        delete calibrationWindow;
    }

    if(selectedCamera && selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) {
        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new SingleCameraCalibrationView(dynamic_cast<SingleCamera*>(selectedCamera), this), "SingleCameraCalibrationView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        calibrationWindow = child;
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA) {
        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new StereoCameraCalibrationView(dynamic_cast<StereoCamera*>(selectedCamera), this), "StereoCameraCalibrationView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        calibrationWindow = child;
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new SingleFileCameraCalibrationView(dynamic_cast<FileCamera*>(selectedCamera), this), "SingleFileCameraCalibrationView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        calibrationWindow = child;
    } else if(selectedCamera && selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new StereoFileCameraCalibrationView(dynamic_cast<FileCamera*>(selectedCamera), this), "StereoFileCameraCalibrationView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
        connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
        calibrationWindow = child;
    }
}

void MainWindow::onSharpnessClick() {

    if(selectedCamera && selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) {
        RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(new SingleCameraSharpnessView(dynamic_cast<SingleCamera*>(selectedCamera), this), "SingleCameraSharpnessView", this);
        mdiArea->addSubWindow(child);
        child->show();
        child->restoreGeometry();
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

void MainWindow::dataTableClick() {

    bool stereoMode = false;
    if(selectedCamera && (selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE)) {
        stereoMode = true;
    }

    DataTable *childWidget = new DataTable(stereoMode,this);
    RestorableQMdiSubWindow *child = new RestorableQMdiSubWindow(childWidget, "DataTable", this);

    if(selectedCamera) {
        if(selectedCamera->getType() == CameraImageType::LIVE_SINGLE_CAMERA || selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
            connect(pupilDetectionWorker, SIGNAL(processedPupilData(quint64, Pupil, QString)), childWidget, SLOT(onPupilData(quint64, Pupil, QString)));
        } else if(selectedCamera->getType() == CameraImageType::LIVE_STEREO_CAMERA || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
            connect(pupilDetectionWorker, SIGNAL(processedStereoPupilData(quint64, Pupil, Pupil, QString)), childWidget, SLOT(onStereoPupilData(quint64, Pupil, Pupil, QString)));
        }
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

    QWidget *childWidget = new GraphPlot(value, pupilDetectionWorker->isStereo(), false, this);
    auto *child = new RestorableQMdiSubWindow(childWidget, "GraphPlot_"+value, this);


    // switch values to connect different slots/signals to GraphPlot and the data
    if(value == DataTable::FRAME_NUMBER) {

        connect(signalPubSubHandler, SIGNAL (cameraFramecount(int)), childWidget, SLOT (appendData(int)));
    } else if(value == DataTable::CAMERA_FPS) {

        connect(signalPubSubHandler, SIGNAL (cameraFPS(double)), childWidget, SLOT (appendData(double)));
    } else if(value == DataTable::PUPIL_FPS) {

        connect(pupilDetectionWorker, SIGNAL (fps(double)), childWidget, SLOT (appendData(double)));
    } else {
        if(pupilDetectionWorker->isStereo()) {
            connect(pupilDetectionWorker, SIGNAL (processedStereoPupilData(quint64, Pupil, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, Pupil, QString)));
        } else {
            connect(pupilDetectionWorker, SIGNAL (processedPupilData(quint64, Pupil, QString)), childWidget, SLOT (appendData(quint64, Pupil, QString)));
        }
    }
    mdiArea->addSubWindow(child);
    child->show();
    child->restoreGeometry();
    connect(child, SIGNAL (onCloseSubWindow()), this, SLOT (updateWindowMenu()));
}

void MainWindow::onOpenImageDirectory() {
    imageDirectory = QFileDialog::getExistingDirectory(this, tr("Image Directory"), recentPath);

    if(!imageDirectory.isEmpty()) {

        recentPath = imageDirectory;

        QStringList lst = recentPath.split('/');
        currentDirectoryLabel->setText("Current directory: .../" + lst[lst.count()-3] + "/" + lst[lst.count()-2] + "/" + lst[lst.count()-1]);
        currentDirectoryLabel->setToolTip(recentPath);

        if(selectedCamera) {
            selectedCamera->close();
        }
        onCameraCalibrationDisabled();

        cameraAct->setDisabled(true);
        cameraSettingsAct->setDisabled(true);
        outputDirectoryAct->setDisabled(true);

        calibrateAct->setDisabled(false);
        playImageDirectoryAct->setDisabled(false);
        stopImageDirectoryAct->setDisabled(false);

        trackAct->setDisabled(false);
        cameraActDisconnectAct->setDisabled(false);
        logFileAct->setDisabled(false);

        const int playbackSpeed = applicationSettings->value("playbackSpeed", generalSettingsDialog->getPlaybackSpeed()).toInt();
        const bool playbackLoop = (bool) applicationSettings->value("playbackLoop", (int) generalSettingsDialog->getPlaybackLoop()).toInt();

        // create simulated FileCamera
        selectedCamera = new FileCamera(imageDirectory, playbackSpeed, playbackLoop, this);
        std::cout<<"FileCamera created using playbackspeed [fps]: "<<playbackSpeed <<std::endl;

        connect(selectedCamera, SIGNAL(finished()), this, SLOT(onPlayImageDirectoryFinished()));

        connect(selectedCamera, SIGNAL (onNewGrabResult(CameraImage)), signalPubSubHandler, SIGNAL (onNewGrabResult(CameraImage)));
        connect(selectedCamera, SIGNAL(fps(double)), signalPubSubHandler, SIGNAL(cameraFPS(double)));
        connect(selectedCamera, SIGNAL(framecount(int)), signalPubSubHandler, SIGNAL(cameraFramecount(int)));

        cameraViewClick();

        if(selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
            connect(dynamic_cast<FileCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
            connect(dynamic_cast<FileCamera*>(selectedCamera)->getCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));
        } else if(selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
            connect(dynamic_cast<FileCamera*>(selectedCamera)->getStereoCameraCalibration(), SIGNAL (finishedCalibration()), this, SLOT (onCameraCalibrationEnabled()));
            connect(dynamic_cast<FileCamera*>(selectedCamera)->getStereoCameraCalibration(), SIGNAL (unavailableCalibration()), this, SLOT (onCameraCalibrationDisabled()));
        }
        onCalibrateClick();

        // Basically only that pupilDetectionSettingsDialog knows which type of camera is connected
        pupilDetectionWorker->setCamera(selectedCamera);
        pupilDetectionSettingsDialog->onSettingsChange();

        // Dirty fix to display the first frame preview in the camera view window before pushing play (i.e. for setting ROI etc.)
        // Problem is that Filecamera only sends signals after play click/start
        dynamic_cast<FileCamera*>(selectedCamera)->start();
        QThread::msleep(5);
        dynamic_cast<FileCamera*>(selectedCamera)->stop();
    }
}

void MainWindow::onPlayImageDirectoryClick() {

    if(playImagesOn) {
        std::cout<<"Pausing FileCamera Click"<<std::endl;
        dynamic_cast<FileCamera*>(selectedCamera)->pause();

        const QIcon icon = QIcon(":/icons/Breeze/actions/22/media-playback-start.svg");
        playImageDirectoryAct->setIcon(icon);
        playImagesOn = false;
    } else {
        std::cout<<"Starting FileCamera Click"<<std::endl;
        dynamic_cast<FileCamera*>(selectedCamera)->start();

        const QIcon icon = QIcon(":/icons/Breeze/actions/22/media-playback-pause.svg");
        playImageDirectoryAct->setIcon(icon);
        playImagesOn = true;
    }
}

void MainWindow::onStopImageDirectoryClick() {

    std::cout<<"Stopping FileCamera Click"<<std::endl;
    dynamic_cast<FileCamera*>(selectedCamera)->stop();

    // Trick to show first frame again
    dynamic_cast<FileCamera*>(selectedCamera)->start();
    QThread::msleep(5);
    dynamic_cast<FileCamera*>(selectedCamera)->stop();

    const QIcon icon = QIcon(":/icons/Breeze/actions/22/media-playback-start.svg");
    playImageDirectoryAct->setIcon(icon);
    playImagesOn = false;
}

void MainWindow::onPlayImageDirectoryFinished() {

    if(playImagesOn) {
        std::cout<<"FileCamera Finished."<<std::endl;

        const QIcon icon = QIcon(":/icons/Breeze/actions/22/media-playback-start.svg");
        playImageDirectoryAct->setIcon(icon);
        playImagesOn = false;
    }
}

void MainWindow::onSerialConnect() {
    const QIcon offlineIcon = QIcon(":icons/Breeze/emblems/22/vcs-normal.svg");
    serialStatusIcon->setPixmap(offlineIcon.pixmap(12, 12));
}

void MainWindow::onSerialDisconnect() {
    const QIcon offlineIcon = QIcon(":icons/Breeze/actions/22/media-record.svg");
    serialStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));
}

void MainWindow::onHwTriggerEnable() {
    hwTriggerOn = true;
    const QIcon offlineIcon = QIcon(":icons/Breeze/emblems/22/vcs-normal.svg");
    hwTriggerStatusIcon->setPixmap(offlineIcon.pixmap(12, 12));
}

void MainWindow::onHwTriggerDisable() {
    hwTriggerOn = false;
    const QIcon offlineIcon = QIcon(":icons/Breeze/actions/22/media-record.svg");
    hwTriggerStatusIcon->setPixmap(offlineIcon.pixmap(16, 16));
}

void MainWindow::onCameraCalibrationEnabled() {
    const QIcon calibIcon = QIcon(":icons/Breeze/emblems/22/vcs-normal.svg");
    calibrationStatusIcon->setPixmap(calibIcon.pixmap(12, 12));
}

void MainWindow::onCameraCalibrationDisabled() {
    const QIcon calibIcon = QIcon(":icons/Breeze/actions/22/media-record.svg");
    calibrationStatusIcon->setPixmap(calibIcon.pixmap(16, 16));
}

void MainWindow::onGeneralSettingsChange() {
    if(selectedCamera) {
        if(selectedCamera->getType() == CameraImageType::SINGLE_IMAGE_FILE || selectedCamera->getType() == CameraImageType::STEREO_IMAGE_FILE) {

            const int playbackSpeed = applicationSettings->value("playbackSpeed", generalSettingsDialog->getPlaybackSpeed()).toInt();

            if(dynamic_cast<FileCamera*>(selectedCamera)->getPlaybackSpeed() != playbackSpeed) {
                dynamic_cast<FileCamera*>(selectedCamera)->setPlaybackSpeed(playbackSpeed);
            }

            const bool playbackLoop = (bool) applicationSettings->value("playbackLoop", (int) generalSettingsDialog->getPlaybackLoop()).toInt();

            if(dynamic_cast<FileCamera*>(selectedCamera)->getPlaybackLoop() != static_cast<int>(playbackLoop)) {
                dynamic_cast<FileCamera*>(selectedCamera)->setPlaybackLoop(playbackLoop);
            }
        }
    }

}

void MainWindow::onSubjectsSettingsChange(QString subject) {

    subjectConfigurationLabel->setText("Current configuration: " + subject);
}

void MainWindow::onSubjectsClick() {
    const QIcon subjectsSelectedIcon = QIcon(":/icons/Breeze/actions/22/im-user-online.svg"); //QIcon::fromTheme("camera-video");
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

