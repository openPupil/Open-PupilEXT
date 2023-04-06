#pragma once

/**
    @author Gábor Bényei
*/

#include <QtCore/QObject>
//#include <QtCore/QFile>
//#include <QtCore/QTextStream>
//
#include <QByteArray>
#include <QSerialPort>
#include <QTextStream>
#include <QTimer>

#include <QSettings>
#include <QCoreApplication>

#include "supportFunctions.h"

#include <QtXml>
#include "pupilDetection.h"

#include "recEventTracker.h"
#include "eyeDataSerializer.h"
#include "connPoolCOM.h"


// TODO: make a kind of HTTP "REST API-like" thing with only a few accepted requests, 
// returning e.g. the last read pupil data? Could be fun :)
/**

    Code made to handle an std::vector of Pupils, in order to comply with new signal-slot strategy, which
    I introduced to manage different pupil detection processing modes (procModes).
    Also it uses EyeDataSerializer class to process every pupil detection output.

*/
class DataStreamer : public QObject {
    Q_OBJECT

public:

    enum DataContainer {CSV = 1, JSON = 2, XML = 3, YAML = 4};

    explicit DataStreamer(
        ConnPoolCOM *connPoolCOM,
        RecEventTracker *recEventTracker,
        QObject *parent
        ); 
    ~DataStreamer() override;
    void close();

    void startUDPStreamer(QUdpSocket *socket, QHostAddress ip, quint16 port, DataContainer dataContainer);
    void startCOMStreamer(int poolIndex, DataContainer dataContainer);
    
    void stopUDPStreamer();
    void stopCOMStreamer();

    int getNumActiveStreamers();

private:

    ConnPoolCOM *connPoolCOM;
    int connPoolCOMIndex = -1;
    
    bool UDPStreamingOn = false;
    QUdpSocket *UDPsocket;
    QHostAddress UDPip;
    quint16 UDPport;

    DataContainer UDPdataContainer;
    DataContainer COMdataContainer;

    QSettings *applicationSettings;
    QChar delim; 
    RecEventTracker *recEventTracker;

public slots:

    void newPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename);

signals:
    //void underlyingConnectionsClosed(); // TODO: use for safety

};



