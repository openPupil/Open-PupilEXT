#pragma once

/**
    @author Gabor Benyei, Attila Boncser
*/

#include <QtCore/QObject>
#include <QByteArray>
#include <QTimer>
#include <QSettings>
#include <QCoreApplication>
#include <QThread>
//#include "devices/Camera.h"
#include "devices/singleCamera.h"
#include "devices/stereoCamera.h"
#include "supportFunctions.h"
#include "recEventTracker.h"

Q_DECLARE_METATYPE(std::vector<double>)

/**
    An instance of this class is supposed to be running only when/while a Basler camera 
    (whose temperature can be queried) is open, which is obviously a real camera device,
    thus a recEventTracker instance also must exist
*/

class CamTempMonitor : public QObject {

    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

    const int checkIntervalSec = 5;
    const int warmupStableDeltaTime = 60;
    const double warmupStableDeltaTemp = 0.5;
 
    bool m_running;
    Camera *camera;
    bool stereo;
    std::vector<std::vector<double>> tempChecks;
 
public:

    // The temperature that we deem as default value when cannot be measured, and the minimum that can be measured
    // Note: not yet used in recEventTracker (problem accessing a const static value in a nonstatic class, perhaps some include problem?)
    static constexpr double MINIMUM_DEVICE_TEMPERATURE = -1.0;

    explicit CamTempMonitor(Camera *camera);
    bool running() const;
 
signals:
    void finished();
    void runningChanged(bool running);
    void camTempChecked(std::vector<double> temperatures);
    void cameraWarmupHasDeltaTimeData();
    void cameraWarmedUp();
 
public slots:
    void run();
    void setRunning(bool running);
    
};


