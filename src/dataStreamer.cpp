#include <iostream>
#include "dataStreamer.h"

/**
    @author Gabor Benyei
*/

DataStreamer::DataStreamer(
    ConnPoolCOM *connPoolCOM,
    RecEventTracker *recEventTracker,
    QObject *parent
    ) : 
    QObject(parent),
    connPoolCOM(connPoolCOM),
    recEventTracker(recEventTracker),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent))
{
    delim = applicationSettings->value("delimiterToUse", ",").toString()[0];
    //delim = applicationSettings->value("delimiterToUse", ',').toChar(); // somehow this just doesnt work

}

void DataStreamer::startUDPStreamer(QUdpSocket *socket, QHostAddress ip, quint16 port, DataContainer dataContainer) {
    UDPStreamingOn = true;
    UDPsocket = socket;
    UDPip = ip;
    UDPport = port;
    UDPdataContainer = dataContainer;
    qDebug() << "Now starting UDP streaming to ip: " << ip << " and port " << port << " using data container " << dataContainer;
}

void DataStreamer::startCOMStreamer(int poolIndex, DataContainer dataContainer) {
    connPoolCOMIndex = poolIndex;
    COMdataContainer = dataContainer;
    qDebug() << "Now starting COM streaming on poolIndex" << poolIndex << " of port: " << connPoolCOM->getInstance(poolIndex)->portName() << "using data container " << dataContainer;
}
    
void DataStreamer::stopUDPStreamer() {
    UDPStreamingOn = false;
    UDPsocket = nullptr;
    UDPip = QHostAddress();
    UDPport = 0;
    UDPdataContainer = DataContainer::CSV;
    qDebug() << "Stopping UDP streaming";
}

void DataStreamer::stopCOMStreamer() {
    connPoolCOMIndex = -1;
    COMdataContainer = DataStreamer::CSV;
    qDebug() << "Stopping COM streaming";
}

// On new pupil data, stream it
void DataStreamer::newPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename) {

    // BG: TODO: improve this code
    uint trialNumber = recEventTracker->getTrialIncrement(timestamp).trialNumber;
    std::vector<double> d = recEventTracker->getTemperatureCheck(timestamp).temperatures;

    bool anyUsed = false;
    if( UDPStreamingOn && UDPsocket != nullptr) {
        QString str = "";
        if(UDPdataContainer == DataContainer::CSV)
            str = EyeDataSerializer::pupilToRowCSV(timestamp, procMode, Pupils, filename, trialNumber, delim, d);
        else if(UDPdataContainer == DataContainer::JSON)
            str = EyeDataSerializer::pupilToJSON(timestamp, procMode, Pupils, filename, trialNumber, d);
        else if(UDPdataContainer == DataContainer::XML)
            str = EyeDataSerializer::pupilToXML(timestamp, procMode, Pupils, filename, trialNumber, d);
        else if(UDPdataContainer == DataContainer::YAML)
            str = EyeDataSerializer::pupilToYAML(timestamp, procMode, Pupils, filename, trialNumber, d);

        UDPsocket->writeDatagram( (str + '\n').toUtf8(), UDPip, UDPport );
        anyUsed = true;
    }
    if(connPoolCOMIndex >= 0 && connPoolCOM->getInstance(connPoolCOMIndex) != nullptr) {
        QString str = "";
        if(COMdataContainer == DataContainer::CSV)
            str = EyeDataSerializer::pupilToRowCSV(timestamp, procMode, Pupils, filename, trialNumber, delim, d);
        else if(COMdataContainer == DataContainer::JSON)
            str = EyeDataSerializer::pupilToJSON(timestamp, procMode, Pupils, filename, trialNumber, d);
        else if(COMdataContainer == DataContainer::XML)
            str = EyeDataSerializer::pupilToXML(timestamp, procMode, Pupils, filename, trialNumber, d);
        else if(COMdataContainer == DataContainer::YAML)
            str = EyeDataSerializer::pupilToYAML(timestamp, procMode, Pupils, filename, trialNumber, d);

        connPoolCOM->getInstance(connPoolCOMIndex)->write( (str + '\n').toUtf8() );
        anyUsed = true;
    }

    if(!anyUsed) { // This is just for extra safety
        qDebug() << "Streamers are not in use, stopping all.";
        //emit underlyingConnectionsClosed();
        stopUDPStreamer();
        stopCOMStreamer();
    }
}

int DataStreamer::getNumActiveStreamers() {
    int num = 0;
    if( UDPsocket != nullptr) {
        num++;
    }
    if(connPoolCOMIndex >= 0 && connPoolCOM->getInstance(connPoolCOMIndex) != nullptr) {
        num++;
    }
    return num;
}

DataStreamer::~DataStreamer() {
    close();
}

// Close the files, filestreams, etc
void DataStreamer::close() {
    
    /*
    if(streamingMethod == StreamingMethod::COM && serialPort != nullptr && serialPort->isOpen()) 
        serialPort->close();
    if(streamingMethod == StreamingMethod::UDP && socket != nullptr && socket->isOpen()) 
        socket->close();
    */
}
