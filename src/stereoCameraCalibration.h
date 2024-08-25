#pragma once

/**
    @author Moritz Lode
*/

#include <QtCore/QThread>
#include <opencv2/core/mat.hpp>
#include <QtCore/QElapsedTimer>
#include "devices/camera.h"
#include "cameraCalibration.h"
#include <QtConcurrent/QtConcurrent>

/**
    Object that handles and conducts stereo camera calibration, should be executed in another thread than the GUI thread

    Calibration can be load and saved from/to file

    Calibration is conducted as follows: onNewImage receives new images from the camera, depending on the current state of the camera calibration defined in
    CalibrationMode, different actions are taken i.e. capturing, calibration, verification

slots:
    onNewImage(): contains main functions of the calibration upon receiving a new camera image
    startCapturing(): starts capturing pattern point collection for calibration upon completion calibration is executed
    startVerifying(): starts verification of a finished calibration
    stop(): stops any current running calibration process, or verification

signals:
    processedImageLowFPS(): outputs the processed image, with calibration information rendered on it
    finishedCalibration(): whenever the calibration is finished
    unavailableCalibration(): only used for reset of the calibration, tells whenever calibration is not available anymore

*/
class StereoCameraCalibration : public QObject {
    Q_OBJECT

public:

    explicit StereoCameraCalibration(QObject *parent = 0);
    ~StereoCameraCalibration() override;

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
        // If already calibrated, the change of pattern renders calibration unvalid
        if(isCalibrated()) {
            reset();
        }
        initReferenceObjectPoints();
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

    int getSharpnessThreshold() {
        return sharpnessThreshold;
    }

    void setSharpnessThreshold(int threshold) {
        sharpnessThreshold = threshold;
    }

    int getCaptureFPS() {
        return captureDelay*1000;
    }

    void setCaptureFPS(int fps) {
        captureDelay = 1000/fps;
    }

    bool isCalibrated() {
        return mode==CalibrationMode::CALIBRATED;
    }

    cv::Mat getCameraMatrix() {
        return cameraMatrix;
    }

    cv::Mat getDistCoefficients() {
        return distCoeffs;
    }

    double getMainRMSE() {
        return intrinsicRMSE;
    }

    double getSecondaryRMSE() {
        return intrinsicRMSESec;
    }

    double getStereoRMSE() {
        return stereoRMSE;
    }

    double getMainMAE() {
        return avgMAE;
    }

    double getSecondaryMAE() {
        return avgMAESec;
    }

    double getWorldMAE() {
        return avgWorldMAE;
    }

    size_t getMaxObservations() {
        return maxCaptures;
    }

    void setMaxObservations(int max) {
        maxCaptures = max;
    }

    size_t getNumberOfObservations() {
        return captureCount;
    }

    cv::Size getImageSize() {
        return imageSize;
    }

    void setVerifyOutputPath(QString path) {
        verifyOutputPath = path;
    }

    QString getVerifyOutputPath() {
        return verifyOutputPath;
    }

    std::vector<cv::Point3f> convertPointsTo3D(const std::vector<cv::Point2f> &distortedPoints,
                                               const std::vector<cv::Point2f> &distortedPointsSecondary);

    std::pair<double, double> undistortPupilDiameters(const Pupil &pupil, const Pupil &pupilSecondary);

private:

    QMutex mutex;

    std::vector<std::vector<cv::Point2f>> imagePoints, imagePointsSecondary;

    cv::Mat cameraMatrix, cameraMatrixSecondary;
    cv::Mat distCoeffs, distCoeffsSecondary;

    cv::Mat rotationMatrix;
    cv::Mat translationMatrix;
    cv::Mat essentialMatrix;
    cv::Mat fundamentalMatrix;
    cv::Mat disparityDepthMatrix;

    cv::Mat rectificationTransform, rectificationTransformSecondary;
    cv::Mat projectionMatrix, projectionMatrixSecondary;
    cv::Mat newCameraMatrix, newCameraMatrixSecondary;

    cv::Mat lmapx, lmapy, rmapx, rmapy;

    std::vector<std::vector<cv::Point3f>> referenceObjectPoints;

    double intrinsicRMSE, intrinsicRMSESec;
    double avgMAE, avgMAESec;
    double stereoRMSE;
    double avgEpipolarMAE;
    double avgWorldMAE;
    int sharpnessThreshold;

    std::vector<float> reprojectionPointsMAE, reprojectionPointsMAESec, reprojectionWorldPointsMAE;

    QFuture<bool> calibrationSuccess;

    std::vector<std::tuple<int, uint64_t, double>> verifyHistory;
    QString verifyOutputPath;
    int verifyFrameNumber;

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

    bool measured = false;

    bool calibrate();
    void initReferenceObjectPoints();

    std::vector<float> reprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints,
                                          const std::vector<std::vector<cv::Point2f>> &f_imagePoints,
                                          std::vector<cv::Mat> m_rvecs, std::vector<cv::Mat> m_tvecs,
                                          const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs);

    std::vector<float> verifyReprojectionErrors(const std::vector<std::vector<cv::Point3f>> &objectPoints,
                                                const std::vector<std::vector<cv::Point2f>> &f_imagePoints,
                                                const cv::Mat &m_cameraMatrix, const  cv::Mat &m_distCoeffs);

    double avgEpipolarError();

    std::vector<float> reprojectionWorldErrors(const std::vector<std::vector<cv::Point2f>> &f_imagePoints,
                                               const std::vector<std::vector<cv::Point2f>> &f_imagePointsSecondary);

    template<typename T> void writeVectorCSV(std::vector<std::tuple<int, uint64_t , T>> data, const std::string &header, const std::string &filename);

public slots:

    void onNewImage(const CameraImage &img);
    void startCapturing();
    void startVerifying();
    void stop();

signals:

    void processedImageLowFPS(CameraImage image);
    void finishedCalibration();
    void unavailableCalibration();

};
