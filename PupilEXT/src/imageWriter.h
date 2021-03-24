
#ifndef PUPILEXT_IMAGEWRITER_H
#define PUPILEXT_IMAGEWRITER_H

/**
    @author Moritz Lode
*/

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
    ImageWriter(QString directory, QString format="bmp", bool stereo=false, QObject *parent = 0);
    ~ImageWriter() override;

private:

    QDir outputDirectory, outputDirectorySecondary;
    QString format;
    bool stereoMode;

public slots:

    void onNewImage(const CameraImage &img);

};


#endif //PUPILEXT_IMAGEWRITER_H
