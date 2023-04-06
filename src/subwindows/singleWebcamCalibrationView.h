#pragma once

/**
    @authors Gábor Bényei
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include "videoView.h"
#include "../cameraCalibration.h"
#include "../devices/singleWebcam.h"
#include "calibrationHelpDialog.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QtWidgets>

/**
    Custom widget for conducting camera calibration on a given single camera using a calibration pattern

    Different pattern are supported. See OpenCVs calibration documentation on the different supported pattern.

    IMPORTANT: This code is almost 1-1- copy paste of single camera calibration view, written by Moritz Lode. I just shortcut

 signals:
    onNewImage(): Signal send to distribute images showing camera calibration results
*/
class SingleWebcamCalibrationView : public QWidget {
    Q_OBJECT

public:

    explicit SingleWebcamCalibrationView(SingleWebcam *camera, QWidget *parent=0);
    ~SingleWebcamCalibrationView() override;

private:

    SingleWebcam *camera;

    QDir settingsDirectory;
    CalibrationHelpDialog *calibrationHelpDialog;

    VideoView *videoView;

    CameraCalibration *calibrationWorker;

    QCheckBox *chessboardCheckbox;
    QCheckBox *circlesCheckbox;
    QCheckBox *asymmetricCirclesCheckbox;
    QCheckBox *verifyFileBox;

    QSpinBox  *horizontalCornerBox;
    QSpinBox  *verticalCornerBox;
    QSpinBox  *squareSizeBox;
    QSpinBox  *sharpnessThresholdBox;

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

    void onNewImage(const CameraImage &img);

};
