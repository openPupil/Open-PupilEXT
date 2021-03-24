
#include "cameraCalibration.h"
#include "pupil-detection-methods/Pupil.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#include <QtConcurrent/QtConcurrent>
#include <fstream>

// Creates a new single camera calibration process
// The calibration process should be attached to a camera and be executed in a seperate thread
CameraCalibration::CameraCalibration(QObject *parent) : QObject(parent), mode(NONE), intrinsicRMSE(0), avgMAE(0), sharpnessThreshold(3), captureCount(0), referenceObjectPoints(1), verifyFrameNumber(0), verifyOutputPath("") {

    captureDelay = 500;
    updateDelay = 33;

    // Factor which scales down images, increases speed for real time pattern detection, points are later optimized at subpixel on full image
    scalingFactor = 0.5;

    int numCornersHor = 10;
    int numCornersVer = 7;
    squareSize = 5; // mm
    boardSize = cv::Size(numCornersHor, numCornersVer);

    usedPattern = CHESSBOARD;
    maxCaptures = 30;

    reset();
    initReferenceObjectPoints();
}

CameraCalibration::~CameraCalibration() {

}

// Resets the calibration state and all values calculated by a previous calibration
void CameraCalibration::reset() {
    mode = NONE;
    captureCount = 0;
    intrinsicRMSE = 0;
    avgMAE = 0;

    imagePoints.clear();

    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrix.at<double>(0,0) = 1.0;

    distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
    emit unavailableCalibration();
}

// Starts the capturing of images for calibration up to the specified number of captures
// Images are detected for calibration pattern and their detection pattern positions saved for calibration
void CameraCalibration::startCapturing() {
    reset();
    mode = CAPTURING;

    timer.start();
}

// Starts the verification of a existing calibration
// Images are detected for calibration pattern and based on their detection pattern a calibration error is calculated (reprojection error)
void CameraCalibration::startVerifying() {
    mode = VERIFYING;
    captureCount = 0;

    verifyHistory.clear();
    verifyFrameNumber = 0;

    timer.start();
}

// Stops the current process i.e. capturing or verification
void CameraCalibration::stop() {
    if(mode == VERIFYING) {
        mode = CALIBRATED;
        if(!verifyOutputPath.isEmpty()) {
            // MAE_px is an average over the feature points of each image
            writeVectorCSV(verifyHistory, "frame,timestamp,MAE_px", verifyOutputPath.toStdString());
        }
    } else {
        mode = NONE;
    }

    captureCount = 0;

    timer.start();
}

