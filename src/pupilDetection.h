
#ifndef PUPILEXT_PUPILDETECTION_H
#define PUPILEXT_PUPILDETECTION_H

/**
    @author Moritz Lode, Gábor Bényei
*/

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QRect>
#include "devices/camera.h"
#include "pupil-detection-methods/PupilDetectionMethod.h"
#include "devices/singleCamera.h"
#include "stereoCameraCalibration.h"

// GB added begin
#include "devices/singleWebcam.h"
// GB added end

Q_DECLARE_METATYPE(Pupil)

// GB added begin
Q_DECLARE_METATYPE(cv::Rect)
Q_DECLARE_METATYPE(std::vector<Pupil>)
Q_DECLARE_METATYPE(std::vector<cv::Rect>)

/**
    Enum for different camera image processing modes. These require separate ROI configurations
*/
enum ProcMode {
    UNDETERMINED = 0,
    SINGLE_IMAGE_ONE_PUPIL = 1,
    SINGLE_IMAGE_TWO_PUPIL = 2,
    STEREO_IMAGE_ONE_PUPIL = 3,
    STEREO_IMAGE_TWO_PUPIL = 4 //,
    // MIRR_IMAGE_ONE_PUPIL = 3
};

/**
    Enum to easily address pupil detection output vector (std::vector<Pupil>) contents
    
    Depending on procMode, Pupils or ROIs vector elements are indexed in the following way:
        SINGLE_IMAGE_ONE_PUPIL :
            [0] = pupil
        SINGLE_IMAGE_TWO_PUPIL :
            [0] = pupilA (eye A)
            [1] = pupilB (eye B)
        STEREO_IMAGE_ONE_PUPIL :
            [0] = pupil (view 1)
            [1] = pupilSecondary (view 2)
        STEREO_IMAGE_TWO_PUPIL :
            [0] = pupilA1 (eye A view 1)
            [1] = pupilA2 (eye A view 2)
            [2] = pupilB1 (eye B view 1)
            [3] = pupilB2 (eye B view 2)
        MIRR_IMAGE_ONE_PUPIL :
            [0] = pupil1 (view 1)
            [1] = pupil2 (view 2)
     
*/
enum PupilVecIdx {
    SINGLE_IMAGE_ONE_PUPIL_MAIN = 0,
    SINGLE_IMAGE_TWO_PUPIL_A = 0,
    SINGLE_IMAGE_TWO_PUPIL_B = 1,
    STEREO_IMAGE_ONE_PUPIL_MAIN = 0,
    STEREO_IMAGE_ONE_PUPIL_SEC = 1,
    STEREO_IMAGE_TWO_PUPIL_A_MAIN = 0,
    STEREO_IMAGE_TWO_PUPIL_A_SEC = 1,
    STEREO_IMAGE_TWO_PUPIL_B_MAIN = 2,
    STEREO_IMAGE_TWO_PUPIL_B_SEC = 3,
    MIRR_IMAGE_ONE_PUPIL_MAIN = 0,
    MIRR_IMAGE_ONE_PUPIL_SEC = 1
};
// GB added end

