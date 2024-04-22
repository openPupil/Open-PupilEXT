#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "subwindows/serialSettingsDialog.h"
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

// GB added begin
#include "supportFunctions.h"
#include "metaSnapshotOrganizer.h"
#include "dataStreamer.h"
#include "camTempMonitor.h"
#include "subwindows/imagePlaybackControlDialog.h"
#include "subwindows/remoteCCDialog.h"
#include "subwindows/streamingSettingsDialog.h"
#include "connPoolCOM.h"
#include "devices/singleWebcam.h"
#include "subwindows/singleWebcamSettingsDialog.h"
#include "subwindows/singleWebcamCalibrationView.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
//#include <QCameraInfo> // Qt 5.3 would be necessary. For OpenCV device enumeration
#include "recEventTracker.h"
#include "SVGIconColorAdjuster.h"
#include "playbackSynchroniser.h"
// GB added end


/**
    Main interface of the software

    Creates all GUI and processing objects and handles/connects their signal-slot connections

    Creates threads for concurrent processing for i.e. calibration and pupil detection

    NOTE: Modified by Gábor Bényei, 2023 jan
    GB NOTE:
        Moved image playback button functionality into a new dialog called ImagePlaybackSettingsDialog.
            Thus, onPlayImageDirectoryClick(), onPlayImageDirectoryFinished(), 
            and onStopImageDirectoryClick() were removed.
        Added trial counter label in statusbar, which can only be seen if a physical camera device is opened.
        Added manual forced reset and manual increment buttons for trial counter using Settings menu.
        Added streaming settings action and its dialog, accessible when a device is opened. Streaming can be 
            started if a streaming connection is alive, and GUI is kept enabled accordingly.
        Recording now necessitates pupil detection tracking to be going on, this way it is safer to handle
            (e.g. new procMode functionality allows pupilDetection output to change, and this should not happen 
        while data is being written. Each line should keep consistency to the hearder of the csv instead).
        Many minor changes were made, not mentioned here, but commented on the spot.
*/
class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    MainWindow();
    ~MainWindow() override;

protected:

    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event);
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);

private:
 
    SignalPubSubHandler *signalPubSubHandler;

    QSettings *applicationSettings;
    QDir settingsDirectory;

    SerialSettingsDialog *serialSettingsDialog;
    PupilDetectionSettingsDialog *pupilDetectionSettingsDialog;
    GeneralSettingsDialog *generalSettingsDialog;
    SubjectSelectionDialog *subjectSelectionDialog;

    SingleCameraSettingsDialog *singleCameraSettingsDialog;
    StereoCameraSettingsDialog *stereoCameraSettingsDialog;

    QString pupilDetectionDataFile;
    QString outputDirectory;
    QString imageDirectory;
    QString recentPath; 

    QMdiArea *mdiArea;
    QToolBar *toolBar;

    RestorableQMdiSubWindow *calibrationWindow; 
    RestorableQMdiSubWindow *cameraViewWindow;
    RestorableQMdiSubWindow *sharpnessWindow;

    QMenu *windowMenu;
    QMenu *singleCameraDevicesMenu; // GB: refactored name

    QAction *cameraAct;
    QAction *cameraSettingsAct;
    QAction* cameraActDisconnectAct;
    QAction *trackAct;
    QAction *recordAct;
    QAction *calibrateAct;
    QAction *logFileAct;
    QAction *outputDirectoryAct;
    QAction *recordImagesAct;

    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *resetGeometryAct;

    QAction *nextAct;
    QAction *previousAct;
    QAction *windowMenuSeparatorAct;
    QAction *subjectsAct;
    QAction *sharpnessAct;

    QLabel *serialStatusIcon;
    QLabel *hwTriggerStatusIcon;
    QLabel *calibrationStatusIcon;
    QLabel *subjectConfigurationLabel;
    QLabel *currentDirectoryLabel;
    
    // GB: TODO: Move trackingOn into class instance, and get rid of others, use nullptr check instead. better like that I think. Also 
    bool trackingOn = false; // GB: also accessible in pupildetection now
    bool recordOn = false;
    bool recordImagesOn = false;
    //bool playImagesOn = false; // GB: from now can be checked via ImagePlaybackControlDialog
    bool hwTriggerOn = false;
    bool cameraPlaying = true;

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

    // GB added begin
    bool streamOn = false;
    bool remoteOn = false;

    RemoteCCDialog *remoteCCDialog;
    StreamingSettingsDialog *streamingSettingsDialog;
    ImagePlaybackControlDialog *imagePlaybackControlDialog;

    // GB NOTE: made these two global to be able to pass singlecameraview instance pointer to ...CameraSettingsDialog constructors:
    SingleCameraView *singleCameraChildWidget; 
    StereoCameraView *stereoCameraChildWidget;
    
    SingleWebcamSettingsDialog *singleWebcamSettingsDialog;

    //QThread *tempMonitorThread;
    CamTempMonitor *camTempMonitor;
    RecEventTracker *recEventTracker;
    quint64 imageRecStartTimestamp;

    ConnPoolCOM *connPoolCOM;

    QSpinBox *webcamDeviceBox;

    QAction *fileOpenAct; // GB: made global to let it disable when image directory is already open
    QAction *streamingSettingsAct;
    QAction *streamAct;

    QAction *forceResetTrialAct;
    QAction *manualIncTrialAct;

    QWidget *trialWidget;
    QLabel *currentTrialLabel;
    QLabel *remoteStatusIcon;

    DataStreamer *dataStreamer;
    QMutex *imageMutex;
    QWaitCondition *imagePublished;
    QWaitCondition *imageProcessed;

    PlaybackSynchroniser *playbackSynchroniser;


    void loadCalibrationWindow();
    void loadSharpnessWindow();

    void stopCamera();
    void startCamera();

    void resetStatus(bool isConnect);

    void openImageDirectory(QString imageDirectory);

    void connectCameraPlaybackChangedSlots();
    // GB added end

