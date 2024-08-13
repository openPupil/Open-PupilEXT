#include <iostream>
#include <QtCore/qfileinfo.h>
#include <QMessageBox>
#include "dataWriter.h"
#include "supportFunctions.h"

// TODO datawriter is receiving pupil signal at the full rate, slowing down the gui thread? move to other thread?
DataWriter::DataWriter(
    const QString& fileName, 
    ProcMode procMode,  
    RecEventTracker *recEventTracker,
    QObject *parent
    ) : 
    QObject(parent),
    recEventTracker(recEventTracker),
    writerReady(false),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    delim = applicationSettings->value("dataWriterDelimiter", ",").toString()[0];
    //delim = applicationSettings->value("delimiterToUse", ',').toChar(); // somehow this just doesnt work

    QString dataStyleStr = applicationSettings->value("dataWriterDataStyle", "PupilEXT-0-1-2").toString();
    if(dataStyleStr == "PupilEXT-0-1-1")
        dataStyle = PUPILEXT_V0_1_1;
    else // if(dataStyleStr == "PupilEXT-0-1-2")
        dataStyle = PUPILEXT_V0_1_2;

    //delim = delimToUse; // only used if dataFormat=='P'
    qDebug() << "New DataWriter object created.";

    // Header definitions of the output file, this must fit the output format in the pupilToRow functions
    header = EyeDataSerializer::getHeaderCSV(procMode, delim, dataStyle);

    qDebug() << fileName;

    /*
    bool pathWriteable = SupportFunctions::preparePath(fileName);
    if(!pathWriteable) {
        QMessageBox MsgBox;
        MsgBox.setText("Recording failure. Could not create path.");
        MsgBox.exec();
    }
    */

    dataFile = new QFile(fileName);
    bool exists = dataFile->exists(); // NOTE: should never happen

    // Open the file in append mode
    // GB: needed to double check, in case of writing to e.g. C:/ or other admin-only folder, 
    // if the exe was started without admin rights, open would fail, and the null-ed dataFile can cause exception later on
    bool fileWriteable = dataFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (!fileWriteable) {
        //std::cout << "Recording failure. Could not open: " << fileName.toStdString() << std::endl;
        delete dataFile;
        dataFile = nullptr;
        QMessageBox MsgBox;
        MsgBox.setText(QString::fromStdString("Recording failure. Could not open: " + fileName.toStdString()));
        MsgBox.exec();
    }

    textStream = new QTextStream(dataFile);

    if(dataFile && textStream)
        writerReady=true;

    // To not write again a header line to the file when it already existed (appending), check it
    if(!exists)
        *textStream << header << Qt::endl;

    //if(!pathWriteable || !fileWriteable)
    if(!fileWriteable)
        this->deleteLater();
}

DataWriter::~DataWriter() {
    close();
}

// Close the file and filestreams
void DataWriter::close() {
    if (dataFile){
        dataFile->close();
        dataFile->deleteLater();
    }
    delete textStream;
    delete dataFile;
    dataFile = nullptr;
    textStream = nullptr;

    std::cout << "DataWriter object deleted." << std::endl;
}

// GB: replacing previous methods for single pupil detection from single or stereo cameras, 
// as well as adding new capability to write data of other processing modes
// On new pupil data, write the pupil detection to file in a new row
void DataWriter::newPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename) {
    if (!textStream)
        return;

    // GB: serialization is moved to EyeDataSerializer as yet the method is used by dataStreamer too
    if (textStream->status() == QTextStream::Ok) {
//        uint _trialNumber = 1;
//        QString _message = "";
//        std::vector<double> _d = {-1.0,-1.0};
        if(recEventTracker) {
            _trialNumber = recEventTracker->getTrialIncrement(timestamp).trialNumber;
            _message = recEventTracker->getMessage(timestamp).messageString;
            _d = recEventTracker->getTemperatureCheck(timestamp).temperatures;
        }
        *textStream << EyeDataSerializer::pupilToRowCSV(timestamp, procMode, Pupils, filename, _trialNumber, delim, dataStyle, _d, _message) << Qt::endl;
    }
}

// GB NOTE: I found two unreferenced functions here, called writePupilData() and writeStereoPupilData().
// I tried to actualize their functionality, now manifested in a single function. This however needs different arguments now

// Given a set of pupil detections, the functions writes the complete set to file
void DataWriter::writePupilData(std::vector<quint64> timestamps, int procMode, const std::vector<std::vector<Pupil>>& pupilData) {

    if (textStream == nullptr)
        return;

    int framePos = 0;
    for(int i=0; i<pupilData.size(); i++) {
        if (textStream->status() == QTextStream::Ok) {
            uint trialNumber = 1;
            QString message = "";
            std::vector<double> d = {0,0};
            if(recEventTracker) {
                recEventTracker->getTrialIncrement(timestamps[i]).trialNumber;
                message = recEventTracker->getMessage(timestamps[i]).messageString;
                d = recEventTracker->getTemperatureCheck(timestamps[i]).temperatures;
            }
            *textStream << EyeDataSerializer::pupilToRowCSV(static_cast<quint64>(framePos), procMode, pupilData[i], "", trialNumber, delim, dataStyle, d, message) << Qt::endl;
        }
        ++framePos;
    }
}



