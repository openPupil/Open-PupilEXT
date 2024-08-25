#pragma once

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include "subwindows/MCUSettingsDialog.h"
#include "subwindows/singleCameraSettingsDialog.h"
#include "devices/singleCamera.h"
#include "devices/stereoCamera.h"
#include "subwindows/pupilDetectionSettingsDialog.h"
#include "subwindows/singleCameraView.h"
#include "dataWriter.h"
#include "imageWriter.h"
#include "subwindows/generalSettingsDialog.h"
#include "subwindows/subjectSelectionDialog.h"
#include "subwindows/stereoCameraSettingsDialog.h"
#include "subwindows/RestorableQMdiSubWindow.h"
#include "signalPubSubHandler.h"
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QSettings>
#include <pylon/TlFactory.h>

#include "supportFunctions.h"
#include "metaSnapshotOrganizer.h"
#include "dataStreamer.h"
#include "camTempMonitor.h"
#include "subwindows/imagePlaybackControlDialog.h"
#include "subwindows/remoteCCDialog.h"
#include "subwindows/streamingSettingsDialog.h"
#include "connPoolCOM.h"
#include "connPoolUDP.h"
#include "devices/singleWebcam.h"
#include "subwindows/singleWebcamSettingsDialog.h"
#include "subwindows/singleWebcamCalibrationView.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include "recEventTracker.h"
#include "SVGIconColorAdjuster.h"
#include "playbackSynchroniser.h"
#include "dataTypes.h"
#include "subwindows/sceneImageView.h"
#include "subwindows/gettingStartedWizard.h"
//#include <QtMultimedia/QCameraInfo>


/**
    Main interface of the software

    Creates all GUI and processing objects and handles/connects their signal-slot connections

    Creates threads for concurrent processing for i.e. calibration and pupil detection
*/
class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    static int const EXIT_CODE_REBOOT;

    MainWindow();
    ~MainWindow() override;

protected:

    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event);
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);

private:
 
    SignalPubSubHandler *signalPubSubHandler;

    QSettings *applicationSettings;
    QDir settingsDirectory;

    QString pupilDetectionDataFile;
    QString outputDirectory;
    QString imageDirectory;
    QString recentPath; 

    QMdiArea *mdiArea;
    QToolBar *toolBar;

    RestorableQMdiSubWindow *calibrationWindow; 
    RestorableQMdiSubWindow *cameraViewWindow;
    RestorableQMdiSubWindow *sharpnessWindow;
    RestorableQMdiSubWindow *dataTableWindow;
    RestorableQMdiSubWindow *sceneImageWindow;

    QIcon fileOpenIcon;
    QIcon cameraSerialConnectionIcon;
    QIcon pupilDetectionSettingsIcon;
    QIcon remoteCCIcon;
    QIcon generalSettingsIcon;
    QIcon singleCameraIcon;
    QIcon stereoCameraIcon;
    QIcon cameraSettingsIcon1;
    QIcon cameraSettingsIcon2;
    QIcon calibrateIcon;
    QIcon sharpnessIcon;
    QIcon subjectsIcon;
    QIcon outputDataFileIcon;
    QIcon streamingSettingsIcon;
    QIcon imagePlaybackControlIcon;
    QIcon dataTableIcon;
    QIcon sceneImageViewIcon;

    QMenu *windowMenu;
    QMenu *baslerCamerasMenu;
//    QMenu *openCVCamerasMenu;

    QAction *cameraViewAct;
    QAction *dataTableAct;
    QAction *sceneImageViewAct;

    QAction *cameraAct;
    QAction *cameraSettingsAct;
    QAction* cameraActDisconnectAct;
    QAction *trackAct;
    QAction *recordAct;
    QAction *calibrateAct;
    QAction *logFileAct;
    QAction *outputDirectoryAct;
    QAction *recordImagesAct;

