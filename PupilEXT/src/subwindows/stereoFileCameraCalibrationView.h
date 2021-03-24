
#ifndef PUPILEXT_STEREOFILECAMERACALIBRATIONVIEW_H
#define PUPILEXT_STEREOFILECAMERACALIBRATIONVIEW_H

/**
    @author Moritz Lode
*/

#include <QtWidgets/QWidget>
#include <QtCore/qdir.h>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include "../stereoCameraCalibration.h"
#include "../devices/fileCamera.h"

/**
    Widget representing the calibration for virtual stereo cameras, where no calibration can be conducted, only already existing calibration results can be loaded from disk
*/
class StereoFileCameraCalibrationView : public QWidget {
    Q_OBJECT

public:

    explicit StereoFileCameraCalibrationView(FileCamera *camera, QWidget *parent=0);
    ~StereoFileCameraCalibrationView() override;

private:

    QDir settingsDirectory;

    FileCamera *camera;
    StereoCameraCalibration *calibrationWorker;

    QCheckBox *chessboardCheckbox;
    QCheckBox *circlesCheckbox;
    QCheckBox *asymmetricCirclesCheckbox;

    QSpinBox  *horizontalCornerBox;
    QSpinBox  *verticalCornerBox;
    QSpinBox  *squareSizeBox;
    QTextEdit *textField;

    QPushButton *loadButton;

    void updateSettings();

public slots:

    //void onCalibrationFinished();
    void onLoadClick();

};


#endif //PUPILEXT_STEREOFILECAMERACALIBRATIONVIEW_H
