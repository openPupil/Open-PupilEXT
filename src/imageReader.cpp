
#include <opencv2/core/utility.hpp>
#include <opencv2/opencv.hpp>
#include <QtConcurrent/QtConcurrent>
#include "imageReader.h"

// Creates a new image reader which opens the given directory and plays back the contained image files
// Image files are read in alphabetically order, using the OpenCV function cv::glob()
// Actual playback process is performed using Qts concurrent thread execution, to no block the GUI thread
// First it is checked wherever a stereo directory structure exists or not, which
ImageReader::ImageReader(QString directory, QMutex *imageMutex, QWaitCondition *imagePublished, QWaitCondition *imageProcessed, int playbackSpeed, bool playbackLoop, QObject *parent) :
    QObject(parent),
    imageDirectory(directory),
    startTimestamp(0),
    playbackSpeed(playbackSpeed),
    noDelay(false),
    stereoMode(false),
    synchronised(false),
    playbackLoop(playbackLoop),
    state(PlaybackState::STOPPED),
    imageMutex(imageMutex),
    imagePublished(imagePublished),
    imageProcessed(imageProcessed),
    currentImageIndex(0) {

    if(!imageDirectory.exists()) {
        throw std::invalid_argument( "Image Directory does not exist." );
    }

    // qDebug() << imageDirectory << Qt::endl;
    // qDebug() << imageDirectory.exists("0") << Qt::endl;
    // qDebug() << imageDirectory.exists("1") << Qt::endl;

    // Check if in directory, a stereo structure with directories 0 and 1 for main and secondary camera are present
    if(imageDirectory.exists("0") && imageDirectory.exists("1")) {
        stereoMode = true;

//        qDebug()<<"ImageReader: Found stereo structure in directory, reading as stereo..." ;
//        qDebug()<<"ImageReader: CAUTION: Make sure images in both directories have corresponding names and that no other files are present." ;

        // Read directory files into list
        // glob sorts the names alphabetically, so filenames without zeros like _19 come after _189
        cv::glob(imageDirectory.filePath("0").toStdString(), filenames, false);
        cv::glob(imageDirectory.filePath("1").toStdString(), filenamesSecondary, false);

        purgeFilenamesVector(filenames);
        purgeFilenamesVector(filenamesSecondary);
        // TODO: add check to ensure the same timestamp has images in both directories

        assert(filenames.size() == filenamesSecondary.size());
    } else {

//        qDebug()<<"ImageReader: Found images in directory, reading..." ;

        // Read directory files into list
        // glob sorts the names alphabetically, so filenames without zeros like _19 come after _189
        cv::glob(imageDirectory.path().toStdString(), filenames, false);

        purgeFilenamesVector(filenames);
    }

//    qDebug()<<"ImageReader: found " << filenames.size() << " images. Ready." ;

    bool ok;
    for(size_t u = 0; u < filenames.size(); u++) {
        acqTimestamps.push_back( QFileInfo(QString::fromStdString(filenames[u])).baseName().toULongLong(&ok, 10) );
    }

    // GB: measure found images px size, for documenting in meta-snapshot
    int b=0;
    cv::Mat checkImg;
    while(b < filenames.size() && (foundImageWidth<=0 || foundImageHeight<=0)) {
        checkImg = cv::imread(filenames[b], cv::IMREAD_GRAYSCALE);
        foundImageWidth = checkImg.cols;
        foundImageHeight = checkImg.rows;
    }

    setPlaybackSpeed(playbackSpeed);
}

ImageReader::~ImageReader() {
    if(state == PlaybackState::PLAYING) {
        state = PlaybackState::STOPPED;

        playbackProcess.waitForFinished();
    }
}

