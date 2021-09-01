
#ifndef PUPILEXT_FILECAMERA_H
#define PUPILEXT_FILECAMERA_H

/**
    @author Moritz Lode
*/

#include "camera.h"
#include "../frameRateCounter.h"
#include "../imageReader.h"
#include "../stereoCameraCalibration.h"

/**
    FileCamera represents virtual camera which replays images from disk given a directory of images

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
    explicit FileCamera(const QString &directory, int playbackSpeed = 30, bool playbackLoop = false, QObject *parent = 0);

    ~FileCamera() override;

    bool isOpen() override;
    void close() override;
    CameraImageType getType() override;

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

    CameraCalibration *getCameraCalibration();
    StereoCameraCalibration *getStereoCameraCalibration();

private:
    FrameRateCounter *frameCounter;
    ImageReader *imageReader;

    bool open;

    CameraCalibration *cameraCalibration;
    StereoCameraCalibration *stereoCameraCalibration;
    QThread *calibrationThread;

signals:

    void fps(double fps);
    void framecount(int framecount);

    void finished();
};

#endif //PUPILEXT_FILECAMERA_H
