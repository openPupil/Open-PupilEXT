#pragma once

/**
    @author Gábor Bényei
*/

#include <QtCore/QObject>
#include <opencv2/core/mat.hpp>
#include "camera.h"

#include <opencv2/opencv.hpp>
#include <QDateTime>
// #include <chrono>

Q_DECLARE_METATYPE(cv::Mat)

/**
    Image event handler for SingleWebcam
*/
class SingleWebcamImageEventHandler : public QObject {
Q_OBJECT

public:
    explicit SingleWebcamImageEventHandler(QObject* parent=0);
    ~SingleWebcamImageEventHandler() override;

signals:
    void onNewGrabResult(CameraImage grabResult);

public slots:
    void OnImageGrabbed(const cv::Mat &image);

};

