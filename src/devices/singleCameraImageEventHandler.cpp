
#include "singleCameraImageEventHandler.h"

#include <QDebug>


SingleCameraImageEventHandler::SingleCameraImageEventHandler(QObject* parent) : QObject(parent), cameraTime(0), systemTime(0) {
    // Set the image mode to 8 bit grayscale
    formatConverter.OutputPixelFormat = Pylon::PixelType_Mono8;
}

SingleCameraImageEventHandler::~SingleCameraImageEventHandler() {

}

// Event handler when images are skipped by the camera
// BG: NOTE: According to Basler docs this will only ever get called when grabStrategy is set to LatestImageOnly or LatestImages, which is never the case for us ..(?)
// https://zh.docs.baslerweb.com/pylonapi/cpp/class_pylon_1_1_c_basler_universal_image_event_handler#function-onimagesskipped
void SingleCameraImageEventHandler::OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages) {
    std::cout << "OnImagesSkipped event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
    std::cout << countOfSkippedImages  << " images have been skipped." << std::endl;
    std::cout << std::endl;

    emit imagesSkipped();
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

        quint64 chrono_time  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        /*
        qDebug() <<
            QString("chrono time = ") << QString::number(chrono_time) << 
            QString("; acq timestamp = ") << QString::number(timeStamp) << 
            QString("; diff = ") << QString::number((int64)chrono_time-(int64)timeStamp) ; // túlcsordul ha másikból vonunk ki.. nagyobból kell
        */

        formatConverter.Convert(pylonImage, ptrGrabResult);
        cv::Mat img = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, (uint8_t *) pylonImage.GetBuffer());

        CameraImage result;
        result.type = CameraImageType::LIVE_SINGLE_CAMERA;
        result.img = img.clone();
        result.timestamp = timeStamp;

        emit onNewGrabResult(result);
    } else {
        std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;

        std::cout << " second try " << ptrGrabResult->GetErrorDescription() << std::endl;

        // If there is faulty connection /hw. interface problem
        if(ptrGrabResult->GetErrorCode() == 3791651083 || // Error code for "The image stream is out of sync."
            ptrGrabResult->GetErrorCode() == 31 || // Error code for device not functioning
            QString::fromStdString(ptrGrabResult->GetErrorDescription().c_str()).contains("sync")) {
            emit imagesSkipped();
        }
    }
}

// For conversion of the camera timestamp to system time, a tuple of timestamps from the start of the image acquisition is used
void SingleCameraImageEventHandler::setTimeSynchronization(uint64 m_cameraTime, uint64 m_systemTime) {

    // cameraTime describes the acquisition start in camera time, systemTime the acquisition start in system time
    cameraTime = m_cameraTime;
    systemTime = m_systemTime;
}
