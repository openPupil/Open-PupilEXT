#ifndef PUPILALGOSIMPLE_DATAWRITER_H
#define PUPILALGOSIMPLE_DATAWRITER_H

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include "pupil-detection-methods/Pupil.h"

#include "recEventTracker.h"
#include <QSettings>
#include <QCoreApplication>

#include "pupilDetection.h"

// BG NOTE: must come here due to eyeDataSerializer.h and this dataWriter.h including each other. Compiler has to know the enum before looking at the other one
enum DataWriterDataStyle {
    PUPILEXT_V0_1_1 = 1,
    PUPILEXT_V0_1_2 = 2
};

#include "eyeDataSerializer.h"


/**
    Class to persist the pupil detection information on disk, in a CSV, comma-separated format

    File is created and opened upon construction, and closed upon destruction

    NOTE: Modified by Gabor Benyei, 2023 jan
    GB NOTE:
        Reorganized code to let it handle an std::vector of Pupils, in order to comply with new signal-slot strategy, which
        I introduced to manage different pupil detection processing modes (procModes).
        WriteMode enum is not used anymore to distinguish between different headers or working mode, but procMode is used instead.
        Also now it uses EyeDataSerializer class to process every pupil detection output.

    newPupilData(): called for each new pupil data, writes the pupil data to the file stream (which is flushes its content to disk occasionally)

    writePupilData(): given a vector of pupil data, write all its entries to file
*/

class DataWriter : public QObject {
    Q_OBJECT

public:

    explicit DataWriter(
        const QString& fileName, 
        ProcMode procMode = ProcMode::SINGLE_IMAGE_ONE_PUPIL, // necessary for writing the proper header
        RecEventTracker *recEventTracker = nullptr, 
        QObject *parent = 0
        );
    ~DataWriter() override;
    void close();

    // GB: found these unreferenced functions. I updated the functionality, now available in a new function under the name writePupilData()
    //void writePupilData(const std::vector<Pupil>& pupilData);
    //void writeStereoPupilData(const std::vector<std::tuple<Pupil, Pupil>> &pupilData);

    // GB: added begin
    void writePupilData(std::vector<quint64> timestamps, int procMode, const std::vector<std::vector<Pupil>>& pupilData);

    QString getDataFileName() {
        return dataFile->fileName();
    };
    bool isReady() {
        return writerReady;
    };
    // GB end

private:

    QSettings *applicationSettings;
    QChar delim;
    DataWriterDataStyle dataStyle;
    RecEventTracker *recEventTracker;

    QString header;

    QFile *dataFile;
    QTextStream *textStream;

    // GB added begin
    bool writerReady;
    // GB added end

public slots:

    // GB: changed to work with vector of pupils due to different procModes
    void newPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename);
};

#endif //PUPILALGOSIMPLE_DATAWRITER_H
