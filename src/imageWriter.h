
#ifndef PUPILEXT_IMAGEWRITER_H
#define PUPILEXT_IMAGEWRITER_H

/**
    @author Moritz Lode, Gabor Benyei
*/

#include <QCoreApplication>
#include <QtCore/qdir.h>
#include "devices/camera.h"

/**
    Class to write camera images to disk

    Supports recording of single and stereo images

    onNewImage(): received new image and writes it to disk, file writing is performed concurrently for maximal performance

    CAUTION: Chosen image format has a large performance impact due to size and disk write speeds

    NOTE: Modified by Gabor Benyei, 2023 jan
    GB NOTE:
        Added getOpenableDirectoryName() and added string preparatory step for output directory name. Now using const qstring reference
*/
class ImageWriter : public QObject {
Q_OBJECT

public:

    // Format: Bmp is fastest as it doesnt compress, jpg is also fast "enough" but results in higher cpu load
    ImageWriter(const QString& directory, bool stereo=false, QObject *parent = 0);
    ~ImageWriter() override;

    // GB added begin
    // GB: this is yet only used by MetaSnapshotOrganizer
    QString getOpenableDirectoryName() {
        if(!stereoMode) {
            return outputDirectory.absolutePath();
        } else {
            return outputDirectory.absolutePath().mid(0, outputDirectory.absolutePath().length()-3);
        }
    };
    // GB added end

private:

    QSettings *applicationSettings;
    QDir outputDirectory, outputDirectorySecondary;
    QString imageWriterFormat;
    QString imageWriterDataRule;
    bool stereoMode;

public slots:

    void onNewImage(const CameraImage &img);

};

#endif //PUPILEXT_IMAGEWRITER_H
