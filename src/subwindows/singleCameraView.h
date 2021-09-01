
#ifndef PUPILEXT_SINGLECAMERAVIEW_H
#define PUPILEXT_SINGLECAMERAVIEW_H

/**
    @author Moritz Lode
*/

#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QWidget>

#include "videoView.h"
#include "../devices/singleCamera.h"
#include "../pupilDetection.h"

/**
    Main view showing the camera images for a single camera, at the same time displays results of the pupil detection rendered on top of the image

    setUpdateFPS(): define the rate with which the camera view is updated, not changing the camera framerate or the pupil detection framerate

 signals:
    onShowROI(bool value): Signals when the display of the ROI is selected by the user
    onShowPupilCenter(bool value): Signals when the display of the pupil center is selected by the user
*/
class SingleCameraView : public QWidget {
    Q_OBJECT

public:

    explicit SingleCameraView(Camera *camera, PupilDetection *pupilDetection, QWidget *parent=0);
    ~SingleCameraView() override;

    void setUpdateFPS(int fps) {
        updateDelay = 1000/fps;
        if(pupilDetection)
            pupilDetection->setUpdateFPS(fps);
    }

private:

    Camera *camera;
    PupilDetection *pupilDetection;

    QSettings *applicationSettings;

    QElapsedTimer timer;
    QElapsedTimer pupilViewTimer;
    int updateDelay;

    QToolBar *toolBar;
    QAction *saveROI;
    QAction *discardROI;
    QAction *plotMenuAct;
    QAction *displayDetailAct;
    QAction *plotCenterAct;
    QAction *plotROIAct;

    QAction *roiMenuAct;
    QAction *customROIAct;
    QAction *smallROIAct;
    QAction *middleROIAct;

    VideoView *videoView;

    QStatusBar *statusBar;
    QLabel *cameraFPSValue;
    QLabel *processingAlgorithmLabel;
    QLabel *processingFPSValue;
    QWidget *statusProcessingFPSWidget;
    QLabel *processingConfigLabel;

    bool displayPupilView;
    bool plotPupilCenter;
    bool plotROIContour;
    bool initPupilViewSize;

    QSize pupilViewSize;

    double currentCameraFPS;

    void loadSettings();

public slots:

    void onPlotMenuClick();
    void onROIMenuClick();

    void saveROISelection(QRectF roi);

    void updateView(const CameraImage &img);
    void updateCameraFPS(double fps);
    void updateProcessingFPS(double fps);
    void updatePupilView(quint64 timestamp, const Pupil &pupil, const QString &filename);
    void updateAlgorithmLabel();

    void onFitClick();
    void on100pClick();
    void onZoomPlusClick();
    void onZoomMinusClick();
    void onSetROIClick(float roiSize);
    void onSaveROIClick();
    void onDiscardROIClick();

    void onDisplayPupilViewClick(bool value);
    void onPlotPupilCenterClick(bool value);
    void onPlotROIClick(bool value);

    void onPupilDetectionStart();
    void onPupilDetectionStop();

    void onSettingsChange();
    void updateConfigLabel(QString config);

signals:

    void onShowROI(bool value);
    void onShowPupilCenter(bool value);

};

#endif //PUPILEXT_SINGLECAMERAVIEW_H
