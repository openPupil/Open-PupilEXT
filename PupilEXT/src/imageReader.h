
#ifndef PUPILEXT_IMAGEREADER_H
#define PUPILEXT_IMAGEREADER_H

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <QtGui/QtGui>
#include "devices/camera.h"


enum PlaybackState { STOPPED=0, PAUSED=1, PLAYING=2 };


/**
    Class to read images from disk

    Supports both single and stereo image recordings, in the format created by this software in recording

    For single camera images, all images are in a single directory, without any other files
    For stereo camera images, two directories exist, 0 for main camera images, 1 for secondary camera images, each image have corresponding filenames

    CAUTION:
    OpenCV's cv::glob is used to list the content of the directory, which returns the files in alphabetically order, a preceeding, zeros are necessary for the correct order
    i.e. 10.jpg and 9.jpg are ordered wrong due to that (1<9), 09.jpg and 10.jpg do not have the problem (0<1)

    CAUTION:
    Depending on the disk read speed, high replay framerates may not be possible due to disk read speed and filesize

    TODO Improvement: Speed could be improved by pre-loading images of the directory into memory, disk read speed is limiting the playback speed

*/
class ImageReader : public QObject {
Q_OBJECT


public:

    explicit ImageReader(QString directory, int playbackSpeed = 30, bool playbackLoop=false, QObject *parent = 0);

    ~ImageReader() override;

    bool isPlaying() {
        return PlaybackState::PLAYING == state;
    }

    bool isPaused() {
        return PlaybackState::PAUSED == state;
    }

    bool isStopped() {
        return PlaybackState::STOPPED == state;
    }

    bool isStereo(){
        return stereoMode;
    }

    int getPlaybackSpeed() {
        return playbackSpeed;
    }

    void setPlaybackSpeed(int fps);

    int getPlaybackLoop() {
        return playbackLoop;
    }

    void setPlaybackLoop(bool loop) {
        playbackLoop = loop;
    }

private:

    QFuture<void> playbackProcess;
    QMutex mutex;

    QDir imageDirectory;

    std::vector<std::string> filenames, filenamesSecondary;

    uint64 startTimestamp;
    int playbackSpeed;
    int playbackDelay;

    int currentImageIndex;

    PlaybackState state;

    bool stereoMode;
    bool noDelay;
    bool playbackLoop;

    void run();
    void runStereo();

public slots:

    void start();
    void stop();
    void pause();

signals:

    void onNewImage(const CameraImage &image);
    void finished();

};


#endif //PUPILEXT_IMAGEREADER_H
