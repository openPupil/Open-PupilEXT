
#ifndef PUPILEXT_SINGLECAMERA_H
#define PUPILEXT_SINGLECAMERA_H

/**
    @authors Moritz Lode, Gábor Bényei
*/


#include <QtCore/QObject>
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include "singleCameraImageEventHandler.h"
#include "camera.h"
#include "../frameRateCounter.h"
#include "../cameraCalibration.h"
#include "../cameraFrameRateCounter.h"


using namespace Pylon;
using namespace Basler_UniversalCameraParams;




/**
    SingleCamera represents a single camera (Basler camera)

    NOTE: Modified by Gábor Bényei, 2023 jan
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

    void autoGainOnce();
    void autoExposureOnce();

    int getExposureTimeValue();
    int getExposureTimeMin();
    int getExposureTimeMax();

    bool isEnabledAcquisitionFrameRate(); // ResultingFrameRate
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
    int getImageROIwidth(); 
    int getImageROIheight(); 
    int getImageROIoffsetX(); 
    int getImageROIoffsetY(); 
    int getImageROIwidthMax(); // both setImageROI and setImageResize depends on this
    int getImageROIheightMax(); // both setImageROI and setImageResize depends on this
    int getBinningVal();
    double getTemperature();
    // GB added end

private:

    QDir settingsDirectory;

    uint64 cameraTime;
    uint64 systemTime;

    bool hardwareTriggerEnabled;
    String_t lineSource;

    CBaslerUniversalInstantCamera camera;
    SingleCameraImageEventHandler *cameraImageEventHandler;

    CameraFrameRateCounter *frameCounter;

    CameraCalibration *cameraCalibration;
    QThread *calibrationThread;

    void synchronizeTime();

    void loadCalibrationFile();

public slots:

    void setGainValue(double value);
    void setExposureTimeValue(int value);
    void setLineSource(String_t value);
    void enableAcquisitionFrameRate(bool enabled);
    void setAcquisitionFPSValue(int value);
    void enableHardwareTrigger(bool enabled);

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

};

#endif //PUPILEXT_SINGLECAMERA_H
