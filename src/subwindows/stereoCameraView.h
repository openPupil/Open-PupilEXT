#pragma once

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
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

 signals:
    onShowROI: Signals when the display of the ROI is selected by the user
    onShowPupilCenter: Signals when the display of the pupil center is selected by the user
*/
class StereoCameraView : public QWidget {
Q_OBJECT

public:

    explicit StereoCameraView(Camera *camera, PupilDetection *pupilDetection,bool playbackFrozen, QWidget *parent = 0);

    ~StereoCameraView() override;


private:

    Camera *camera;
    PupilDetection *pupilDetection;

    QSettings *applicationSettings;

    QToolBar *toolBar;
    QAction *freezeAct;
    bool playbackFrozen;
    QAction *saveROI;
    QAction *resetROI;
    QAction *discardROISelection;
    QAction *viewportMenuAct;
    QAction *plotMenuAct;
    QAction *displayDetailAct;
    QAction *plotCenterAct;
    QAction *plotROIAct;

    QMenu *autoParamMenu;
    QMenu *roiMenu;

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


    bool displayPupilView;
    bool plotPupilCenter;
    bool plotROIContour;
    bool initPupilViewSize;
    //QSize pupilViewSize;
    //QSize pupilViewSizeSec;
    
    double currentCameraFPS;

    QAction *pupilDetectionMenuAct;
    QLabel *processingModeLabel;

    ColorFill pupilColorFill = ColorFill::NO_FILL;
    float pupilColorFillThreshold = 0.0;
    QSpinBox *autoParamPupSizeBox;
    QSlider *autoParamSlider;

    std::vector<QSize> pupilViewSize;

    QAction *showAutoParamAct;
    bool showAutoParamOverlay;

    QAction *showPositioningGuideAct;
    bool showPositioningGuide;

    QRectF tempROIs[4]; // 0 -> mainVideoView.ROI1Selection, 1 -> secondaryVideoView->ROI1Selection
                        // 2 -> mainVideoView.ROI2Selection, 3 -> secondaryVideoView->ROI2Selection
    
    void updateProcModeLabel();

    void loadSettings();
    bool isAutoParamModificationEnabled();

    void paintEvent(QPaintEvent *event) override {
        mainVideoView->drawOverlay();
        secondaryVideoView->drawOverlay();
    };

public slots:

    void onViewportMenuClick();
    void onPlotMenuClick();
    //void onROIMenuClick();

    void updateView(const CameraImage &img);
    void updateCameraFPS(double fps);
    void updateProcessingFPS(double fps);
    //void updatePupilView(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);
    void updatePupilView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void updateAlgorithmLabel();

    void onFitClick();
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
    void onPupilDetectionConfigChanged(QString config);

    void onPupilDetectionMenuClick();

    void saveMainROI1Selection(QRectF roi);
    void saveMainROI2Selection(QRectF roi);
    void saveSecondaryROI1Selection(QRectF roi);
    void saveSecondaryROI2Selection(QRectF roi);

    void displayFileCameraFrame(int frameNumber);

    void updateForPupilDetectionProcMode();
    //void updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void onPupilColorFillChanged(int itemIndex);
    void onPupilColorFillThresholdChanged(double value);

    void onShowAutoParamOverlay(bool state);
    void onShowPositioningGuide(bool state);
    void onImageROIChanged(const QRect& ROI);
    void onSensorSizeChanged(const QSize& size);
    void onAutoParamPupSize(int value);

    void onFreezeClicked();
    void onCameraPlaybackChanged();

signals:
    void onShowROI(bool value);
    void onShowPupilCenter(bool value);
    void onChangePupilColorFill(int colorFill);
    void onChangePupilColorFillThreshold(float value);
    void onChangeShowAutoParamOverlay(bool state);
    void onChangeShowPositioningGuide(bool state);
    void cameraPlaybackChanged();
    void doingPupilDetectionROIediting(bool state);

};
