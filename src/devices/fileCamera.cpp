
#include "fileCamera.h"

// Creates a virtual camera, behaving like a physical camera while playing image files from the given directory
// Playback speed in frames per second can be adjusted through "playbackSpeed"
FileCamera::FileCamera(const QString &directory, int playbackSpeed, bool playbackLoop, QObject *parent) : Camera(parent),
                        imageReader(new ImageReader(directory, playbackSpeed, playbackLoop)),
                        frameCounter(new FrameRateCounter()),
                        stereoCameraCalibration(nullptr),
                        cameraCalibration(nullptr),
                        calibrationThread(nullptr) {

    connect(imageReader, SIGNAL(onNewImage(CameraImage)), this, SIGNAL(onNewGrabResult(CameraImage)));
    connect(imageReader, SIGNAL(onNewImage(CameraImage)), frameCounter, SLOT(count()));
    connect(imageReader, SIGNAL(finished()), this, SIGNAL(finished()));

    // GB: added this line
    connect(imageReader, SIGNAL(endReached()), this, SIGNAL(endReached()));

    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    connect(frameCounter, SIGNAL(framecount(int)), this, SIGNAL(framecount(int)));

    // Its possible to load existing calibration files for offline pupil measuring
    // CAUTION: calibration file must correspond to the camera with which the offline files were recorded otherwise the calculations are incorrect
    if(imageReader->isStereo()) {
        stereoCameraCalibration = new StereoCameraCalibration();
        calibrationThread = new QThread();
        // Calibration worker thread
        stereoCameraCalibration->moveToThread(calibrationThread);
        connect(calibrationThread, SIGNAL (finished()), calibrationThread, SLOT (deleteLater()));
        calibrationThread->start();
        calibrationThread->setPriority(QThread::HighPriority);
    } else {
        cameraCalibration = new CameraCalibration();
        calibrationThread = new QThread();
        // calibration worker thread
        cameraCalibration->moveToThread(calibrationThread);
        connect(calibrationThread, SIGNAL (finished()), calibrationThread, SLOT (deleteLater()));
        calibrationThread->start();
        calibrationThread->setPriority(QThread::HighPriority);
    }

    open = true;
}

FileCamera::~FileCamera() = default;

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
    imageReader->step1frame(true);
}

void FileCamera::step1framePrev() {
    if(imageReader->isPlaying())
        imageReader->pause();
    imageReader->step1frame(false);
}