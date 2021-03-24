
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


// QTs event signal eventloop queues images for us, we run this pupildetection worker in a extra thread and every call to newImage is queued automatically
// This may however queue a large number of images if the processing speed is slow, increasing the memory potentially until it is full and the application is killed
// We dont have access to the eventloop to handle this, so its up to the user for now to not use a rate too high

// Creates a new pupil detection worker which include all pupil detection algorithms
// Should be run on a seperate thread
PupilDetection::PupilDetection(QObject *parent) : QObject(parent),
                                                  camera(nullptr),
                                                  frameCounter(new FrameRateCounter(parent)),
                                                  stereoMode(false),
                                                  useOutlineConfidence(true),
                                                  useROIPreProcessing(false),
                                                  useImageUndistort(false),
                                                  usePupilUndistort(false),
                                                  trackingOn(false),
                                                  calibrated(false),
                                                  showROI(true),
                                                  showPupilCenter(false),
                                                  currentConfigLabel("Default"),
                                                  ROI(),
                                                  ROISecondary() {

    drawDelay = 33; // ~30fps

    // we initialize the algorithms here one time and save them in a list, the created objects are then changed through index change of the list
    pupilDetectionMethods.push_back(new ElSe());
    pupilDetectionMethods.push_back(new ExCuSe());
    pupilDetectionMethods.push_back(new PuRe());
    pupilDetectionMethods.push_back(new PuReST());
    pupilDetectionMethods.push_back(new Starburst());
    pupilDetectionMethods.push_back(new Swirski2D());
    // TODO finish integrating Swirski3D
    //double focal_length = 0.5;
    //pupilDetectionMethods.push_back(new Swirski3D(new PuReST(), focal_length, 5, 0.5));

    // We need a second instance of every detection method for stereo detection due to the threaded execution and potential non-thread safe algorithm code
    pupilDetectionMethodsSecondary.push_back(new ElSe());
    pupilDetectionMethodsSecondary.push_back(new ExCuSe());
    pupilDetectionMethodsSecondary.push_back(new PuRe());
    pupilDetectionMethodsSecondary.push_back(new PuReST());
    pupilDetectionMethodsSecondary.push_back(new Starburst());
    pupilDetectionMethodsSecondary.push_back(new Swirski2D());
    //pupilDetectionMethodsSecondary.push_back(new Swirski3D(new PuReST(), focal_length, 5, 0.5));

    // Default algorithm PuRe
    pupilDetectionIndex = 2;

    // Processing speed frame counter
    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    connect(this, SIGNAL(processedPupilData(quint64, Pupil, QString)), frameCounter, SLOT(count()));
    connect(this, SIGNAL(processedStereoPupilData(quint64, Pupil, Pupil, QString)), frameCounter, SLOT(count()));

    drawTimer.start();
    processingTimer.start();
}

PupilDetection::~PupilDetection() {

}

// Attaches a camera to the pupil detection process
// Checks if the camera is calibrated and stereo or single, sets the detection mode accordingly
void PupilDetection::setCamera(Camera *m_camera) {

    camera = m_camera;

    stereoMode = camera->getType()==CameraImageType::LIVE_STEREO_CAMERA || camera->getType()==CameraImageType::STEREO_IMAGE_FILE;
    calibrated = false;

    if(camera->getType()==CameraImageType::LIVE_STEREO_CAMERA) {
        stereoCalibration = dynamic_cast<StereoCamera*>(camera)->getCameraCalibration();
        calibrated = static_cast<bool>(stereoCalibration->isCalibrated());
    } else if(camera->getType()==CameraImageType::STEREO_IMAGE_FILE) {
        stereoCalibration = dynamic_cast<FileCamera*>(camera)->getStereoCameraCalibration();
        calibrated = static_cast<bool>(stereoCalibration->isCalibrated());
    } else if(camera->getType()==CameraImageType::LIVE_SINGLE_CAMERA) {
        singleCalibration = dynamic_cast<SingleCamera*>(camera)->getCameraCalibration();
        calibrated = static_cast<bool>(singleCalibration->isCalibrated());
    } else if(camera->getType()==CameraImageType::SINGLE_IMAGE_FILE) {
        singleCalibration = dynamic_cast<FileCamera*>(camera)->getCameraCalibration();
        calibrated = static_cast<bool>(singleCalibration->isCalibrated());
    }
}