// Sets the speed with which the images are played back
// This may be limited by the disk read speed and may not reach speeds above 30fps depending on the file sizes etc.
void ImageReader::setPlaybackSpeed(int fps) {
    if(fps <= 0) {
        noDelay = true;
        playbackSpeed = 0;
        playbackDelay = 33; // We set playback delay to some value so that the timestamps which are incremented by that delay are valid
    } else {
        noDelay = false;
        playbackSpeed = fps;
        playbackDelay = (int) (1000.0f / fps); // delay in ms
    }
}

// Executes the play back of the images through reading them in order, creating a CameraImage object and sending the onNewImage signal
// The play back can be stopped by changing the PlaybackState variable state
// When the playback is finished, a finished signal is send (also send when stopping the play back early)
void ImageReader::run() {

    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    while(currentImageIndex < filenames.size()) {
        if (state != PlaybackState::PLAYING) {
//            qDebug() << "Image Reader: Run Loop found Stop/Pause signal" ;
            break;
        }


        std::chrono::steady_clock::time_point beginProcess = std::chrono::steady_clock::now();
        std::chrono::duration<int, std::milli> elapsedDuration = std::chrono::duration_cast<std::chrono::milliseconds>(beginProcess - startTime);
        int elapsedTime = elapsedDuration.count();
        cv::Mat img = cv::imread(filenames[currentImageIndex], cv::IMREAD_GRAYSCALE);
        if(img.data && elapsedTime >= playbackDelay) {

            if (synchronised){
                const QMutexLocker locker(imageMutex);
//                qDebug() << "imageReader locking";
//                qDebug() << "Current image index: " << currentImageIndex;
                runImpl(startTime, elapsedDuration, img);
                //qDebug() << "New Image generated, unlocking";
                imageProcessed->wakeAll();
                if (state == PlaybackState::PLAYING) {
//                    qDebug() << "Waiting for processing";
                    imagePublished->wait(imageMutex);
                }
            }
            else {
//                qDebug() << "Current image index: " << currentImageIndex;
                runImpl(startTime, elapsedDuration, img);
            }


            currentImageIndex++;
        }
        else if (!img.data){
//            std::cerr << "Image Reader: Image could not be read, skipping: " << filenames[currentImageIndex] ;
            currentImageIndex++;
        }
        if (playbackLoop && currentImageIndex == filenames.size()) {
//            qDebug() << "ImageReader: end reached, resetting playback, endless looping " ;
            currentImageIndex = 0;
        }
        //qDebug() << "Looping";
    }
//    qDebug() << "Loop ended";

    // Playback loop finished, either due to end of files, or pause/stop action
    if(state != PlaybackState::PAUSED) {
        state = PlaybackState::STOPPED;
        // to signal when we automatically reached the end
        if(currentImageIndex == filenames.size()){
            emit endReached();
            lastCommissionedFrameNumber = -1; // corner case
        }
        currentImageIndex = 0;
//        qDebug() << "finished()";
        emit finished();
    }
    else {
//        qDebug() << "paused()";
        emit paused();
    }
    imageProcessed->wakeAll();
    imagePublished->wakeAll();
}

void ImageReader::runImpl(std::chrono::steady_clock::time_point& startTime, std::chrono::duration<int, std::milli> elapsedDuration,
                          cv::Mat& img){
    startTime += elapsedDuration;
    //std::chrono::steady_clock::time_point beginRead = std::chrono::steady_clock::now();

    //std::cout<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginRead).count();

    startTimestamp += playbackDelay;

    //std::cout<<"Reading image: "<< filename ;

    CameraImage cimg;
    cimg.type = CameraImageType::SINGLE_IMAGE_FILE;
    cimg.img = img.clone();
    //cimg.timestamp = startTimestamp;
    cimg.timestamp = acqTimestamps[currentImageIndex]; // using the file name, not the time of image reading operation
    cimg.frameNumber = currentImageIndex; // playbackControlDialog needs it
    cimg.filename = filenames[currentImageIndex];
    img.release();

    /*
    if (!noDelay) {
        int durProcess = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - beginProcess).count();
        if (playbackDelay - durProcess > 0) {
            QThread::msleep(playbackDelay - durProcess);
        }
    }
    */
//    qDebug() << "lastCommissionedFrameNumber: " << lastCommissionedFrameNumber;
    lastCommissionedFrameNumber = currentImageIndex;

    emit onNewImage(cimg);
}

