#pragma once

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <QtCore/QElapsedTimer>
#include "devices/camera.h"

/**
    Object that performs the sharpness calculations based on a detected calibration pattern on images.

    Outputs processed images on which the current pattern detection and sharpness value are drawn.
    Currently only supports chessboard pattern due to OpenCV limitations.

    Supports only single camera images by design, as each camera should be evaluated for sharpness separably.

slots:
    onNewImage(): on each new camera image, pattern detection and sharpness estimation is performed

signals:
    processedImageLowFPS(): Outputs camera images with drawn pattern and sharpness information
*/
class SharpnessCalculation : public QObject {
    Q_OBJECT

public:

    explicit SharpnessCalculation(QObject *parent = 0);
    ~SharpnessCalculation() override;

    cv::Size getBoardSize() {
        return boardSize;
    }

    void setBoardSize(cv::Size pboardSize) {
        boardSize = pboardSize;
    }

    void setBoardSize(const int cornersHorizontal, const int cornersVertical) {
        setBoardSize(cv::Size(cornersHorizontal, cornersVertical));
    }

private:

    cv::Size boardSize;

    QElapsedTimer timer;
    int drawDelay;
    double scalingFactor;

public slots:

    void onNewImage(const CameraImage &cimg);

signals:

    void processedImageLowFPS(CameraImage image);

};