// Starts the algorithm by connecting the camera image signals to the processing callbacks
void PupilDetection::startDetection() {

    trackingOn = true;
    if(camera) {
        if(stereoMode) {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImage(CameraImage)));
        } else {
            //runtimeHistory.clear();
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewImage(CameraImage)));
        }
        emit processingStarted();
    }
}

// Stops the pupil detection by disconnecting the signals of new images to the processing
void PupilDetection::stopDetection() {

    if(camera && trackingOn) {
        trackingOn = false;

        if(stereoMode) {
            // For debugging, measuring times
            //auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
            //writeVectorCSV(runtimeHistory, "timestamp,runtime[ms]", pupilDetectionMethods[pupilDetectionIndex]->title() + "_" + std::to_string(timestamp) + "_triangulateHistory.csv");
            disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImage(CameraImage)));
        } else {
            // Runtime history
            //auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
            //writeVectorCSV(runtimeHistory, "timestamp,runtime[ms]", pupilDetectionMethods[pupilDetectionIndex]->title() + "_" + std::to_string(timestamp) + "_runtimeHistory.csv");
            disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewImage(CameraImage)));
        }
        emit processingFinished();
    }
}

// Changes the applied pupil detection algorithm
// The change is performed by first disconnecting the signal to stop potential frames, switch the algorithm and connect them again
// Emits a signal to signal the algorithm changed
void PupilDetection::setAlgorithm(QString method) {

    if(camera && trackingOn) {
        if(stereoMode) {
            disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImage(CameraImage)));
        } else {
            disconnect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewImage(CameraImage)));
        }
    }

    frameCounter->reset();

    int i = 0;
    for(auto pm: pupilDetectionMethods) {
        if(pm->title() == method.toStdString())
            pupilDetectionIndex = i;
        i++;
    }

    emit algorithmChanged();

    if(camera && trackingOn) {
        if(stereoMode) {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewStereoImage(CameraImage)));
        } else {
            connect(camera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(onNewImage(CameraImage)));
        }
    }
}