// Executes the play back of the stereo images through reading them in order, creating a CameraImage object and sending the onNewImage signal
// The play back can be stopped by changing the PlaybackState variable state
// When the playback is finished, a finished signal is send (also send when stopping the play back early)
// Stereo images are read using QTs concurrent run to (potentially) reduce file read time
void ImageReader::runStereo() {


    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    // We assume both lists filenames and filenamesSecondary are same length
    while(currentImageIndex < filenames.size()) {
        if (state != PlaybackState::PLAYING) {
            qDebug() << "Image Reader: Run Loop found Stop/Pause signal" ;
            break;
        }

        //qDebug() << "imageReader locking";
        std::chrono::steady_clock::time_point beginProcess = std::chrono::steady_clock::now();
        std::chrono::duration<int, std::milli> elapsedDuration = std::chrono::duration_cast<std::chrono::milliseconds>(beginProcess - startTime);
        int elapsedTime = elapsedDuration.count();
        QFutureSynchronizer<cv::Mat> synchronizer;
        // Read images from disk asynchronous to save time
        synchronizer.addFuture(QtConcurrent::run(cv::imread, filenames[currentImageIndex], cv::IMREAD_GRAYSCALE));
        synchronizer.addFuture(QtConcurrent::run(cv::imread, filenamesSecondary[currentImageIndex], cv::IMREAD_GRAYSCALE));
        synchronizer.waitForFinished();
        cv::Mat img = synchronizer.futures().at(0).result();
        cv::Mat imgSecondary = synchronizer.futures().at(1).result();

        if(img.data && imgSecondary.data && elapsedTime >= playbackDelay) {

            if (synchronised) {
                const QMutexLocker locker(imageMutex);
                qDebug() << "imageReader locking";
                qDebug() << "Current image index: " << currentImageIndex;
                runStereoImpl(startTime, elapsedDuration, img, imgSecondary);
                imageProcessed->wakeAll();
                if (state == PlaybackState::PLAYING) {
                    imagePublished->wait(imageMutex);
                }
            } else {
                qDebug() << "Current image index: " << currentImageIndex;
                runStereoImpl(startTime, elapsedDuration, img, imgSecondary);
            }
            currentImageIndex++;
        }
        else if (!img.data || !imgSecondary.data){
            std::cerr << "Image Reader: Image could not be read, skipping: " << filenames[currentImageIndex] ;
            currentImageIndex++;
        }
        if (playbackLoop && currentImageIndex == filenames.size()) {
            qDebug() << "ImageReader: end reached, resetting playback, endless looping " ;
            currentImageIndex = 0;
        }
    }
    qDebug() << "Loop ended";

    // Playback loop finished, either due to end of files, or pause/stop action
    if(state != PlaybackState::PAUSED) {
        state = PlaybackState::STOPPED;
        // to signal when we automatically reached the end
        if(currentImageIndex == filenames.size()){
            emit endReached();
            qDebug() << "endReached";
            lastCommissionedFrameNumber = -1; // corner case
        }
        currentImageIndex = 0;
        qDebug() << "finished()";
        emit finished();
    }

    imageProcessed->wakeAll();
    imagePublished->wakeAll();

}

