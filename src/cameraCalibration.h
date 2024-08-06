
#ifndef PUPILEXT_CAMERACALIBRATION_H
#define PUPILEXT_CAMERACALIBRATION_H

/**
    @author Moritz Lode
*/

#include <QtCore/QThread>
#include <opencv2/core/mat.hpp>
#include <QtCore/QElapsedTimer>
#include <QtCore/QMutex>
#include "devices/camera.h"
#include "pupil-detection-methods/Pupil.h"
#include <QtConcurrent/QtConcurrent>

enum CalibrationPattern {CHESSBOARD = 0, CIRCLES_GRID = 1, ASYMMETRIC_CIRCLES_GRID = 2};
enum CalibrationMode {NONE = 0, CAPTURING = 1, CALIBRATING = 2, CALIBRATED = 3, VERIFYING=4};


/**
    Object that handles and conducts single camera calibration, should be executed in another thread than the GUI thread to not block it

    Calibration can be loaded and saved from/to file

    Calibration is conducted as follows:
    onNewImage receives new images from the camera, depending on the current state of the camera calibration defined in CalibrationMode, different actions
    are taken i.e. capturing images for calibration, calibration, verification

slots:
    onNewImage(): contains main functions of the calibration upon receiving a new camera image
    startCapturing(): starts capturing pattern point collection for calibration upon completion calibration is executed
    startVerifying(): starts verification of a finished calibration
    stop(): stops any current running calibration process, or verification

signals:
    processedImage(): outputs the processed image, with calibration information rendered on it
    finishedCalibration(): whenever the calibration is finished
    unavailableCalibration(): only used for reset of the calibration, tells whenever calibration is not available anymore

*/
class CameraCalibration : public QObject {
    Q_OBJECT

public:

    explicit CameraCalibration(QObject *parent = 0);
    ~CameraCalibration() override;

    void reset();

    void saveToFile(QString filename);
    void loadFromFile(QString filename);

    cv::Size getBoardSize() {
        return boardSize;
    }

    void setBoardSize(cv::Size boardSize) {
        boardSize = boardSize;

        // If already calibrated, the change of board size renders calibration unvalid
        if(isCalibrated()) {
            reset();
        }
        initReferenceObjectPoints();
    }

    void setBoardSize(const int cornersHorizontal, const int cornersVertical) {
        setBoardSize(cv::Size(cornersHorizontal, cornersVertical));
    }

    int getSquareSize() {
        return squareSize;
    }

    void setSquareSize(int m_squareSize) {
        squareSize = m_squareSize;
        // If already calibrated, the change of board size renders calibration unvalid
        if(isCalibrated()) {
            reset();
        }
    }

    int getPattern() {
        return usedPattern;
    }

    void setPattern(int pattern) {
        usedPattern = pattern;
        // If already calibrated, the change of pattern renders calibration unvalid
        if(isCalibrated()) {
            reset();
        }
        initReferenceObjectPoints();
    }

    int getCaptureFPS() {
        return captureDelay*1000;
    }

    void setCaptureFPS(int fps) {
        captureDelay = 1000/fps;
    }

    int getSharpnessThreshold() {
        return sharpnessThreshold;
    }

    void setSharpnessThreshold(int threshold) {
        sharpnessThreshold = threshold;
    }

    int getUpdateFPS() {
        return updateDelay*1000;
    }

    void setUpdateFPS(int fps) {
        updateDelay = 1000/fps;
    }

    int isCalibrated() {
        return mode==CALIBRATED;
    }

    cv::Mat getCameraMatrix() {
        return cameraMatrix;
    }

    cv::Mat getDistCoefficients() {
        return distCoeffs;
    }

    double getRMSE() {
        return intrinsicRMSE;
    }

    double getAvgMAE() {
        return avgMAE;
    }

    int getMaxObservations() {
        return maxCaptures;
    }

    void setMaxObservations(int max) {
        maxCaptures = max;
    }

    int getNumberOfObservations() {
        return captureCount;
    }

    cv::Size getImageSize() {
        return imageSize;
    }

    cv::Mat undistortImage(const cv::Mat &img);

    void setVerifyOutputPath(QString path) {
        verifyOutputPath = path;
    }

    QString getVerifyOutputPath() {
        return verifyOutputPath;
    }

    double undistortPupilDiameter(const Pupil &pupil);

private:

    QMutex mutex;

    std::vector<std::vector<cv::Point2f>> imagePoints;

    std::vector<std::vector<cv::Point3f>> referenceObjectPoints;

    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    cv::Mat newCameraMatrix;

    QFuture<bool> calibrationSuccess;

    cv::Mat undistMap1, undistMap2;

    double intrinsicRMSE;
    double avgMAE;
    int sharpnessThreshold;

    std::vector<float> reprojectionPointsMAE;

    std::vector<std::tuple<int, uint64_t, double>> verifyHistory;
    int verifyFrameNumber;
    QString verifyOutputPath;

    int mode;
    int usedPattern;
    int maxCaptures;
    int captureCount;
    double scalingFactor;

    QElapsedTimer timer;
    int captureDelay;
    int updateDelay;

    cv::Size boardSize;
    int squareSize;

    cv::Size imageSize;
    cv::Rect covered;

    bool calibrate();
    void initReferenceObjectPoints();

    static std::vector<float> reprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints,
                                                const std::vector<std::vector<cv::Point2f>> &f_imagePoints,
                                                std::vector<cv::Mat> m_rvecs, std::vector<cv::Mat> m_tvecs,
                                                const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs);

    std::vector<float> verifyReprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints,
                                                const std::vector<std::vector<cv::Point2f>> &f_imagePoints,
                                                const cv::Mat &m_cameraMatrix, const cv::Mat &m_distCoeffs);

    // Helper functions for persisting calibration error information to csv
    template <typename T> void writeVectorCSV(std::vector<std::tuple<int, uint64_t , T>> data, const std::string &header, const std::string &filename);

public slots:

    void onNewImage(const CameraImage &img);
    void startCapturing();
    void startVerifying();
    void stop();

signals:

    void processedImage(CameraImage image);
    void finishedCalibration();
    void unavailableCalibration();

};


#endif //PUPILEXT_CAMERACALIBRATION_H
