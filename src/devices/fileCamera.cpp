
#include <iostream>
#include "fileCamera.h"

// Creates a virtual camera, behaving like a physical camera while playing image files from the given directory
// Playback speed in frames per second can be adjusted through "playbackSpeed"
FileCamera::FileCamera(const QString &directory, QMutex *imageMutex, QWaitCondition *imagePublished, QWaitCondition *imageProcessed, int playbackSpeed, bool playbackLoop, QObject *parent) : Camera(parent),
                        imageReader(new ImageReader(directory, imageMutex, imagePublished, imageProcessed, playbackSpeed, playbackLoop, this)),
                        frameCounter(new FrameRateCounter(this)),
                        stereoCameraCalibration(nullptr),
                        cameraCalibration(nullptr),
                        calibrationThread(nullptr) {

    connect(imageReader, SIGNAL(onNewImage(CameraImage)), this, SIGNAL(onNewGrabResult(CameraImage)));
    connect(imageReader, SIGNAL(onNewImage(CameraImage)), frameCounter, SLOT(count()));
    connect(imageReader, SIGNAL(finished()), this, SIGNAL(finished()));

    connect(imageReader, SIGNAL(endReached()), this, SIGNAL(endReached()));
    connect(imageReader, SIGNAL(paused()), this, SIGNAL(paused()));

    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    connect(frameCounter, SIGNAL(framecount(int)), this, SIGNAL(framecount(int)));

    // Its possible to load existing calibration files for offline pupil measuring
    // CAUTION: calibration file must correspond to the camera with which the offline files were recorded otherwise the calculations are incorrect
    if(imageReader->isStereo()) {
        stereoCameraCalibration = new StereoCameraCalibration(nullptr);
        calibrationThread = new QThread();
        // Calibration worker thread
        stereoCameraCalibration->moveToThread(calibrationThread);
        calibrationThread->start();
        calibrationThread->setPriority(QThread::HighPriority);
    } else {
        cameraCalibration = new CameraCalibration(nullptr);
        calibrationThread = new QThread();
        // calibration worker thread
        cameraCalibration->moveToThread(calibrationThread);
        calibrationThread->start();
        calibrationThread->setPriority(QThread::HighPriority);
    }

    open = true;
}

FileCamera::~FileCamera(){
    if (cameraCalibration != nullptr)
        cameraCalibration->deleteLater();
    if (stereoCameraCalibration != nullptr)
        stereoCameraCalibration->deleteLater();
    if (calibrationThread != nullptr) {
        calibrationThread->quit();
        calibrationThread->deleteLater();
    }
}

void FileCamera::startGrabbing(){
    start();
}
void FileCamera::stopGrabbing(){
    pause();        
}

bool FileCamera::isOpen() {
    return open;
}

void FileCamera::close() {
    imageReader->stop();
    open = false;
}

void FileCamera::start() {
    imageReader->start();
}

void FileCamera::pause() {
    imageReader->pause();
}

void FileCamera::stop() {
    imageReader->stop();
}

CameraImageType FileCamera::getType() {
    return imageReader->isStereo() ? CameraImageType::STEREO_IMAGE_FILE : CameraImageType::SINGLE_IMAGE_FILE;
}

CameraCalibration *FileCamera::getCameraCalibration() {
    return cameraCalibration;
}

StereoCameraCalibration *FileCamera::getStereoCameraCalibration() {
    return stereoCameraCalibration;
}


void FileCamera::step1frameNext() {
    if(imageReader->isPlaying())
        imageReader->pause();
    //imageReader->step1frame(true);
}

void FileCamera::step1framePrev() {
    if(imageReader->isPlaying())
        imageReader->pause();
    //imageReader->step1frame(false);
}

int FileCamera::getImageROIwidth(){
    return imageReader->getImageWidth();
}

int FileCamera::getImageROIheight(){
    return imageReader->getImageHeight();
}

// Yet these are just placeholders in case of filecamera, but could be meaningful if the imagerec-meta was read, and the sensor size was read from it
int FileCamera::getImageROIwidthMax(){
    return getImageROIwidth();
}

// Yet these are just placeholders in case of filecamera, but could be meaningful if the imagerec-meta was read, and the sensor size was read from it
int FileCamera::getImageROIheightMax(){
    return getImageROIheight();
}

int FileCamera::getImageROIoffsetX(){
    return 0;
}

int FileCamera::getImageROIoffsetY(){
    return 0;
}

QRectF FileCamera::getImageROI(){
    return QRectF(0,0,getImageROIwidth(), getImageROIheight());
}

ImageReader *FileCamera::getImageReader() const {
    return imageReader;
}
