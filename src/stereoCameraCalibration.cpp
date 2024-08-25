
#include "stereoCameraCalibration.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#include <fstream>

// Creates a new stereo camera calibration process
// The calibration process should be attached to a camera and be executed in a seperate thread
StereoCameraCalibration::StereoCameraCalibration(QObject *parent) : QObject(parent),
                                                                    stereoRMSE(0), intrinsicRMSE(0), intrinsicRMSESec(0), avgMAE(0), avgMAESec(0),
                                                                    captureCount(0), sharpnessThreshold(3), referenceObjectPoints(1), verifyOutputPath(""), verifyFrameNumber(0) {
    captureDelay = 500;
    updateDelay = 100;

    scalingFactor = 0.5;

    int numCornersHor = 10;
    int numCornersVer = 7;
    squareSize = 5; // mm
    boardSize = cv::Size(numCornersHor, numCornersVer);

    usedPattern = CHESSBOARD;
    maxCaptures = 50;

    reset();
    initReferenceObjectPoints();
}

StereoCameraCalibration::~StereoCameraCalibration() {

}

void StereoCameraCalibration::initReferenceObjectPoints() {
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

// Resets the calibration state and all values calculated by a previous calibration
void StereoCameraCalibration::reset() {
    mode = NONE;
    captureCount = 0;
    intrinsicRMSE = 0;
    intrinsicRMSESec = 0;
    avgMAE = 0;
    avgMAESec = 0;
    stereoRMSE = 0;

    imagePoints.clear();
    imagePointsSecondary.clear();

    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrix.at<double>(0,0) = 1.0;

    cameraMatrixSecondary = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrixSecondary.at<double>(0,0) = 1.0;

    distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
    distCoeffsSecondary = cv::Mat::zeros(8, 1, CV_64F);

    emit unavailableCalibration();
}

// Starts the capturing of images for calibration up to the specified number of captures
// Images are detected for calibration pattern and their detection pattern positions saved for calibration
void StereoCameraCalibration::startCapturing() {
    reset();
    mode = CAPTURING;

    timer.start();
}

// Starts the verification of a existing calibration
// Images are detected for calibration pattern and based on their detection pattern a calibration error is calculated (reprojection error)
void StereoCameraCalibration::startVerifying() {
    mode = VERIFYING;
    captureCount = 0;

    verifyHistory.clear();
    verifyFrameNumber = 0;

    timer.start();
}

// Stops the current process i.e. capturing or verification
void StereoCameraCalibration::stop() {
    if(mode == VERIFYING) {
        mode = CALIBRATED;
        if(!verifyOutputPath.isEmpty()) {
            // distanceError instead of MAE_mm because we write every single distance in the pattern and image instead of an average over the image
            writeVectorCSV(verifyHistory, "frame,timestamp,distanceError_mm", verifyOutputPath.toStdString());
        }
    } else{
        mode = NONE;
    }

    captureCount = 0;

    timer.start();
}

// Event handler that received new (stereo) images
// All calculations on the images are performed here based on the current mode
void StereoCameraCalibration::onNewImage(const CameraImage &cimg) {

    CameraImage mimg = cimg;
    if (cimg.img.channels() == 1) {
        cv::cvtColor(cimg.img, mimg.img, cv::COLOR_GRAY2BGR);
        cv::cvtColor(cimg.imgSecondary, mimg.imgSecondary, cv::COLOR_GRAY2BGR);
    } else {
        mimg.img = cimg.img.clone();
        mimg.imgSecondary = cimg.imgSecondary.clone();    }

    cv::Mat img = mimg.img;
    cv::Mat imgSecondary = mimg.imgSecondary;

    if(mode==CAPTURING && captureCount < maxCaptures) {

        if(timer.elapsed() > captureDelay && !img.empty()) {
            timer.restart();

            assert(img.size() == imgSecondary.size());

            imageSize = img.size();

            cv::Mat downscaledImg, downscaledImgSecondary;
            cv::resize(img, downscaledImg, cv::Size(), scalingFactor, scalingFactor);
            cv::resize(imgSecondary, downscaledImgSecondary, cv::Size(), scalingFactor, scalingFactor);

            std::vector<cv::Point2f> cornerPointsBuf, cornerPointsBufSecondary;

            bool found = false, foundSec = false;

            switch(usedPattern) {
                case CHESSBOARD:
                    try {
                        found = findChessboardCorners(downscaledImg, boardSize, cornerPointsBuf, cv::CALIB_CB_FAST_CHECK);
                    } catch(...) {
                        std::cout<<"Error in finding chessboard in main camera image."<<std::endl;
                        found = false;
                    }
                    try {
                        foundSec = findChessboardCorners(downscaledImgSecondary, boardSize, cornerPointsBufSecondary, cv::CALIB_CB_FAST_CHECK);
                    } catch(...) {
                        std::cout<<"Error in finding chessboard in secondary camera image."<<std::endl;
                        foundSec = false;
                    }
                    break;

                case CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf );
                    foundSec = findCirclesGrid(imgSecondary, boardSize, downscaledImgSecondary );
                    break;

                case ASYMMETRIC_CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf, cv::CALIB_CB_ASYMMETRIC_GRID );
                    foundSec = findCirclesGrid(imgSecondary, boardSize, downscaledImgSecondary, cv::CALIB_CB_ASYMMETRIC_GRID );
                    break;

                default:
                    found = false;
                    foundSec = false;
                    break;
            }

            if(found && foundSec) {
                bool lowquality = false;

                // Improve the found corners' coordinate accuracy for chessboard
                if(usedPattern == CHESSBOARD) {
                    assert(cornerPointsBuf.size() == cornerPointsBufSecondary.size());
                    for(int i=0; i<cornerPointsBuf.size(); i++) {
                        cornerPointsBuf[i] *= (1.0f/scalingFactor);
                        cornerPointsBufSecondary[i] *= (1.0f/scalingFactor);
                    }

                    cv::Mat grayImg, grayImgSecondary;
                    if(img.channels() > 1) {
                        cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
                        cvtColor(imgSecondary, grayImgSecondary, cv::COLOR_BGR2GRAY);
                    }
                    cv::cornerSubPix(grayImg, cornerPointsBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));
                    cv::cornerSubPix(grayImgSecondary, cornerPointsBufSecondary, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));

                    cv::Scalar patternEstimate = cv::estimateChessboardSharpness(grayImg, boardSize, cornerPointsBuf);
                    cv::Scalar patternEstimateSec = cv::estimateChessboardSharpness(grayImgSecondary, boardSize, cornerPointsBufSecondary);

                    std::cout<<"Avg. Sharpness"<<patternEstimate[0]<<", Avg. min. brightness"<<patternEstimate[1]<<", average max. brightness"<<patternEstimate[3]<<std::endl;
                    std::cout<<"Sec: Avg. Sharpness"<<patternEstimateSec[0]<<", Avg. min. brightness"<<patternEstimateSec[1]<<", average max. brightness"<<patternEstimateSec[3]<<std::endl;

                    if(patternEstimate[0] > sharpnessThreshold && patternEstimateSec[0] > sharpnessThreshold) {
                        lowquality = true;
                    }
                    cv::putText(img, "SHARPNESS: " + std::to_string(patternEstimate[0]) + "px", cv::Point(0.1*img.cols, 0.9*img.rows), cv::FONT_HERSHEY_PLAIN, 4, lowquality ? cv::Scalar(0,0,255): cv::Scalar(255,0,0), 3);
                    cv::putText(imgSecondary, "SHARPNESS: " + std::to_string(patternEstimateSec[0]) + "px", cv::Point(0.1*imgSecondary.cols, 0.9*imgSecondary.rows), cv::FONT_HERSHEY_PLAIN, 4, lowquality ? cv::Scalar(0,0,255): cv::Scalar(255,0,0), 3);
                }

                // Draw the corners.
                cv::putText(img, std::to_string(captureCount)+"/"+ std::to_string(maxCaptures) + " (Stereo)", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
                cv::drawChessboardCorners(img, boardSize, cv::Mat(cornerPointsBuf), found);
                cv::putText(imgSecondary, std::to_string(captureCount)+"/"+ std::to_string(maxCaptures) + " (Stereo)", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
                cv::drawChessboardCorners(imgSecondary, boardSize, cv::Mat(cornerPointsBufSecondary), foundSec);

                if(!lowquality) {
                    imagePoints.push_back(cornerPointsBuf);
                    imagePointsSecondary.push_back(cornerPointsBufSecondary);

                    captureCount++;
                }
            } else {
                cv::putText(img, "Pattern not found (Stereo)", cv::Point(0.25*img.cols, 0.25*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);
                cv::putText(imgSecondary, "Pattern not found (Stereo)", cv::Point(0.25*img.cols, 0.25*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);
            }

            emit processedImageLowFPS(mimg);
        }
    } else if(mode==CAPTURING) {

        cv::putText(img, "CALIBRATING", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
        cv::putText(imgSecondary, "CALIBRATING", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);

        emit processedImageLowFPS(mimg);

        calibrationSuccess = QtConcurrent::run(this, &StereoCameraCalibration::calibrate);
        //calibrationSuccess.waitForFinished();
        mode = CALIBRATING;
    } else if(mode==CALIBRATING) {

        cv::putText(img, "CALIBRATING", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
        cv::putText(imgSecondary, "CALIBRATING", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);

        emit processedImageLowFPS(mimg);

        if(calibrationSuccess.isFinished() && calibrationSuccess.result()) {
            // Emit data
            mode = CALIBRATED;

            newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0);
            newCameraMatrixSecondary = getOptimalNewCameraMatrix(cameraMatrixSecondary, distCoeffsSecondary, imageSize, 1, imageSize, 0);

            cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, rectificationTransform, projectionMatrix, imageSize, CV_32F, lmapx, lmapy);
            cv::initUndistortRectifyMap(cameraMatrixSecondary, distCoeffsSecondary, rectificationTransformSecondary, projectionMatrixSecondary, imageSize, CV_32F, rmapx, rmapy);

            emit finishedCalibration();
        } else if(calibrationSuccess.isFinished() && !calibrationSuccess.result()) {
            // repeat process
            mode = CAPTURING;
            captureCount = 0;
            imagePoints.clear();
            imagePointsSecondary.clear();
        }
    } else if(mode==CALIBRATED) {
        if(timer.elapsed() > updateDelay && !img.empty()) {
            timer.restart();

            cv::Mat undist, undistSec;

            cv::remap(img, undist, lmapx, lmapy, cv::INTER_LINEAR);
            cv::remap(imgSecondary, undistSec, rmapx, rmapy, cv::INTER_LINEAR);

            cv::putText(undist, "RECTIFIED", cv::Point(0.1*undist.cols, 0.1*undist.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
            cv::putText(undist, "MAIN RMSE: " + std::to_string(intrinsicRMSE) + "px", cv::Point(0.1 * undist.cols, 0.2 * undist.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            cv::putText(undist, "AVG. MAE: " + std::to_string(avgMAE) + "px", cv::Point(0.1 * undist.cols, 0.3 * undist.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            cv::putText(undist, "STEREO RMSE: " + std::to_string(stereoRMSE) + "px", cv::Point(0.1 * undist.cols, 0.8 * undist.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            cv::putText(undist, "AVG. DISTANCE: "+ std::to_string(avgWorldMAE), cv::Point(0.1 * undist.cols, 0.9 * undist.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            mimg.img = undist;

            cv::putText(undistSec, "RECTIFIED", cv::Point(0.1*undistSec.cols, 0.1*undistSec.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
            cv::putText(undistSec, "SEC. RMSE: " + std::to_string(intrinsicRMSESec) + "px (Avg. " + std::to_string(avgMAESec) + ")", cv::Point(0.1 * undistSec.cols, 0.2 * undistSec.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            cv::putText(undistSec, "AVG. MAE: " + std::to_string(avgMAESec) + "px", cv::Point(0.1 * undist.cols, 0.3 * undist.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 0, 255), 3);
            mimg.imgSecondary = undistSec;

            emit processedImageLowFPS(mimg);
        }
    } else if(mode==VERIFYING) {

        // TODO check if updatedelay is too fast
        if(timer.elapsed() > updateDelay && !img.empty()) {
            timer.restart();

            // Check if verifying imagesize is same as saved one
            assert(imageSize.width == img.cols && imageSize.height == img.rows);

            cv::Mat downscaledImg, downscaledImgSecondary;
            cv::resize(img, downscaledImg, cv::Size(), scalingFactor, scalingFactor);
            cv::resize(imgSecondary, downscaledImgSecondary, cv::Size(), scalingFactor, scalingFactor);

            std::vector<std::vector<cv::Point2f>> verifyImagePoints, verifyImagePointsSecondary;
            std::vector<cv::Point2f> cornerPointsBuf, cornerPointsBufSecondary;

            bool found = false, foundSec = false;

            switch(usedPattern) {
                case CHESSBOARD:
                    try {
                        found = findChessboardCorners(downscaledImg, boardSize, cornerPointsBuf, cv::CALIB_CB_FAST_CHECK);
                    } catch(...) {
                        std::cout<<"Error in finding chessboard in main camera image."<<std::endl;
                        found = false;
                    }
                    try {
                        foundSec = findChessboardCorners(downscaledImgSecondary, boardSize, cornerPointsBufSecondary, cv::CALIB_CB_FAST_CHECK);
                    } catch(...) {
                        std::cout<<"Error in finding chessboard in secondary camera image."<<std::endl;
                        foundSec = false;
                    }
                    break;

                case CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf );
                    foundSec = findCirclesGrid(imgSecondary, boardSize, downscaledImgSecondary );
                    break;

                case ASYMMETRIC_CIRCLES_GRID:
                    found = findCirclesGrid(img, boardSize, cornerPointsBuf, cv::CALIB_CB_ASYMMETRIC_GRID );
                    foundSec = findCirclesGrid(imgSecondary, boardSize, downscaledImgSecondary, cv::CALIB_CB_ASYMMETRIC_GRID );
                    break;

                default:
                    found = false;
                    foundSec = false;
                    break;
            }

            if(found && foundSec) {
                bool lowquality = false;

                // Improve the found corners' coordinate accuracy for chessboard
                if(usedPattern == CHESSBOARD) {
                    assert(cornerPointsBuf.size() == cornerPointsBufSecondary.size());
                    for(int i=0; i<cornerPointsBuf.size(); i++) {
                        cornerPointsBuf[i] *= (1.0f/scalingFactor);
                        cornerPointsBufSecondary[i] *= (1.0f/scalingFactor);
                    }

                    cv::Mat grayImg, grayImgSecondary;
                    if(img.channels() > 1) {
                        cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
                        cvtColor(imgSecondary, grayImgSecondary, cv::COLOR_BGR2GRAY);
                    }
                    cv::cornerSubPix(grayImg, cornerPointsBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));
                    cv::cornerSubPix(grayImgSecondary, cornerPointsBufSecondary, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));

                    cv::Scalar patternEstimate = cv::estimateChessboardSharpness(grayImg, boardSize, cornerPointsBuf);
                    cv::Scalar patternEstimateSec = cv::estimateChessboardSharpness(grayImgSecondary, boardSize, cornerPointsBufSecondary);

                    std::cout<<"Avg. Sharpness"<<patternEstimate[0]<<", Avg. min. brightness"<<patternEstimate[1]<<", average max. brightness"<<patternEstimate[3]<<std::endl;
                    std::cout<<"Sec: Avg. Sharpness"<<patternEstimateSec[0]<<", Avg. min. brightness"<<patternEstimateSec[1]<<", average max. brightness"<<patternEstimateSec[3]<<std::endl;

                    if(patternEstimate[0] > sharpnessThreshold && patternEstimateSec[0] > sharpnessThreshold) {
                        lowquality = true;
                    }
                    cv::putText(img, "SHARPNESS: " + std::to_string(patternEstimate[0]) + "px", cv::Point(0.1*img.cols, 0.8*img.rows), cv::FONT_HERSHEY_PLAIN, 4, lowquality ? cv::Scalar(0,0,255): cv::Scalar(255,0,0), 3);
                    cv::putText(imgSecondary, "SHARPNESS: " + std::to_string(patternEstimateSec[0]) + "px", cv::Point(0.1*imgSecondary.cols, 0.8*imgSecondary.rows), cv::FONT_HERSHEY_PLAIN, 4, lowquality ? cv::Scalar(0,0,255): cv::Scalar(255,0,0), 3);
                }

                // Draw the corners.
                cv::drawChessboardCorners(img, boardSize, cv::Mat(cornerPointsBuf), found);
                cv::drawChessboardCorners(imgSecondary, boardSize, cv::Mat(cornerPointsBufSecondary), foundSec);

                if(!lowquality) {
                    verifyImagePoints.push_back(cornerPointsBuf);
                    verifyImagePointsSecondary.push_back(cornerPointsBufSecondary);

                    referenceObjectPoints.resize(verifyImagePoints.size(), referenceObjectPoints[0]);

                    std::vector<float> verifiedAvgMAE = verifyReprojectionErrors(referenceObjectPoints, verifyImagePoints, cameraMatrix, distCoeffs);
                    std::vector<float> verifiedAvgMAESec = verifyReprojectionErrors(referenceObjectPoints, verifyImagePointsSecondary, cameraMatrixSecondary, distCoeffsSecondary);

                    //std::cout << "Verifying Average Re-projection error of main camera: "<< verifiedAvgRMSE[0] << std::endl;
                    //std::cout << "Verifying Average Re-projection error of secondary camera: "<< verifiedAvgRMSSec[0] << std::endl;

                    std::vector<cv::Point3f> worldPoints = convertPointsTo3D(cornerPointsBuf, cornerPointsBufSecondary);

                    std::vector<double> distances;
                    for(int i=0; i<worldPoints.size()-1;) {
                        double dist = cv::norm(worldPoints[i]-worldPoints[i+1]);
                        distances.push_back(dist);
                        if(i%boardSize.width==boardSize.width-2) {
                            i+=2;
                        } else {
                            i++;
                        }
                        //projectionErrorHistory.push_back(std::make_pair((i * 1000) + cimg.frameNumber, dist));
                        verifyHistory.push_back(std::make_tuple(verifyFrameNumber, cimg.timestamp, fabs(dist - squareSize)));
                    }
                    double avgDistance = std::accumulate(distances.begin(), distances.end(), 0.0) / distances.size();
                    verifyFrameNumber++;

                    cv::putText(img, "VERIFY", cv::Point(0.1*img.cols, 0.1*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
                    cv::putText(img, "MAIN MAE: "+ std::to_string(verifiedAvgMAE[0]) + "px", cv::Point(0.1*img.cols, 0.2*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);

                    cv::putText(imgSecondary, "VERIFY", cv::Point(0.1*imgSecondary.cols, 0.1*imgSecondary.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
                    cv::putText(imgSecondary, "SEC. MAE: "+ std::to_string(verifiedAvgMAESec[0]) + "px", cv::Point(0.1*imgSecondary.cols, 0.2*imgSecondary.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);

                    cv::putText(img, "AVG. DISTANCE: "+ std::to_string(avgDistance) + "mm", cv::Point(0.1*img.cols, 0.9*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);
                }

            } else {
                cv::putText(img, "Pattern not found (Stereo)", cv::Point(0.25*img.cols, 0.25*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);
                cv::putText(imgSecondary, "Pattern not found (Stereo)", cv::Point(0.25*img.cols, 0.25*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);
            }

            emit processedImageLowFPS(mimg);
        }
    } else {
        if(timer.elapsed() > updateDelay && !img.empty()) {
            timer.restart();

            emit processedImageLowFPS(mimg);
        }
    }
}

// Performs the actual camera calibration using OpenCV calibration routines and the captured calibration pattern points
// First performs single calibrations for both cameras, then stereo calibration together
bool StereoCameraCalibration::calibrate() {
    QMutexLocker locker(&mutex);

    referenceObjectPoints.resize(imagePoints.size(), referenceObjectPoints[0]);

    std::vector<cv::Mat> rvecs, rvecsSec;
    std::vector<cv::Mat> tvecs, tvecsSec;

    // Find intrinsic camera parameters
    intrinsicRMSE = cv::calibrateCamera(referenceObjectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, cv::CALIB_FIX_K3 + cv::CALIB_FIX_K4 + cv::CALIB_FIX_K5 + cv::CALIB_FIX_K6);
    intrinsicRMSESec = cv::calibrateCamera(referenceObjectPoints, imagePointsSecondary, imageSize, cameraMatrixSecondary, distCoeffsSecondary, rvecsSec, tvecsSec, cv::CALIB_FIX_K3 + cv::CALIB_FIX_K4 + cv::CALIB_FIX_K5 + cv::CALIB_FIX_K6);

    std::cout << "Re-projection error reported by single calibration of Main camera: " << intrinsicRMSE << std::endl;
    std::cout << "Re-projection error reported by single calibration of Secondary camera: " << intrinsicRMSESec << std::endl;

    bool ok = cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs);
    bool okSec = cv::checkRange(cameraMatrixSecondary) && cv::checkRange(distCoeffsSecondary);

    std::vector<float> reprojectionRMS = reprojectionErrors(referenceObjectPoints, imagePoints, rvecs, tvecs, cameraMatrix, distCoeffs);
    std::vector<float> reprojectionRMSSec = reprojectionErrors(referenceObjectPoints, imagePointsSecondary, rvecsSec, tvecsSec, cameraMatrixSecondary, distCoeffsSecondary);

    reprojectionPointsMAE = reprojectionRMS;
    reprojectionPointsMAESec = reprojectionRMSSec;

    avgMAE = std::accumulate(reprojectionRMS.begin(), reprojectionRMS.end(), 0.0) / reprojectionRMS.size();
    avgMAESec = std::accumulate(reprojectionRMSSec.begin(), reprojectionRMSSec.end(), 0.0) / reprojectionRMSSec.size();

    std::vector<double> diff(reprojectionRMS.size());
    std::transform(reprojectionRMS.begin(), reprojectionRMS.end(), diff.begin(), [this](double x) { return x - avgMAE; });
    double sqSum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdDev = std::sqrt(sqSum / reprojectionRMS.size());

    std::transform(reprojectionRMSSec.begin(), reprojectionRMSSec.end(), diff.begin(), [this](double x) { return x - avgMAE; });
    double sqSumSec = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdDevSec = std::sqrt(sqSumSec / reprojectionRMSSec.size());

    std::cout << "Average Re-projection error of single calibration Main camera: " << avgMAE << " [" << stdDev << "]" << std::endl;
    std::cout << "Average Re-projection error of single calibration Secondary camera: " << avgMAESec << " [" << stdDevSec << "]" << std::endl;

    std::cout << "Stereo calibrating... " << std::endl;

    stereoRMSE = cv::stereoCalibrate(referenceObjectPoints, imagePoints, imagePointsSecondary,
                                     cameraMatrix, distCoeffs,
                                     cameraMatrixSecondary, distCoeffsSecondary,
                                     imageSize, rotationMatrix, translationMatrix, essentialMatrix, fundamentalMatrix,
                                 cv::CALIB_USE_INTRINSIC_GUESS + cv::CALIB_FIX_K3 + cv::CALIB_FIX_K4 + cv::CALIB_FIX_K5 + cv::CALIB_FIX_K6,
                                     cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5) );

    std::cout << "Stereo calibrating finished with intrinsicRMSE: " << stereoRMSE << std::endl;

    avgEpipolarMAE = avgEpipolarError();
    std::cout << "Average Epipolar Error: " << avgEpipolarMAE << std::endl;
    std::cout<<std::endl;

    // Rectification is only needed for the projection matrices output
    cv::stereoRectify(cameraMatrix, distCoeffs, cameraMatrixSecondary, distCoeffsSecondary, imageSize, rotationMatrix, translationMatrix, rectificationTransform, rectificationTransformSecondary, projectionMatrix, projectionMatrixSecondary, disparityDepthMatrix, 0, -1);

    std::cout << "rotationMatrix: "<< std::endl << rotationMatrix<< std::endl;
    std::cout << "translationMatrix: "<< std::endl << translationMatrix<< std::endl;
    std::cout << "projectionMatrix: "<< std::endl << projectionMatrix<< std::endl;

    //projectionErrorHistory.clear();

    reprojectionWorldPointsMAE = reprojectionWorldErrors(imagePoints, imagePointsSecondary);

    avgWorldMAE = std::accumulate(reprojectionWorldPointsMAE.begin(), reprojectionWorldPointsMAE.end(), 0.0) / reprojectionWorldPointsMAE.size();
    std::cout << "Average World MAE: " << avgWorldMAE << std::endl;

    // Debug only, save detailed error values as file
    //auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    //writeVectorCSV(projectionErrorHistory, "timestamp,runtime[ms]", "stereocameracalibration_" + std::to_string(timestamp) + "_worldreprojectiondistances.csv");

    return ok && okSec;
}

// Transforms points in both camera images to real world coordinates
std::vector<cv::Point3f> StereoCameraCalibration::convertPointsTo3D(const std::vector<cv::Point2f> &distortedPoints, const std::vector<cv::Point2f> &distortedPointsSecondary) {

    std::vector<cv::Point2f> undistCornerPointsBuf, undistCornerPointsBufSecondary;

    cv::undistortPoints(cv::Mat(distortedPoints), undistCornerPointsBuf, cameraMatrix, distCoeffs,
                        rectificationTransform, projectionMatrix);
    cv::undistortPoints(cv::Mat(distortedPointsSecondary), undistCornerPointsBufSecondary, cameraMatrixSecondary,
                        distCoeffsSecondary, rectificationTransformSecondary,
                        projectionMatrixSecondary);

    cv::Mat homogenPoints(4, static_cast<int>(distortedPoints.size()), CV_32F);

    cv::triangulatePoints(projectionMatrix, projectionMatrixSecondary, undistCornerPointsBuf,
                          undistCornerPointsBufSecondary, homogenPoints);

    std::vector<cv::Point3f> worldPoints(distortedPoints.size());

    // Output of triangulate points are in homogenous coordinates, convert to cartesian
    for (int i = 0; i < homogenPoints.cols; i++) {
        float scale = homogenPoints.at<float>(3, i) != 0.f ? homogenPoints.at<float>(3, i) : 1.f;
        worldPoints.at(i).x = homogenPoints.at<float>(0, i) / scale;
        worldPoints.at(i).y = homogenPoints.at<float>(1, i) / scale;
        worldPoints.at(i).z = homogenPoints.at<float>(2, i) / scale;
    }

    return worldPoints;
}

// Undistort pupil detections sizes based on the undistortion of pupil contour points
// This only undistorts pixel values, not physical measures which are already undistorted by design
std::pair<double, double> StereoCameraCalibration::undistortPupilDiameters(const Pupil &pupil, const Pupil &pupilSecondary) {
    // Undistorting using remap should be faster than using the undistort function
    if(mode!=CALIBRATED)
        return std::make_pair(pupil.diameter(), pupilSecondary.diameter());

    Pupil rotPupil(pupil);
    rotPupil.angle += 360-rotPupil.angle;
    Pupil rotPupilSecondary(pupilSecondary);
    rotPupilSecondary.angle += 360-rotPupilSecondary.angle;

    // convert pupil detection from pixel into mm through stereocalibration
    // Select the top left and top right corner of the bounding rects of the pupils as the points we triangulate
    cv::Point2f mainPointsArr[4]; // rotatedRect points in order: bottomLeft, topLeft, topRight, bottomRight
    rotPupil.points(mainPointsArr);
    int secondPoint = rotPupil.size.width > rotPupil.size.height ? 2 : 0; // if the pupil major axis is horizontal use topLeft and topRight, else topLeft and bottomLeft
    //cv::line(mimg.img,mainPointsArr[1], mainPointsArr[secondPoint], cv::Scalar(255, 0, 0));

    cv::Point2f secondaryPointsArr[4];
    rotPupilSecondary.points(secondaryPointsArr);
    //cv::line(mimg.imgSecondary,secondaryPointsArr[1], secondaryPointsArr[secondPoint], cv::Scalar(255, 0, 0));

    std::vector<cv::Point2f> mainPoints{mainPointsArr[1], mainPointsArr[secondPoint]}, secondaryPoints{secondaryPointsArr[1], secondaryPointsArr[secondPoint]};

    std::vector<cv::Point2f> undistCornerPointsBuf, undistCornerPointsBufSecondary;

    cv::undistortPoints(cv::Mat(mainPoints), undistCornerPointsBuf, cameraMatrix, distCoeffs, cv::Mat(), newCameraMatrix);
    cv::undistortPoints(cv::Mat(secondaryPoints), undistCornerPointsBufSecondary, cameraMatrixSecondary, distCoeffsSecondary, cv::Mat(), newCameraMatrixSecondary);

    return std::make_pair(cv::norm(undistCornerPointsBuf[0] - undistCornerPointsBuf[1]), cv::norm(undistCornerPointsBufSecondary[0] - undistCornerPointsBufSecondary[1]));
}

// Reprojection error of the calibration
// This error describe the pixel reprojection errors, for physical measure errors see reprojectionWorldErrors
std::vector<float> StereoCameraCalibration::reprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints, const std::vector<std::vector<cv::Point2f>> &f_imagePoints, std::vector<cv::Mat> m_rvecs, std::vector<cv::Mat> m_tvecs, const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs) {

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

        reprojErrs[i] = err / imagePoints2.size();
    }

    return reprojErrs;
}

// Verifies the reprojection error of the calibration based on a new set of images
std::vector<float> StereoCameraCalibration::verifyReprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints, const std::vector<std::vector<cv::Point2f>> &f_imagePoints, const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs) {

    // Verify works different as we dont have the rotation and translation vectors of each pattern view (rvecs, tvecs)
    // We use solvePNP to get the rvecs, tvecs vectors first
    std::vector<cv::Mat> m_rvecs(objectPoints.size()),  m_tvecs(objectPoints.size());

    for(int i = 0; i < (int) objectPoints.size(); ++i ) {
        cv::solvePnP(objectPoints[i], f_imagePoints[i], m_cameraMatrix, m_distCoeffs, m_rvecs[i], m_tvecs[i]);
    }

    return reprojectionErrors(objectPoints, f_imagePoints, m_rvecs, m_tvecs, m_cameraMatrix, m_distCoeffs);
}

// Computes an reprojection error in physical measures of the calibration
// Based on the known measures of the calibration pattern, an error is calculated using the measured distance between the pattern points through the stereo triangulation
// Example: a known chessboard pattern used for calibration has a square size of 3mm, using the stereo triangulation the system measures a distance between the corner points of the pattern
// of 3.02mm, resulting in a measurement/reprojection error of 0.02mm
// The following calculates an average error over all pattern points in an image
std::vector<float> StereoCameraCalibration::reprojectionWorldErrors(const std::vector<std::vector<cv::Point2f>> &f_imagePoints, const std::vector<std::vector<cv::Point2f>> &f_imagePointsSecondary) {

    std::vector<cv::Point2f> imagePoints2;
    std::vector<float> reprojErrs;

    int i;
    reprojErrs.resize(f_imagePoints.size());

    for( i = 0; i < (int) f_imagePoints.size(); ++i ) {
        std::vector<cv::Point3f> worldPoints = convertPointsTo3D(f_imagePoints[i], f_imagePointsSecondary[i]);

        std::vector<double> distances;
        for(int j=0; j<worldPoints.size()-1;) {
            double dist = cv::norm(worldPoints[j]-worldPoints[j+1]);
            distances.push_back(dist);
            // TODO this function doesnt actually return an error but the measured sizes, we need to calc error to square size
            if(j%boardSize.width==boardSize.width-2) {
                j+=2;
            } else {
                j++;
            }
            //projectionErrorHistory.push_back(std::make_pair((i * 1000) + j, dist));
        }
        reprojErrs[i] = std::accumulate(distances.begin(), distances.end(), 0.0) / distances.size();
    }

    return reprojErrs;
}

double StereoCameraCalibration::avgEpipolarError() {
    // Source: https://github.com/oreillymedia/Learning-OpenCV-3_examples/blob/master/example_19-03.cpp

    // CALIBRATION QUALITY CHECK
    // because the output fundamental matrix implicitly includes all the output information,
    // we can check the quality of calibration using the epipolar geometry constraint: m2^t*F*m1=0

    double err = 0;
    int npoints = 0;
    std::vector<cv::Vec3f> lines, linesSec;

    for(int i = 0; i < imagePoints.size(); i++ ) {
        int npt = (int)imagePoints[i].size();
        cv::Mat imgpt, imgptSec;

        imgpt = cv::Mat(imagePoints[i]);
        imgptSec = cv::Mat(imagePointsSecondary[i]);
        undistortPoints(imgpt, imgpt, cameraMatrix, distCoeffs, cv::Mat(), cameraMatrix);
        undistortPoints(imgptSec, imgptSec, cameraMatrixSecondary, distCoeffsSecondary, cv::Mat(), cameraMatrixSecondary);
        computeCorrespondEpilines(imgpt, 1, fundamentalMatrix, lines);
        computeCorrespondEpilines(imgptSec, 2,fundamentalMatrix, linesSec);

        for(int j = 0; j < npt; j++ ) {
            double errij = fabs(imagePoints[i][j].x*linesSec[j][0] +
                                imagePoints[i][j].y*linesSec[j][1] + linesSec[j][2]) +
                           fabs(imagePointsSecondary[i][j].x*lines[j][0] +
                                        imagePointsSecondary[i][j].y*lines[j][1] + lines[j][2]);
            err += errij;
        }
        npoints += npt;
    }

    return err/npoints;
}

// Saves the calibration to file using OpenCVs own FileStorage interface and format, the file format is XML
void StereoCameraCalibration::saveToFile(QString filename) {
    cv::FileStorage fs(filename.toStdString(), cv::FileStorage::WRITE);

    if (!fs.isOpened()) {
        std::cerr << "CameraCalibration: File failed to open " << filename.toStdString() << std::endl;
        return;
    }

    fs << "boardSize_width"  << boardSize.width
       << "boardSize_height" << boardSize.height
       << "squareSize"      << squareSize
       << "imageSize"   << imageSize
       << "usedPattern" << usedPattern
       << "maxCaptures" << maxCaptures
       << "calibrationDate" << QDate::currentDate().toString().toStdString()

       << "cameraMatrix"   << cameraMatrix
        << "cameraMatrixSecondary"   << cameraMatrixSecondary
        << "distCoeffs"   << distCoeffs
        << "distCoeffsSecondary"   << distCoeffsSecondary

        << "rotationMatrix" << rotationMatrix
        << "translationMatrix" << translationMatrix
        << "essentialMatrix" << essentialMatrix
        << "fundamentalMatrix" << fundamentalMatrix

        << "rectificationTransform" << rectificationTransform
        << "rectificationTransformSecondary" << rectificationTransformSecondary
        << "projectionMatrix" << projectionMatrix
        << "projectionMatrixSecondary" << projectionMatrixSecondary

        << "intrinsicRMSE" << intrinsicRMSE
        << "intrinsicRMSESec" << intrinsicRMSESec

        << "avgMAE" << avgMAE
        << "avgMAESec" << avgMAESec

        << "reprojectionPointsMAE" << reprojectionPointsMAE
        << "reprojectionPointsMAESec" << reprojectionPointsMAESec

        << "avgEpipolarMAE" << avgEpipolarMAE
        << "stereoRMSE" << stereoRMSE
        << "reprojectionWorldPointsMAE" << reprojectionWorldPointsMAE
        << "avgWorldMAE" << avgWorldMAE;
}

// Loads an existing calibration from file (XML) using OpenCVs FileStorage interface
// The loaded should be saved previously using the saveToFile function
void StereoCameraCalibration::loadFromFile(QString filename) {
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

    fs["cameraMatrix"] >> cameraMatrix;
    fs["cameraMatrixSecondary"]   >> cameraMatrixSecondary;
    fs["distCoeffs"]   >> distCoeffs;
    fs["distCoeffsSecondary"]   >> distCoeffsSecondary;

    fs["rotationMatrix"] >> rotationMatrix;
    fs["translationMatrix"]   >> translationMatrix;
    fs["essentialMatrix"]   >> essentialMatrix;
    fs["fundamentalMatrix"]   >> fundamentalMatrix;

    fs["rectificationTransform"] >> rectificationTransform;
    fs["rectificationTransformSecondary"]   >> rectificationTransformSecondary;
    fs["projectionMatrix"]   >> projectionMatrix;
    fs["projectionMatrixSecondary"]   >> projectionMatrixSecondary;

    fs["intrinsicRMSE"] >> intrinsicRMSE;
    fs["intrinsicRMSESec"] >> intrinsicRMSESec;

    fs["avgMAE"] >> avgMAE;
    fs["avgMAESec"] >> avgMAESec;

    fs["reprojectionPointsMAE"] >> reprojectionPointsMAE;
    fs["reprojectionPointsMAESec"] >> reprojectionPointsMAESec;

    fs["avgEpipolarMAE"] >> avgEpipolarMAE;

    fs["stereoRMSE"] >> stereoRMSE;

    fs["reprojectionWorldPointsMAE"] >> reprojectionWorldPointsMAE;
    fs["avgWorldMAE"] >> avgWorldMAE;

    if(cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs)) {

        newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0);
        newCameraMatrixSecondary = getOptimalNewCameraMatrix(cameraMatrixSecondary, distCoeffsSecondary, imageSize, 1, imageSize, 0);

        cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, rectificationTransform, projectionMatrix, imageSize, CV_32F, lmapx, lmapy);
        cv::initUndistortRectifyMap(cameraMatrixSecondary, distCoeffsSecondary, rectificationTransformSecondary, projectionMatrixSecondary, imageSize, CV_32F, rmapx, rmapy);

        mode = CALIBRATED;
        emit finishedCalibration();
    }
}

template <typename T> void StereoCameraCalibration::writeVectorCSV(std::vector<std::tuple<int, uint64_t , T>> data, const std::string &header, const std::string &filename) {

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