/**
    Object that performs the pupil detection on images, should be executed in an own thread

    Supports single and stereo camera pupil detection

    Modified by Gábor Bényei, 2023 jan
    GB NOTES: 
        Added different processing modes. These use separate ROI variables, all have their own 
        setters, getters, and all processing modes correspond to 1/2/4 detected pupils.

        Removed onShowROI() and onShowPupilCenter() as these are now drawn as overlays in videoViews.

        Now there are 4 std::vector<PupilDetectionMethod*> pupilDetectionMethods variables 
        (probably a vector of vectors could be introduced) because of proper multithreading for 
        "Stereo camera for two pupils" processing mode. I also added a method to populate these vectors 
        upon pupilDetection instantiation, called populateWithMethods().

slots:
    
    // On each new camera image, pupil detection is performed:
    onNewSingleImageForOnePupil(): formerly onNewImage
    onNewSingleImageForTwoPupil()
    onNewStereoImageForOnePupil(): formerly onNewStereoImage
    onNewStereoImageForTwoPupil()
    onNewMirrImageForOnePupil()

    setAlgorithm(): select the algorithm to apply
    setCurrentProcMode(): set current processing mode
    
    setROIsingleImageOnePupil() // formerly void setROI(QRectF roi);
    setROIsingleImageTwoPupilA()
    setROIsingleImageTwoPupilB()
    setROIstereoImageOnePupil1() // formerly void setROI(QRectF roi);
    setROIstereoImageOnePupil2() // formerly void setSecondaryROI(QRectF roi);
    setROIstereoImageTwoPupilA1()
    setROIstereoImageTwoPupilA2()
    setROIstereoImageTwoPupilB1()
    setROIstereoImageTwoPupilB2()
    setROImirrImageOnePupil1()
    setROImirrImageOnePupil2()

signals:

    processedImage(): Outputs camera images with rendered pupil detection results

    // Pupil measurements of the pupil detection:
    processedImage()
    processedPupilData()

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
        return pupilDetectionMethods1;
    }

    QString getCurrentConfigLabel() {
        return currentConfigLabel;
    }

    void setCurrentConfigLabel(QString config) {
        currentConfigLabel = config;
    }

    // GB: getCurrentMethod() and its stereo pair were refactored and declaration moved a few lines down

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

        // GB added emit in order to inform any existing videoView that ROI is not used for pupil detection
        emit onROIPreprocessingChanged(value);
        // GB added end
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

    // GB modified begin
    // GB NOTE: refactored, and added necessary codes to let pupilDetection run with 
    // all maximum of 4 threads, using 4 different pupilDetectionMethods variables thread-safely
    PupilDetectionMethod* getCurrentMethod1() {
        return pupilDetectionMethods1[pupilDetectionIndex];
    }
    PupilDetectionMethod* getMethod1(std::string method) {
        for(auto pm: pupilDetectionMethods1) {
            if(pm->title() == method)
                return pm;
        }
        return nullptr;
    }
    //
    PupilDetectionMethod* getCurrentMethod2() {
        return pupilDetectionMethods2[pupilDetectionIndex];
    }
    PupilDetectionMethod* getMethod2(std::string method) {
        for(auto pm: pupilDetectionMethods2) {
            if(pm->title() == method)
                return pm;
        }
        return nullptr;
    }
    // GB modified end
    // GB added begin
    //
    PupilDetectionMethod* getCurrentMethod3() {
        return pupilDetectionMethods3[pupilDetectionIndex];
    }
    PupilDetectionMethod* getMethod3(std::string method) {
        for(auto pm: pupilDetectionMethods3) {
            if(pm->title() == method)
                return pm;
        }
        return nullptr;
    }
    //
    PupilDetectionMethod* getCurrentMethod4() {
        return pupilDetectionMethods4[pupilDetectionIndex];
    }
    PupilDetectionMethod* getMethod4(std::string method) {
        for(auto pm: pupilDetectionMethods4) {
            if(pm->title() == method)
                return pm;
        }
        return nullptr;
    }
    //
    bool isStereo() {
        if( camera &&
            (   camera->getType() == CameraImageType::LIVE_STEREO_CAMERA ||
                camera->getType() == CameraImageType::STEREO_IMAGE_FILE ) )
            return true;
        else
            return false;
    }
    bool hasOpenCamera() {
        return (camera && camera->isOpen());
    }
    // GB added end

    void startDetection();
    void stopDetection();


private:

    Camera *camera;

    CameraCalibration *singleCalibration;
    StereoCameraCalibration *stereoCalibration;

    int pupilDetectionIndex;
    QString currentConfigLabel;

    FrameRateCounter *frameCounter;

    // GB modified/added begin
    ProcMode currentProcMode;

    //std::vector<PupilDetectionMethod*> pupilDetectionMethods;
    //std::vector<PupilDetectionMethod*> pupilDetectionMethodsSecondary;
    // GB: refactored and added two new to best work with also with "stereo camera for two pupil" proc mode
    std::vector<PupilDetectionMethod*> pupilDetectionMethods1;
    std::vector<PupilDetectionMethod*> pupilDetectionMethods2;
    std::vector<PupilDetectionMethod*> pupilDetectionMethods3;
    std::vector<PupilDetectionMethod*> pupilDetectionMethods4;
    
    // NOTE:
    // in case of e.g.: ROIstereoImageTwoPupilA1
    // the NUMBER in the end denotes the different VIEWPOINTS of the same pupil
    // the LETTER denotes different EYES
    cv::Rect ROIsingleImageOnePupil; // formerly cv::Rect ROI;
    cv::Rect ROIsingleImageTwoPupilA;
    cv::Rect ROIsingleImageTwoPupilB;
    cv::Rect ROIstereoImageOnePupil1; // formerly cv::Rect ROI;
    cv::Rect ROIstereoImageOnePupil2; // formerly cv::Rect ROISecondary;
    cv::Rect ROIstereoImageTwoPupilA1;
    cv::Rect ROIstereoImageTwoPupilA2;
    cv::Rect ROIstereoImageTwoPupilB1;
    cv::Rect ROIstereoImageTwoPupilB2;
    cv::Rect ROImirrImageOnePupil1;
    cv::Rect ROImirrImageOnePupil2;
    // GB end

    QElapsedTimer processingTimer;

    QElapsedTimer drawTimer;
    int drawDelay;

    QMutex mutex;

    bool calibrated;
    bool trackingOn;
    bool useOutlineConfidence;
    bool useROIPreProcessing;
    bool usePupilUndistort;
    bool useImageUndistort;
    //bool showROI;
    //bool showPupilCenter;

    std::vector<std::pair<uint64_t, long>> runtimeHistory;

    template<typename T> void writeVectorCSV(std::vector<std::pair<uint64_t , T>> data, const std::string &header, const std::string &filename);

    void populateWithMethods(std::vector<PupilDetectionMethod*> &vec); // GB added

    //cv::Size expectedImageSize = cv::Size(0,0);
    //void resizeROIs(cv::Size newImageSize);
    //cv::Rect resizeCvRect(const cv::Rect &rect, float factorX, float factorY);

    bool autoParamEnabled = false; // true as long as there is demand for autoParam. Also for informing other class instances through getter
    float autoParamPupSizePercent = 50;
    bool autoParamScheduled = false; // true only if a new performAutoParam() is necessary shortly. (need this, because performAutoParam cannot do anything when called without ROIs defined)
    
    bool autoParamSettingsEnabled = false; // True if pupil detection algorithm has Automatic Parametrization setting selected.

    void performAutoParam(); // GB added

    PupilDetectionMethod* getCurrentMethod(){
        return getCurrentMethod1();
    };

public slots:

    void setAlgorithm(QString method);
    void setConfigLabel(QString config);
    
    // GB added/modified:
    void onNewSingleImageForOnePupil(const CameraImage &img); // formerly onNewImage
    void onNewSingleImageForTwoPupil(const CameraImage &img); // formerly did not exist
    void onNewStereoImageForOnePupil(const CameraImage &simg); // formerly onNewStereoImage
    void onNewStereoImageForTwoPupil(const CameraImage &simg); // formerly did not exist
    void onNewMirrImageForOnePupil(const CameraImage &simg); // formerly did not exist

    void setAutoParamEnabled(bool state);
    void setAutoParamPupSizePercent(float value);
    bool isAutoParamSettingsEnabled();
    void setAutoParamSettingsEnabled(bool enabled);
    float getAutoParamPupSizePercent();
    //void performAutoParam();
    void setAutoParamScheduled(bool state);

    bool isTrackingOn();
    ProcMode getCurrentProcMode();
    void setCurrentProcMode(int val);
    
    QRect getROIsingleImageOnePupil();
    QRect getROIsingleImageTwoPupilA();
    QRect getROIsingleImageTwoPupilB();
    QRect getROIstereoImageOnePupil1();
    QRect getROIstereoImageOnePupil2();
    QRect getROIstereoImageTwoPupilA1();
    QRect getROIstereoImageTwoPupilA2();
    QRect getROIstereoImageTwoPupilB1();
    QRect getROIstereoImageTwoPupilB2();
    QRect getROImirrImageOnePupil1();
    QRect getROImirrImageOnePupil2();
    
    void setROIsingleImageOnePupil(QRectF roi); // formerly void setROI(QRectF roi);
    void setROIsingleImageTwoPupilA(QRectF roi);
    void setROIsingleImageTwoPupilB(QRectF roi);
    void setROIstereoImageOnePupil1(QRectF roi); // formerly void setROI(QRectF roi);
    void setROIstereoImageOnePupil2(QRectF roi); // formerly void setSecondaryROI(QRectF roi);
    void setROIstereoImageTwoPupilA1(QRectF roi);
    void setROIstereoImageTwoPupilA2(QRectF roi);
    void setROIstereoImageTwoPupilB1(QRectF roi);
    void setROIstereoImageTwoPupilB2(QRectF roi);
    void setROImirrImageOnePupil1(QRectF roi);
    void setROImirrImageOnePupil2(QRectF roi);
    // GB end

signals:

    void processedImage(const CameraImage &image);
    
    // GB added/modified begin
    void processedPlaybackImage(quint64 timestamp, int frameNumber);
    void processedImage(CameraImage mimg, int currentProcMode, std::vector<cv::Rect> ROIs, std::vector<Pupil> Pupils);
    void processedPupilData(quint64 timestamp, int currentProcMode, const std::vector<Pupil> &Pupils, const QString &filename);

    void onROIPreprocessingChanged(bool state);
    // GB added/modified end

    void processingStarted();
    void processingFinished();

    void fps(double fps);
    void algorithmChanged();
    void configChanged(QString config);

};


#endif //PUPILEXT_PUPILDETECTION_H
