
#include <QtConcurrent/QtConcurrent>
#include "pupilDetection.h"
#include "pupil-detection-methods/ElSe.h"
#include "pupil-detection-methods/ExCuSe.h"
#include "pupil-detection-methods/PuRe.h"
#include "pupil-detection-methods/Starburst.h"
#include "pupil-detection-methods/Swirski3D.h"
#include "pupil-detection-methods/PuReST.h"
#include "pupil-detection-methods/Swirski2D.h"
#include "devices/stereoCamera.h"
#include "devices/fileCamera.h"

#include <fstream>
#include <cmath>


// QTs event signal eventloop queues images for us, we run this pupildetection worker in a extra thread and every call to newImage is queued automatically
// This may however queue a large number of images if the processing speed is slow, increasing the memory potentially until it is full and the application is killed
// We dont have access to the eventloop to handle this, so its up to the user for now to not use a rate too high

void PupilDetection::populateWithMethods(std::vector<PupilDetectionMethod*> &vec) {
    vec.push_back(new ElSe());
    vec.push_back(new ExCuSe());
    vec.push_back(new PuRe());
    vec.push_back(new PuReST());
    vec.push_back(new Starburst());
    vec.push_back(new Swirski2D());

    // TODO finish integrating Swirski3D
    //double focal_length = 0.5;
    //pupilDetectionMethods.push_back(new Swirski3D(new PuReST(), focal_length, 5, 0.5));
}

// Creates a new pupil detection worker which include all pupil detection algorithms
// Should be run on a seperate thread
PupilDetection::PupilDetection(QMutex *imageMutex, QWaitCondition *imagePublished, QWaitCondition *imageProcessed, QObject *parent) : QObject(parent),
                                                  camera(nullptr),
                                                  frameCounter(new FrameRateCounter(parent)),
                                                  useOutlineConfidence(true),
                                                  useROIPreProcessing(false),
                                                  useImageUndistort(false),
                                                  usePupilUndistort(false),
                                                  trackingOn(false),
                                                  calibrated(false),
                                                  //showROI(true),
                                                  //showPupilCenter(false),
                                                  autoParamEnabled(false),
                                                  currentConfigLabel("Default"),
                                                  currentProcMode(ProcMode::UNDETERMINED),
                                                  ROIsingleImageOnePupil(),
                                                  ROIsingleImageTwoPupilA(),
                                                  ROIsingleImageTwoPupilB(),
                                                  ROIstereoImageOnePupil1(),
                                                  ROIstereoImageOnePupil2(),
                                                  ROIstereoImageTwoPupilA1(),
                                                  ROIstereoImageTwoPupilA2(),
                                                  ROIstereoImageTwoPupilB1(),
                                                  ROIstereoImageTwoPupilB2(),
                                                  ROImirrImageOnePupil1(),
                                                  ROImirrImageOnePupil2(),
                                                  imageMutex(imageMutex),
                                                  imagePublished(imagePublished),
                                                  imageProcessed(imageProcessed)
                                                  {

    drawDelay = 33; // ~30fps

    // GB begin: 
    // GB NOTE: refactored and added two new to best work with also with "stereo camera for two pupil" proc mode,
    // also added "populateWithMethods" function to shorten code

    // we initialize the algorithms here one time and save them in a list, the created objects are then changed through index change of the list
    populateWithMethods(pupilDetectionMethods1);
    populateWithMethods(pupilDetectionMethods2);
    populateWithMethods(pupilDetectionMethods3);
    populateWithMethods(pupilDetectionMethods4);

    // Default algorithm PuRe
    pupilDetectionIndex = 2;

    // Processing speed frame counter
    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    
    // BG: NOTE: signals-slots moved from here to keep up with procMode changes by user

    assert( connect(this, SIGNAL(processedPupilData(quint64, int, std::vector<Pupil>, QString)), frameCounter, SLOT(count())) );
    // GB end

    drawTimer.start();
    processingTimer.start();

    //configureCameraConnection();
}

PupilDetection::~PupilDetection() {
}

// Attaches a camera to the pupil detection process
// Checks if the camera is calibrated and stereo or single, sets the detection mode accordingly
void PupilDetection::setCamera(Camera *m_camera) {

    if (camera != m_camera) {
        camera = m_camera;

        calibrated = false;

        if (camera->getType() == CameraImageType::LIVE_STEREO_CAMERA) {
            stereoCalibration = dynamic_cast<StereoCamera *>(camera)->getCameraCalibration();
            calibrated = static_cast<bool>(stereoCalibration->isCalibrated());
        } else if (camera->getType() == CameraImageType::STEREO_IMAGE_FILE) {
            stereoCalibration = dynamic_cast<FileCamera *>(camera)->getStereoCameraCalibration();
            calibrated = static_cast<bool>(stereoCalibration->isCalibrated());
        } else if (camera->getType() == CameraImageType::LIVE_SINGLE_CAMERA) {
            singleCalibration = dynamic_cast<SingleCamera *>(camera)->getCameraCalibration();
            calibrated = static_cast<bool>(singleCalibration->isCalibrated());
        } else if (camera->getType() == CameraImageType::SINGLE_IMAGE_FILE) {
            singleCalibration = dynamic_cast<FileCamera *>(camera)->getCameraCalibration();
            calibrated = static_cast<bool>(singleCalibration->isCalibrated());
        }
            // GB added begin
        else if (camera->getType() == CameraImageType::LIVE_SINGLE_WEBCAM) {
            singleCalibration = dynamic_cast<SingleWebcam *>(camera)->getCameraCalibration();
            calibrated = static_cast<bool>(singleCalibration->isCalibrated());
        }
        configureCameraConnection(true);
    }
    // GB added end
}

// Starts the algorithm by connecting the camera image signals to the processing callbacks
// GB: now the distinction between stereo/single modes is made using procMode enum
void PupilDetection::startDetection() {

    // TODO: check if ROIs are set ?

    // BG: NOTE: maybe not here? But one algorithm is changed, we certainly need to re-parameter
    if(autoParamEnabled)
        performAutoParam();

    trackingOn = true;
    if(camera) {
        //configureCameraConnection();
        emit processingStarted();
    }
}

// Stops the pupil detection by disconnecting the signals of new images to the processing
// GB: now the distinction between stereo/single modes is made using procMode enum
void PupilDetection::stopDetection() {

    if(camera && trackingOn) {
        trackingOn = false;


        emit processingFinished();
        imageProcessed->wakeAll();
        imagePublished->wakeAll();
        qDebug() << "Woke up imageProcessing";
    }
    qDebug() << "Woke up imageProcessing";
}

// Changes the applied pupil detection algorithm
// The change is performed by first disconnecting the signal to stop potential frames, switch the algorithm and connect them again
// Emits a signal to signal the algorithm changed
void PupilDetection::setAlgorithm(QString method) {

    configureCameraConnection(false);

    frameCounter->reset();

    int i = 0;
    for(auto pm: pupilDetectionMethods1) {
        //if(pm->title() == method.toStdString())
        if(QString::fromStdString(pm->title()).toLower() == method.toLower()) // GB: modified for tolower to care for when someone is setting this through an UDP command and had a case-typo
            pupilDetectionIndex = i;
        i++;
    }

    // GB begin
    // NOTE: maybe not here? But one algorithm is changed, we certainly need to re-parameter
    if(autoParamEnabled)
        autoParamScheduled = true;
    // GB end

    emit algorithmChanged();

    configureCameraConnection(true);
}

