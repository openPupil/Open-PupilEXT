#pragma once

/**
    See https://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html
    @author Moritz Lode
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include "videoView.h"
#include "../cameraCalibration.h"
#include "../devices/stereoCamera.h"
#include "calibrationHelpDialog.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QtCore/QElapsedTimer>
#include <QtCore/qdir.h>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QtWidgets>

/**
    Custom widget for conducting camera calibration on a given stereo camera

signals:
    onNewImage(): Signal send to distribute images showing camera calibration results
*/
class StereoCameraCalibrationView : public QWidget {
    Q_OBJECT

public:

    explicit StereoCameraCalibrationView(StereoCamera *camera, QWidget *parent=0);
    ~StereoCameraCalibrationView() override;

private:

    StereoCamera *camera;

    QDir settingsDirectory;
    CalibrationHelpDialog *calibrationHelpDialog;

    VideoView *mainVideoView;
    VideoView *secondaryVideoView;

    StereoCameraCalibration *calibrationWorker;

    QCheckBox *chessboardCheckbox;
    QCheckBox *circlesCheckbox;
    QCheckBox *asymmetricCirclesCheckbox;
    QCheckBox *verifyFileBox;

    QSpinBox  *horizontalCornerBox;
    QSpinBox  *verticalCornerBox;
    QSpinBox  *squareSizeBox;
    QSpinBox *sharpnessThresholdBox;

    QPushButton *calibrateButton;
    QPushButton *verifyButton;
    QPushButton *stopButton;
    QPushButton *saveButton;
    QPushButton *loadButton;

    void updateSettings();
    void loadConfigFile();

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:

    void updateView(const CameraImage &cimg);

    void onCalibrationFinished();
    void onCalibrateClick();
    void onSaveClick();
    void onLoadClick();
    void onStopClick();
    void onVerifyClick();
    void onCheckboxClick();
    void onSizeBoxChange();
    void onSharpnessThresholdBoxChange();
    void onShowHelpDialog();
    void onVerifyFileChecked(bool value);

signals:

    void onNewImage(CameraImage img);

};
