
#ifndef PUPILEXT_SINGLECAMERAIMAGEEVENTHANDLER_H
#define PUPILEXT_SINGLECAMERAIMAGEEVENTHANDLER_H

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <opencv2/core/mat.hpp>
#include <pylon/PylonImage.h>
#include <pylon/ImageEventHandler.h>
#include <pylon/PylonIncludes.h>
#include "camera.h"

using namespace Pylon;

Q_DECLARE_METATYPE(cv::Mat)

/**
    Image event handler, gets called at each new image received from a single camera (Basler camera)

    setTimeSynchronization(): set the camera and system times which are used to sync the camera timestamps, usually only a single time at camera initialisation

    onNewGrabResult(): signal send at each new image, distributing the images from the camera
*/
class SingleCameraImageEventHandler : public QObject, public CImageEventHandler {
Q_OBJECT

public:

    explicit SingleCameraImageEventHandler(QObject* parent=0);

    ~SingleCameraImageEventHandler() override;

    void setTimeSynchronization(uint64 cameraTime, uint64 systemTime);

    void OnImagesSkipped( CInstantCamera& camera, size_t countOfSkippedImages) override;
    void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult) override;

private:

    uint64 cameraTime;
    uint64 systemTime;

    CImageFormatConverter formatConverter;
    CPylonImage pylonImage;

signals:

    void onNewGrabResult(CameraImage grabResult);

};

#endif //PUPILEXT_SINGLECAMERAIMAGEEVENTHANDLER_H
