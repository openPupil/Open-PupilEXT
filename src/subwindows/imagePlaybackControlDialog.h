#pragma once

/**
    @author Gabor Benyei, Attila Boncser
*/

#include <QtWidgets/QWidget>

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QSettings>

#include "playbackDial.h"
#include "playbackSlider.h"
#include "../devices/fileCamera.h"
#include "../pupilDetection.h"

#include "../recEventTracker.h"
#include "timestampSpinBox.h"

#include <QKeyEvent> // DEV

/**
    This dialog gives the opportunity of playing and stopping image playback for ths user.
    It also shows basic information about the currently played/displayed frame.
    There is a dial widget for stepping frame-by-frame, if the user would like to inspect 
    interesting frames more closely. On the slider a green tick shows the position of file read, 
    (that last went into pupilDetection) and the slider handle marks the actually displayed image position
    (that just arrived from pupilDetection).

    This dialog is designed to be only visible and interactable while an image directory is opened.
*/


class ImagePlaybackControlDialog : public QWidget {
    Q_OBJECT

public:

    explicit ImagePlaybackControlDialog(FileCamera *fileCamera, PupilDetection *pupilDetection, RecEventTracker *recEventTracker, QWidget *parent = nullptr);
    ~ImagePlaybackControlDialog() override;

    bool getSyncRecordCsv();
    bool getSyncStream();

    bool getPlayImagesOn();

private:

    RecEventTracker *recEventTracker;

    QSettings *applicationSettings;

    FileCamera *fileCamera;

    PupilDetection *pupilDetection;

    bool playImagesOn = false;

    
    int playbackSpeed = 1;
    int selectedFrameVal = 1;
    int lastPlayedFrame = 0;
    bool playbackLoop = false;
    bool syncRecordCsv = false;
    bool tempSyncRecordCsv = false;
    bool syncStream = false;
    bool tempSyncStream = false;
    bool endReached = false;
    bool finished = false;
    bool paused = true;

    QSpinBox* playbackFPSVal;

    QGroupBox *infoGroup;
    //QLineEdit *timestampVal;
    TimestampSpinBox *timestampVal;
    QSpinBox *selectedFrameBox;
    QLabel *timestampHumanValLabel;
    QLabel *numImagesTotalLabel;
    QLabel *elapsedTimeValLabel;
    QLabel *acqFPSValLabel;
    QLabel *percentValLabel;
    QLabel *trialValLabel;
    QLabel *messageValLabel;

    int numImagesTotal = -1;
    uint64_t lastTimestamp = 0;
    uint64_t startTimestamp = 0;
    QTime timeTotal;
    uint64_t elapsedMs = 0.0;
    float acqFPS = 0.0;

    
    PlaybackSlider *slider;
    PlaybackDial *dial;

    QPushButton *startPauseButton;
    QPushButton *stopButton;

    QCheckBox *loopBox;
    QCheckBox *syncRecordCsvBox;
    QCheckBox *syncStreamBox;

    void createForm();
    void enableWidgets();
    void disableWidgets();
//    void resetState();
 //   void saveUniversalSettings();
 //   void updateForm();

public slots:

    void updateInfo(quint64 timestamp, int frameNumber);
    void updateInfo(const CameraImage &img);
    
    void onAutomaticFinish();
    
    void readSettings();
    //void saveUniversalSettings();
    void updateForm();

    void onFrameSelected(int frameNumber);
    void onTimestampSelected(double frameNumber);
    void onCameraPlaybackChanged();

    void onPlaybackStartApproved();
    void onPlaybackPauseApproved();
    void onPlaybackStopApproved();

private slots:
    void onStartPauseButtonClick();
    void onStopButtonClick();
    void onEndReached();
    void onFinished();

    
    void updateInfoInternal(int frameNumber);

    // not needed anymore, as signals always go through pupildetection instance
//    void onPupilDetectionStart();
//    void onPupilDetectionStop();

    void onDialForward();
    void onDialBackward();
    void onSliderValueChanged(int val);
    void updateSliderColorTick(const CameraImage &cimg);

    
    void setPlaybackSpeed(int playbackSpeed);
    void setPlaybackLoop(bool m_state);
    void setSyncRecordCsv(bool m_state);
    void setSyncStream(bool m_state);

    //void onSettingsChange();

signals:
    void stillImageChange(int frameNumber);

    void onPlaybackStartInitiated();
    void onPlaybackPauseInitiated();
    void onPlaybackStopInitiated();

    void onPlaybackSafelyStarted();
    void onPlaybackSafelyPaused();
    void onPlaybackSafelyStopped();

    void cameraPlaybackChanged();
    void cameraPlaybackPositionChanged();

};

