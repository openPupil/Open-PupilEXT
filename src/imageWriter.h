#pragma once

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
*/
class ImageWriter : public QObject {
Q_OBJECT

public:

    // Format: Bmp is fastest as it doesnt compress, jpg is also fast "enough" but results in higher cpu load
    ImageWriter(const QString& directory, bool stereo=false, QObject *parent = 0);
    ~ImageWriter() override;

    // this is yet only used by MetaSnapshotOrganizer
    QString getOpenableDirectoryName() {
        if(!stereoMode) {
            return outputDirectory.absolutePath();
        } else {
            return outputDirectory.absolutePath().mid(0, outputDirectory.absolutePath().length()-3);
        }
    };

private:

    QSettings *applicationSettings;
    QDir outputDirectory, outputDirectorySecondary;
    QString imageWriterFormatString;
    QString imageWriterDataRule;
    bool stereoMode;

    std::vector<int> writeParams = std::vector<int>();

public slots:

    void onNewImage(const CameraImage &img);

};