// Slot callback for receiving new single camera images
// Performs the processing/pupil detection
// Emits the pupil detection result as a signal, as well as processed images with plotted pupil contours
// Depending on the configuration, performs undistortion on the images or pupil detections
// GB: renamed and modified


void PupilDetection::onNewSingleImageForOnePupil(CameraImage *cimg) {
    if (synchronised) {
        const QMutexLocker locker(imageMutex);
        onNewSingleImageForOnePupilImpl(cimg);
//        qDebug() << "pupilDetection locking";
  //      qDebug() << "pupilDetection image processed, unlocking";
        imagePublished->wakeAll();
        if (trackingOn) {
            qDebug() << "Locking image processing";
            imageProcessed->wait(imageMutex);
        }
    }
    else {
        onNewSingleImageForOnePupilImpl(cimg);
    }
}

void PupilDetection::onNewSingleImageForOnePupilImpl(CameraImage *image) {

    // Processing fps restriction not working correctly, timers overhead seem to break timing, left out for now
    //qDebug()<<cimg->filename;
    if (!trackingOn) {
        //qDebug()<<"Single: onNewSingleImageForOnePupil: Tracking is stopped but receiving signals."; // GB: changed text
        //qDebug() << image->frameNumber;
        emit processedImage(image);
        return;
    }

    cv::Mat bwFrame = image->img;

    // Undistorting the whole image is rather slow (~4ms on our test system), use contour point undistort instead (>~1ms)
    if(!usePupilUndistort && useImageUndistort) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        bwFrame = singleCalibration->undistortImage(image->img);
        //qDebug()<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 ;
    }

    cv::Rect roi = cv::Rect(0, 0, bwFrame.cols, bwFrame.rows);

    // GB modified begin
    // GB: like this the global ROI variables can inform performAutoParam() about ROI sizes
    if(useROIPreProcessing && !ROIsingleImageOnePupil.empty() && roi != ROIsingleImageOnePupil && ROIsingleImageOnePupil.width<=bwFrame.cols && ROIsingleImageOnePupil.height<=bwFrame.rows) {
        roi = ROIsingleImageOnePupil;
        bwFrame = bwFrame(ROIsingleImageOnePupil);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIsingleImageOnePupil = roi;

    if(autoParamEnabled && autoParamScheduled) {
        performAutoParam();
        autoParamScheduled = false;
    }
    // GB modified end

    if (bwFrame.channels() > 1) {
        cv::cvtColor(bwFrame, bwFrame, cv::COLOR_BGR2GRAY);
    }

    Pupil pupil = Pupil();

    // Pupil detection
    try {
        if(useOutlineConfidence) {

            //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            pupilDetectionMethods1[pupilDetectionIndex]->runWithConfidence(bwFrame, pupil);
            //runtimeHistory.push_back(std::make_pair(cimg->timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
        } else {
            //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            pupilDetectionMethods1[pupilDetectionIndex]->run(bwFrame, pupil);
            //runtimeHistory.push_back(std::make_pair(cimg->timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
        }
    } catch (...) {
        pupil.clear();
    }

    // Shift the pupil center position to be in the coordinate of the whole image instead of the ROI
    if(useROIPreProcessing) {
        pupil.shift(roi.tl());
    }

    // Undistort the pupil contour points to get an undistorted pupil size
    if(usePupilUndistort && !useImageUndistort) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        pupil.undistortedDiameter = singleCalibration->undistortPupilDiameter(pupil);
        //qDebug()<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 ;
    } else if(!usePupilUndistort && useImageUndistort) {
        pupil.undistortedDiameter = pupil.diameter();
    }

    pupil.algorithmName = pupilDetectionMethods1[pupilDetectionIndex]->title();


    // GB modified begin
    std::vector<Pupil> Pupils;
    Pupils.push_back(pupil);

    // Drawing of pupil detections on the image is only performed at ~30fps
    if ((trackingOn && drawTimer.elapsed() > drawDelay) ||
        (camera->getType() == SINGLE_IMAGE_FILE && !static_cast<FileCamera*>(camera)->isPlaying() && static_cast<FileCamera*>(camera)->getLastCommissionedFrameNumber() == image->frameNumber)) {
        // GB NOTE: need to emit the processed image and data also when the user hits pause or stop, and we are waiting there for the last read image to arrive processed

        drawTimer.start();

        CameraImage *mimg = image;
        mimg->img = image->img.clone();

        if(!usePupilUndistort && useImageUndistort) {
            mimg->img = singleCalibration->undistortImage(image->img);
        }
        // GB modified: not necessary to copy twice
        //else {
        //    mimg->img = cimg->img.clone();
        //}
        // GB end
        /*

        // GB: moved code to singleCameraView code
        if (mimg->img.channels() == 1) {
            cv::cvtColor(mimg->img, mimg->img, cv::COLOR_GRAY2BGR);
        }

        //if(showROI)
            cv::rectangle(mimg->img, roi, cv::Scalar(255, 0, 255 ), 3);

        if(pupil.valid(-2.0)) {
            cv::ellipse(mimg->img, pupil, cv::Scalar( 0, 200, 255 ), 1); //BG: it was 0,0,255
            //if(showPupilCenter)
                cv::circle(mimg->img, pupil.center, 1, CV_RGB(255,0,0),3);
        } else {
            cv::putText(mimg->img, "NO PUPIL FOUND", cv::Point(static_cast<int>(0.25 * mimg->img.cols),
                                                              static_cast<int>(0.25 * mimg->img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 255), 4);
        }
        emit processedImage(mimg);
        */

        std::vector<cv::Rect> ROIs;
        if(useROIPreProcessing) {
            ROIs.push_back(ROIsingleImageOnePupil);
        } else {
            ROIs.push_back(roi);
            //ROIs.push_back(cv::Rect(0,0, bwFrame.size().width, bwFrame.size().height));
        }
        emit processedImage(mimg, currentProcMode, ROIs, Pupils);

        // to inform imagePlaybackControlDialog about the just processed image
        if(camera->getType() == SINGLE_IMAGE_FILE) {
            emit processedImage(mimg);
            qDebug() << image->frameNumber;
        }
        //qDebug() << "frameNumber left pupilDetection: " << cimg->frameNumber;
    }

    //emit processedSingleImageForOnePupilData(cimg->timestamp, pupil, QString::fromStdString(cimg->filename)); // Gabor Benyei (kheki4) on 2022.11.02, NOTE: refactored

    emit processedPupilData(image->timestamp, currentProcMode, Pupils, QString::fromStdString(image->filename));
    // GB modified end



}