//    QAction *closeAct;
//    QAction *closeAllAct;
//    QAction *tileAct;
    QAction *cascadeAct;
    QAction *resetGeometryAct;

    QAction *nextAct;
    QAction *previousAct;
    QAction *windowMenuSeparatorAct;
    QAction *subjectsAct;
    QAction *sharpnessAct;

    QLabel *serialStatusIcon;
    QLabel *hwTriggerStatusIcon;
    QLabel *warmedUpStatusIcon;
    QLabel *calibrationStatusIcon;
    QLabel *subjectConfigurationLabel;
    QLabel *currentStatusMessageLabel;
    
    // TODO: Move trackingOn into class instance, and get rid of others, use nullptr check instead. better like that I think. Also
    bool trackingOn = false; // NOTE: also accessible in pupildetection now
    bool recordOn = false;
    bool recordImagesOn = false;
    //bool playImagesOn = false; // NOTE: from now can be checked via ImagePlaybackControlDialog
    bool hwTriggerOn = false;
    bool cameraPlaying = true;

    void loadIcons();
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();

    QWidget* activeMdiChild() const;

    static Pylon::DeviceInfoList_t enumerateCameraDevices();

    Camera *selectedCamera;

    PupilDetection *pupilDetectionWorker;
    QThread *pupilDetectionThread;

    DataWriter *dataWriter;
    ImageWriter *imageWriter;

    bool streamOn = false;

    // TODO: These are all dialogs that get opened DIRECTLY as a child of main window (not inside an MDI subwindow).
    //  Currently, on MacOS, they can be erroneously occluded by the main window once they lose focus.
    //  This is probably a Qt bug.
    // TODO: Also, always only one of them can exist at once, so it could be good to rethink their
    //  instantiation and memory management.
    MCUSettingsDialog *MCUSettingsDialogInst;
    PupilDetectionSettingsDialog *pupilDetectionSettingsDialog;
    GeneralSettingsDialog *generalSettingsDialog;
    SubjectSelectionDialog *subjectSelectionDialog;
    SingleCameraSettingsDialog *singleCameraSettingsDialog;
    StereoCameraSettingsDialog *stereoCameraSettingsDialog;
    RemoteCCDialog *remoteCCDialog;
    StreamingSettingsDialog *streamingSettingsDialog;
    SingleWebcamSettingsDialog *singleWebcamSettingsDialog;

    ImagePlaybackControlDialog *imagePlaybackControlDialog;

    // made these two global to be able to pass singlecameraview instance pointer to ...CameraSettingsDialog constructors:
    SingleCameraView *singleCameraChildWidget; 
    StereoCameraView *stereoCameraChildWidget;

    //QThread *tempMonitorThread;
    CamTempMonitor *camTempMonitor;
    RecEventTracker *recEventTracker;
    quint64 imageRecStartTimestamp;

    ConnPoolCOM *connPoolCOM;
    ConnPoolUDP *connPoolUDP;

    QSpinBox *webcamDeviceBox;

    QAction *fileOpenAct; // GB: made global to let it disable when image directory is already open
    QAction *toggleFullscreenAct;
    QAction *streamingSettingsAct;
    QAction *streamAct;

    QAction *forceResetTrialAct;
    QAction *manualIncTrialAct;
    QAction *forceResetMessageAct;

    QWidget *trialWidget;
    QLabel *currentTrialLabel;
    QFrame* trialWidgetLayoutSep;
    QWidget *messageWidget;
    QLabel *currentMessageLabel;
    QFrame* messageWidgetLayoutSep;
    QLabel *remoteStatusIcon;

    GettingStartedWizard* aboutWizard = nullptr;
    GettingStartedWizard* aboutAndUserGuideWizard = nullptr;
    GettingStartedWizard* userGuideWizard = nullptr;

    DataStreamer *dataStreamer;
    QMutex *imageMutex;
    QWaitCondition *imagePublished;
    QWaitCondition *imageProcessed;

    PlaybackSynchroniser *playbackSynchroniser;

    QMessageBox *imagesSkippedMsgBox = nullptr;

    void loadCalibrationWindow();
    void loadSharpnessWindow();
    void loadDataTableWindow();
    void loadSceneImageWindow();

    void stopCamera();
    void startCamera();

    void resetStatus(bool isConnect);

    void openImageDirectory(QString imageDirectory);
    void setRecentPath(QString path);

    void connectCameraPlaybackChangedSlots();

private slots:

    void onSerialConnect();
    void onSerialDisconnect();

    void onHwTriggerEnable();
    void onHwTriggerDisable();

    void onDeviceWarmupHasDeltaTimeData();
    void onDeviceWarmedUp();
    void onDeviceWarmedUpReset();

    void onWebcamStartedToOpen();
    void onWebcamCouldNotBeOpened();
    void onWebcamSuccessfullyOpened();

    void onCameraCalibrationEnabled();
    void onCameraCalibrationDisabled();

    void onOpenImageDirectory();

    void onCameraClick();
    void onCameraDisconnectClick();
    void onCameraSettingsClick();
    void onSingleCameraSettingsClick();
    void onStereoCameraSettingsClick();

    void onCalibrateClick();
    void onSubjectsClick();

    void onTrackActClick();
    void onRecordClick();

    void onGeneralSettingsChange();

    void onPupilDetectionProcModeChange(int);

    void cameraViewClick();

    void onRecordImageClick();

    void singleCameraSelected(QAction *action);
    void stereoCameraSelected();

    void onCreateGraphPlot(const DataTypes::DataType &value);

    void dataTableClick();
    void sceneImageViewClick();
    void toggleFullscreen();

    void setLogFile();
    void setOutputDirectory();

    void updateMenus();
    void updateBaslerCamerasMenu();