// Event handler that received new images
// All calculations on the images are performed here based on the current mode
void CameraCalibration::onNewImage(const CameraImage &cimg) {
    // Callback that receives new camera image
    // Here different modes of calibration are employed, controlled through mode variable

    CameraImage mimg = cimg;
    if (cimg.img.channels() == 1) {
        cv::cvtColor(cimg.img, mimg.img, cv::COLOR_GRAY2BGR);
    } else {
        mimg.img = cimg.img.clone();
    }
    cv::Mat img = mimg.img;

    // Depending on the current state(mode) take different actions for the new image
    if(mode==CAPTURING && captureCount < maxCaptures) {
        // Collect new images and their detected feature points for calibration until enough (maxCaptures) are collected
        if(timer.elapsed() > captureDelay && !img.empty()) {
            timer.restart();

            imageSize = img.size();

            cv::Mat downscaledImg;
            cv::resize(img, downscaledImg, cv::Size(), scalingFactor, scalingFactor);

            std::vector<cv::Point2f> cornerPointsBuf;

            bool found = false;

            switch(usedPattern) {
                case CHESSBOARD:
                    try {
                        found = cv::findChessboardCorners(downscaledImg, boardSize, cornerPointsBuf, cv::CALIB_CB_FAST_CHECK);
                    } catch(...) {
                        std::cout<<"Error in finding chessboard."<<std::endl;
                        found = false;
                    }
                    break;

                case CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf );
                    break;

                case ASYMMETRIC_CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf, cv::CALIB_CB_ASYMMETRIC_GRID );
                    break;

                default:
                    found = false;
                    break;
            }

            if(found) {
                // Low quality is determined by a specified sharpness threshold
                bool lowquality = false;

                // Improve the found corners' coordinate accuracy for chessboard
                if(usedPattern == CHESSBOARD) {
                    for(int i=0; i<cornerPointsBuf.size(); i++) {
                        cornerPointsBuf[i] *= (1.0f/scalingFactor);
                    }

                    cv::Mat imgGray = img;
                    if(imgGray.channels() > 1)
                        cvtColor(imgGray, imgGray, cv::COLOR_BGR2GRAY);
                    cv::cornerSubPix(imgGray, cornerPointsBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));

                    cv::Scalar patternEstimate = cv::estimateChessboardSharpness(imgGray, boardSize, cornerPointsBuf);
                    std::cout<<"Avg. Sharpness: "<<patternEstimate[0]<<", Avg. min. brightness: "<<patternEstimate[1]<<", Avg. max. brightness: "<<patternEstimate[3]<<std::endl;
                    if(patternEstimate[0] > sharpnessThreshold) {
                        lowquality = true;
                    }
                    cv::putText(img, "SHARPNESS: " + std::to_string(patternEstimate[0]) + "px", cv::Point(static_cast<int>(0.1 * img.cols), static_cast<int>(static_cast<int>(
                            0.9 * img.rows))), cv::FONT_HERSHEY_PLAIN, 4, lowquality ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0), 3);
                }

                // Draw the corners.
                cv::putText(img, std::to_string(captureCount)+"/"+ std::to_string(maxCaptures), cv::Point(static_cast<int>(static_cast<int>(
                        0.1 * img.cols)), static_cast<int>(static_cast<int>(0.1 * img.rows))), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 0), 3);
                drawChessboardCorners(img, boardSize, cv::Mat(cornerPointsBuf), found);

                if(!lowquality) {
                    imagePoints.push_back(cornerPointsBuf);
                    captureCount++;
                }
            } else {
                cv::putText(img, "Pattern not found", cv::Point(static_cast<int>(static_cast<int>(0.25 * img.cols)), static_cast<int>(static_cast<int>(
                        0.25 * img.rows))), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            }

            emit processedImage(mimg);
        }
    } else if(mode==CAPTURING) {
        // When enough images were collected, perform calibration
        // Calibration is performed concurrent in another thread as otherwise it would block the current function and images from the camera will be queued increasing memory load significantly

        cv::putText(img, "CALIBRATING", cv::Point(static_cast<int>(static_cast<int>(0.1 * img.cols)), static_cast<int>(static_cast<int>(
                0.1 * img.rows))), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 0), 3);

        emit processedImage(mimg);

        //bool success = calibrate();
        calibrationSuccess = QtConcurrent::run(this, &CameraCalibration::calibrate);

        mode = CALIBRATING;
    } else if(mode==CALIBRATING) {
        // Calibration is currently performed in another thread, in this time new images are received and just displayed until calibration is finished

        cv::putText(img, "CALIBRATING", cv::Point(static_cast<int>(static_cast<int>(0.1 * img.cols)), static_cast<int>(static_cast<int>(
                0.1 * img.rows))), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 0), 3);

        emit processedImage(mimg);

        if(calibrationSuccess.isFinished() && calibrationSuccess.result()) {
            // Calibration finished, change state and calculate undistort-matrix for image undistortion
            mode = CALIBRATED;

            // Initials the maps for image mapping for undistortion using cv::remap
            newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0);
            initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(), newCameraMatrix, imageSize, CV_32F, undistMap1, undistMap2);

            emit finishedCalibration();
        } else if(calibrationSuccess.isFinished() && !calibrationSuccess.result()) {
            // Calibration failed, repeat process
            mode = CAPTURING;
            captureCount = 0;
            imagePoints.clear();
        }
    } else if(mode==CALIBRATED) {
        // Display the undistorted images to the user together with the calibration error
        if(timer.elapsed() > updateDelay && !img.empty()) {
            timer.restart();

            cv::Mat undist = img.clone();
            //cv::undistort(img, undist, cameraMatrix, distCoeffs); // replaced with remap, should be faster
            cv::remap(img, undist, undistMap1, undistMap2, cv::INTER_LINEAR);

            cv::putText(undist, "UNDISTORTED", cv::Point(static_cast<int>(static_cast<int>(0.1 * undist.cols)), static_cast<int>(static_cast<int>(
                    0.1 * undist.rows))), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 0), 3);
            cv::putText(undist, "RMSE: " + std::to_string(intrinsicRMSE) + " px", cv::Point(static_cast<int>(0.1 * undist.cols), static_cast<int>(0.2 * undist.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            cv::putText(undist, "avg. MAE: "+ std::to_string(avgMAE) + "px", cv::Point(static_cast<int>(0.1 * undist.cols), static_cast<int>(0.3 * undist.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);

            mimg.img = undist;

            emit processedImage(mimg);
        }
    } else if(mode==VERIFYING) {
        // Verification mode, detecting the calibration pattern and calculating the reprojection error live for each image

        if(timer.elapsed() > captureDelay && !img.empty()) {
            timer.restart();

            // Verifying imagesize is same as calibrated one
            assert(imageSize.width == img.cols && imageSize.height == img.rows);

            cv::Mat downscaledImg;
            cv::resize(img, downscaledImg, cv::Size(), scalingFactor, scalingFactor);

            std::vector<std::vector<cv::Point2f>> verifyImagePoints;
            std::vector<cv::Point2f> cornerPointsBuf;

            bool found = false;

            switch(usedPattern) {
                case CHESSBOARD:
                    try {
                        found = findChessboardCorners(downscaledImg, boardSize, cornerPointsBuf, cv::CALIB_CB_FAST_CHECK);
                    } catch(...) {
                        std::cout<<"Error in finding chessboard pattern."<<std::endl;
                        found = false;
                    }
                    break;

                case CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf );
                    break;

                case ASYMMETRIC_CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf, cv::CALIB_CB_ASYMMETRIC_GRID );
                    break;

                default:
                    found = false;
                    break;
            }

            if(found) {
                bool lowquality = false;

                // Improve the found corners' coordinate accuracy for chessboard
                if(usedPattern == CHESSBOARD) {
                    for(int i=0; i<cornerPointsBuf.size(); i++) {
                        cornerPointsBuf[i] *= (1.0f/scalingFactor);
                    }

                    cv::Mat imgGray = img;
                    if(imgGray.channels() > 1)
                        cvtColor(imgGray, imgGray, cv::COLOR_BGR2GRAY);
                    cv::cornerSubPix(imgGray, cornerPointsBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));

                    cv::Scalar patternEstimate = cv::estimateChessboardSharpness(imgGray, boardSize, cornerPointsBuf);
                    //std::cout<<"Avg. Sharpness: "<<patternEstimate[0]<<", Avg. min. brightness: "<<patternEstimate[1]<<", Avg. max. brightness: "<<patternEstimate[3]<<std::endl;
                    if(patternEstimate[0] > sharpnessThreshold) {
                        lowquality = true;
                    }
                    cv::putText(img, "SHARPNESS: " + std::to_string(patternEstimate[0]) + "px", cv::Point(
                            static_cast<int>(0.1 * img.cols), static_cast<int>(0.9 * img.rows)), cv::FONT_HERSHEY_PLAIN, 4, lowquality ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0), 3);
                }

                // Draw the corners.
                drawChessboardCorners(img, boardSize, cv::Mat(cornerPointsBuf), found);

                if(!lowquality) {
                    verifyImagePoints.push_back(cornerPointsBuf);
                    referenceObjectPoints.resize(verifyImagePoints.size(), referenceObjectPoints[0]);

                    std::vector<float> verifiedMAE = verifyReprojectionErrors(referenceObjectPoints, verifyImagePoints, cameraMatrix, distCoeffs); // TODO this actually only returns one average over complete image
                    double avgVerifiedMAE = std::accumulate(verifiedMAE.begin(), verifiedMAE.end(), 0.0) / verifiedMAE.size();
                    for(auto &err: verifiedMAE) {
                        verifyHistory.push_back(std::make_tuple(verifyFrameNumber, cimg.timestamp, err));
                    }
                    verifyFrameNumber++;

                    std::cout << "Verifying Average Re-projection error of main camera: "<< avgVerifiedMAE << std::endl;

                    cv::putText(img, "VERIFY", cv::Point(static_cast<int>(0.1 * img.cols),
                                                         static_cast<int>(0.1 * img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255, 0, 0), 3);
                    cv::putText(img, "avg. MAE: " + std::to_string(avgVerifiedMAE) + "px", cv::Point(
                            static_cast<int>(0.1 * img.cols), static_cast<int>(0.2 * img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
                }
            } else {
                cv::putText(img, "Pattern not found", cv::Point(static_cast<int>(0.25 * img.cols),
                                                                static_cast<int>(0.25 * img.rows)), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            }

            emit processedImage(mimg);
        }
    } else {
        // None of the states i.e. uncalibrated and not collecting, just display the camera image
        if(timer.elapsed() > updateDelay && !img.empty()) {
            timer.restart();

            emit processedImage(mimg);
        }
    }
}

// Performs the actual camera calibration using OpenCV calibration routines and the captured calibration pattern points
bool CameraCalibration::calibrate() {
    QMutexLocker locker(&mutex);

    referenceObjectPoints.resize(imagePoints.size(), referenceObjectPoints[0]);

    std::vector<cv::Mat> rvecs, tvecs;

    // Find intrinsic and extrinsic camera parameters
    intrinsicRMSE = cv::calibrateCamera(referenceObjectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs);

    std::cout << "Re-projection error reported by calibrateCamera: " << intrinsicRMSE << std::endl;

    bool ok = cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs);

    // Calculate reprojection error
    std::vector<float> reprojectionError = reprojectionErrors(referenceObjectPoints, imagePoints, rvecs, tvecs, cameraMatrix, distCoeffs);

    reprojectionPointsMAE = reprojectionError;
    avgMAE = std::accumulate(reprojectionError.begin(), reprojectionError.end(), 0.0) / reprojectionError.size();

    std::vector<double> diff(reprojectionError.size());
    std::transform(reprojectionError.begin(), reprojectionError.end(), diff.begin(), [this](double x) { return x - avgMAE; });
    double sqSum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdDev = std::sqrt(sqSum / reprojectionError.size());

    std::cout << "Average Re-projection error over observations: " << avgMAE << " [" << stdDev << "]" << std::endl;

    return ok;
}

// Initial the reference positions i.e. the ideal pattern point positions used for calibration and calculation of reprojection errors
void CameraCalibration::initReferenceObjectPoints() {
    referenceObjectPoints[0].clear();

    switch(usedPattern) {
        case CHESSBOARD:
        case CIRCLES_GRID:
            for (int i = 0; i < boardSize.height; ++i)
                for (int j = 0; j < boardSize.width; ++j) {
                    referenceObjectPoints[0].push_back(cv::Point3f(float(j * squareSize), float(i * squareSize), 0));
                }
            break;

        case ASYMMETRIC_CIRCLES_GRID:
            for (int i = 0; i < boardSize.height; i++)
                for (int j = 0; j < boardSize.width; j++) {
                    referenceObjectPoints[0].push_back(cv::Point3f(float((2 * j + i % 2) * squareSize), float(i * squareSize), 0));
                }
            break;

        default:
            break;
    }
}

// Calculate the reprojection error of a calibration, based on reference points, captured pattern points, and their rotation/translation vectors calculated in the calibration process
// This means that this error can only the calculated for the captured image in the calibration process, for a verification of independend images see verifyReprojectionErrors
// Returns an vector of reprojection error per image
// The reprojection error describes an average error per image over all pattern points in an image
std::vector<float> CameraCalibration::reprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints, const std::vector<std::vector<cv::Point2f>> &f_imagePoints, std::vector<cv::Mat> m_rvecs, std::vector<cv::Mat> m_tvecs, const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs) {

    std::vector<cv::Point2f> imagePoints2;
    std::vector<float> reprojErrs;

    int i;
    double err;
    reprojErrs.resize(objectPoints.size());

    for( i = 0; i < (int) objectPoints.size(); ++i ) {
        // Reproject actual objectpoint (optimal distance ie. the checkerboard size)
        cv::projectPoints(cv::Mat(objectPoints[i]), m_rvecs[i], m_tvecs[i], m_cameraMatrix, m_distCoeffs, imagePoints2);
        // Calculate distance of reprojection and actual point on saved imagepoints
        err = cv::norm(cv::Mat(f_imagePoints[i]), cv::Mat(imagePoints2), CV_L2);

        // This is what opencv computed for the rmse
//        int n = (int)objectPoints[i].size();
//        perViewErrors[i] = (float)std::sqrt(err*err/n);
//        totalErr += err*err;
//        totalPoints += n;
//        return std::sqrt(totalErr/totalPoints);

// Debug only, to save a detailed verification error as file
//        for(int j = 0; j < (int) objectPoints[i].size(); ++j ) {
//            double single_err = cv::norm(cv::Mat(f_imagePoints[i][j]), cv::Mat(imagePoints2[j]), CV_L2);
//            projectionErrorHistory.push_back(std::make_pair((i * 1000) + j, single_err));
//        }
        reprojErrs[i] = static_cast<float>(err / imagePoints2.size());
    }

    return reprojErrs;
}

// Calculate the reprojection error of a finished calibration, based on reference points and independendly captured pattern points
// Returns an vector of reprojection error per image
// The reprojection error describes an average error per image over all pattern points in an image
std::vector<float> CameraCalibration::verifyReprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints, const std::vector<std::vector<cv::Point2f>> &f_imagePoints, const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs) {

    // Verify works different as we dont have the rotation and translation vectors of each pattern view (rvecs, tvecs)
    // We use solvePNP to get the rvecs, tvecs vectors first (extrinsic vectors for each image)
    std::vector<cv::Mat> m_rvecs(objectPoints.size()),  m_tvecs(objectPoints.size());

    for(int i = 0; i < (int) objectPoints.size(); ++i ) {
        cv::solvePnP(objectPoints[i], f_imagePoints[i], m_cameraMatrix, m_distCoeffs, m_rvecs[i], m_tvecs[i]);
    }

    return reprojectionErrors(objectPoints, f_imagePoints, m_rvecs, m_tvecs, m_cameraMatrix, m_distCoeffs);
}

// Saves the calibration to file using OpenCVs own FileStorage interface and format, the file format is XML
void CameraCalibration::saveToFile(QString filename) {
    // Save the calibration using opencv given functions in a XML format
    cv::FileStorage fs(filename.toStdString(), cv::FileStorage::WRITE);

    if (!fs.isOpened()) {
        std::cerr << "CameraCalibration: File failed to open " << filename.toStdString() << std::endl;
        return;
    }

    fs << "boardSize_width"  << boardSize.width
       << "boardSize_height" << boardSize.height
       << "squareSize"         << squareSize
       << "imageSize" << imageSize
       << "usedPattern" << usedPattern
        << "maxCaptures" << maxCaptures
        << "calibrationDate" << QDate::currentDate().toString().toStdString()

        << "cameraMatrix" << cameraMatrix
        << "distCoeffs" << distCoeffs

//        << "undistMap1" << undistMap1
//        << "undistMap2" << undistMap2

        << "intrinsicRMSE" << intrinsicRMSE
        << "avgMAE" << avgMAE
        << "reprojectionPointsMAE" << reprojectionPointsMAE;
}

// Loads an existing calibration from file (XML) using OpenCVs FileStorage interface
// The loaded should be saved previously using the saveToFile function
void CameraCalibration::loadFromFile(QString filename) {
    cv::FileStorage fs(filename.toStdString(), cv::FileStorage::READ);

    reset();

    if (!fs.isOpened()) {
        std::cerr << "CameraCalibration: File failed to open " << filename.toStdString() << std::endl;
        return;
    }

    int boardWidth, boardHeight;

    fs["boardSize_width"]  >> boardWidth;
    fs["boardSize_height"] >> boardHeight;
    boardSize = cv::Size(boardWidth, boardHeight);

    fs["squareSize"]  >> squareSize;
    fs["imageSize"]  >> imageSize;

    fs["usedPattern"] >> usedPattern;
    fs["maxCaptures"] >> maxCaptures;
    //fs["calibrationDate"] >> calibrationDate;

    fs["cameraMatrix"] >> cameraMatrix;
    fs["distCoeffs"]   >> distCoeffs;
//    fs["undistMap1"]   >> undistMap1;
//    fs["undistMap2"]   >> undistMap2;

    fs["intrinsicRMSE"] >> intrinsicRMSE;
    fs["avgMAE"] >> avgMAE;
    fs["reprojectionPointsMAE"] >> reprojectionPointsMAE;

    // If the calibration is "valid", calculate mappings for the undistortion
    if(cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs)) {

        newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0);
        initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(), newCameraMatrix, imageSize, CV_32F, undistMap1, undistMap2);

        if(cv::checkRange(undistMap1) && cv::checkRange(undistMap2)) {
            mode = CALIBRATED;
            emit finishedCalibration();
        }
    }
}