// Slot callback for receiving new single camera images that contain two pupils/eyes
// Performs the processing/pupil detection
// Emits the pupil detection result as a signal, as well as processed images with plotted pupil contours
// Depending on the configuration, performs undistortion on the images or pupil detections
void PupilDetection::onNewSingleImageForTwoPupil(CameraImage *cimg) {
    if (synchronised) {
        const QMutexLocker locker(imageMutex);
        onNewSingleImageForTwoPupilImpl(cimg);
//        qDebug() << "pupilDetection locking";
        //      qDebug() << "pupilDetection image processed, unlocking";
        imagePublished->wakeAll();
        if (trackingOn) {
            qDebug() << "Locking image processing";
            imageProcessed->wait(imageMutex);
        }
    }
    else {
        onNewSingleImageForTwoPupilImpl(cimg);
    }

}

void PupilDetection::onNewSingleImageForTwoPupilImpl(CameraImage *cimg) {

    if (!trackingOn) {
        qDebug() << cimg->frameNumber;
        emit processedImage(cimg);
        return;
    }

    // BG: NOTE: by default we only use the left and right halves of the input image
    cv::Rect roiA = cv::Rect(0, 0, (int)std::floor(cimg->img.cols/2)-1, cimg->img.rows);
    cv::Rect roiB = cv::Rect((int)std::ceil(cimg->img.cols/2)+1, 0, cimg->img.cols, cimg->img.rows);
    cv::Mat bwFrameA = cimg->img;
    cv::Mat bwFrameB = cimg->img;

    // GB: like this the global ROI variables can inform performAutoParam() about ROI sizes
    if(useROIPreProcessing && !ROIsingleImageTwoPupilA.empty() && roiA != ROIsingleImageTwoPupilA && ROIsingleImageTwoPupilA.width<=bwFrameA.cols && ROIsingleImageTwoPupilA.height<=bwFrameA.rows) {
        roiA = ROIsingleImageTwoPupilA;
        bwFrameA = bwFrameA(ROIsingleImageTwoPupilA);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIsingleImageTwoPupilA = roiA;

    if(useROIPreProcessing && !ROIsingleImageTwoPupilB.empty() && roiB != ROIsingleImageTwoPupilB && ROIsingleImageTwoPupilB.width<=bwFrameB.cols && ROIsingleImageTwoPupilB.height<=bwFrameB.rows) {
        roiB = ROIsingleImageTwoPupilB;
        bwFrameB = bwFrameB(ROIsingleImageTwoPupilB);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIsingleImageTwoPupilB = roiB;

    if(autoParamEnabled && autoParamScheduled) {
        performAutoParam();
        autoParamScheduled = false;
    }

    if (cimg->img.channels() > 1) {
        cv::cvtColor(bwFrameA, bwFrameA, cv::COLOR_BGR2GRAY);
        cv::cvtColor(bwFrameB, bwFrameB, cv::COLOR_BGR2GRAY);
    }

    // We execute pupil detection for main and secondary images concurrently using treads, we execute both in separate threads, then wait till both are finished
    QFutureSynchronizer<Pupil> synchronizer;
    Pupil pupilA;
    Pupil pupilB;

    try {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        if(useOutlineConfidence) {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods1[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameA));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods2[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameB));
        } else {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods1[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameA));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods2[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameB));
        }
        synchronizer.waitForFinished();
        // Unhandled exceptions in the QtConcurrent::run function are thrown at the result() call
        pupilA = synchronizer.futures().at(0).result();
        pupilB = synchronizer.futures().at(1).result();
    } catch (...) {
        pupilA.clear();
        pupilB.clear();
    }

    // Shift the pupil position back to the original image coordinates instead of ROI
    if(useROIPreProcessing) {
        pupilA.shift(roiA.tl());
        pupilB.shift(roiB.tl());
    }

    if(usePupilUndistort && !useImageUndistort) {
        pupilA.undistortedDiameter = singleCalibration->undistortPupilDiameter(pupilA);
        pupilB.undistortedDiameter = singleCalibration->undistortPupilDiameter(pupilB);
    } else if(!usePupilUndistort && useImageUndistort) {
        pupilA.undistortedDiameter = pupilA.diameter();
        pupilB.undistortedDiameter = pupilB.diameter();
    }

    pupilA.algorithmName = pupilDetectionMethods1[pupilDetectionIndex]->title();
    pupilB.algorithmName = pupilA.algorithmName;

    // GB TODO: ? Implement pythagorean px-mm mapping via calibration?

    std::vector<Pupil> Pupils;
    Pupils.push_back(pupilA);
    Pupils.push_back(pupilB);

    if ((trackingOn && drawTimer.elapsed() > drawDelay) ||
        (camera->getType() == SINGLE_IMAGE_FILE && !static_cast<FileCamera*>(camera)->isPlaying() && static_cast<FileCamera*>(camera)->getLastCommissionedFrameNumber()==cimg->frameNumber)) {
        // GB NOTE: need to emit the processed image and data also when the user hits pause or stop, and we are waiting there for the last read image to arrive processed

        drawTimer.start();

        CameraImage *mimg = cimg;
//        mimg->img = cimg->img.clone();
// //        mimg->imgB = cimg->img.clone(); // would be the same

        if(!usePupilUndistort && useImageUndistort) {
            mimg->img = singleCalibration->undistortImage(cimg->img);
        } else {
            mimg->img = cimg->img.clone();
        }

        std::vector<cv::Rect> ROIs;
        if(useROIPreProcessing) {
            ROIs.push_back(ROIsingleImageTwoPupilA);
            ROIs.push_back(ROIsingleImageTwoPupilB);
        } else {
            ROIs.push_back(roiA);
            ROIs.push_back(roiB);
            // ROIs.push_back(cv::Rect(0, 0, bwFrameA.size().width, bwFrameA.size().height));
            // ROIs.push_back(cv::Rect((int)std::ceil(cimg->img.cols/2)+1, 0, bwFrameB.size().width, bwFrameB.size().height));
        }
        emit processedImage(mimg, currentProcMode, ROIs, Pupils);

        // to inform imagePlaybackControlDialog about the just processed image
        if(camera->getType() == SINGLE_IMAGE_FILE)
                emit processedImage(mimg);
    }
    emit processedPupilData(cimg->timestamp, currentProcMode, Pupils, QString::fromStdString(cimg->filename));

}

// Slot callback for receiving new stereo camera images
// Performs the processing/pupil detection
// Emits the pupil detection result as a signal, as well as processed images with plotted pupil contours
// Depending on the configuration, performs undistortion on the pupil detections
// GB: renamed and modified
void PupilDetection::onNewStereoImageForOnePupil(CameraImage *simg) {

    if (synchronised) {
        const QMutexLocker locker(imageMutex);
        onNewStereoImageForOnePupilImpl(simg);
//        qDebug() << "pupilDetection locking";
        //      qDebug() << "pupilDetection image processed, unlocking";
        imagePublished->wakeAll();
        if (trackingOn) {
            qDebug() << "Locking image processing";
            imageProcessed->wait(imageMutex);
        }
    }
    else {
        onNewStereoImageForOnePupilImpl(simg);
    }

}