//    void updateOpenCVCamerasMenu();
    void updateWindowMenu();
    void about();
    void userGuide();
    void openSourceDialog();
    void resetGeometry();
//    void closeActiveSubWindow();
//    void closeAllSubWindows();

    void onSubjectsSettingsChange(QString subject);
    void onSharpnessClick();

    void offerResetApplicationSettings();
    void offerRestartApplication();

//    void onGettingsStartedWizardFinish();

    void singleWebcamSelected(QAction *action);
    void onSingleWebcamSettingsClick();

    void onStreamClick();
    void onStreamingSettingsClick();

    void onPlaybackStartInitiated();
    void onPlaybackPauseInitiated();
    void onPlaybackStopInitiated();

    void onRemoteConnStateChanged();
    //void onStreamingConnStateChanged();

    void updateCurrentTrialLabel();
    void safelyResetTrialCounter();
    void safelyResetTrialCounter(const quint64 &timestamp);
    void forceResetTrialCounter();
    void forceResetTrialCounter(const quint64 &timestamp);
    void incrementTrialCounter();
    void incrementTrialCounter(const quint64 &timestamp);
    void logRemoteMessage(const quint64 &timestamp, const QString &str);
    void updateCurrentMessageLabel();
    void safelyResetMessageRegister();
    void safelyResetMessageRegister(const quint64 &timestamp);
    void forceResetMessageRegister();
    void forceResetMessageRegister(const quint64 &timestamp);

    void onStreamingUDPConnect();
    void onStreamingUDPDisconnect();
    void onStreamingCOMConnect();
    void onStreamingCOMDisconnect();

    void onImagesSkipped();
    void onImagesSkippedMsgClose();

    void createCamTempMonitor();
    void destroyCamTempMonitor();

public slots:

    // NOTE: definitions of functions for programmatic control of GUI elements (their names beginning with PRG...)
    // are stored in PRGmainwindow.cpp, to keep mainwindow.cpp cleaner
    void PRGlogRemoteMessage(const quint64 &timestamp, const QString &str);

    void PRGopenSingleCamera(const QString &camName);
    void PRGopenStereoCamera(const QString &camName1, const QString &camName2);
    void PRGopenSingleWebcam(int deviceID);
    void PRGcloseCamera();
    void PRGtrackStart();
    void PRGtrackStop();
    void PRGrecordStart();
    void PRGrecordStop();
    void PRGrecordImageStart();
    void PRGrecordImageStop();
    void PRGstreamStart();
    void PRGstreamStop();
    void PRGincrementTrialCounter(const quint64 &timestamp);
    void PRGforceResetTrialCounter(const quint64 &timestamp);
    // NOTE: there is no programmatic implementation for resetting the message register. It can be done by sending a blank message
    void PRGsetOutPath(const QString &str);
    void PRGsetCsvPathAndName(const QString &str);
    
    void PRGsetGlobalDelimiter(const QString &str);
    void PRGsetImageOutputFormat(QString format);
    void PRGsetPupilDetectionAlgorithm(const QString &alg);
    void PRGsetPupilDetectionUsingROI(const QString &state);
    void PRGsetPupilDetectionCompOutlineConf(const QString &state);
    void PRGconnectRemoteUDP(QString conf);
    void PRGconnectRemoteCOM(QString conf);
    void PRGconnectStreamUDP(QString conf);
    void PRGconnectStreamCOM(QString conf);
    void PRGconnectMicrocontrollerUDP(QString conf);
    void PRGconnectMicrocontrollerCOM(QString conf);
    void PRGdisconnectRemoteUDP();
    void PRGdisconnectRemoteCOM();
    void PRGdisconnectStreamUDP();
    void PRGdisconnectStreamCOM();
    void PRGdisconnectMicrocontroller();

    void PRGenableHWT(bool state);
    void PRGstartHWT();
    void PRGstopHWT();
    void PRGsetHWTlineSource(int lineSourceNum);
    void PRGsetHWTruntime(float runtimeMinutes);
    void PRGsetHWTframerate(int fps);
    void PRGenableSWTframerateLimiting(const QString &state);
    void PRGsetSWTframerate(int fps);

    void PRGsetExposure(int value);
    void PRGsetGain(double value);

    void onCameraFreezePressed();

    void onCameraPlaybackChanged();

    void onCameraUnexpectedlyDisconnected();

    void onStereoCamerasOpened();
    void onStereoCamerasClosed();

signals:
    void commitTrialCounterIncrement(quint64 timestamp);
    void commitTrialCounterReset(quint64 timestamp);
    void commitRemoteMessage(quint64 timestamp, QString str);
    void commitMessageRegisterReset(quint64 timestamp);

    void cameraPlaybackChanged();

    void playbackStartApproved();
    void playbackPauseApproved();
    void playbackStopApproved();

};
