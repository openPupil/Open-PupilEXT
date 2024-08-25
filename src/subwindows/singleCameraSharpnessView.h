#pragma once

/**
    @authors Moritz Lode, Attila Boncser
*/

#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QWidget>

#include "videoView.h"
#include "../devices/singleCamera.h"
#include "../pupilDetection.h"
#include "../sharpnessCalculation.h"
#include "calibrationHelpDialog.h"

/**
    A widget that shows a live view of a single camera. In the live view a pattern detection is applied similar to the calibration view.
    When a calibration pattern is found, a sharpness value is calculated and displayed to the user.

    This view is meant to optimize the sharpness of the camera system. It also gives the user an overview of a "good" sharpness value, which can then be
    specified in the calibration process to filter bad images.

    Pattern detectiona and sharpness are calculated in a seperate thread to not block the GUI, the sharpness is calculated using the OpenCV function estimateChessboardSharpness() thus
    only chessboard pattern are supported at the moment.
*/
class SingleCameraSharpnessView : public QWidget {
    Q_OBJECT

public:

    explicit SingleCameraSharpnessView(SingleCamera *camera, QWidget *parent=0);
    ~SingleCameraSharpnessView() override;


private:

    Camera *camera;
    CameraCalibration *cameraCalibration;

    SharpnessCalculation *sharpnessWorker;
    QThread sharpnessThread;

    CalibrationHelpDialog *calibrationHelpDialog;

    QSettings *applicationSettings;

    QSpinBox  *horizontalCornerBox;
    QSpinBox  *verticalCornerBox;
    QToolBar *toolBar;

    VideoView *videoView;

    QStatusBar *statusBar;
    QLabel *cameraFPSValue;


private slots:

    void updateView(const CameraImage &img);
    void updateCameraFPS(double fps);

    void onFitClick();
    void on100pClick();
    void onZoomPlusClick();
    void onZoomMinusClick();

    void onSizeBoxChange();
    void onShowHelpDialog();

};
