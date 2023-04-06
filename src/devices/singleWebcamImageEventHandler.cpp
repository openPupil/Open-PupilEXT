
#include "singleWebcamImageEventHandler.h"


SingleWebcamImageEventHandler::SingleWebcamImageEventHandler(QObject* parent) : QObject(parent) {}

SingleWebcamImageEventHandler::~SingleWebcamImageEventHandler() {}

void SingleWebcamImageEventHandler::OnImageGrabbed(const cv::Mat &image) {

    uint64 timeStamp = QDateTime::currentMSecsSinceEpoch(); // std::chrono::duration_cast<std::chrono::milliseconds> (); //0; //ptrGrabResult->GetTimeStamp();
    
    // cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

    CameraImage result;
    result.type = CameraImageType::LIVE_SINGLE_WEBCAM;
    result.img = image.clone();
    result.timestamp = timeStamp;

    emit onNewGrabResult(result);
}

