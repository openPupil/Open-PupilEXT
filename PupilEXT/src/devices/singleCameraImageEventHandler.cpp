
#include "singleCameraImageEventHandler.h"


SingleCameraImageEventHandler::SingleCameraImageEventHandler(QObject* parent) : QObject(parent), cameraTime(0), systemTime(0) {
    // Set the image mode to 8 bit grayscale
    formatConverter.OutputPixelFormat = Pylon::PixelType_Mono8;
}

SingleCameraImageEventHandler::~SingleCameraImageEventHandler() {

}

// Event handler when images are skipped by the camera
void SingleCameraImageEventHandler::OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages) {
    std::cout << "OnImagesSkipped event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
    std::cout << countOfSkippedImages  << " images have been skipped." << std::endl;
    std::cout << std::endl;
}

// Event handler when a new image was grabbed from the camera
// Receives the grab result pointer which contains the grabbed image and meta information
// After a successful image grab, the image timestamp is converted to system time using the previously acquired starting timestamps
// The image is converted to a 8 bit grayscale cv::Mat format and bundled with meta information in a CameraImage object
// Emits signal onNewGrabResult containing the newly grabbed CameraImage
void SingleCameraImageEventHandler::OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult) {
    //std::cout << "OnImageGrabbed event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;

    // Image grabbed successfully?
    if (ptrGrabResult->GrabSucceeded()) {
        //std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;
        //std::cout << "SizeY: " << ptrGrabResult->GetHeight() << std::endl;

        uint64 timeStamp = ptrGrabResult->GetTimeStamp();
        // cameraTime describes the acquisition start in camera time, systemTime the acquisition start in system time
        timeStamp = ((timeStamp-cameraTime) / 1000000) + systemTime;

        formatConverter.Convert(pylonImage, ptrGrabResult);
        cv::Mat img = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, (uint8_t *) pylonImage.GetBuffer());

        CameraImage result;
        result.type = CameraImageType::LIVE_SINGLE_CAMERA;
        result.img = img.clone();
        result.timestamp = timeStamp;

        emit onNewGrabResult(result);
    } else {
        std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
    }
}

// For conversion of the camera timestamp to system time, a tuple of timestamps from the start of the image acquisition is used
void SingleCameraImageEventHandler::setTimeSynchronization(uint64 m_cameraTime, uint64 m_systemTime) {

    // cameraTime describes the acquisition start in camera time, systemTime the acquisition start in system time
    cameraTime = m_cameraTime;
    systemTime = m_systemTime;
}
