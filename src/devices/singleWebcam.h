#pragma once

/**
    @author Gabor Benyei, Attila Boncser
*/

#include <QtCore/QObject>
#include "singleWebcamImageEventHandler.h"
#include "camera.h"
#include "../frameRateCounter.h"
#include "../cameraCalibration.h"
#include "../cameraFrameRateCounter.h"

#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <opencv2/videoio/videoio_c.h>
#include <QDebug>


/**
    
    This class runs in a thread to simulate framegrabbing process, using OpenCV input

*/
class GrabberDummy : public QObject {
    Q_OBJECT
 
    bool m_running;
    double resizeFactor = 1.0;
    double queryFPS = 30;

    int deviceID;
    cv::VideoCapture* m_camera;

    cv::Mat image;
 
public:
    explicit GrabberDummy(int deviceID);

    bool running() const;

    cv::Size getImageSize() const;
 
signals:
    void startedToOpenCamera();
    void couldNotOpenCamera();
    void successfullyOpenedCamera();
    void finished();

    void GrabberDummyEvent(cv::Mat image);
 
public slots:
    void run();
    void stop();

    bool isOpen();

    int getFPSValue();
    double getBrightnessValue();
    double getContrastValue();
    double getGainValue();
    double getExposureValue();
    double getResizeFactor();
    
    bool setFPSValue(int value);
    bool setBrightnessValue(double value);
    bool setContrastValue(double value);
    bool setGainValue(double value);
    bool setExposureValue(double value);
    void setResizeFactor(double value);

    bool registerEventHandler(SingleWebcamImageEventHandler *handler, const char *method);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/**

    SingleWebcam class that encapsulates a GrabberDummy instance in a separate thread to read 
    OpenCV compatible camera input. Also supports minimal camera settings, but their 
    compatibility is not guaranteed (OpenCV managed, depends on platform, driver and camera)

*/
class SingleWebcam : public Camera {
Q_OBJECT

public:

    explicit SingleWebcam(int deviceID, QString friendlyName, QObject* parent=0);
    ~SingleWebcam() override;

    QString getFriendlyName();
    QString getDeviceID();

    bool isOpen() override;
    void close() override;
    CameraImageType getType() override;

    void startGrabbing() override;
    void stopGrabbing() override;

    int getImageROIwidth() override;
    int getImageROIheight() override;
    int getImageROIwidthMax() override;
    int getImageROIheightMax() override;
    int getImageROIoffsetX() override; 
    int getImageROIoffsetY() override;
    QRectF getImageROI() override;

    bool isHardwareTriggerEnabled();

    CameraCalibration* getCameraCalibration();
    QString getCalibrationFilename();

    bool isGrabbing() override;

private:

    QDir settingsDirectory;

    QString friendlyName;

    SingleWebcamImageEventHandler *cameraImageEventHandler;
    QThread *grabbingThread;
    GrabberDummy *grabberDummy;

    CameraFrameRateCounter *frameCounter;

    CameraCalibration *cameraCalibration;
    QThread *calibrationThread;

    void loadCalibrationFile();

public slots:
    int getFPSValue();
    double getBrightnessValue();
    double getContrastValue();
    double getGainValue();
    double getExposureValue();
    double getResizeFactor();

    bool setFPSValue(int value);
    bool setBrightnessValue(double value);
    bool setContrastValue(double value);
    bool setGainValue(double value);
    bool setExposureValue(double value);
    void setResizeFactor(double value);

signals:
    void fps(double fps);
    void framecount(int framecount);

    void startedToOpenCamera();
    void couldNotOpenCamera();
    void successfullyOpenedCamera();

};

