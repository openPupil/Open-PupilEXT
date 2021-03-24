
#ifndef PUPILEXT_STEREOCAMERAIMAGEEVENTHANDLER_H
#define PUPILEXT_STEREOCAMERAIMAGEEVENTHANDLER_H

/**
    @author Moritz Lode
*/


#include <QtCore/QObject>
#include <opencv2/core/mat.hpp>
#include <pylon/PylonImage.h>
#include <pylon/ImageEventHandler.h>
#include <pylon/PylonIncludes.h>
#include <QtCore/QMutex>
#include "camera.h"

using namespace Pylon;

/**
    Image event handler for stereo camera, gets called for EACH of the stereo camera images, meaning it will get called two times for a single stereo-image

    CAUTION:
    To sync the images, the camera provided framecount is used and it is assumed that the framecount of both cameras matches due to sync acquisition start (See StereoCamera)
    Its important for stereo synchronization, that only after opening of the stereo camera (acquisition start) the hardware triggers are started

    setTimeSynchronization(): set the camera and system times which are used to sync the camera timestamps, usually only a single time at camera initialisation

    onNewGrabResult(): signal send at each new stereo image, distributing the new stereo images of both cameras
*/
class StereoCameraImageEventHandler : public QObject, public CImageEventHandler {
Q_OBJECT

public:

    explicit StereoCameraImageEventHandler(QObject* parent=0);

    ~StereoCameraImageEventHandler() override;

    void setTimeSynchronization(uint64_t m_mainCameraTime, uint64_t m_secondaryCameraTime, uint64_t m_systemTime);

    void OnImagesSkipped( CInstantCamera& camera, size_t countOfSkippedImages) override;
    void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult) override;

private:

    QMutex mutex;

    uint64_t cameraTime[2] = {0, 0};
    uint64_t systemTime;

    CImageFormatConverter formatConverter;
    CPylonImage pylonImage;

    CameraImage stereoImage;

signals:

    void onNewGrabResult(CameraImage grabResult);

};

#endif //PUPILEXT_STEREOCAMERAIMAGEEVENTHANDLER_H
