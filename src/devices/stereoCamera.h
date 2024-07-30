
#ifndef PUPILEXT_STEREOCAMERA_H
#define PUPILEXT_STEREOCAMERA_H

/**
    @authors Moritz Lode, Bényei Gábor
*/


#include "camera.h"
#include <QtCore/QObject>
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCameraArray.h>
#include "../frameRateCounter.h"
#include "stereoCameraImageEventHandler.h"
#include "cameraConfigurationEventHandler.h"
#include "../stereoCameraCalibration.h"
#include "../cameraFrameRateCounter.h"
#include "hardwareTriggerConfiguration.h"

using namespace Pylon;
using namespace Basler_UniversalCameraParams;

/**
    StereoCamera represents a stereo camera (two Basler cameras), main camera and secondary camera

    The camera settings of the main camera are used for the secondary camera

    NOTE: Modified by Gábor Bényei, 2023 jan
    BG NOTE: 
        Added getters/setters for changing image acquisition ROI size and offset,
        binning, as well as camera coreboard temperature

    getFriendlyNames(): return the Basler device names for both cameras in a vector list
    attachCameras(): defines the two camera devices by device names
    open(): opens the two cameras and starts grabbing, IMPORTANT: cameras need to be opened before starting the hardware trigger to guarantee sync acquisition

    getCameraCalibration(): return a stereo calibration object for the two cameras

    loadMainFromFile(): load camera configuration for the main camera from file
    saveMainToFile(): save camera configuration for the main camera  to file

    CAUTION: all camera settings are set to both cameras, default values are taken from the main camera and applied to the secondary camera

signals:
    fps(double fps): frames per second of the file camera playback
    framecount(int framecount): current framecount of the file camera playback

*/
class StereoCamera : public Camera {
    Q_OBJECT

public:

    explicit StereoCamera(QObject *parent);

    explicit StereoCamera(const String_t &fullnameRight, const String_t &fullnameLeft, QObject* parent=0);
    explicit StereoCamera(const CDeviceInfo &diRight, const CDeviceInfo &diLeft, QObject* parent=0);

    ~StereoCamera() override;

    bool isOpen() override;
    void close() override;
    CameraImageType getType() override;

    void startGrabbing() override;
    void stopGrabbing() override;

    std::vector<QString> getFriendlyNames();

    void autoGainOnce();
    void autoExposureOnce();

    int getExposureTimeValue();
    int getExposureTimeMin();
    int getExposureTimeMax();

    bool isEnabledAcquisitionFrameRate();
    bool isEmulated();
    double getResultingFrameRateValue();

    int getAcquisitionFPSValue();
    int getAcquisitionFPSMin();
    int getAcquisitionFPSMax();

    double getGainValue();
    double getGainMin();
    double getGainMax();

    void attachCameras(const CDeviceInfo &diMain, const CDeviceInfo &diSecondary);
    void open(bool enableHardwareTrigger);

    String_t getLineSource();

    StereoCameraCalibration *getCameraCalibration();
    QString getCalibrationFilename();

    void loadMainFromFile(const String_t &filename);
    //void loadSecondaryFromFile(const String_t &filename); // removed this as stereo camera configuration is only set by main and secondary is adapted
    void saveMainToFile(const String_t &filename);
    //void saveSecondaryToFile(const String_t &filename);

    // BG added begin
    int getImageROIwidth() override; 
    int getImageROIheight() override; 
    int getImageROIoffsetX() override; 
    int getImageROIoffsetY() override;
    int getImageROIwidthMax() override; // both setImageROI and setImageResize depends on this
    int getImageROIheightMax() override; // both setImageROI and setImageResize depends on this
    QRectF getImageROI() override;
    int getBinningVal();
    std::vector<double> getTemperatures();

    bool isGrabbing() override;
    // BG added end

private:

    QDir settingsDirectory;

    uint64 cameraMainTime;
    uint64 cameraSecondaryTime;
    uint64 systemTime;

    String_t lineSource;

    CBaslerUniversalInstantCameraArray cameras;
    StereoCameraImageEventHandler *cameraImageEventHandler = nullptr;
    CameraConfigurationEventHandler *cameraConfigurationEventHandler0 = nullptr;
    CameraConfigurationEventHandler *cameraConfigurationEventHandler1 = nullptr;
    HardwareTriggerConfiguration* hardwareTriggerConfiguration0 = nullptr;
    HardwareTriggerConfiguration* hardwareTriggerConfiguration1 = nullptr;
    CameraFrameRateCounter *frameCounter;

    StereoCameraCalibration *cameraCalibration;
    QThread *calibrationThread;

    void synchronizeTime();
    void loadCalibrationFile();
    void genericExceptionOccured(const GenericException &e);

    void safelyCloseCameras();

public slots:

    void setGainValue(double value);
    void setExposureTimeValue(int value);
    void setLineSource(String_t value);
    void enableAcquisitionFrameRate(bool enabled);
    void setAcquisitionFPSValue(int value);
    void resynchronizeTime();

    // BG added begin
    bool setBinningVal(int value);
    bool setImageROIwidth(int width);
    bool setImageROIheight(int height);
    bool setImageROIoffsetX(int offsetX);
    bool setImageROIoffsetY(int offsetY);

    bool setImageROIwidthEmu(int width);
    bool setImageROIheightEmu(int height);
    bool setImageROIoffsetXEmu(int offsetX);
    bool setImageROIoffsetYEmu(int offsetY); 
    // BG added end

signals:

    void fps(double fps);
    void framecount(int framecount);
    void cameraDeviceRemoved();
    void imagesSkipped();

};


#endif //PUPILEXT_STEREOCAMERA_H