// Undistorts a given image using the calibration
// If no calibration is load, the unchanged image is returned
cv::Mat CameraCalibration::undistortImage(const cv::Mat &img) {
    // Undistorting using remap should be faster than using the undistort function
    if(mode!=CALIBRATED)
        return img;

    cv::Mat undist;
    cv::remap(img, undist, undistMap1, undistMap2, cv::INTER_LINEAR);
    return undist;
}

// Undistorts only the pupil size of a given pupil detection, rather then the complete image (faster)
// This is done by undistorting only contour points of the pupil and calculating the new pupil size using the undistorted points
double CameraCalibration::undistortPupilDiameter(const Pupil &pupil) {
    // Undistorting using remap should be faster than using the undistort function
    if(mode!=CALIBRATED || !pupil.valid(-2))
        return pupil.diameter();

    Pupil rotPupil(pupil);
    rotPupil.angle += 360-rotPupil.angle;

    // Select the top left and top right corner of the bounding rects of the pupils as the points we undistort
    cv::Point2f mainPointsArr[4]; // rotatedRect points in order: bottomLeft, topLeft, topRight, bottomRight
    rotPupil.points(mainPointsArr);
    int secondPoint = rotPupil.size.width > rotPupil.size.height ? 2 : 0; // if the pupil major axis is horizontal use topLeft and topRight, else topLeft and bottomLeft

    std::vector<cv::Point2f> mainPoints{mainPointsArr[1], mainPointsArr[secondPoint]};

    std::vector<cv::Point2f> undistCornerPointsBuf;

    cv::undistortPoints(cv::Mat(mainPoints), undistCornerPointsBuf, cameraMatrix, distCoeffs, cv::Mat(), newCameraMatrix);

    return cv::norm(undistCornerPointsBuf[0] - undistCornerPointsBuf[1]);
}

// Helper function to persist error values to csv file
template<typename T>
void CameraCalibration::writeVectorCSV(std::vector<std::tuple<int, uint64_t, T>> data, const std::string &header, const std::string &filename) {
    // Helper function to verify information to disk

    std::ofstream file(filename);

    if(file.is_open()) {

        file << header << std::endl;

        for(typename std::vector<std::tuple<int, uint64_t , T>>::iterator it = data.begin(); it != data.end(); ++it) {
            file << std::get<0>((*it)) << "," << std::get<1>((*it)) << "," << std::get<2>((*it)) << std::endl;
        }

        file.close();
    } else {
        std::cerr<<"Failed to open file : "<<filename<<std::endl;
    }
}