void PupilDetection::onNewStereoImageForOnePupilImpl(CameraImage *simg) {

    // at the moment, the images are not undistorted completely but only the major axis points are undistorted after detection for absolute unit conversion
    // This creates a discrepancy between the undistorted pixel size and the physical measure, as a fix, undistortedDiamter can be calculated using useUndistort
    if (!trackingOn) {
        qDebug() << simg->frameNumber;
        emit processedImage(simg);
        return;
    }

    cv::Rect roi = cv::Rect(0, 0, simg->img.cols, simg->img.rows);
    cv::Rect roiSecondary = cv::Rect(0, 0, simg->img.cols, simg->img.rows);
    cv::Mat bwFrame = simg->img;
    cv::Mat bwFrameSecondary = simg->imgSecondary;

    // GB modified begin
    // GB: like this the global ROI variables can inform performAutoParam() about ROI sizes
    if(useROIPreProcessing && !ROIstereoImageOnePupil1.empty() && roi != ROIstereoImageOnePupil1 && ROIstereoImageOnePupil1.width<=bwFrame.cols && ROIstereoImageOnePupil1.height<=bwFrame.rows) {
        roi = ROIstereoImageOnePupil1;
        bwFrame = bwFrame(ROIstereoImageOnePupil1);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIstereoImageOnePupil1 = roi;

    if(useROIPreProcessing && !ROIstereoImageOnePupil2.empty() && roiSecondary != ROIstereoImageOnePupil2 && ROIstereoImageOnePupil2.width<=bwFrameSecondary.cols && ROIstereoImageOnePupil2.height<=bwFrameSecondary.rows) {
        roiSecondary = ROIstereoImageOnePupil2;
        bwFrameSecondary = bwFrameSecondary(ROIstereoImageOnePupil2);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIstereoImageOnePupil2 = roiSecondary;

    if(autoParamEnabled && autoParamScheduled) {
        performAutoParam();
        autoParamScheduled = false;
    }
    // GB modified end

    if (simg->img.channels() > 1) {
        cv::cvtColor(bwFrame, bwFrame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(bwFrameSecondary, bwFrameSecondary, cv::COLOR_BGR2GRAY);
    }

    // We execute pupil detection for main and secondary images concurrently using treads, we execute both in separate threads, then wait till both are finished
    QFutureSynchronizer<Pupil> synchronizer;
    Pupil pupil;
    Pupil pupilSecondary;

    try {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        if(useOutlineConfidence) {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods1[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrame));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods2[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameSecondary));
        } else {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods1[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrame));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods2[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameSecondary));
        }
        synchronizer.waitForFinished();
        // Unhandled exceptions in the QtConcurrent::run function are thrown at the result() call
        pupil = synchronizer.futures().at(0).result();
        pupilSecondary = synchronizer.futures().at(1).result();
    } catch (...) {
        pupil.clear();
        pupilSecondary.clear();
    }
    //runtimeHistory.push_back(std::make_pair(simg->timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()));

    // Shift the pupil position back to the original image coordinates instead of ROI
    if(useROIPreProcessing) {
        pupil.shift(roi.tl());
        pupilSecondary.shift(roiSecondary.tl());
    }

    if(usePupilUndistort && !useImageUndistort) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::pair<double, double> diameters = stereoCalibration->undistortPupilDiameters(pupil, pupilSecondary);
        pupil.undistortedDiameter = diameters.first;
        pupilSecondary.undistortedDiameter = diameters.second;
        //qDebug()<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 ;
    } else if(!usePupilUndistort && useImageUndistort) {
        pupil.undistortedDiameter = pupil.diameter();
        pupilSecondary.undistortedDiameter = pupil.diameter();
    }

    pupil.algorithmName = pupilDetectionMethods1[pupilDetectionIndex]->title();
    pupilSecondary.algorithmName = pupil.algorithmName;

    // If both pupil detections are valid and the camera is calibrated, we can perform unit conversion to absolute measure
    if(pupil.valid(-2.0) && pupilSecondary.valid(-2.0) && calibrated) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        Pupil rotPupil(pupil);
        rotPupil.angle += 360-rotPupil.angle;
        Pupil rotPupilSecondary(pupilSecondary);
        rotPupilSecondary.angle += 360-rotPupilSecondary.angle;

        // convert pupil detection from pixel into mm through stereo calibration
        // Select the top left and top right corner of the bounding rects of the pupils as the points we triangulate
        cv::Point2f mainPointsArr[4]; // rotatedRect points in order: bottomLeft, topLeft, topRight, bottomRight
        rotPupil.points(mainPointsArr);
        int secondPoint = rotPupil.size.width > rotPupil.size.height ? 2 : 0; // if the pupil major axis is horizontal use topLeft and topRight, else topLeft and bottomLeft
        //cv::line(mimg->img,mainPointsArr[1], mainPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        cv::Point2f secondaryPointsArr[4];
        rotPupilSecondary.points(secondaryPointsArr);
        //cv::line(mimg->imgSecondary,secondaryPointsArr[1], secondaryPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        std::vector<cv::Point2f> mainPoints{mainPointsArr[1], mainPointsArr[secondPoint]}, secondaryPoints{secondaryPointsArr[1], secondaryPointsArr[secondPoint]};
        std::vector<cv::Point3f> worldPoints = stereoCalibration->convertPointsTo3D(mainPoints, secondaryPoints);

        pupil.physicalDiameter = static_cast<float>(cv::norm(worldPoints[0] - worldPoints[1]));
        pupilSecondary.physicalDiameter = pupil.physicalDiameter;
        //runtimeHistory.push_back(std::make_pair(simg->timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
    }

    // GB modified begin:
    std::vector<Pupil> Pupils;
    Pupils.push_back(pupil);
    Pupils.push_back(pupilSecondary);

    if ((trackingOn && drawTimer.elapsed() > drawDelay) ||
        (camera->getType() == STEREO_IMAGE_FILE && !static_cast<FileCamera*>(camera)->isPlaying() && static_cast<FileCamera*>(camera)->getLastCommissionedFrameNumber()==simg->frameNumber)) {
        // GB NOTE: need to emit the processed image and data also when the user hits pause or stop, and we are waiting there for the last read image to arrive processed

        drawTimer.start();
        CameraImage *mimg = simg;
        mimg->img = simg->img.clone();
        mimg->imgSecondary = simg->imgSecondary.clone();


        std::vector<cv::Rect> ROIs;
        if(useROIPreProcessing) {
            ROIs.push_back(ROIstereoImageOnePupil1);
            ROIs.push_back(ROIstereoImageOnePupil2);
        } else {
            ROIs.push_back(roi);
            ROIs.push_back(roiSecondary);
            //ROIs.push_back(cv::Rect(0,0, bwFrame.size().width, bwFrame.size().height));
            //ROIs.push_back(cv::Rect(0,0, bwFrameSecondary.size().width, bwFrameSecondary.size().height));
        }
        emit processedImage(mimg, currentProcMode, ROIs, Pupils);

        // to inform imagePlaybackControlDialog about the just processed image->
        if(camera->getType() == STEREO_IMAGE_FILE)
                emit processedImage(mimg);
    }

    //emit processedStereoImageForOnePupilData(simg->timestamp, pupil, pupilSecondary, QString::fromStdString(simg->filename)); // Gabor Benyei (kheki4) on 2022.11.02, NOTE: refactored
    emit processedPupilData(simg->timestamp, currentProcMode, Pupils, QString::fromStdString(simg->filename));
    // GB modified end
}
// Slot callback for receiving new stereo camera images, associated with two viewpoints, both looking at both eyes
// Performs the processing/pupil detection
// Emits the pupil detection result as a signal, as well as processed images with plotted pupil contours
// Depending on the configuration, performs undistortion on the pupil detections
void PupilDetection::onNewStereoImageForTwoPupil(CameraImage *simg) {

    if (synchronised) {
        const QMutexLocker locker(imageMutex);
        onNewStereoImageForTwoPupilImpl(simg);
//        qDebug() << "pupilDetection locking";
        //      qDebug() << "pupilDetection image processed, unlocking";
        imagePublished->wakeAll();
        if (trackingOn) {
            qDebug() << "Locking image processing";
            imageProcessed->wait(imageMutex);
        }
    }
    else {
        onNewStereoImageForTwoPupilImpl(simg);
    }
}

