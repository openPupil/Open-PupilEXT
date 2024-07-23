
#ifndef PUPILEXT_FILECAMERA_H
#define PUPILEXT_FILECAMERA_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "camera.h"
#include "../frameRateCounter.h"
#include "../imageReader.h"
#include "../stereoCameraCalibration.h"

//#include "../offlineEventLogReader.h"

/**
    FileCamera represents virtual camera which replays images from disk given a directory of images

    NOTE: Modified by Gábor Bényei, 2023 jan
    BG NOTE: 
        Added methods for FileCamera to work nicely with the new PlaybackControlDialog

    start(): start file playback
    stop(): stop file playback
    getCameraCalibration(): return the camera calibration object, as it is a virtual camera, the calibration files of the camera with which the images were recorded is loaded here
    getStereoCameraCalibration(): if images are stereo camera recordings

signals:
    fps(double fps): frames per second of the file camera playback
    framecount(int framecount): current framecount of the file camera playback
    finished(): signals finishing of the image file playback
*/
class FileCamera : public Camera
{
    Q_OBJECT

public:
    explicit FileCamera(const QString &directory, QMutex *imageMutex,  QWaitCondition *imagePublished, QWaitCondition *imageProcessed, int playbackSpeed = 30, bool playbackLoop = false, QObject *parent = 0);

    ~FileCamera() override;

    bool isOpen() override;
    void close() override;
    CameraImageType getType() override;

    void startGrabbing() override;
    void stopGrabbing() override;

    void start();
    void stop();
    void pause();

    int getPlaybackSpeed()
    {
        return imageReader->getPlaybackSpeed();
    }

    void setPlaybackSpeed(int fps)
    {
        imageReader->setPlaybackSpeed(fps);
    }

    int getPlaybackLoop()
    {
        return imageReader->getPlaybackLoop();
    }

    void setPlaybackLoop(bool loop)
    {
        imageReader->setPlaybackLoop(loop);
    }

    bool isGrabbing(){
        return imageReader->isPlaying();
    }

    // GB added begin
    bool isPlaying() {
        return imageReader->isPlaying();
    }
    QString getImageDirectoryName() {
        return imageReader->getImageDirectoryName();
    }
    QString getImageWidth() {
        return QString::number(imageReader->getImageWidth());
    }
    QString getImageHeight() {
        return QString::number(imageReader->getImageHeight());
    }

    cv::Mat getStillImageSingle(int frameNumber) {
        return imageReader->getStillImageSingle(frameNumber);
    }
    std::vector<cv::Mat> getStillImageStereo(int frameNumber) {
        return imageReader->getStillImageStereo(frameNumber);
    }
    int getFrameNumberForTimestamp(uint64_t timestamp) {
        return imageReader->getFrameNumberForTimestamp(timestamp);
    }
    uint64_t getTimestampForFrameNumber(int frameNumber) {
        return imageReader->getTimestampForFrameNumber(frameNumber);
    }
    int getNumImagesTotal() {
        return imageReader->getNumImagesTotal();
    }
    uint64_t getRecordingDuration() {
        return imageReader->getRecordingDuration();
    }
    void seekToFrame(int frameNumber) {
        imageReader->seekToFrame(frameNumber);
    }
    /*uint64_t getLastCommissionedTimestamp() {
        return imageReader->getLastCommissionedTimestamp();
    }*/
    int getLastCommissionedFrameNumber() {
        return imageReader->getLastCommissionedFrameNumber();
    }
    // GB added end

    int getImageROIwidth() override;
    int getImageROIheight() override;
    int getImageROIwidthMax() override;
    int getImageROIheightMax() override;
    int getImageROIoffsetX() override; 
    int getImageROIoffsetY() override;
    QRectF getImageROI() override;

    CameraCalibration *getCameraCalibration();
    StereoCameraCalibration *getStereoCameraCalibration();

    ImageReader *getImageReader() const;

private:
    FrameRateCounter *frameCounter;
    ImageReader *imageReader;

    bool open;

    CameraCalibration *cameraCalibration;
    StereoCameraCalibration *stereoCameraCalibration;
    QThread *calibrationThread;

    // GB added begin
    //OfflineEventLogReader *offlineEventLogReader;
    // GB added end

signals:

    void fps(double fps);
    void framecount(int framecount);

    void finished();
    void paused();

    // GB added begin
    void endReached();
    // GB added end

public slots:
    // GB added begin
    void step1frameNext();
    void step1framePrev();
    // GB added end

};

#endif //PUPILEXT_FILECAMERA_H
