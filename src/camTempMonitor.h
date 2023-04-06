#pragma once

/**
    @author Gabor Benyei
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
 
    bool m_running;
    Camera *camera;
    bool stereo;
 
public:
    explicit CamTempMonitor(Camera *camera);
    bool running() const;
 
signals:
    void finished();
    void runningChanged(bool running);
    void camTempChecked(std::vector<double> temperatures);
 
public slots:
    void run();
    void setRunning(bool running);
    
};


