
#include <opencv2/core/utility.hpp>
#include <opencv2/opencv.hpp>
#include <QtConcurrent/QtConcurrent>
#include "imageReader.h"

// Creates a new image reader which opens the given directory and plays back the contained image files
// Image files are read in alphabetically order, using the OpenCV function cv::glob()
// Actual playback process is performed using Qts concurrent thread execution, to no block the GUI thread
// First it is checked wherever a stereo directory structure exists or not, which
ImageReader::ImageReader(QString directory, int playbackSpeed, bool playbackLoop, QObject *parent) :
    QObject(parent), imageDirectory(directory), startTimestamp(0), playbackSpeed(playbackSpeed), noDelay(false), stereoMode(false), playbackLoop(playbackLoop), state(PlaybackState::STOPPED), currentImageIndex(0) {

    if(!imageDirectory.exists()) {
        throw std::invalid_argument( "Image Directory does not exists." );
    }

    // Check if in directory, a stereo structure with directories 0 and 1 for main and secondary camera are present
    if(imageDirectory.exists("0") && imageDirectory.exists("1")) {
        stereoMode = true;

        std::cout<<"ImageReader: Found stereo structure in directory, reading as stereo..." << std::endl;
        std::cout<<"ImageReader: CAUTION: Make sure images in both directories have corresponding names and that no other files are present." << std::endl;

        // Read directory files into list
        // glob sorts the names alphabetically, so filenames without zeros like _19 come after _189
        cv::glob(imageDirectory.filePath("0").toStdString(), filenames, false);
        cv::glob(imageDirectory.filePath("1").toStdString(), filenamesSecondary, false);

        assert(filenames.size() == filenamesSecondary.size());
    } else {

        std::cout<<"ImageReader: Found images in directory, reading..." << std::endl;

        // Read directory files into list
        // glob sorts the names alphabetically, so filenames without zeros like _19 come after _189
        cv::glob(imageDirectory.path().toStdString(), filenames, false);
    }

    std::cout<<"ImageReader: found " << filenames.size() << " images. Ready." << std::endl;

    setPlaybackSpeed(playbackSpeed);
}

ImageReader::~ImageReader() {
    if(state == PlaybackState::PLAYING) {
        QMutexLocker locker(&mutex);
        state = PlaybackState::STOPPED;

        playbackProcess.waitForFinished();
    }
}

// Sets the speed with which the images are played back
// This may be limited by the disk read speed and may not reach speeds above 30fps depending on the file sizes etc.
void ImageReader::setPlaybackSpeed(int fps) {
    QMutexLocker locker(&mutex);
    if(fps <= 0) {
        noDelay = true;
        playbackDelay = 33; // We set playback delay to some value so that the timestamps which are incremented by that delay are valid
    } else {
        playbackDelay = (int) 1000.0f / fps; // delay in ms
    }
}

// Executes the play back of the images through reading them in order, creating a CameraImage object and sending the onNewImage signal
// The play back can be stopped by changing the PlaybackState variable state
// When the playback is finished, a finished signal is send (also send when stopping the play back early)
void ImageReader::run() {

    for(; currentImageIndex < filenames.size(); currentImageIndex++) {
        std::chrono::steady_clock::time_point beginProcess = std::chrono::steady_clock::now();

        if(state != PlaybackState::PLAYING) {
            std::cout<<"Image Reader: Run Loop found Stop/Pause signal"<<std::endl;
            break;
        }

        //std::chrono::steady_clock::time_point beginRead = std::chrono::steady_clock::now();
        cv::Mat img = cv::imread(filenames[currentImageIndex], cv::IMREAD_GRAYSCALE);
        //std::cout<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginRead).count()<<std::endl;

        startTimestamp += playbackDelay;

        // Due to the continue in the next if, we check for end of the loop here
        if(playbackLoop && currentImageIndex == filenames.size()-1) {
            std::cout << "ImageReader: end reached, resetting playback, endless looping " << std::endl;
            currentImageIndex = 0;
        }

        if(!img.data) {
            std::cerr << "Image Reader: Image could not be read, skipping: " << filenames[currentImageIndex] << std::endl;
            continue;
        }

        //std::cout<<"Reading image: "<< filename << std::endl;

        CameraImage cimg;
        cimg.type = CameraImageType::SINGLE_IMAGE_FILE;
        cimg.img = img.clone();
        cimg.timestamp = startTimestamp;
        cimg.filename = filenames[currentImageIndex];
        img.release();

        if(!noDelay) {
            int durProcess = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginProcess).count();
            if(playbackDelay-durProcess > 0) {
                QThread::msleep(playbackDelay-durProcess);
            }
        }
        emit onNewImage(cimg);
    }

    // Playback loop finished, either due to end of files, or pause/stop action
    if(state != PlaybackState::PAUSED) {
        state = PlaybackState::STOPPED;
        currentImageIndex = 0;
    }
    emit finished();
}

