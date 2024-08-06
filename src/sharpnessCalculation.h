#ifndef PUPILEXT_SHARPNESSCALCULATION_H
#define PUPILEXT_SHARPNESSCALCULATION_H

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
    processedImage(): Outputs camera images with drawn pattern and sharpness information
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

    int getUpdateFPS() {
        return updateDelay*1000;
    }

    void setUpdateFPS(int fps) {
        updateDelay = 1000/fps;
    }

private:

    cv::Size boardSize;

    QElapsedTimer timer;
    int updateDelay;
    double scalingFactor;

public slots:

    void onNewImage(const CameraImage &cimg);

signals:

    void processedImage(CameraImage image);

};


#endif //PUPILEXT_SHARPNESSCALCULATION_H