void PupilDetection::onNewStereoImageForTwoPupilImpl(CameraImage *simg) {

    // at the moment, the images are not undistorted completely but only the major axis points are undistorted after detection for absolute unit conversion
    // This creates a discrepancy between the undistorted pixel size and the physical measure, as a fix, undistortedDiamter can be calculated using useUndistort

    if (!trackingOn) {
        qDebug() << simg->frameNumber;
        emit processedImage(simg);
        return;
    }

    cv::Rect roiA1 = cv::Rect(0, 0, simg->img.cols, simg->img.rows);
    cv::Rect roiA2 = cv::Rect(0, 0, simg->img.cols, simg->img.rows);
    cv::Rect roiB1 = cv::Rect(0, 0, simg->img.cols, simg->img.rows);
    cv::Rect roiB2 = cv::Rect(0, 0, simg->img.cols, simg->img.rows);
    cv::Mat bwFrameA1 = simg->img;
    cv::Mat bwFrameA2 = simg->imgSecondary;
    cv::Mat bwFrameB1 = simg->img;
    cv::Mat bwFrameB2 = simg->imgSecondary;

    // GB: like this the global ROI variables can inform performAutoParam() about ROI sizes
    if(useROIPreProcessing && !ROIstereoImageTwoPupilA1.empty() && roiA1 != ROIstereoImageTwoPupilA1 && ROIstereoImageTwoPupilA1.width<=bwFrameA1.cols && ROIstereoImageTwoPupilA1.height<=bwFrameA1.rows) {
        roiA1 = ROIstereoImageTwoPupilA1;
        bwFrameA1 = bwFrameA1(ROIstereoImageTwoPupilA1);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIstereoImageTwoPupilA1 = roiA1;

    if(useROIPreProcessing && !ROIstereoImageTwoPupilA2.empty() && roiA2 != ROIstereoImageTwoPupilA2 && ROIstereoImageTwoPupilA2.width<=bwFrameA2.cols && ROIstereoImageTwoPupilA2.height<=bwFrameA2.rows) {
        roiA2 = ROIstereoImageTwoPupilA2;
        bwFrameA2 = bwFrameA2(ROIstereoImageTwoPupilA2);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIstereoImageTwoPupilA2 = roiA2;

    if(useROIPreProcessing && !ROIstereoImageTwoPupilB1.empty() && roiB1 != ROIstereoImageTwoPupilB1 && ROIstereoImageTwoPupilB1.width<=bwFrameB1.cols && ROIstereoImageTwoPupilB1.height<=bwFrameB1.rows) {
        roiB1 = ROIstereoImageTwoPupilB1;
        bwFrameB1 = bwFrameB1(ROIstereoImageTwoPupilB1);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIstereoImageTwoPupilB1 = roiB1;

    if(useROIPreProcessing && !ROIstereoImageTwoPupilB2.empty() && roiB2 != ROIstereoImageTwoPupilB2 && ROIstereoImageTwoPupilB2.width<=bwFrameB2.cols && ROIstereoImageTwoPupilB2.height<=bwFrameB2.rows) {
        roiB2 = ROIstereoImageTwoPupilB2;
        bwFrameB2 = bwFrameB2(ROIstereoImageTwoPupilB2);
    } else if(autoParamEnabled && autoParamScheduled)
        ROIstereoImageTwoPupilB2 = roiB2;

    if(autoParamEnabled && autoParamScheduled) {
        performAutoParam();
        autoParamScheduled = false;
    }

    if (simg->img.channels() > 1) {
        cv::cvtColor(bwFrameA1, bwFrameA1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(bwFrameA2, bwFrameA2, cv::COLOR_BGR2GRAY);
        cv::cvtColor(bwFrameB1, bwFrameB1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(bwFrameB2, bwFrameB2, cv::COLOR_BGR2GRAY);
    }

    // We execute pupil detection for main and secondary images concurrently using treads, we execute both in separate threads, then wait till both are finished
    QFutureSynchronizer<Pupil> synchronizer;
    Pupil pupilA1;
    Pupil pupilA2;
    Pupil pupilB1;
    Pupil pupilB2;


    try {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        if(useOutlineConfidence) {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods1[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameA1));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods2[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameA2));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods3[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameB1));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods4[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameB2));
        } else {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods1[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameA1));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods2[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameA2));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods3[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameB1));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods4[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameB2));
        }
        synchronizer.waitForFinished();
        // Unhandled exceptions in the QtConcurrent::run function are thrown at the result() call
        pupilA1 = synchronizer.futures().at(0).result();
        pupilA2 = synchronizer.futures().at(1).result();
        pupilB1 = synchronizer.futures().at(2).result();
        pupilB2 = synchronizer.futures().at(3).result();
    } catch (...) {
        pupilA1.clear();
        pupilA2.clear();
        pupilB1.clear();
        pupilB2.clear();
    }
    //runtimeHistory.push_back(std::make_pair(simg->timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()));

    // Shift the pupil position back to the original image coordinates instead of ROI
    if(useROIPreProcessing) {
        pupilA1.shift(roiA1.tl());
        pupilA2.shift(roiA2.tl());
        pupilB1.shift(roiB1.tl());
        pupilB2.shift(roiB2.tl());
    }

    if(usePupilUndistort && !useImageUndistort) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::pair<double, double> diameters1 = stereoCalibration->undistortPupilDiameters(pupilA1, pupilB1);
        pupilA1.undistortedDiameter = diameters1.first;
        pupilB1.undistortedDiameter = diameters1.second;
        std::pair<double, double> diameters2 = stereoCalibration->undistortPupilDiameters(pupilA2, pupilB2);
        pupilA2.undistortedDiameter = diameters2.first;
        pupilB2.undistortedDiameter = diameters2.second;
        //qDebug()<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 ;
    } else if(!usePupilUndistort && useImageUndistort) {
        pupilA1.undistortedDiameter = pupilA1.diameter();
        pupilA2.undistortedDiameter = pupilA1.diameter();
        pupilB1.undistortedDiameter = pupilB1.diameter();
        pupilB2.undistortedDiameter = pupilB1.diameter();
    }

    pupilA1.algorithmName = pupilDetectionMethods1[pupilDetectionIndex]->title();
    pupilA2.algorithmName = pupilA1.algorithmName;
    pupilB1.algorithmName = pupilDetectionMethods1[pupilDetectionIndex]->title();
    pupilB2.algorithmName = pupilB1.algorithmName;

    // Eye A
    // If both pupil detections are valid and the camera is calibrated, we can perform unit conversion to absolute measure
    if(pupilA1.valid(-2.0) && pupilA2.valid(-2.0) && calibrated) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        Pupil rotPupilA1(pupilA1);
        rotPupilA1.angle += 360-rotPupilA1.angle;
        Pupil rotPupilA2(pupilA2);
        rotPupilA2.angle += 360-rotPupilA2.angle;

        // convert pupil detection from pixel into mm through stereo calibration
        // Select the top left and top right corner of the bounding rects of the pupils as the points we triangulate
        cv::Point2f mainPointsArrA[4]; // rotatedRect points in order: bottomLeft, topLeft, topRight, bottomRight
        rotPupilA1.points(mainPointsArrA);
        int secondPointA = rotPupilA1.size.width > rotPupilA1.size.height ? 2 : 0; // if the pupil major axis is horizontal use topLeft and topRight, else topLeft and bottomLeft
        //cv::line(mimg->img,mainPointsArr[1], mainPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        cv::Point2f secondaryPointsArrA[4];
        rotPupilA2.points(secondaryPointsArrA);
        //cv::line(mimg->imgSecondary,secondaryPointsArr[1], secondaryPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        std::vector<cv::Point2f> mainPointsA{mainPointsArrA[1], mainPointsArrA[secondPointA]}, secondaryPointsA{secondaryPointsArrA[1], secondaryPointsArrA[secondPointA]};
        std::vector<cv::Point3f> worldPointsA = stereoCalibration->convertPointsTo3D(mainPointsA, secondaryPointsA);

        pupilA1.physicalDiameter = static_cast<float>(cv::norm(worldPointsA[0] - worldPointsA[1]));
        pupilA2.physicalDiameter = pupilA1.physicalDiameter;
        //runtimeHistory.push_back(std::make_pair(simg->timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
    }

    // Eye B
    // If both pupil detections are valid and the camera is calibrated, we can perform unit conversion to absolute measure
    if(pupilB1.valid(-2.0) && pupilB2.valid(-2.0) && calibrated) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        Pupil rotPupilB1(pupilB1);
        rotPupilB1.angle += 360-rotPupilB1.angle;
        Pupil rotPupilB2(pupilB2);
        rotPupilB2.angle += 360-rotPupilB2.angle;

        // convert pupil detection from pixel into mm through stereo calibration
        // Select the top left and top right corner of the bounding rects of the pupils as the points we triangulate
        cv::Point2f mainPointsArrB[4]; // rotatedRect points in order: bottomLeft, topLeft, topRight, bottomRight
        rotPupilB1.points(mainPointsArrB);
        int secondPointB = rotPupilB1.size.width > rotPupilB1.size.height ? 2 : 0; // if the pupil major axis is horizontal use topLeft and topRight, else topLeft and bottomLeft
        //cv::line(mimg->img,mainPointsArr[1], mainPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        cv::Point2f secondaryPointsArrB[4];
        rotPupilB2.points(secondaryPointsArrB);
        //cv::line(mimg->imgSecondary,secondaryPointsArr[1], secondaryPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        std::vector<cv::Point2f> mainPointsB{mainPointsArrB[1], mainPointsArrB[secondPointB]}, secondaryPointsB{secondaryPointsArrB[1], secondaryPointsArrB[secondPointB]};
        std::vector<cv::Point3f> worldPointsB = stereoCalibration->convertPointsTo3D(mainPointsB, secondaryPointsB);

        pupilB1.physicalDiameter = static_cast<float>(cv::norm(worldPointsB[0] - worldPointsB[1]));
        pupilB2.physicalDiameter = pupilB1.physicalDiameter;
        //runtimeHistory.push_back(std::make_pair(simg->timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
    }

    std::vector<Pupil> Pupils;
    Pupils.push_back(pupilA1);
    Pupils.push_back(pupilA2);
    Pupils.push_back(pupilB1);
    Pupils.push_back(pupilB2);

    if ((trackingOn && drawTimer.elapsed() > drawDelay) ||
        (camera->getType() == STEREO_IMAGE_FILE && !static_cast<FileCamera*>(camera)->isPlaying() && static_cast<FileCamera*>(camera)->getLastCommissionedFrameNumber()==simg->frameNumber)) {
        // GB NOTE: need to emit the processed image and data also when the user hits pause or stop, and we are waiting there for the last read image to arrive processed

        drawTimer.start();
        CameraImage *mimg = simg;
        mimg->img = simg->img.clone();
        mimg->imgSecondary = simg->imgSecondary.clone();

        std::vector<cv::Rect> ROIs;
        if(useROIPreProcessing) {
            ROIs.push_back(ROIstereoImageTwoPupilA1);
            ROIs.push_back(ROIstereoImageTwoPupilA2);
            ROIs.push_back(ROIstereoImageTwoPupilB1);
            ROIs.push_back(ROIstereoImageTwoPupilB2);
        } else {
            ROIs.push_back(roiA1);
            ROIs.push_back(roiA2);
            ROIs.push_back(roiB1);
            ROIs.push_back(roiB2);

        }
        emit processedImage(mimg, currentProcMode, ROIs, Pupils);

        // to inform imagePlaybackControlDialog about the just processed image
        if(camera->getType() == STEREO_IMAGE_FILE)
                emit processedImage(mimg);
    }

    emit processedPupilData(simg->timestamp, currentProcMode, Pupils, QString::fromStdString(simg->filename));
}

