
#ifndef PUPILEXT_SINGLECAMERAVIEW_H
#define PUPILEXT_SINGLECAMERAVIEW_H

/**
    @author Moritz Lode, Gábor Bényei
*/

#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QWidget>

#include "videoView.h"
#include "../devices/singleCamera.h"
#include "../pupilDetection.h"

#include "../devices/fileCamera.h"

/**
    Main view showing the camera images for a single camera, at the same time displays results of the pupil detection rendered on top of the image

    setUpdateFPS(): define the rate with which the camera view is updated, not changing the camera framerate or the pupil detection framerate

    NOTE: Modified by Gábor Bényei, 2023 jan
    GB NOTE: onShowROI() onShowPupilCenter() and not handled in pupilDetection anymore
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
    QAction *resetROI;
    QAction *discardROISelection;
    QAction *plotMenuAct;
    QAction *displayDetailAct;
    QAction *plotCenterAct;
    QAction *plotROIAct;

    //QAction *roiMenuAct;
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

    //QSize pupilViewSize;

    double currentCameraFPS;

    void loadSettings();

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
    
    void updateProcModeLabel();

    QRectF tempROIRect1;
    QRectF tempROIRect2;
    // GB added end

public slots:

    void onPlotMenuClick();
    //void onROIMenuClick();

    void updateView(const CameraImage &img);
    void updateCameraFPS(double fps);
    void updateProcessingFPS(double fps);
    //void updatePupilView(quint64 timestamp, const Pupil &pupil, const QString &filename);
    void updatePupilView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void updateAlgorithmLabel();

    void onFitClick();
    //void onAcqImageROIchanged(); //added by kheki4 on 2022.10.24, NOTE: to properly re-fit view when image acquisition ROI is changed
    void on100pClick();
    void onZoomPlusClick();
    void onZoomMinusClick();
    void onSetROIClick(float roiSize);
    void onSaveROIClick();
    void onResetROIClick();
    void onDiscardROISelectionClick();

    void onDisplayPupilViewClick(bool value);
    void onPlotPupilCenterClick(bool value);
    void onPlotROIClick(bool value);

    void onPupilDetectionStart();
    void onPupilDetectionStop();

    void onSettingsChange();
    void updateConfigLabel(QString config);

    // GB modified/added begin
    void onPupilDetectionMenuClick();

    void saveROI1Selection(QRectF roiR); // GB modified and renamed
    void saveROI2Selection(QRectF roiR);

    void displayFileCameraFrame(int frameNumber);

    void updateForPupilDetectionProcMode();
    void updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void onPupilColorFillChanged(int itemIndex);
    void onPupilColorFillThresholdChanged(double value);

    void onShowAutoParamOverlay(bool state);
    void onAutoParamPupSize(int value);
    // GB modified/added end

signals:
    // GB modified: not handled in pupilDetection anymore
    void onShowROI(bool value);
    void onShowPupilCenter(bool value);
    void onChangePupilColorFill(int colorFill);
    void onChangePupilColorFillThreshold(float value);
    void onChangeShowAutoParamOverlay(bool state);
    // GB end

};

#endif //PUPILEXT_SINGLECAMERAVIEW_H
