
#ifndef PUPILEXT_PUPILDETECTION_H
#define PUPILEXT_PUPILDETECTION_H

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QRect>
#include "devices/camera.h"
#include "pupil-detection-methods/PupilDetectionMethod.h"
#include "devices/singleCamera.h"
#include "stereoCameraCalibration.h"

Q_DECLARE_METATYPE(Pupil)

/**
    Object that performs the pupil detection on images, should be executed in an own thread

    Supports single and stereo camera pupil detection

slots:

    onNewImage(): on each new camera image, pupil detection is performed
    onNewStereoImage(): on each new stereo camera image, pupil detection is performed concurrently

    setAlgorithm(): select the algorithm to apply
    setROI(): define the ROI for pupil detectionm
    setSecondaryROI(): for stereo pupil detection, select secondary ROI

signals:

    processedImage(): Outputs camera images with rendered pupil detection results
    processedPupilData(): Pupil measurements of the pupil detection, in form of a Pupil class object

    processingStarted(): signal to notify pupil detection start
    processingFinished(): signal to notify pupil detection end

    fps(double fps): processing frame rate of the pupil detection
    algorithmChanged(): signal to notify pupil detection algorithm change, used for interface updates
*/
class PupilDetection : public QObject {
    Q_OBJECT

public:

    explicit PupilDetection(QObject *parent = 0);
    ~PupilDetection() override;

    std::vector<PupilDetectionMethod*> getMethods() {
        return pupilDetectionMethods;
    }

    QString getCurrentConfigLabel() {
        return currentConfigLabel;
    }

    void setCurrentConfigLabel(QString config) {
        currentConfigLabel = config;
    }

    PupilDetectionMethod* getCurrentMethod() {
        return pupilDetectionMethods[pupilDetectionIndex];
    }

    PupilDetectionMethod* getMethod(std::string method) {
        for(auto pm: pupilDetectionMethods) {
            if(pm->title() == method)
                return pm;
        }
        return nullptr;
    }

    PupilDetectionMethod* getCurrentSecondaryMethod() {
        return pupilDetectionMethodsSecondary[pupilDetectionIndex];
    }

    PupilDetectionMethod* getSecondaryMethod(std::string method) {
        for(auto pm: pupilDetectionMethodsSecondary) {
            if(pm->title() == method)
                return pm;
        }
        return nullptr;
    }

    bool isStereo() {
        return stereoMode;
    }

    void setUpdateFPS(int fps) {
        drawDelay = 1000/fps;
    }

    bool isOutlineConfidenceEnabled() {
        return useOutlineConfidence;
    }

    bool isROIPreProcessingEnabled() {
        return useROIPreProcessing;
    }

    void enableOutlineConfidence(bool value) {
        useOutlineConfidence = value;
    }

    void enableROIPreProcessing(bool value) {
        useROIPreProcessing = value;
    }

    bool isPupilUndistortionEnabled() {
        return usePupilUndistort;
    }

    void enablePupilUndistortion(bool value) {
        usePupilUndistort = value;
    }

    bool isImageUndistortionEnabled() {
        return useImageUndistort;
    }

    void enableImageUndistortion(bool value) {
        useImageUndistort = value;
    }

    void setCamera(Camera *m_camera);

    bool hasCamera() {
        return camera != nullptr;
    }

    void startDetection();
    void stopDetection();


private:

    Camera *camera;

    CameraCalibration *singleCalibration;
    StereoCameraCalibration *stereoCalibration;

    std::vector<PupilDetectionMethod*> pupilDetectionMethods;
    std::vector<PupilDetectionMethod*> pupilDetectionMethodsSecondary;

    int pupilDetectionIndex;
    QString currentConfigLabel;

    FrameRateCounter *frameCounter;
    cv::Rect ROI;
    cv::Rect ROISecondary;

    QElapsedTimer processingTimer;

    QElapsedTimer drawTimer;
    int drawDelay;

    QMutex mutex;

    bool stereoMode;
    bool calibrated;
    bool trackingOn;
    bool useOutlineConfidence;
    bool useROIPreProcessing;
    bool usePupilUndistort;
    bool useImageUndistort;
    bool showROI;
    bool showPupilCenter;

    std::vector<std::pair<uint64_t, long>> runtimeHistory;

    template<typename T> void writeVectorCSV(std::vector<std::pair<uint64_t , T>> data, const std::string &header, const std::string &filename);

public slots:

    void onNewImage(const CameraImage &img);
    void onNewStereoImage(const CameraImage &simg);

    void onShowROI(bool value);
    void onShowPupilCenter(bool value);


    void setAlgorithm(QString method);
    void setConfigLabel(QString config);

    void setROI(QRectF roi);
    void setSecondaryROI(QRectF roi);

signals:

    void processedImage(const CameraImage &image);
    void processedPupilData(quint64 timestamp, const Pupil &pupil, const QString &filename);
    void processedStereoPupilData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);

    void processingStarted();
    void processingFinished();

    void fps(double fps);
    void algorithmChanged();
    void configChanged(QString config);

};


#endif //PUPILEXT_PUPILDETECTION_H
