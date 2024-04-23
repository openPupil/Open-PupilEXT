#pragma once

/**
    @author Gábor Bényei
*/

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QByteArray>
#include <QSerialPort>
#include <QTextStream>
#include <QTimer>
#include <QElapsedTimer>

#include <iostream>
#include <QtCore/qfileinfo.h>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QtXml>

#include <vector>
#include "devices/camera.h"
#include "supportFunctions.h"



struct TrialIncrement {
    quint64 timestamp = 0;
    uint trialNumber = 1;
};
struct TemperatureCheck {
    quint64 timestamp = 0;
    std::vector<double> temperatures = {0,0};
};
struct Message {
    quint64 timestamp = 0;
    QString messageString;
};

/**
    
    Purpose of this class is threefold:
        1, Keep track of trial numbering
        2, In case of an opened camera input device (BUFFER mode): 
            it represents temporary buffer to store frame-related info in, without slowing image processing 
            via adding extra data fields to some or each CameraImage instance that is passed down the pupil detection chain.
            Also it can save event vectors into an offline_event_log.csv
        3, When an image directory is opened for reading (STORAGE mode):
            Tries to open an offline_event_log.csv containing trial increment events, etc.
    
*/
class RecEventTracker : public QObject {
    Q_OBJECT

    enum EventReplayMode {
        BUFFER = 0,
        STORAGE = 1
    };

public:

    // buffer mode (used for live camera input)
    explicit RecEventTracker(QObject *parent = 0);

    // storage mode (vectors filled and can be read anytime)
    explicit RecEventTracker(const QString& fileName, QObject *parent = 0);
    bool isStorageReady();
    
    ~RecEventTracker();
    void close();
    void saveOfflineEventLog(uint64 timestampFrom, uint64 timestampTo, const QString& fileName);

    // BUFFER mode only
    uint getLastCommissionedTrialNumber();

    // STORAGE mode only
    bool isReady();
    uint getTrialAtTimestamp(quint64 timestamp);

    // for both modes
    TrialIncrement getTrialIncrement(quint64 timestamp);
    TemperatureCheck getTemperatureCheck(quint64 timestamp);
    //Message getMessage(quint64 timestamp);

public slots:
    // For adding elements to vectors in BUFFER mode (timestamp is updated via image grab handler emitted CameraImages automatically)
    void addTrialIncrement(const quint64 &timestamp);
    void addTemperatureCheck(std::vector<double> d);
    //void updateGrabTimestamp(CameraImage cimg);

    // For filling up vectors in STORAGE mode
    void addTrialIncrement(quint64 timestamp, uint trialNumber);
    void addTemperatureCheck(quint64 timestamp, std::vector<double> d);

    // BUFFER mode only
    void resetBufferTrialCounter(const quint64 &timestamp);

    // for both modes
    void addMessage(const quint64 &timestamp, const QString &str);

private:

    EventReplayMode mode;
    QSettings *applicationSettings;

    // both modes
    std::vector<TrialIncrement> trialIncrements;
    std::vector<TemperatureCheck> temperatureChecks;
    std::vector<Message> messages;
    QChar delim;
    QFile *dataFile = nullptr;

    quint16 foundEventLogVersion = 1;
    const quint16 currentEventLogVersion = 1;

    // BUFFER mode only
    //quint64 currentGrabTimestamp = 0;
    //QElapsedTimer timeSinceLastGrab; // for registering "sub-frame" time resolution of events
    uint bufferTrialCounter = 1;

    // STORAGE mode only
    bool storageReady = false;

    QChar determineDelimiter(QString _text);

};
