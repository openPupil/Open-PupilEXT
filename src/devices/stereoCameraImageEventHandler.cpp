
#include <opencv2/core.hpp>
#include "stereoCameraImageEventHandler.h"

// Creates a new stereo image event handler for a StereoCamera
StereoCameraImageEventHandler::StereoCameraImageEventHandler(QObject* parent) : QObject(parent), systemTime(0), stereoImage() {

    formatConverter.OutputPixelFormat = Pylon::PixelType_Mono8;

    stereoImage.timestamp = 0;
    stereoImage.type = CameraImageType::LIVE_STEREO_CAMERA;
}

StereoCameraImageEventHandler::~StereoCameraImageEventHandler() {

}

// Event handler that is executed if for any of the two cameras in the stereo camera images were skipped
// If image skipping happens in one of the two cameras, one may assume that the images are not in sync
// anymore and the onImageGrabbed event handler may not be able produce any stereo images due to unsync framecount
// BG: NOTE: According to Basler docs this will only ever get called when grabStrategy is set to LatestImageOnly or LatestImages, which is never the case for us ..(?)
// https://zh.docs.baslerweb.com/pylonapi/cpp/class_pylon_1_1_c_basler_universal_image_event_handler#function-onimagesskipped
void StereoCameraImageEventHandler::OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages) {
    std::cout << "OnImagesSkipped event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
    std::cout << countOfSkippedImages  << " images have been skipped." << std::endl;
    std::cout << std::endl;

    emit imagesSkipped();
}

// Event handler that is executed for EACH image acquisition of EACH camera
// This means that for each hardware trigger signal to the two cameras in a StereoCamera, this handler is called two times
// In order to produce a single stereo camera image, combining the two camera acquisitions , OnImageGrabbed synchronized the
// image grab results based on their framenumber. For two consecutive(!) images with the same framenumber, a single new stereo
// camera image is produced and emitted through onNewGrabResult (containing both images).
// In case only a single image was acquired and its consecutive image is of a different framenumber, the first image is dropped.
void StereoCameraImageEventHandler::OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult) {
    //std::cout << "OnImageGrabbed event for device " << ptrGrabResult->GetCameraContext() << std::endl;

    if (ptrGrabResult->GrabSucceeded()) {
        // As we modify the stereoImage in potentially different threads (Pylon image event handler) lock the mutex for the update
        mutex.lock();

        intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
        int64_t frameNumber = ptrGrabResult->GetImageNumber();

        uint64_t timeStamp = ptrGrabResult->GetTimeStamp();
        timeStamp = ((timeStamp-cameraTime[cameraContextValue]) + systemTime) / 1000000;

        //std::cout<< "Grabresult from camera" << cameraContextValue << ": frameNumber:  " << frameNumber << ", timestamp: " << timeStamp <<std::endl;

        formatConverter.Convert(pylonImage, ptrGrabResult);
        cv::Mat img(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, (uint8_t *) pylonImage.GetBuffer());

        // If the current image and its framenumber are part of the existing stereoImage, then the stereoImage is complete and gets emitted
        // To make sure stereo image consists of two images at the same time from both cameras, their framenumber is checked
        // We assume that when both cameras are started grabbing at the same time, the framenumbers should match (at each camera acquisition start, the framenumber is reset)
        // Combining images based on timestamps showed to be error prone as the time difference between the two images started to drift for unknown reasons

        //int diff = std::abs((int)(timeStamp - stereoImage.timestamp));
        //std::cout<<"Grabresult diff: "<<diff<<std::endl;
        if (camera.GetDeviceInfo().GetModelName().find("Emu") != String_t::npos){
            stereoImage.timestamp += frameNumber;
        }
        //std::cout << "Image frameNumber: " << stereoImage.frameNumber << std::endl;
        //std::cout << "Received frameNumber: " << frameNumber << " Device id: " << camera.GetDeviceInfo().GetDeviceGUID() <<  std::endl;
        if(stereoImage.frameNumber == frameNumber) {
            // If framenumber matches the already contained image in the stereo image this means the missing second images is now found
            // Cameracontextvalue describes the index of the camera in a basler camera array (main or secondary)
            if(cameraContextValue == 0) {
                stereoImage.img = img.clone();
            } else if(cameraContextValue == 1) {
                stereoImage.imgSecondary = img.clone();
            }
            //std::cout<< "Stereoimage complete: " << stereoImage.frameNumber << " " << stereoImage.timestamp <<std::endl;
            //std::cout<< "-------------------------------" <<std::endl;
            emit onNewGrabResult(stereoImage);
//            std::cout << "STEREO GRAB RESULT: " << stereoImage.frameNumber << " AT TIME: " << stereoImage.timestamp << std::endl;
        } else {
            // Else, we have a "new" stereo image, set the timestamp, image and wait for the second missing one, then emit
            stereoImage.timestamp = timeStamp;
            stereoImage.frameNumber = frameNumber;
            if(cameraContextValue == 0) {
                stereoImage.img = img.clone();
            } else if(cameraContextValue == 1) {
                stereoImage.imgSecondary = img.clone();
            }
        }
        mutex.unlock();
    } else {
        std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;

        // If there is faulty connection /hw. interface problem
        if(ptrGrabResult->GetErrorCode() == 3791651083 || // Error code for "The image stream is out of sync."
           ptrGrabResult->GetErrorCode() == 31 || // Error code for device not functioning
           QString::fromStdString(ptrGrabResult->GetErrorDescription().c_str()).contains("sync")) {
            emit imagesSkipped();
        }
    }

}

// Set the timestamps of both cameras and the system time for a given point in time, used for converting between from cameratime to systemtime
void StereoCameraImageEventHandler::setTimeSynchronization(uint64_t m_mainCameraTime, uint64_t m_secondaryCameraTime, uint64_t m_systemTime) {

    cameraTime[0] = m_mainCameraTime;
    cameraTime[1] = m_secondaryCameraTime;

    systemTime = m_systemTime;
}