// Executes the play back of the stereo images through reading them in order, creating a CameraImage object and sending the onNewImage signal
// The play back can be stopped by changing the PlaybackState variable state
// When the playback is finished, a finished signal is send (also send when stopping the play back early)
// Stereo images are read using QTs concurrent run to (potentially) reduce file read time
void ImageReader::runStereo() {

    // We assume both lists filenames and filenamesSecondary are same length
    for(; currentImageIndex < filenames.size(); currentImageIndex++) {
        std::chrono::steady_clock::time_point beginProcess = std::chrono::steady_clock::now();

        if(state != PlaybackState::PLAYING) {
            std::cout<<"Image Reader: Run Loop found Stop/Pause signal"<<std::endl;
            break;
        }

        QFutureSynchronizer<cv::Mat> synchronizer;
        // Read images from disk asynchronous to save time
        synchronizer.addFuture(QtConcurrent::run(cv::imread, filenames[currentImageIndex], cv::IMREAD_GRAYSCALE));
        synchronizer.addFuture(QtConcurrent::run(cv::imread, filenamesSecondary[currentImageIndex], cv::IMREAD_GRAYSCALE));
        synchronizer.waitForFinished();
        cv::Mat img = synchronizer.futures().at(0).result();
        cv::Mat imgSecondary = synchronizer.futures().at(1).result();

        startTimestamp += playbackDelay;

        // Due to the continue in the next if, we check for end of the loop here
        if(playbackLoop && currentImageIndex == filenames.size()-1) {
            std::cout << "ImageReader: end reached, resetting playback, endless looping " << std::endl;
            currentImageIndex = 0;
        }

        if(!img.data || !imgSecondary.data) {
            std::cerr << "Image Reader: StereoImage could not be read, skipping: " << filenames[currentImageIndex] << " and " << filenamesSecondary[currentImageIndex] << std::endl;
            continue;
        }

        //std::cout<<"Reading image "<< filename << std::endl;

        CameraImage cimg;
        cimg.type = CameraImageType::STEREO_IMAGE_FILE;
        cimg.img = img.clone();
        cimg.imgSecondary = imgSecondary.clone();
        cimg.timestamp = startTimestamp;
        cimg.frameNumber = currentImageIndex;
        cimg.filename = filenames[currentImageIndex];
        img.release();
        imgSecondary.release();

        if(!noDelay) {
            int durProcess = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginProcess).count();
            if(playbackDelay-durProcess > 0) {
                QThread::msleep(playbackDelay-durProcess);
            }
        }
        emit onNewImage(cimg);
    }

    // Playback loop finished, either due to end of files, or pause/stop action
    if(state != PlaybackState::PAUSED) {
        state = PlaybackState::STOPPED;
        currentImageIndex = 0;
    }
    emit finished();
}

// Starts the image reader play back in another thread
void ImageReader::start() {

    if(state == PlaybackState::PLAYING)
        return;

    std::cout<<"Image Reader: Starting ImageReader thread."<<std::endl;

    state = PlaybackState::PLAYING;
    startTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if(stereoMode) {
        playbackProcess = QtConcurrent::run(this, &ImageReader::runStereo);
    } else {
        playbackProcess = QtConcurrent::run(this, &ImageReader::run);
    }
}

// Pause the image play back at the current image, calling start again will proceed at the paused image
void ImageReader::pause() {

    if(state != PlaybackState::PLAYING)
        return;

    std::cout<<"Image Reader: Pausing ImageReader thread."<<std::endl;
    QMutexLocker locker(&mutex);
    state = PlaybackState::PAUSED;

    playbackProcess.waitForFinished();
}

// Stops the image play back and sets the play position back to start, calling start again will play the first image again
void ImageReader::stop() {
    if(state == PlaybackState::STOPPED)
        return;

    std::cout<<"Image Reader: Stopping ImageReader thread."<<std::endl;
    QMutexLocker locker(&mutex);
    state = PlaybackState::STOPPED;
    currentImageIndex = 0;

    playbackProcess.waitForFinished();
}