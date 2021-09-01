#include <iostream>
#include <QtCore/qfileinfo.h>
#include "dataWriter.h"

// TODO datawriter is receiving pupil signal at the full rate, slowing down the gui thread? move to other thread?

DataWriter::DataWriter(const QString& fileName, int mode, QObject *parent) : QObject(parent) {

    // Header definitions of the output file, this must fit the output format in the pupilToRow functions
    header = "filename,timestamp_ms,algorithm,diameter_px,undistortedDiameter_px,physicalDiameter_mm,width_px,height_px,axisRatio,center_x,center_y,angle_deg,circumference_px,confidence,outlineConfidence";
    stereoHeader = "filename,timestamp_ms,algorithm,diameterMain_px,diameterSec_px,undistortedDiameterMain_px,undistortedDiameterSec_px,physicalDiameter_mm,widthMain_px,heightMain_px,axisRatioMain,widthSec_px,heightSec_px,axisRatioSec,centerMain_x,centerMain_y,centerSec_x,centerSec_y,angleMain_deg,angleSec_deg,circumferenceMain_px,circumferenceSec_px,confidenceMain,outlineConfidenceMain,confidenceSec,outlineConfidenceSec";

    std::cout<<fileName.toStdString()<<std::endl;

    dataFile = new QFile(fileName);

    bool exists = dataFile->exists();

    // Open the file in append mode
    if (!dataFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cout << "Recording failure. Could not open: " << fileName.toStdString() << std::endl;
        delete dataFile;
        dataFile = nullptr;
    }

    textStream = new QTextStream(dataFile);

    // To not write again a header line to the file when it already existed (appending), check it
    if(!exists) {
        if(mode==WriteMode::SINGLE) {
            *textStream << header << Qt::endl;
        } else if(mode==WriteMode::STEREO) {
            *textStream << stereoHeader << Qt::endl;
        }
    }
}

DataWriter::~DataWriter() {
    close();
}

// Close the file and fielstreams
void DataWriter::close() {

    if (dataFile){
        dataFile->close();
        dataFile->deleteLater();
    }
    delete textStream;
    delete dataFile;
    dataFile = nullptr;
    textStream = nullptr;
}

// On new pupil data, write the pupil detection to file in a new row
void DataWriter::newPupilData(quint64 timestamp, const Pupil &pupil, const QString &filename) {

    if (!textStream)
        return;

    if (textStream->status() == QTextStream::Ok) {
        *textStream<<pupilToRow(timestamp, pupil, filename) << Qt::endl;
    }
}

// Upon a new stereo pupil detection, write it to file in a new row (stereo format)
void DataWriter::newStereoPupilData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename) {

    if (!textStream)
        return;

    if (textStream->status() == QTextStream::Ok) {
        *textStream<<pupilToStereoRow(timestamp, pupil, pupilSec, filename) << Qt::endl;
    }
}

// Converts a pupil detection to a string row that is written to file
// CAUTION: This must exactly reproduce the format defined by the header fields
QString DataWriter::pupilToRow(quint64 timestamp, const Pupil &pupil, const QString &filepath) {

    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    //"filename,timestamp[ms],algorithm,diameter[px],physicaldiameter[mm],width[px],height[px],axis_ratio,center_x,center_y,angle[deg],circumference[px],confidence,outline_confidence";

    return filename + ',' + QString::number(timestamp) + ',' + QString::fromStdString(pupil.algorithmName) + ',' + QString::number(pupil.diameter()) + ',' + QString::number(pupil.undistortedDiameter)  + ',' + QString::number(pupil.physicalDiameter)
        + ',' + QString::number(pupil.width()) + ',' + QString::number(pupil.height()) + ',' + QString::number((double)pupil.width() / pupil.height())
        + ',' + QString::number(pupil.center.x) + ',' + QString::number(pupil.center.y) + ',' + QString::number(pupil.angle)
        + ',' + QString::number(pupil.circumference()) + ',' + QString::number(pupil.confidence) + ',' + QString::number(pupil.outline_confidence);
}

// Converts a pupil detection to a string row that is written to file
// CAUTION: This must exactly reproduce the format defined by the header fields
QString DataWriter::pupilToStereoRow(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filepath) {

    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    //"filename,timestamp[ms],algorithm,diameter[px],diameterSec[px],physicaldiameter[mm],width[px],height[px],axis_ratio,widthSec[px],heightSec[px],axis_ratioSec,center_x,center_y,center_xSec,center_ySec,angle[deg],angleSec[deg],circumference[px],circumferenceSec[px],confidence,outline_confidence,confidenceSec,outline_confidenceSec";

    return filename + ',' + QString::number(timestamp) + ',' + QString::fromStdString(pupil.algorithmName) + ',' + QString::number(pupil.diameter()) + ',' + QString::number(pupilSec.diameter())
           + ',' + QString::number(pupil.undistortedDiameter) + ',' + QString::number(pupilSec.undistortedDiameter)
           + ',' + QString::number(pupil.physicalDiameter) + ',' + QString::number(pupil.width()) + ',' + QString::number(pupil.height()) + ',' + QString::number((double)pupil.width() / pupil.height())
           + ',' + QString::number(pupilSec.width()) + ',' + QString::number(pupilSec.height()) + ',' + QString::number((double)pupilSec.width() / pupilSec.height())
           + ',' + QString::number(pupil.center.x) + ',' + QString::number(pupil.center.y) + ',' + QString::number(pupilSec.center.x) + ',' + QString::number(pupilSec.center.y)
           + ',' + QString::number(pupil.angle) + ',' + QString::number(pupilSec.angle)
           + ',' + QString::number(pupil.circumference()) + ',' + QString::number(pupilSec.circumference()) + ',' + QString::number(pupil.confidence) + ',' + QString::number(pupil.outline_confidence)
           + ',' + QString::number(pupilSec.confidence) + ',' + QString::number(pupilSec.outline_confidence);
}

// Given a set of pupil detections, the functions writes the complete set to file
void DataWriter::writePupilData(const std::vector<Pupil>& pupilData) {

    if (textStream == nullptr)
        return;

    int framePos = 0;
    for(const auto& pupil: pupilData) {
        if (textStream->status() == QTextStream::Ok) {
            *textStream << pupilToRow(static_cast<quint64>(framePos), pupil, "") << Qt::endl;
        }
        ++framePos;
    }
}
// Given a set of pupil detections, the functions writes the complete set to file
void DataWriter::writeStereoPupilData(const std::vector<std::tuple<Pupil, Pupil>>& pupilData) {

    if (textStream == nullptr)
        return;

    int framePos = 0;
    for(const auto& pupil_tup: pupilData) {
        if (textStream->status() == QTextStream::Ok) {
            *textStream<<pupilToStereoRow(static_cast<quint64>(framePos), std::get<0>(pupil_tup), std::get<1>(pupil_tup), "") << Qt::endl;
        }
        ++framePos;
    }
}
