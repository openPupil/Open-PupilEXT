
#ifndef PUPILEXT_SINGLEFILECAMERACALIBRATIONVIEW_H
#define PUPILEXT_SINGLEFILECAMERACALIBRATIONVIEW_H

/**
    @author Moritz Lode
*/

#include <QtWidgets/QWidget>
#include <QtCore/qdir.h>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include "../cameraCalibration.h"
#include "../devices/fileCamera.h"

/**
    Widget for the calibration of a virtual single camera, where no calibration can be conducted thus already existing calibration results can be loaded from disk.

    Loads a calibration file created through the SingleCalibration process and the SingleCameraCalibrationView.
*/
class SingleFileCameraCalibrationView : public QWidget {
    Q_OBJECT

public:

    explicit SingleFileCameraCalibrationView(FileCamera *camera, QWidget *parent=0);
    ~SingleFileCameraCalibrationView() override;

private:

    QDir settingsDirectory;

    FileCamera *camera;
    CameraCalibration *calibrationWorker;

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

    // void onCalibrationFinished();
    void onLoadClick();

};


#endif //PUPILEXT_SINGLEFILECAMERACALIBRATIONVIEW_H
