
#ifndef PUPILEXT_STEREOCAMERAVIEW_H
#define PUPILEXT_STEREOCAMERAVIEW_H

/**
    @author Moritz Lode, Gábor Bényei
*/

#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/QLabel>

#include "videoView.h"
#include "../pupilDetection.h"
#include "../devices/stereoCamera.h"

#include "../devices/fileCamera.h"
#include <QSpinBox>
#include <QSlider>

/**
    Main view showing the two camera images side-by-side for a stereo camera, at the same time displays results of the pupil detection rendered

    setUpdateFPS(): define the rate with which the camera view is updated, not changing the camera framerate or the pupil detection framerate

 signals:
    onShowROI: Signals when the display of the ROI is selected by the user
    onShowPupilCenter: Signals when the display of the pupil center is selected by the user
*/
class StereoCameraView : public QWidget {
Q_OBJECT

public:

    explicit StereoCameraView(Camera *camera, PupilDetection *pupilDetection, QWidget *parent = 0);

    ~StereoCameraView() override;

    void setUpdateFPS(int fps) {
        updateDelay = 1000 / fps;
        if (pupilDetection)
            pupilDetection->setUpdateFPS(fps);
    }

private:

    Camera *camera;
    PupilDetection *pupilDetection;

    QSettings *applicationSettings;

    QElapsedTimer timer;
    int updateDelay;

    QToolBar *toolBar;
    QAction *saveROI;
    QAction *discardROI;
    QAction *plotMenuAct;
    QAction *displayDetailAct;
    QAction *plotCenterAct;
    QAction *plotROIAct;

    //QAction *roiMenuAct;
    QAction *customROIAct;
    QAction *smallROIAct;
    QAction *middleROIAct;

    VideoView *mainVideoView;
    VideoView *secondaryVideoView;

    QStatusBar *statusBar;
    QLabel *cameraFPSValue;
    QLabel *processingAlgorithmLabel;
    QLabel *processingFPSValue;
    QWidget *statusProcessingFPSWidget;
    QLabel *processingConfigLabel;
    QElapsedTimer pupilViewTimer;

    

    bool displayPupilView;
    bool plotPupilCenter;
    bool plotROIContour;
    bool initPupilViewSize;
    //QSize pupilViewSize;
    //QSize pupilViewSizeSec;
    
    double currentCameraFPS;

    // GB added begin
    QAction *pupilDetectionMenuAct;
    QLabel *processingModeLabel;

    ColorFill pupilColorFill = ColorFill::NO_FILL;
    float pupilColorFillThreshold = 0.0;
    QSpinBox *autoParamPupSizeBox;
    QSlider *autoParamSlider;

    std::vector<QSize> pupilViewSize;

    QAction *showAutoParamAct;

    bool showAutoParamOverlay;
    
    void updateProcModeLabel(); // GB added
    // GB added end

    void loadSettings();

public slots:

    void onPlotMenuClick();
    //void onROIMenuClick();

    void updateView(const CameraImage &img);
    void updateCameraFPS(double fps);
    void updateProcessingFPS(double fps);
    //void updatePupilView(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);
    void updatePupilView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);

    void onFitClick();
    void on100pClick();
    void onZoomPlusClick();
    void onZoomMinusClick();
    void onSetROIClick(float roiSize);
    void onSaveROIClick();
    void onDiscardROIClick();
    void onPupilDetectionStart();
    void onPupilDetectionStop();

    void onDisplayPupilViewClick(bool value);
    void onPlotPupilCenterClick(bool value);
    void onPlotROIClick(bool value);
    void onSettingsChange();
    void updateAlgorithmLabel();
    void updateConfigLabel(QString config);

    // GB modified/added begin
    void onPupilDetectionMenuClick();

    void saveMainROI1Selection(QRectF roi); // GB modified and renamed
    void saveMainROI2Selection(QRectF roi);
    void saveSecondaryROI1Selection(QRectF roi); // GB modified and renamed
    void saveSecondaryROI2Selection(QRectF roi);

    void displayFileCameraFrame(int frameNumber);

    void updateForPupilDetectionProcMode();
    //void updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void onPupilColorFillChanged(int itemIndex);
    void onPupilColorFillThresholdChanged(double value);

    void onShowAutoParamOverlay(bool state);
    void onAutoParamPupSize(int value);
    // GB modified/added end

signals:

    // GÁBOR modified: not handled in pupilDetection anymore
    void onShowROI(bool value);
    void onShowPupilCenter(bool value);
    void onChangePupilColorFill(int colorFill);
    void onChangePupilColorFillThreshold(float value);
    void onChangeShowAutoParamOverlay(bool state);
    // GÁBOR end

};



#endif //PUPILEXT_STEREOCAMERAVIEW_H
