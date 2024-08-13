#pragma once

/**
    @author Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include <QtCore/QObject>
#include <QtGui/QtGui>
#include "devices/camera.h"
#include <vector>
#include <algorithm>


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
*/
class ImageReader : public QObject {
Q_OBJECT


public:

    explicit ImageReader(QString directory, QMutex *imageMutex, QWaitCondition *imagePublished, QWaitCondition *imageProcessed, int playbackSpeed = 30, bool playbackLoop=false, QObject *parent = 0);

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

    cv::Mat getStillImageSingle(int frameNumber);
    std::vector<cv::Mat> getStillImageStereo(int frameNumber);

    QString getImageDirectoryName() {
        return imageDirectory.absolutePath();
    }
    int getImageWidth() {
        return foundImageWidth;
    }
    int getImageHeight() {
        return foundImageHeight;
    }
    
    int getFrameNumberForTimestamp(uint64_t &timestamp) {
        // Global private var and indexing back from currentImageIndex, this is the fastest I guess.
        // However, there is a conversion from uint64_t to quint64 every time the comparison happens..
        // I could not get around this, as CameraImage employs uint64_t 
        // (I guess because in case of real cameras, it gets the value from Pylon, which uses uint64_t)
        for(imgNumSeekerIdx = acqTimestamps.size() - 1; imgNumSeekerIdx>=0; imgNumSeekerIdx--) {
            if(acqTimestamps[imgNumSeekerIdx] == timestamp) 
                return imgNumSeekerIdx;
        }
        return currentImageIndex; 
    }
    
    uint64_t getTimestampForFrameNumber(int frameNumber) {
        if(acqTimestamps.size() > frameNumber)
            return acqTimestamps[frameNumber];
        else
            return 0;
    }

    /*uint64_t getLastCommissionedTimestamp() {
        if(lastCommissionedFrameNumber>0)
            return acqTimestamps[lastCommissionedFrameNumber];
        else 
            return 0;
    }*/
    int getLastCommissionedFrameNumber() {
        if(lastCommissionedFrameNumber>0)
            return lastCommissionedFrameNumber;
        else 
            return 0;
    }

    int getNumImagesTotal() {
        return (int)acqTimestamps.size();
    };

    uint64_t getRecordingDuration() {
        return acqTimestamps[acqTimestamps.size()-1] - acqTimestamps[0];
    }
    void seekToFrame(int frameNumber) {
        if(frameNumber<0)
            frameNumber=0;
        currentImageIndex = frameNumber;
    }

    void setSynchronised(bool synchronised);

private:

    QFuture<void> playbackProcess;
    QMutex mutex;
    QMutex *imageMutex;
    QWaitCondition *imagePublished;
    QWaitCondition *imageProcessed;

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
    bool synchronised;

    std::vector<quint64> acqTimestamps;
    int imgNumSeekerIdx = 0;
    int lastCommissionedFrameNumber = -1; 

    int foundImageWidth = 0;
    int foundImageHeight = 0;

    void purgeFilenamesVector(std::vector<cv::String> &filenames);

    void run();
    void runImpl(std::chrono::steady_clock::time_point& startTime, std::chrono::duration<int, std::milli> elapsedDuration, cv::Mat &img);
    void runStereo();
    void runStereoImpl(std::chrono::steady_clock::time_point& startTime, std::chrono::duration<int, std::milli> elapsedDuration, cv::Mat &img, cv::Mat &imgSecondary);



public slots:

    void start();
    void stop();
    void pause();

    void step1frame(bool next);

signals:

    void onNewImage(CameraImage image);
    void finished();

    void paused();

    // NOTE: we need this specific signal, to let imagePlaybackControlDialog know
    // that the playback finished automatically. The dialog alonw only knows about 
    // playback changes that were caused by GUI interactions. Without this signal, it would be clueless
    void endReached();

};
