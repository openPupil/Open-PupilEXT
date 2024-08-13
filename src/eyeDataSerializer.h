#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QByteArray>
#include <QSerialPort>
#include <QTextStream>
#include <QTimer>

#include <QSettings>
#include <QCoreApplication>

#include "supportFunctions.h"

#include <QtXml>
#include "pupilDetection.h"
#include "dataWriter.h"

#include <QStringBuilder>

/**
    
    Transforms every pupildetection output into text, mostly for sending through dataStreamer, but also dataWriter uses the CSV version

*/
class EyeDataSerializer : public QObject {
    Q_OBJECT

public:

    static QString getHeaderCSV(int procMode, QChar delim, DataWriterDataStyle dataStyle);
    
    static QString pupilToRowCSV(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, QChar delim, DataWriterDataStyle dataStyle, const std::vector<double> &temperatures, const QString& message);
    static QString pupilToJSON(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const std::vector<double> &temperatures, const QString& message);
    static QString pupilToXML(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const std::vector<double> &temperatures, const QString& message);
    static QString pupilToYAML(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const std::vector<double> &temperatures, const QString& message);

    static void populatePupilNodeJSON(quint64 &timestamp, QJsonObject &dObj, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, double temperature, const QString& message);
    static void populatePupilNodeXML(quint64 &timestamp, QDomElement &dObj, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, double temperature, const QString& message);
    static void populatePupilNodeYAML(quint64 &timestamp, QString &obj, ushort depth, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum,  double temperature, const QString& message);
    static void addRowYAML(QString &obj, QString key, QString value, ushort depth, bool isLeaf);

};