// GB: I found this function like this, and did not bother it
template <typename T> void PupilDetection::writeVectorCSV(std::vector<std::pair<uint64_t , T>> data, const std::string &header, const std::string &filename) {
    // Debug helper function
    std::ofstream file(filename);

    if(file.is_open()) {

        file << header ;

        for(typename std::vector<std::pair<uint64_t , T>>::iterator it = data.begin(); it != data.end(); ++it) {
            file << std::get<0>((*it)) << "," << std::get<1>((*it)) ;
        }

        file.close();
    } else {
        std::cerr<<"Failed to open file : "<<filename;
    }
}

// When the config changes, emit a signal to inform others of the current settings config i.e. subject configuration
void PupilDetection::setConfigLabel(QString config) {
    currentConfigLabel = config;
        if (config =="Automatic Parametrization")
        setAutoParamSettingsEnabled(true);
    else
        setAutoParamSettingsEnabled(false);

    emit configChanged(config);
}


// GB: added for allowing pupilDetectionSettingsDialog to check if there is detection going on
// TODO: use only this everywhere, and remove mainwondow's trackingOn bool
bool PupilDetection::isTrackingOn() {
    return trackingOn;
}

QRect PupilDetection::getROIsingleImageOnePupil() {
    return QRect(ROIsingleImageOnePupil.x, ROIsingleImageOnePupil.y, ROIsingleImageOnePupil.width, ROIsingleImageOnePupil.height);
}
QRect PupilDetection::getROIsingleImageTwoPupilA() {
    return QRect(ROIsingleImageTwoPupilA.x, ROIsingleImageTwoPupilA.y, ROIsingleImageTwoPupilA.width, ROIsingleImageTwoPupilA.height);
}
QRect PupilDetection::getROIsingleImageTwoPupilB() {
    return QRect(ROIsingleImageTwoPupilB.x, ROIsingleImageTwoPupilB.y, ROIsingleImageTwoPupilB.width, ROIsingleImageTwoPupilB.height);
}
QRect PupilDetection::getROIstereoImageOnePupil1() {
    return QRect(ROIstereoImageOnePupil1.x, ROIstereoImageOnePupil1.y, ROIstereoImageOnePupil1.width, ROIstereoImageOnePupil1.height);
}
QRect PupilDetection::getROIstereoImageOnePupil2() {
    return QRect(ROIstereoImageOnePupil2.x, ROIstereoImageOnePupil2.y, ROIstereoImageOnePupil2.width, ROIstereoImageOnePupil2.height);
}
QRect PupilDetection::getROIstereoImageTwoPupilA1() {
    return QRect(ROIstereoImageTwoPupilA1.x, ROIstereoImageTwoPupilA1.y, ROIstereoImageTwoPupilA1.width, ROIstereoImageTwoPupilA1.height);
}
QRect PupilDetection::getROIstereoImageTwoPupilA2() {
    return QRect(ROIstereoImageTwoPupilA2.x, ROIstereoImageTwoPupilA2.y, ROIstereoImageTwoPupilA2.width, ROIstereoImageTwoPupilA2.height);
}
QRect PupilDetection::getROIstereoImageTwoPupilB1() {
    return QRect(ROIstereoImageTwoPupilB1.x, ROIstereoImageTwoPupilB1.y, ROIstereoImageTwoPupilB1.width, ROIstereoImageTwoPupilB1.height);
}
QRect PupilDetection::getROIstereoImageTwoPupilB2() {
    return QRect(ROIstereoImageTwoPupilB2.x, ROIstereoImageTwoPupilB2.y, ROIstereoImageTwoPupilB2.width, ROIstereoImageTwoPupilB2.height);
}
QRect PupilDetection::getROImirrImageOnePupil1() {
    return QRect(ROImirrImageOnePupil1.x, ROImirrImageOnePupil1.y, ROImirrImageOnePupil1.width, ROImirrImageOnePupil1.height);
}
QRect PupilDetection::getROImirrImageOnePupil2() {
    return QRect(ROImirrImageOnePupil2.x, ROImirrImageOnePupil2.y, ROImirrImageOnePupil2.width, ROImirrImageOnePupil2.height);
}

