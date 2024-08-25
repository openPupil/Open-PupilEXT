#pragma once

/**
    Under construction
*/

#include <QtCore>
#include <QtWidgets/QWidget>
#include <QSettings>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include "../pupil-detection-methods/Pupil.h"
#include "./sceneImageWidget.h"

class SceneImageView : public QWidget {
Q_OBJECT

public:

    explicit SceneImageView(bool sceneFrozen=false, QWidget *parent=0);
    ~SceneImageView() override;

private:

    QSettings *applicationSettings;

//    QElapsedTimer timer;
//    int drawDelay;

    QToolBar *toolBar;
    QAction *configAMenuAct;
    QAction *configBMenuAct;

    QAction *configA1Act;
    QAction *configA2Act;
    QAction *configA3Act;

    QAction *configB1Act;
    QAction *configB2Act;

    QAction *freezeAct;
    bool sceneFrozen;
//    QAction *calibrateAct;
//    QAction *validateAct;
    QPushButton *calibrateButton;
    QPushButton *validateButton;

    SceneImageWidget *sceneImageWidget;

    QStatusBar *statusBar;
    QLabel *status1Label;
    QLabel *status1Value;

    void loadSettings();

private slots:
    void onConfigAMenuClick();
    void onConfigBMenuClick();

    void onConfigA1Selected();
    void onConfigA2Selected();
    void onConfigA3Selected();

    void onConfigB1Selected();
    void onConfigB2Selected();

public slots:

//    void onPupilDetectionStart();
//    void onPupilDetectionStop();

    void onSettingsChange();

    void updateView(const int &procMode, const std::vector<Pupil> &Pupils);

    void onFreezeClicked();
    void onCameraPlaybackChanged();
    void onCalibrateClicked();
    void onValidateClicked();

signals:

    void cameraPlaybackChanged();
    void startGazeCalibration();
    void cancelGazeCalibration();
    void startGazeValidation();
    void cancelGazeValidation();

};
