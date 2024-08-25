#pragma once

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <QtCore/QElapsedTimer>
#include <opencv2/core/hal/interface.h>
#include "devices/camera.h"


/**
    Header-only class to calculate the framerate in frames per second for a camera given images with timestamps.

    Difference to the universal FrameRateCounter is that this class is specifically for camera images, using the images timestamps for
    frame rate calculates rather than the signal time.

    count(): called for each image, framerate is calculated based on the timestamps of the images
*/
class CameraFrameRateCounter : public QObject {
Q_OBJECT

public:

    CameraFrameRateCounter(QObject *parent=nullptr): QObject(parent), totalFrameCount(0), secFrameCount(0), fps_(0.0), lastTimestamp(0), cumDiff(0) {
        connect(&timeout_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }

    ~CameraFrameRateCounter() = default;

    double fps() {
        return fps_;
    }

    size_t frameCount() {
        return totalFrameCount;
    }

    void reset() {
        fps_ = 0;
        totalFrameCount = 0;
        secFrameCount = 0;
        lastTimestamp = 0;
        cumDiff = 0;
        m_timer.restart();
        timeout_timer.start(4000); // timeouts every 4 seconds, if for 4 seconds no new images are received, the timeout callback is executed

        emit fps(fps_);
        emit framecount(totalFrameCount);
    }

public slots:

    // Slot that is connected to the image event of a camera
    void count(CameraImage image) {
        // Start counting the time so that at each second, a fps value is output
        // The timeout time is needed as if the count slot is not called due to no images being received, the framerate will not update
        if(totalFrameCount == 0) {
            m_timer.start();
            timeout_timer.start(4000);
            emit fps(fps_);
            emit framecount(totalFrameCount);
        }

        // When a second is over, calculate the (average) fps value based on the average frame difference of the images
        if(m_timer.elapsed() > 999) {
            // At each elapsed second, calculate the fps given the current secFrameCount (images of the last second)
            if(cumDiff==0 || secFrameCount==0) {
                fps_ = 0.0;
            } else {
                fps_ = 1000.0 / ((double) cumDiff / secFrameCount);
            }            m_timer.restart();
            timeout_timer.start(4000);
            secFrameCount = 0;
            cumDiff = 0;
            emit fps(fps_);
            emit framecount(totalFrameCount);
        }

        cumDiff += image.timestamp - lastTimestamp; // Store the frame timestamp distances to calculate the average framerate per second
        lastTimestamp = image.timestamp;
        totalFrameCount++;
        secFrameCount++;
    }

protected:

    QElapsedTimer m_timer;
    QTimer timeout_timer;

    int totalFrameCount;
    int secFrameCount;
    uint64 lastTimestamp;
    uint64 cumDiff;

    double fps_;

private:

    CameraFrameRateCounter(const CameraFrameRateCounter& other) = delete;
    CameraFrameRateCounter& operator=(const CameraFrameRateCounter& rhs);

private slots:

    // Slot callback that is called when the timeout runs out
    // Due to no new frames being received, the new framerate is calculated which should go to zero
    void onTimeout() {
        if(m_timer.elapsed() > 999) {
            if(cumDiff==0 || secFrameCount==0) {
                fps_ = 0.0;
            } else {
                fps_ =  (double) 1000.0f / (cumDiff / secFrameCount);
            }
            m_timer.restart();
            timeout_timer.start(4000);
            secFrameCount = 0;
            cumDiff = 0;
            emit fps(fps_);
            emit framecount(totalFrameCount);
        }
    };

signals:

    void fps(double fps);
    void framecount(int framecount);

};