void
ImageReader::runStereoImpl(std::chrono::steady_clock::time_point &startTime, std::chrono::duration<int, std::milli> elapsedDuration,
                           cv::Mat &img, cv::Mat &imgSecondary) {

    startTime += elapsedDuration;
    //std::chrono::steady_clock::time_point beginRead = std::chrono::steady_clock::now();

    //std::cout<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginRead).count();

    startTimestamp += playbackDelay;

    //std::cout<<"Reading image: "<< filename ;

    CameraImage cimg;
    cimg.type = CameraImageType::STEREO_IMAGE_FILE;
    cimg.img = img.clone();
    cimg.imgSecondary = imgSecondary.clone();
    //cimg.timestamp = startTimestamp;
    cimg.timestamp = acqTimestamps[currentImageIndex]; // using the file name, not the time of image reading operation
    cimg.frameNumber = currentImageIndex;
    cimg.filename = filenames[currentImageIndex];
    img.release();
    imgSecondary.release();

    /*
    if (!noDelay) {
        int durProcess = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - beginProcess).count();
        if (playbackDelay - durProcess > 0) {
            QThread::msleep(playbackDelay - durProcess);
        }
    }
    */
    lastCommissionedFrameNumber = currentImageIndex;

    emit onNewImage(cimg);
}

// Starts the image reader play back in another thread
void ImageReader::start() {

    if(state == PlaybackState::PLAYING)
        return;

//    qDebug()<<"Image Reader: Starting ImageReader thread.";

    state = PlaybackState::PLAYING;
    startTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

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

    qDebug()<<"-----------------------------------------------------------Image Reader: Pausing ImageReader thread.";
    state = PlaybackState::PAUSED;

    imageProcessed->wakeAll();
    imagePublished->wakeAll();
    playbackProcess.waitForFinished();

}

// Stops the image play back and sets the play position back to start, calling start again will play the first image again
void ImageReader::stop() {
    if(state == PlaybackState::STOPPED)
        return;

    qDebug()<<"Image Reader: Stopping ImageReader thread.";
    state = PlaybackState::STOPPED;
    //currentImageIndex = 0;

    imageProcessed->wakeAll();
    imagePublished->wakeAll();
    playbackProcess.waitForFinished();
}


void ImageReader::purgeFilenamesVector(std::vector<cv::String> &filenames) {

    if(filenames.empty())
        return;

    // we find the most frequent extension in the folder (which must be the image extension we use)
    std::vector<cv::String> fileExtensions;
    std::vector<int> fileExtensionFreqencies;
    size_t dotPos = cv::String::npos;
    cv::String currExt;
    //
    for(int c=0; c<filenames.size(); c++) {
        dotPos = filenames[c].find_last_of('.');
        if(dotPos != cv::String::npos) {
            currExt = filenames[c].substr(dotPos, filenames[c].length()-(dotPos));

            auto whereInVector = std::find(fileExtensions.begin(), fileExtensions.end(), currExt);
            if(whereInVector == fileExtensions.end()) { // if the extension can NOT be found in the vector, add it
                fileExtensions.push_back(currExt);
                fileExtensionFreqencies.push_back(1);
                //std::cout << "Found files in the folder with the following extension = " << currExt ;
            } else {
                fileExtensionFreqencies[(int)(whereInVector-fileExtensions.begin())]++;
            }
        } else {
            // töröljük simán a listából
        }
    }
    int mostFreqIndex = std::max_element(fileExtensionFreqencies.begin(), fileExtensionFreqencies.end())-fileExtensionFreqencies.begin();
    //std::cout << "Freq of the most frequent extension = " << fileExtensionFreqencies[mostFreqIndex] ;
    //std::cout << "The most frequent extension = " << fileExtensions[mostFreqIndex] ;


    size_t iterUntil = filenames.size()-1;
    size_t iterIndex = 0;
    bool flaggedForDeletion = false;
    while(iterIndex <= iterUntil) {
        //filenames[iter_index].find(s2) != std::string::npos
        flaggedForDeletion = false;

        dotPos = filenames[iterIndex].find_last_of('.');
        if(dotPos != cv::String::npos) {
            currExt = filenames[iterIndex].substr(dotPos, filenames[iterIndex].length()-(dotPos)); //also handles if the filename ends with dot but there are no characters afterwards

            if(currExt != fileExtensions[mostFreqIndex]) {
                flaggedForDeletion = true;
            }
        } else {
            flaggedForDeletion = true;
        }

        if(flaggedForDeletion) {
            std::cout << "Deleting unusual filename from detected filenames vector = " << filenames[iterIndex] ;
            filenames.erase(filenames.begin()+iterIndex); //no index increment, because element was deleted, but iter_until needs to decrease
            iterUntil--;
        } else {
            iterIndex++;
        }
    }

}

