#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include "sharpnessCalculation.h"

// Creates a new sharpness worker
// This should be run in a separate thread as the pattern detection is slow
SharpnessCalculation::SharpnessCalculation(QObject *parent) : QObject(parent) {

    drawDelay = 33;

    scalingFactor = 0.5; // Scales the image down to increase pattern detection speed, for sharpness calculation, the original resolution is used

    int numCornersHor = 10;
    int numCornersVer = 7;
    boardSize = cv::Size(numCornersHor, numCornersVer);

    timer.start();
}

SharpnessCalculation::~SharpnessCalculation() {

}

// Receives a new image and performs pattern detection on it
// If successful, emits a processed image containing the detected pattern and sharpness value
void SharpnessCalculation::onNewImage(const CameraImage &cimg) {


    if(timer.elapsed() > drawDelay && !cimg.img.empty()) {
        timer.restart();

        // If user changes boardsize in between frames it could result in errors
        cv::Size currentBoardSize = boardSize;
        const CameraImage &mimg = cimg;

        if (cimg.img.channels() == 1) {
            cv::cvtColor(cimg.img, mimg.img, cv::COLOR_GRAY2BGR);
        } else {
            mimg.img = cimg.img.clone();
        }
        cv::Mat img = mimg.img;

        cv::Mat downscaledImg;
        cv::resize(img, downscaledImg, cv::Size(), scalingFactor, scalingFactor);

        std::vector<cv::Point2f> cornerPointsBuf;

        bool found = false;

        try {
            found = cv::findChessboardCorners(downscaledImg, currentBoardSize, cornerPointsBuf, cv::CALIB_CB_FAST_CHECK);
        } catch(...) {
            found = false;
        }

        if(found) {
            // Improve the found corners' coordinate accuracy for chessboard
            for(int i=0; i<cornerPointsBuf.size(); i++) {
                cornerPointsBuf[i] *= (1.0f/scalingFactor);
            }

            cv::Mat imgGray = img;
            if(imgGray.channels() > 1)
                cvtColor(imgGray, imgGray, cv::COLOR_BGR2GRAY);
            cv::cornerSubPix(imgGray, cornerPointsBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ));

            cv::Scalar patternEstimate = cv::estimateChessboardSharpness(imgGray, currentBoardSize, cornerPointsBuf);
            //std::cout<<"Avg. Sharpness: "<<patternEstimate[0]<<", Avg. min. brightness: "<<patternEstimate[1]<<", Avg. max. brightness: "<<patternEstimate[3]<<std::endl;

            cv::putText(img, "AVG. SHARPNESS: " + std::to_string(patternEstimate[0]) + "px", cv::Point(0.1*img.cols, 0.9*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,0,0), 3);
            // Draw the corners.
            drawChessboardCorners(img, currentBoardSize, cv::Mat(cornerPointsBuf), found);
        } else {
            cv::putText(img, "No Chessboard pattern found!", cv::Point(0.25*img.cols, 0.25*img.rows), cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0,0,255), 3);
        }

        emit processedImageLowFPS(mimg);

    }



}
