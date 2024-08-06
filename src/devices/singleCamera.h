
#ifndef PUPILEXT_SINGLECAMERA_H
#define PUPILEXT_SINGLECAMERA_H

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
*/


#include <QtCore/QObject>
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include "singleCameraImageEventHandler.h"
#include "cameraConfigurationEventHandler.h"
#include "camera.h"
#include "../frameRateCounter.h"
#include "../cameraCalibration.h"
#include "../cameraFrameRateCounter.h"
#include "hardwareTriggerConfiguration.h"


using namespace Pylon;
using namespace Basler_UniversalCameraParams;




/**
    SingleCamera represents a single camera (Basler camera)

    NOTE: Modified by Gabor Benyei, 2023 jan
    BG NOTE: 
        Added getters/setters for changing image acquisition ROI size and offset,
        binning, as well as camera coreboard temperature

    getFriendlyName(): return the Basler device name in a friendly formatting
    getFullName(): return the full Basler device name
    getCameraCalibration(): return the camera's calibration object

    loadFromFile(): load camera configuration from file
    saveToFile(): save camera configuration to file

    Camera configuration is set through various member functions

signals:
    fps(double fps): frames per second of the file camera playback
    framecount(int framecount): current framecount of the file camera playback

*/
class SingleCamera : public Camera {
Q_OBJECT

public:

    explicit SingleCamera(const String_t &fullname, QObject* parent=0);
    explicit SingleCamera(const CDeviceInfo& di, QObject* parent=0);

    ~SingleCamera() override;

    QString getFriendlyName();
    QString getFullName();
    QString getDeviceID();

    bool isOpen() override;
    void close() override;
    CameraImageType getType() override;

    void startGrabbing() override;
    void stopGrabbing() override;

    void autoGainOnce();
    void autoExposureOnce();

    int getExposureTimeValue();
    int getExposureTimeMin();
    int getExposureTimeMax();

    bool isEnabledAcquisitionFrameRate(); // ResultingFrameRate
    bool isEmulated();
    double getResultingFrameRateValue(); // ResultingFrameRate

    int getAcquisitionFPSValue();
    int getAcquisitionFPSMin();
    int getAcquisitionFPSMax();

    double getGainValue();
    double getGainMin();
    double getGainMax();

    String_t getLineSource();
    bool isHardwareTriggerEnabled();

    CameraCalibration* getCameraCalibration();
    QString getCalibrationFilename();

    void loadFromFile(const String_t &filename);
    void saveToFile(const String_t &filename);

    // GB added begin
    int getImageROIwidth() override; 
    int getImageROIheight() override; 
    int getImageROIoffsetX() override; 
    int getImageROIoffsetY() override; 
    QRectF getImageROI() override;
    int getImageROIwidthMax() override; // both setImageROI and setImageResize depends on this
    int getImageROIheightMax() override; // both setImageROI and setImageResize depends on this
    int getBinningVal();
    double getTemperature();
    bool isGrabbing() override;
    // GB added end

private:

    QDir settingsDirectory;

    uint64 cameraTime;
    uint64 systemTime;

    bool hardwareTriggerEnabled;
    String_t lineSource;

    CBaslerUniversalInstantCamera camera;
    SingleCameraImageEventHandler *cameraImageEventHandler;

    CameraConfigurationEventHandler *cameraConfigurationEventHandler = nullptr;
    HardwareTriggerConfiguration *hardwareTriggerConfiguration = nullptr;
    CAcquireContinuousConfiguration *softwareTriggerConfiguration = nullptr;
    CameraFrameRateCounter *frameCounter;
    CameraCalibration *cameraCalibration;
    QThread *calibrationThread;

    void synchronizeTime();
    void loadCalibrationFile();
    void genericExceptionOccured(const GenericException &e);

public slots:

    void setGainValue(double value);
    void setExposureTimeValue(int value);
    void setLineSource(String_t value);
    void enableAcquisitionFrameRate(bool enabled);
    void setAcquisitionFPSValue(int value);
    void enableHardwareTrigger(bool state);

    // GB added begin
    bool setBinningVal(int value);
    bool setImageROIwidth(int width);
    bool setImageROIheight(int height);
    bool setImageROIoffsetX(int offsetX);
    bool setImageROIoffsetY(int offsetY);
    // GB added end

signals:

    void fps(double fps);
    void framecount(int framecount);
    void cameraDeviceRemoved();
    void imagesSkipped();

};

#endif //PUPILEXT_SINGLECAMERA_H