cv::Mat ImageReader::getStillImageSingle(int frameNumber) {
    if(filenames.size() > frameNumber)
        return cv::imread(filenames[frameNumber], cv::IMREAD_GRAYSCALE);
    else
        return cv::Mat();
}

std::vector<cv::Mat> ImageReader::getStillImageStereo(int frameNumber) {
    if(filenames.size() >= frameNumber && filenamesSecondary.size() > frameNumber) {
        std::vector<cv::Mat> vec = {cv::imread(filenames[frameNumber], cv::IMREAD_GRAYSCALE), cv::imread(filenamesSecondary[frameNumber], cv::IMREAD_GRAYSCALE)};
        return vec;
    } else
        return std::vector<cv::Mat>{cv::Mat(), cv::Mat()};
}

// This is not computationally expensive, so currently run on the main thread
void ImageReader::step1frame(bool next) {
    if(state == PlaybackState::PLAYING)
        return;

    state = PlaybackState::PAUSED;

    // NOTE: at this point, currentImageIndex is marking the NEXT (yet unplayed) frame for the run() method

    if(next)
        currentImageIndex+=1;
    else
        currentImageIndex-=1;

    if(currentImageIndex < 0)
        currentImageIndex+=(int)filenames.size();
    if(currentImageIndex > filenames.size()-1)
        currentImageIndex%=(int)filenames.size();

    //qDebug() << currentImageIndex;

    if(stereoMode) {
        QFutureSynchronizer<cv::Mat> synchronizer;
        // Read images from disk asynchronous to save time
        synchronizer.addFuture(QtConcurrent::run(cv::imread, filenames[currentImageIndex], cv::IMREAD_GRAYSCALE));
        synchronizer.addFuture(QtConcurrent::run(cv::imread, filenamesSecondary[currentImageIndex], cv::IMREAD_GRAYSCALE));
        synchronizer.waitForFinished();
        cv::Mat img = synchronizer.futures().at(0).result();
        cv::Mat imgSecondary = synchronizer.futures().at(1).result();

        if(!img.data || !imgSecondary.data) {
            std::cerr << "Image Reader: StereoImage could not be read, skipping: " << filenames[currentImageIndex] << " and " << filenamesSecondary[currentImageIndex] ;
        }

        CameraImage cimg;
        cimg.type = CameraImageType::STEREO_IMAGE_FILE;
        cimg.img = img.clone();
        cimg.imgSecondary = imgSecondary.clone();
        //cimg.timestamp = startTimestamp;
        cimg.timestamp = acqTimestamps[currentImageIndex]; // using the file name, not the time of image reading operation
        cimg.frameNumber = currentImageIndex;
        cimg.filename = filenames[currentImageIndex];
        img.release();
        imgSecondary.release();

        emit onNewImage(cimg);
    } else {
        cv::Mat img = cv::imread(filenames[currentImageIndex], cv::IMREAD_GRAYSCALE);

        if(!img.data) {
            std::cerr << "Image Reader: Image could not be read, skipping: " << filenames[currentImageIndex] ;
        }

        CameraImage cimg;
        cimg.type = CameraImageType::SINGLE_IMAGE_FILE;
        cimg.img = img.clone();
        //cimg.timestamp = startTimestamp;
        cimg.timestamp = acqTimestamps[currentImageIndex]; // using the file name, not the time of image reading operation
        cimg.frameNumber = currentImageIndex;
        cimg.filename = filenames[currentImageIndex];
        img.release();

        emit onNewImage(cimg);
    }
}

void ImageReader::setSynchronised(bool synchronised) {
    ImageReader::synchronised = synchronised;
}



