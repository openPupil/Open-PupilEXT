#ifndef PUPILALGOSIMPLE_DATAWRITER_H
#define PUPILALGOSIMPLE_DATAWRITER_H

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include "pupil-detection-methods/Pupil.h"


enum WriteMode {SINGLE = 0, STEREO = 1};


/**
    Class to persist the pupil detection information on disk, in a CSV, comma-separated format

    File is created and opened upon construction, and closed upon destruction

    newPupilData(): called for each new pupil data, writes the pupil data to the file stream (which is flushes its content to disk occasionally)

    writePupilData(): given a vector of pupil data, write all its entries to file
*/
class DataWriter : public QObject {
    Q_OBJECT

public:

    explicit DataWriter(const QString& fileName, int mode=WriteMode::SINGLE, QObject *parent = 0);
    ~DataWriter() override;
    void close();

    void writePupilData(const std::vector<Pupil>& pupilData);
    void writeStereoPupilData(const std::vector<std::tuple<Pupil, Pupil>> &pupilData);

private:

    QString method;
    QString header;
    QString stereoHeader;

    QFile *dataFile;
    QTextStream *textStream;

    static QString pupilToRow(quint64 timestamp, const Pupil &pupil, const QString &filepath);
    static QString pupilToStereoRow(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filepath);

public slots:

    void newPupilData(quint64 timestamp, const Pupil &pupil, const QString &filename);
    void newStereoPupilData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);

};


#endif //PUPILALGOSIMPLE_DATAWRITER_H