void PupilDetection::setROIsingleImageOnePupil(QRectF roi) {
    if(!roi.isEmpty())
        ROIsingleImageOnePupil = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIsingleImageTwoPupilA(QRectF roi) {
    if(!roi.isEmpty())
        ROIsingleImageTwoPupilA = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIsingleImageTwoPupilB(QRectF roi) {
    if(!roi.isEmpty())
        ROIsingleImageTwoPupilB = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIstereoImageOnePupil1(QRectF roi) {
    if(!roi.isEmpty())
        ROIstereoImageOnePupil1 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIstereoImageOnePupil2(QRectF roi) {
    if(!roi.isEmpty())
        ROIstereoImageOnePupil2 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIstereoImageTwoPupilA1(QRectF roi) {
    if(!roi.isEmpty())
        ROIstereoImageTwoPupilA1 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIstereoImageTwoPupilA2(QRectF roi) {
    if(!roi.isEmpty())
        ROIstereoImageTwoPupilA2 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIstereoImageTwoPupilB1(QRectF roi) {
    if(!roi.isEmpty())
        ROIstereoImageTwoPupilB1 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROIstereoImageTwoPupilB2(QRectF roi) {
    if(!roi.isEmpty())
        ROIstereoImageTwoPupilB2 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROImirrImageOnePupil1(QRectF roi) {
    if(!roi.isEmpty())
        ROImirrImageOnePupil1 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}
void PupilDetection::setROImirrImageOnePupil2(QRectF roi) {
    if(!roi.isEmpty())
        ROImirrImageOnePupil2 = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()), static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}

ProcMode PupilDetection::getCurrentProcMode() {
    return (ProcMode)currentProcMode;
}

void PupilDetection::setCurrentProcMode(int val) {

    configureCameraConnection(true);

    currentProcMode = (ProcMode)val;

    configureCameraConnection(false);
}

void PupilDetection::setAutoParamEnabled(bool state) {
    autoParamEnabled = state;

    // GB TODO: do update here onse more for sure?
}

bool PupilDetection::isAutoParamSettingsEnabled(){
    return autoParamSettingsEnabled;
}

void PupilDetection::setAutoParamSettingsEnabled(bool enabled){
    autoParamSettingsEnabled = enabled;
}

void PupilDetection::setAutoParamPupSizePercent(float value) {
    autoParamPupSizePercent = value;
}

float PupilDetection::getAutoParamPupSizePercent() {
    return autoParamPupSizePercent;
}

void PupilDetection::setAutoParamScheduled(bool state) {
    autoParamScheduled = state;
}

void PupilDetection::performAutoParam() {

    qDebug() << "autoParamEnabled = " << autoParamEnabled;

    if(!autoParamEnabled)
        return;

    std::vector<PupilDetectionMethod*> algInstances;

    std::vector<cv::Rect> rois;

    switch(currentProcMode) {
        case SINGLE_IMAGE_ONE_PUPIL:
            algInstances.push_back(getCurrentMethod1());
            rois.push_back(ROIsingleImageOnePupil);
            break;
        case SINGLE_IMAGE_TWO_PUPIL:
            algInstances.push_back(getCurrentMethod1());
            algInstances.push_back(getCurrentMethod2());
            rois.push_back(ROIsingleImageTwoPupilA);
            rois.push_back(ROIsingleImageTwoPupilB);
            break;
        case STEREO_IMAGE_ONE_PUPIL:
            algInstances.push_back(getCurrentMethod1());
            algInstances.push_back(getCurrentMethod2());
            rois.push_back(ROIstereoImageOnePupil1);
            rois.push_back(ROIstereoImageOnePupil2);
            break;
        case STEREO_IMAGE_TWO_PUPIL:
            algInstances.push_back(getCurrentMethod1());
            algInstances.push_back(getCurrentMethod2());
            algInstances.push_back(getCurrentMethod3());
            algInstances.push_back(getCurrentMethod4());
            rois.push_back(ROIstereoImageTwoPupilA1);
            rois.push_back(ROIstereoImageTwoPupilA2);
            rois.push_back(ROIstereoImageTwoPupilB1);
            rois.push_back(ROIstereoImageTwoPupilB2);
            break;
        default:
            return;
    } 

    // TODO: MEG KELL OLDANI !!

    // useROIPreProcessing &&  (( NEMJÓ, MINDENKÉPP KELL LÉTEZŐ ROI))
    if((rois[0].width == 0 || rois[0].height == 0))
        return;


    // min 2 and max 8 mm means that the minimum is 1/4th of 8, 
    // or in other words: 2 = 0.25 *8;
    // This rule of thumb however, affects e.g. ElSe's capabilities negatively sometimes,
    // so I used 0.2 here

    float minToMaxDia = 0.2f;

    // NOTE: These are only DIAMETER values!
    float pupSizeFactorMin = (autoParamPupSizePercent/100.0f *minToMaxDia);
    if(pupSizeFactorMin < 0.01f)
        pupSizeFactorMin = 0.01f;
    float pupSizeFactorMax = (autoParamPupSizePercent/100.0f);

    for(size_t c=0; c<algInstances.size(); c++) {
        // for each algorithm instance, we perform the pupilDetectionMethod-type-specific automatic parametrization

        //std::string algName = pupilDetectionMethods1[pupilDetectionIndex]->title();
        float roiWidth = (float)rois[c].width;
        float roiHeight = (float)rois[c].height;
        float minDim = (roiWidth<=roiHeight) ? roiWidth : roiHeight;
        // now we get the RADIUS values
        float minRadius = pupSizeFactorMin*minDim /2;
        float maxRadius = pupSizeFactorMax*minDim /2;

        //qDebug() << "roiWidth = " << roiWidth;
        //qDebug() << "roiHeight =" << roiHeight;
        //qDebug() << "minDim =" << minDim;
        //qDebug() << "minRadius = " << minRadius;
        //qDebug() << "maxRadius =" << maxRadius;
        
        if(pupilDetectionIndex == 0) {
                // ELSE
                ElSe *alg = dynamic_cast<ElSe*>(algInstances[c]);
                
                alg->minAreaRatio = minRadius*minRadius*M_PI / (roiWidth*roiHeight);
                alg->maxAreaRatio = maxRadius*maxRadius*M_PI / (roiWidth*roiHeight);

                qDebug() << "Set AutoParam for algorithm ElSe, instance " << c;
                qDebug() << "minAreaRatio =" << alg->minAreaRatio;
                qDebug() << "maxAreaRatio =" << alg->maxAreaRatio;
                
        } else if(pupilDetectionIndex == 1) {
                // EXCUSE
                ExCuSe *alg = dynamic_cast<ExCuSe*>(algInstances[c]);

                alg->max_ellipse_radi = maxRadius;

                qDebug() << "Set AutoParam for algorithm ExCuSe, instance " << c;
                qDebug() << "max_ellipse_radi =" << alg->max_ellipse_radi;
                
        } if(pupilDetectionIndex == 2) {
                // PURE
                PuRe *alg = dynamic_cast<PuRe*>(algInstances[c]);

                // These are just a relative reference. User cannot set these to keep them constant
                // Anyway I guess a camera calibration that supports a precise px-to-mm mapping, could also be utilized here
                // Now we just back-calculate the roi width (= inter-canthi distance) 
                
                // GB TODO: distance or half of distance is the ROI width? check paper!
                
                alg->meanCanthiDistanceMM = 8.0f / maxRadius * roiWidth;
                alg->maxPupilDiameterMM = 8.0f;
                alg->minPupilDiameterMM = 8.0f * minToMaxDia; //2.0f;

                qDebug() << "Set AutoParam for algorithm PuRe, instance " << c;
                qDebug() << "meanCanthiDistanceMM =" << alg->meanCanthiDistanceMM;
                qDebug() << "maxPupilDiameterMM =" << alg->maxPupilDiameterMM;
                qDebug() << "minPupilDiameterMM =" << alg->minPupilDiameterMM;

        } if(pupilDetectionIndex == 3) {
                // PUREST
                PuReST *alg = dynamic_cast<PuReST*>(algInstances[c]);

                // same:
                alg->meanCanthiDistanceMM = 8.0f / maxRadius * roiWidth;
                alg->maxPupilDiameterMM = 8.0f;
                alg->minPupilDiameterMM = 8.0f * minToMaxDia; //2.0f;

                qDebug() << "Set AutoParam for algorithm PuReSt, instance " << c;
                qDebug() << "meanCanthiDistanceMM =" << alg->meanCanthiDistanceMM;
                qDebug() << "maxPupilDiameterMM =" << alg->maxPupilDiameterMM;
                qDebug() << "minPupilDiameterMM =" << alg->minPupilDiameterMM;

        } if(pupilDetectionIndex == 4) {
                // STARBURST
                Starburst *alg = dynamic_cast<Starburst*>(algInstances[c]);

                alg->edge_threshold = 20;		//threshold of pupil edge points detection
                alg->corneal_reflection_ratio_to_image_size = roiHeight / (maxRadius* 0.2f); // approx max size of the reflection relative to image height -> height/this
                alg->crWindowSize = maxRadius * 1.5;		    //corneal reflection search window size

                qDebug() << "Set AutoParam for algorithm Starburst, instance " << c;
                qDebug() << "edge_threshold =" << alg->edge_threshold;
                qDebug() << "corneal_reflection_ratio_to_image_size =" << alg->corneal_reflection_ratio_to_image_size;
                qDebug() << "crWindowSize =" << alg->crWindowSize;

        } if(pupilDetectionIndex == 5) {
                // SWIRSKI2D
                Swirski2D *alg = dynamic_cast<Swirski2D*>(algInstances[c]);

                alg->params.Radius_Min = minRadius;
                alg->params.Radius_Max = maxRadius;

                qDebug() << "Set AutoParam for algorithm Swirski2D, instance " << c;
                qDebug() << "params.Radius_Min =" << alg->params.Radius_Min;
                qDebug() << "params.Radius_Max =" << alg->params.Radius_Max;
        }

    }
}

void PupilDetection::setSynchronised(bool synchronised) {
    PupilDetection::synchronised = synchronised;
}

void PupilDetection::configureCameraConnection(bool connectOrDisconnect) {
    if(!camera)
        return;

    if(!connectOrDisconnect) {
        disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewSingleImageForOnePupil(CameraImage))); // Gabor Benyei (kheki4) on 2022.11.02, NOTE: refactored
        disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewSingleImageForTwoPupil(CameraImage)));
        disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImageForOnePupil(CameraImage))); // Gabor Benyei (kheki4) on 2022.11.02, NOTE: refactored
        disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImageForTwoPupil(CameraImage)));
        disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewMirrImageForOnePupil(CameraImage)));
    } else {
        if (currentProcMode == ProcMode::SINGLE_IMAGE_ONE_PUPIL) {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewSingleImageForOnePupil(CameraImage)));
        } else if (currentProcMode == ProcMode::SINGLE_IMAGE_TWO_PUPIL) {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewSingleImageForTwoPupil(CameraImage)));
        } else if (currentProcMode == ProcMode::STEREO_IMAGE_ONE_PUPIL) {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImageForOnePupil(CameraImage)));
        } else if (currentProcMode == ProcMode::STEREO_IMAGE_TWO_PUPIL) {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImageForTwoPupil(CameraImage)));
            // } else if(currentProcMode == ProcMode::MIRR_IMAGE_ONE_PUPIL) {
            //     connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewMirrImageForOnePupil(CameraImage)));
        } else {
            qDebug() << "Could not determine pupilDetection proc mode or it is still undetermined";
        }
    }
}