// Slot callback for receiving new single camera images
// Performs the processing/pupil detection
// Emits the pupil detection result as a signal, as well as processed images with plotted pupil contours
// Depending on the configuration, performs undistortion on the images or pupil detections
void PupilDetection::onNewImage(const CameraImage &cimg) {

    // Processing fps restriction not working correctly, timers overhead seem to break timing, left out for now
    //std::cout<<cimg.filename<<std::endl;

    if (!trackingOn) {
        std::cout<<"Single: Tracking is stopped but receiving signals."<<std::endl;
        return;
    }

    cv::Mat bwFrame = cimg.img;

    // Undistorting the whole image is rather slow (~4ms on our test system), use contour point undistort instead (>~1ms)
    if(!usePupilUndistort && useImageUndistort) {
        //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        bwFrame = singleCalibration->undistortImage(cimg.img);
        //std::cout<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 <<std::endl;
    }

    cv::Rect roi = cv::Rect(0, 0, bwFrame.cols, bwFrame.rows);

    if(useROIPreProcessing && !ROI.empty() && roi != ROI && ROI.width<=bwFrame.cols && ROI.height<=bwFrame.rows) {
        roi = ROI;
        bwFrame = bwFrame(ROI);
    }

    if (bwFrame.channels() > 1) {
        cv::cvtColor(bwFrame, bwFrame, cv::COLOR_BGR2GRAY);
    }

    Pupil pupil = Pupil();

    // Pupil detection
    try {
        if(useOutlineConfidence) {
            //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            pupilDetectionMethods[pupilDetectionIndex]->runWithConfidence(bwFrame, pupil);
            //runtimeHistory.push_back(std::make_pair(cimg.timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
        } else {
            //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            pupilDetectionMethods[pupilDetectionIndex]->run(bwFrame, pupil);
            //runtimeHistory.push_back(std::make_pair(cimg.timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
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
        //std::cout<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 <<std::endl;
    } else if(!usePupilUndistort && useImageUndistort) {
        pupil.undistortedDiameter = pupil.diameter();
    }

    pupil.algorithmName = pupilDetectionMethods[pupilDetectionIndex]->title();

    // Drawing of pupil detections on the image is only performed at ~30fps
    if (trackingOn && drawTimer.elapsed() > drawDelay) {
        drawTimer.start();

        CameraImage mimg = cimg;
        mimg.img = cimg.img.clone();

        if(!usePupilUndistort && useImageUndistort) {
            mimg.img = singleCalibration->undistortImage(cimg.img);
        } else {
            mimg.img = cimg.img.clone();
        }

        if (mimg.img.channels() == 1) {
            cv::cvtColor(mimg.img, mimg.img, cv::COLOR_GRAY2BGR);
        }

        if(showROI)
            cv::rectangle(mimg.img, roi, cv::Scalar(255, 0, 255 ), 3);

        if(pupil.valid(-2.0)) {
            cv::ellipse(mimg.img, pupil, cv::Scalar( 0, 0, 255 ), 1);
            if(showPupilCenter)
                cv::circle(mimg.img, pupil.center, 1, CV_RGB(255,0,0),3);
        } else {
            cv::putText(mimg.img, "NO PUPIL FOUND", cv::Point(static_cast<int>(0.25 * mimg.img.cols),
                                                              static_cast<int>(0.25 * mimg.img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 255), 4);
        }

        emit processedImage(mimg);
    }

    emit processedPupilData(cimg.timestamp, pupil, QString::fromStdString(cimg.filename));
}

// Slot callback for receiving new stereo camera images
// Performs the processing/pupil detection
// Emits the pupil detection result as a signal, as well as processed images with plotted pupil contours
// Depending on the configuration, performs undistortion on the pupil detections
void PupilDetection::onNewStereoImage(const CameraImage &simg) {
    // at the moment, the images are not undistorted completely but only the major axis points are undistorted after detection for absolute unit conversion
    // This creates a discrepancy between the undistorted pixel size and the physical measure, as a fix, undistortedDiamter can be calculated using useUndistort

    if (!trackingOn) {
        std::cout<<"Stereo: Tracking is stopped but receiving signals."<<std::endl;
        return;
    }

    cv::Rect roi = cv::Rect(0, 0, simg.img.cols, simg.img.rows);
    cv::Rect roiSecondary = cv::Rect(0, 0, simg.img.cols, simg.img.rows);
    cv::Mat bwFrame = simg.img;
    cv::Mat bwFrameSecondary = simg.imgSecondary;

    if(useROIPreProcessing && !ROI.empty() && roi != ROI && ROI.width<=bwFrame.cols && ROI.height<=bwFrame.rows) {
        roi = ROI;
        bwFrame = bwFrame(ROI);
    }

    if(useROIPreProcessing && !ROISecondary.empty() && roiSecondary != ROISecondary && ROISecondary.width<=bwFrameSecondary.cols && ROISecondary.height<=bwFrameSecondary.rows) {
        roiSecondary = ROISecondary;
        bwFrameSecondary = bwFrameSecondary(ROISecondary);
    }

    if (simg.img.channels() > 1) {
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
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrame));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethodsSecondary[pupilDetectionIndex], &PupilDetectionMethod::runWithConfidence, bwFrameSecondary));
        } else {
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethods[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrame));
            synchronizer.addFuture(QtConcurrent::run(pupilDetectionMethodsSecondary[pupilDetectionIndex], &PupilDetectionMethod::run, bwFrameSecondary));

