
#ifndef PUPILEXT_CAMERAVIEW_H
#define PUPILEXT_CAMERAVIEW_H


#include <QtWidgets/QWidget>

class CameraView : public QWidget {
Q_OBJECT

public:

    
    explicit inline CameraView(QWidget *parent = 0) : QWidget(parent){}

    virtual ~CameraView(){}

    void setUpdateFPS(int fps) {
        updateDelay = 1000/fps;
        if(pupilDetection)
            pupilDetection->setUpdateFPS(fps);
    }

    Camera *camera;
    PupilDetection *pupilDetection;

    QSettings *applicationSettings;

    QElapsedTimer timer;
    int updateDelay;

    QToolBar *toolBar;
    QAction *freezeAct;
    QString freezeText;
    bool playbackFrozen;
    QAction *saveROI;
    QAction *resetROI;
    QAction *discardROISelection;
    QAction *plotMenuAct;
    QAction *displayDetailAct;
    QAction *plotCenterAct;
    QAction *plotROIAct;

    QMenu *autoParamMenu;

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
    
    virtual void updateProcModeLabel() = 0;

    bool isAutoParamModificationEnabled();

public slots:

    void onPlotMenuClick();
    //void onROIMenuClick();

    void updateView(const CameraImage &img);
    void updateCameraFPS(double fps);
    void updateProcessingFPS(double fps);
    //void updatePupilView(quint64 timestamp, const Pupil &pupil, const QString &filename);
    virtual void updatePupilView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) = 0;
    void updateAlgorithmLabel();

    virtual void onFitClick() = 0;
    //void onAcqImageROIchanged(); //added by kheki4 on 2022.10.24, NOTE: to properly re-fit view when image acquisition ROI is changed
    virtual void on100pClick() = 0;
    virtual void onZoomPlusClick() = 0;
    virtual void onZoomMinusClick() = 0;
    virtual void onSetROIClick(float roiSize) = 0;
    virtual void onSaveROIClick() = 0;
    virtual void onResetROIClick() = 0;
    virtual void void onDiscardROISelectionClick() = 0;

    virtual void onDisplayPupilViewClick(bool value) = 0;
    // TODO: REFACTOR
    void onPlotPupilCenterClick(bool value);
    // TODO: REFACTOR
    void onPlotROIClick(bool value);

    void onPupilDetectionStart();
    void onPupilDetectionStop();

    void onSettingsChange();
    void onPupilDetectionConfigChanged(QString config);

    // GB modified/added begin
    void onPupilDetectionMenuClick();

    void saveROI1Selection(QRectF roiR); // GB modified and renamed
    void saveROI2Selection(QRectF roiR);

    virtual void displayFileCameraFrame(int frameNumber) = 0;

    virtual void updateForPupilDetectionProcMode() = 0;
    void updateView(const CameraImage &cimg, const int &procMode, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    // TODO: REFACTOR
    void onPupilColorFillChanged(int itemIndex);
    // TODO: REFACTOR
    void onPupilColorFillThresholdChanged(double value);

    // TODO: REFACTOR
    void onShowAutoParamOverlay(bool state);
    virtual void onAutoParamPupSize(int value) = 0;

    void onFreezeClicked();
    void onCameraPlaybackChanged();
    // GB modified/added end

signals:
    // GB modified: not handled in pupilDetection anymore
    void onShowROI(bool value);
    void onShowPupilCenter(bool value);
    void onChangePupilColorFill(int colorFill);
    void onChangePupilColorFillThreshold(float value);
    void onChangeShowAutoParamOverlay(bool state);
    void cameraPlaybackChanged();
    // GB end

protected:
// TODO: implement in child
    virtual void onPupilDetectionStopInternal() = 0;
}

#endif //PUPILEXT_CAMERAVIEW_H