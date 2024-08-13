#pragma once

/**
    @author Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QWidget>

#include "videoView.h"
#include "../devices/singleCamera.h"
#include "../pupilDetection.h"

#include "../devices/fileCamera.h"

/**
    Main view showing the camera images for a single camera, at the same time displays results of the pupil detection rendered on top of the image

    NOTE: Modified by Gabor Benyei, 2023 jan
    GB NOTE: onShowROI() onShowPupilCenter() and not handled in pupilDetection anymore
*/
class SingleCameraView : public QWidget {
    Q_OBJECT

public:

    explicit SingleCameraView(Camera *camera, PupilDetection *pupilDetection, bool playbackFrozen,  QWidget *parent=0);
    ~SingleCameraView() override;

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
    
    QRectF tempROIRect1;
    QRectF tempROIRect2;

    void updateProcModeLabel();
    void loadSettings();
    bool isAutoParamModificationEnabled();

    void paintEvent(QPaintEvent *event) override {
        videoView->drawOverlay();
    };

public slots:

    void onViewportMenuClick();
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
    void onPupilDetectionConfigChanged(QString config);

    void onPupilDetectionMenuClick();

    void saveROI1Selection(QRectF roiR);
    void saveROI2Selection(QRectF roiR);

    void displayFileCameraFrame(int frameNumber);

    void updateForPupilDetectionProcMode();
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