//            std::function<Pupil(const cv::Mat&)> run = [&](const cv::Mat &img){ return pupilDetectionMethods[pupilDetectionIndex]->run(img); };
//            std::function<Pupil(const cv::Mat&)> runSecondary = [&](const cv::Mat &img){ return pupilDetectionMethodsSecondary[pupilDetectionIndex]->run(img); };
//            synchronizer.addFuture(QtConcurrent::run(run, bwFrame));
//            synchronizer.addFuture(QtConcurrent::run(runSecondary, bwFrameSecondary));
        }
        synchronizer.waitForFinished();
        // Unhandled exceptions in the QtConcurrent::run function are thrown at the result() call
        pupil = synchronizer.futures().at(0).result();
        pupilSecondary = synchronizer.futures().at(1).result();
    } catch (...) {
        pupil.clear();
        pupilSecondary.clear();
    }
    //runtimeHistory.push_back(std::make_pair(simg.timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()));

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
        //std::cout<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0 <<std::endl;
    } else if(!usePupilUndistort && useImageUndistort) {
        pupil.undistortedDiameter = pupil.diameter();
        pupilSecondary.undistortedDiameter = pupil.diameter();
    }

    pupil.algorithmName = pupilDetectionMethods[pupilDetectionIndex]->title();
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
        //cv::line(mimg.img,mainPointsArr[1], mainPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        cv::Point2f secondaryPointsArr[4];
        rotPupilSecondary.points(secondaryPointsArr);
        //cv::line(mimg.imgSecondary,secondaryPointsArr[1], secondaryPointsArr[secondPoint], cv::Scalar(255, 0, 0));

        std::vector<cv::Point2f> mainPoints{mainPointsArr[1], mainPointsArr[secondPoint]}, secondaryPoints{secondaryPointsArr[1], secondaryPointsArr[secondPoint]};
        std::vector<cv::Point3f> worldPoints = stereoCalibration->convertPointsTo3D(mainPoints, secondaryPoints);

        pupil.physicalDiameter = static_cast<float>(cv::norm(worldPoints[0] - worldPoints[1]));
        pupilSecondary.physicalDiameter = pupil.physicalDiameter;
        //runtimeHistory.push_back(std::make_pair(simg.timestamp, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()));
    }

    if (trackingOn && drawTimer.elapsed() > drawDelay) {
        drawTimer.start();
        CameraImage mimg = simg;
        mimg.img = simg.img.clone();
        mimg.imgSecondary = simg.imgSecondary.clone();

        if (mimg.img.channels() == 1) {
            cv::cvtColor(mimg.img, mimg.img, cv::COLOR_GRAY2BGR);
            cv::cvtColor(mimg.imgSecondary, mimg.imgSecondary, cv::COLOR_GRAY2BGR);
        }

        if(showROI) {
            cv::rectangle(mimg.img, roi, cv::Scalar(255, 0, 255 ), 3);
            cv::rectangle(mimg.imgSecondary, roiSecondary, cv::Scalar(255, 0, 255 ), 3);
        }

        if(pupil.valid(-2.0)) {
            cv::ellipse(mimg.img, pupil, cv::Scalar( 0, 0, 255 ), 1);
            if(showPupilCenter)
                cv::circle(mimg.img, pupil.center, 1, CV_RGB(255,0,0),3);
        } else {
            cv::putText(mimg.img, "NO PUPIL FOUND", cv::Point(static_cast<int>(0.25 * mimg.img.cols),
                                                              static_cast<int>(0.25 * mimg.img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 255), 4);
        }

        if(pupilSecondary.valid(-2.0)) {
            cv::ellipse(mimg.imgSecondary, pupilSecondary, cv::Scalar( 0, 0, 255 ), 1);
            if(showPupilCenter)
                cv::circle(mimg.imgSecondary, pupilSecondary.center, 1, CV_RGB(255,0,0),3);
        } else {
            cv::putText(mimg.imgSecondary, "NO PUPIL FOUND", cv::Point(static_cast<int>(0.25 * mimg.img.cols),
                                                                       static_cast<int>(0.25 * mimg.img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 255), 4);
        }

        emit processedImage(mimg);
    }

    emit processedStereoPupilData(simg.timestamp, pupil, pupilSecondary, QString::fromStdString(simg.filename));
}

// Set the ROI for the main camera image
void PupilDetection::setROI(QRectF roi) {
    if(!roi.isEmpty())
        ROI = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()),
                       static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}

// Set the ROI for the secondary camera image
void PupilDetection::setSecondaryROI(QRectF roi) {
    if(!roi.isEmpty())
        ROISecondary = cv::Rect(static_cast<int>(roi.topLeft().x()), static_cast<int>(roi.topLeft().y()),
                                static_cast<int>(roi.width()), static_cast<int>(roi.height()));
}

template <typename T> void PupilDetection::writeVectorCSV(std::vector<std::pair<uint64_t , T>> data, const std::string &header, const std::string &filename) {
    // Debug helper function
    std::ofstream file(filename);

    if(file.is_open()) {

        file << header << std::endl;

        for(typename std::vector<std::pair<uint64_t , T>>::iterator it = data.begin(); it != data.end(); ++it) {
            file << std::get<0>((*it)) << "," << std::get<1>((*it)) << std::endl;
        }

        file.close();
    } else {
        std::cerr<<"Failed to open file : "<<filename<<std::endl;
    }
}

// Draws the ROI rectangle on the processed image
void PupilDetection::onShowROI(bool value) {
    showROI = value;
}

// Draws the pupil center on the processed image
void PupilDetection::onShowPupilCenter(bool value) {
    showPupilCenter = value;
}

// When the config changes, emit a signal to inform others of the current settings config i.e. subject configuration
void PupilDetection::setConfigLabel(QString config) {
    currentConfigLabel = config;
    emit configChanged(config);
}
