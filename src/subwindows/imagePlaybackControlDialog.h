#pragma once

/**
    @author Gábor Bényei
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

#include <QKeyEvent> // DEV

/**
    
    This dialog gives the opportunity of playing and stopping image playback for ths user.
    It also shows basic information about the currently played/displayed frame.
    There is a dial widget for stepping frame-by-frame, if the user would like to inspect 
    interesting frames more closely. On the slider a green tick shows the position of file read, 
    (that last went into pupilDetection) and the slider handle marks the actually diaplayed image position
    (that just arrived from pupilDetection).

    This dialog is designed to be only visible and interactable while an image directory is opened. 

    IMPORTANT:
        The dial steps to next or previous frames in a way that the read image will pass through the 
        pupil detection processing, but the Slider clicks will only seek images and display them in the 
        camera view, but pupilDetection is not performed on them

    GB NOTE:
        Playback speed and loop settings were moved here from GeneralSettingsDialog, 
        so were play and stop buttons from main window.
    
    GB NOTE: 
        In case of pupil detection /tracking going on, handling "dead time" between pausing/stopping 
        playback and actual end of pupildetection signals should be consistent with the 
        Enabled/Disabled state of button controls in this dialog. 
        This is as far as I know fulfilled, but I did not employ any full proof async code.

    GB NOTE:
        recEventTracker->resetReplay(timestamp) should be called:
        1, Upon playback safely ended (meaning that the last read image got processed through pupilDetection)
            but when stopped (not paused), it should also call recEventTracker->resetReplay() to reset to beginning
        2, Also when someone uses the slider to jump to a timepoint
        This is supposed to ok, but if widgets enable/disable does erroneously let the user click anywhere, problems can arise

*/


class ImagePlaybackControlDialog : public QWidget {
    Q_OBJECT

public:

    explicit ImagePlaybackControlDialog(FileCamera *fileCamera, PupilDetection *pupilDetection, RecEventTracker *recEventTracker, QWidget *parent = nullptr);
    ~ImagePlaybackControlDialog() override;

    bool getSyncRecordCsv();
    bool getSyncStream();


private:

    RecEventTracker *recEventTracker;

    QSettings *applicationSettings;

    FileCamera *fileCamera;

    PupilDetection *pupilDetection;

    bool playImagesOn = false;

    
    int playbackSpeed;
    bool playbackLoop;
    bool syncRecordCsv;
    bool syncStream;

    QSpinBox* playbackFPSVal;

    QGroupBox *infoGroup;
    QLineEdit *timestampVal;
    QLabel *timestampHumanValLabel;
    QLabel *imgNumberValLabel;
    QLabel *elapsedTimeValLabel;
    QLabel *acqFPSValLabel;
    QLabel *percentValLabel;
    QLabel *trialValLabel;

    QElapsedTimer drawTimer;
    int drawDelay;

    int numImagesTotal = -1;
    bool waitingForReset = false;
    uint64_t stalledTimestamp = 0;
    bool playbackStalled = false;
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
 //   void saveSettings();
 //   void updateForm();

public slots:

    void updateInfo(quint64 timestamp, int frameNumber);
    void updateInfo(const CameraImage &img);
    
    void onAutomaticFinish();
    
    void readSettings();
    //void saveSettings();
    void updateForm();

private slots:
    void onStartPauseButtonClick();
    void onStopButtonClick();
    void onFinish();
    
    void updateInfoInternal(int frameNumber);

    void onPupilDetectionStart();
    void onPupilDetectionStop();

    void onDialForward();
    void onDialBackward();
    void onSliderValueChanged(int val);
    void updateSliderColorTick(const CameraImage &cimg);

    
    void setPlaybackSpeed(int playbackSpeed);
    void setPlaybackLoop(int m_state);
    void setSyncRecordCsv(int m_state);
    void setSyncStream(int m_state);

    //void onSettingsChange();

signals:
    void stillImageChange(int frameNumber);

    void onPlaybackSafelyStarted();
    void onPlaybackSafelyPaused();
    void onPlaybackSafelyStopped();

};

