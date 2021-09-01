#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
    @author Moritz Lode
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


/**
    Main interface of the software

    Creates all GUI and processing objects and handles/connects their signal-slot connections

    Creates threads for concurrent processing for i.e. calibration and pupil detection
*/
class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    MainWindow();
    ~MainWindow() override;

protected:

    void closeEvent(QCloseEvent *event) override;

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

    QString logFileName;
    QString outputDirectory;
    QString imageDirectory;
    QString recentPath;

    QMdiArea *mdiArea;
    QToolBar *toolBar;

    RestorableQMdiSubWindow *calibrationWindow;
    RestorableQMdiSubWindow *cameraViewWindow;


    QMenu *windowMenu;
    QMenu *singleCameraDevicesMenu;

    QAction *cameraAct;
    QAction *cameraSettingsAct;
    QAction* cameraActDisconnectAct;
    QAction *trackAct;
    QAction *recordAct;
    QAction *calibrateAct;
    //QAction *focusAct;
    QAction *logFileAct;
    QAction *outputDirectoryAct;
    QAction *recordImagesAct;
    QAction *stopImageDirectoryAct;


    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *resetGeometryAct;

    QAction *nextAct;
    QAction *previousAct;
    QAction *windowMenuSeparatorAct;
    QAction *playImageDirectoryAct;
    QAction *subjectsAct;
    QAction *sharpnessAct;

    QLabel *serialStatusIcon;
    QLabel *hwTriggerStatusIcon;
    QLabel *calibrationStatusIcon;
    QLabel *subjectConfigurationLabel;
    QLabel *currentDirectoryLabel;

    bool trackingOn = false;
    bool recordOn = false;
    bool recordImagesOn = false;
    bool playImagesOn = false;
    bool hwTriggerOn = false;

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

private slots:

    void onSerialConnect();
    void onSerialDisconnect();

    void onHwTriggerEnable();
    void onHwTriggerDisable();

    void onCameraCalibrationEnabled();
    void onCameraCalibrationDisabled();

    void onOpenImageDirectory();
    void onPlayImageDirectoryClick();
    void onPlayImageDirectoryFinished();
    void onStopImageDirectoryClick();

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
};

#endif // MAINWINDOW_H