private slots:

    void onSerialConnect();
    void onSerialDisconnect();

    void onHwTriggerEnable();
    void onHwTriggerDisable();

    void onCameraCalibrationEnabled();
    void onCameraCalibrationDisabled();

    void onOpenImageDirectory();

    void onCameraClick();
    void onCameraDisconnectClick();
    void onCameraSettingsClick();
    void onSingleCameraSettingsClick();

    void onCalibrateClick();
    void onSubjectsClick();

    void onTrackActClick();
    void onRecordClick();

    void onGeneralSettingsChange();

    void cameraViewClick();

    void onRecordImageClick();

    void singleCameraSelected(QAction *action);
    void stereoCameraSelected();

    void onCreateGraphPlot(const QString &value);

    void dataTableClick();

    void setLogFile();
    void setOutputDirectory();

    void updateMenus();
    void updateCameraMenu();
    void updateWindowMenu();
    void about();
    void openSourceDialog();
    void resetGeometry();

    void onSubjectsSettingsChange(QString subject);
    void onSharpnessClick();

    void onGettingsStartedWizardFinish();

    // GB added begin
    void singleWebcamSelected();
    void onSingleWebcamSettingsClick();

    void onStreamClick();
    void onStreamingSettingsClick();

    void onRemoteEnable();
    void onRemoteDisable();

    void onPlaybackSafelyStarted();
    void onPlaybackSafelyPaused();
    void onPlaybackSafelyStopped();

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

    void onStreamingUDPConnect();
    void onStreamingUDPDisconnect();
    void onStreamingCOMConnect();
    void onStreamingCOMDisconnect();
    // GB added end

public slots:

    // GB added begin
    // GB NOTE: definitions of functions for programmatic control of GUI elements (their names beginning with PRG...)
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
    void PRGdisconnectRemoteUDP();
    void PRGdisconnectRemoteCOM();
    void PRGdisconnectStreamUDP();
    void PRGdisconnectStreamCOM();

    void onCameraFreezePressed();

    void onCameraPlaybackChanged();
    // GB added end

signals:
    void commitTrialCounterIncrement(quint64 timestamp);
    void commitTrialCounterReset(quint64 timestamp);
    void commitRemoteMessage(quint64 timestamp, QString str);

    void cameraPlaybackChanged();

};

#endif // MAINWINDOW_H
