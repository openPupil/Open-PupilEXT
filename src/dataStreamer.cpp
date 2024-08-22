#include <iostream>
#include "dataStreamer.h"

/**
    @author Gabor Benyei
*/

DataStreamer::DataStreamer(
    ConnPoolCOM *connPoolCOM,
    ConnPoolUDP *connPoolUDP,
    RecEventTracker *recEventTracker,
    QObject *parent
    ) : 
    QObject(parent),
    connPoolCOM(connPoolCOM),
    connPoolUDP(connPoolUDP),
    recEventTracker(recEventTracker),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent))
{
    delim = applicationSettings->value("dataWriterDelimiter", ",").toString()[0];
    //delim = applicationSettings->value("delimiterToUse", ',').toChar(); // somehow this just doesnt work

}

void DataStreamer::startUDPStreamer(int poolIndex, DataContainer dataContainer) {
    connPoolUDPIndex = poolIndex;
    UDPdataContainer = dataContainer;
    qDebug() << "Now starting UDP streaming to ip: " << connPoolUDP->getInstance(poolIndex)->objectName() << " and port " << connPoolUDP->getInstance(poolIndex)->localPort() << " using data container " << dataContainer;
}

void DataStreamer::startCOMStreamer(int poolIndex, DataContainer dataContainer) {
    connPoolCOMIndex = poolIndex;
    COMdataContainer = dataContainer;
    qDebug() << "Now starting COM streaming on poolIndex" << poolIndex << " of port: " << connPoolCOM->getInstance(poolIndex)->portName() << "using data container " << dataContainer;
}
    
void DataStreamer::stopUDPStreamer() {
    connPoolUDPIndex = -1;
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

    _trialNumber = recEventTracker->getTrialIncrement(timestamp).trialNumber;
    _message = recEventTracker->getMessage(timestamp).messageString;
    _d = recEventTracker->getTemperatureCheck(timestamp).temperatures;

    bool anyUsed = false;
    if(connPoolUDPIndex >= 0 && connPoolUDP->getInstance(connPoolUDPIndex) != nullptr) {
    //if( UDPStreamingOn && UDPsocket != nullptr) {
        QString str = "";
        if(UDPdataContainer == DataContainer::CSV)
            str = EyeDataSerializer::pupilToRowCSV(timestamp, procMode, Pupils, filename, _trialNumber, delim, DataWriterDataStyle::PUPILEXT_V0_1_2, _message, _d);
        else if(UDPdataContainer == DataContainer::JSON)
            str = EyeDataSerializer::pupilToJSON(timestamp, procMode, Pupils, filename, _trialNumber, _message, _d);
        else if(UDPdataContainer == DataContainer::XML)
            str = EyeDataSerializer::pupilToXML(timestamp, procMode, Pupils, filename, _trialNumber, _message, _d);
        else if(UDPdataContainer == DataContainer::YAML)
            str = EyeDataSerializer::pupilToYAML(timestamp, procMode, Pupils, filename, _trialNumber, _message, _d);

        connPoolUDP->writeToInstance( connPoolUDPIndex, (str + '\n').toUtf8() );
        anyUsed = true;
    }
    if(connPoolCOMIndex >= 0 && connPoolCOM->getInstance(connPoolCOMIndex) != nullptr) {
        QString str = "";
        if(COMdataContainer == DataContainer::CSV)
            str = EyeDataSerializer::pupilToRowCSV(timestamp, procMode, Pupils, filename, _trialNumber, delim, DataWriterDataStyle::PUPILEXT_V0_1_2, _message, _d);
        else if(COMdataContainer == DataContainer::JSON)
            str = EyeDataSerializer::pupilToJSON(timestamp, procMode, Pupils, filename, _trialNumber, _message, _d);
        else if(COMdataContainer == DataContainer::XML)
            str = EyeDataSerializer::pupilToXML(timestamp, procMode, Pupils, filename, _trialNumber, _message, _d);
        else if(COMdataContainer == DataContainer::YAML)
            str = EyeDataSerializer::pupilToYAML(timestamp, procMode, Pupils, filename, _trialNumber, _message, _d);

        connPoolCOM->writeToInstance( connPoolCOMIndex, (str + '\n').toUtf8() );
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
    if(connPoolUDPIndex >= 0 && connPoolUDP->getInstance(connPoolUDPIndex) != nullptr) {
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
