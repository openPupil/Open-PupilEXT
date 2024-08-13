#pragma once

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>

/**
    Header-only class to calculate the framerate in frames per second for general objects

    Difference to the CameraFrameRateCounter is that the calculated fps are not based on image timestamps but can be anything arbitrary using the signals time

    count(): based on number of calls of this function, framerate is calculated
*/
class FrameRateCounter : public QObject {
Q_OBJECT

public:

    FrameRateCounter(QObject *parent=nullptr): QObject(parent), totalFrameCount(0), secFrameCount(0), fps_(0.0)	{
        connect(&timeout_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }

    ~FrameRateCounter() = default;

    double fps() {
        return fps_;
    }

    int frameCount() {
        return totalFrameCount;
    }

    void reset() {
        fps_ = 0;
        totalFrameCount = 0;
        secFrameCount = 0;
        m_timer.restart();
        timeout_timer.start(4000); // timeouts every 4 seconds

        emit fps(fps_);
        emit framecount(totalFrameCount);
    }

public slots:

    // Slot that is connected to an arbitrary signal which is supposed to be counted
    void count() {
        // Start counting the time so that at each second, a fps value is output
        // The timeout time is needed as if the count slot is not called due to no signals being received, the framerate will not update
        if(totalFrameCount == 0) {
            m_timer.start();
            timeout_timer.start(4000);
            emit fps(fps_);
            emit framecount(totalFrameCount);
        }

        // When a second is over, calculate the (average) fps value based on the average frames received in that timeinterval
        if(m_timer.elapsed() > 999) {
            fps_ =  float(secFrameCount) / (m_timer.elapsed() / 1000.0f);
            m_timer.restart();
            timeout_timer.start(4000);
            secFrameCount = 0;
            emit fps(fps_);
            emit framecount(totalFrameCount);
        }

        // Increase the signals received in the one second interval
        totalFrameCount++;
        secFrameCount++;
    }

protected:

    QElapsedTimer m_timer;
    QTimer timeout_timer;

    int totalFrameCount;
    int secFrameCount;

    double fps_;

private:

    FrameRateCounter(const FrameRateCounter& other) = delete;
    FrameRateCounter& operator=(const FrameRateCounter& rhs);

private slots:

    // Slot callback that is called when the timeout runs out
    // Due to no new signals being received, the new framerate is calculated which should go to zero
    void onTimeout() {
        if(m_timer.elapsed() > 999) {
            fps_ =  float(secFrameCount) / (m_timer.elapsed() / 1000.0f);
            m_timer.restart();
            timeout_timer.start(4000);
            secFrameCount = 0;
            emit fps(fps_);
            emit framecount(totalFrameCount);
        }
    };

signals:

    void fps(double fps);
    void framecount(int framecount);

};
