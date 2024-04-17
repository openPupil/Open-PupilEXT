
#include <QtCore/qstandardpaths.h>
#include <QtConcurrent>
#include <opencv2/imgcodecs.hpp>
#include <QtCore/qthreadpool.h>
#include "imageWriter.h"
#include "supportFunctions.h"

// Creates a new image writer that outputs images in the given directory
// If stereo is true, a stereo directory structure is created in the given directory
ImageWriter::ImageWriter(const QString& directory, bool stereo, QObject *parent) :
    QObject(parent),
    stereoMode(stereo),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent))
    {

    imageWriterFormat = applicationSettings->value("imageWriterFormat", "bmp").toString();

    outputDirectory = QDir(directory);

    if(stereoMode) {
        outputDirectorySecondary = outputDirectory;
        if(!outputDirectory.exists("0")) {
            outputDirectory.mkdir("0");
        }
        outputDirectory.cd("0");

        if(!outputDirectorySecondary.exists("1")) {
            outputDirectorySecondary.mkdir("1");
        }
        outputDirectorySecondary.cd("1");
    }
}

ImageWriter::~ImageWriter() = default;

// Slot callback which receives new camera images
// Write the received image to disk using the specified image format
// File writing is executed using Qts concurrent to not block the GUI thread for writing
void ImageWriter::onNewImage(const CameraImage &img) {

    QString filepath = outputDirectory.filePath(QString::number(img.timestamp) + "." + imageWriterFormat);

    // std::cout<<"Saving image: " << filepath.toStdString() << std::endl;

    // Write every image over a thread pool managed by QT, this way nothing blocks and we can write images very fast (cpu heavy)
    QtConcurrent::run(cv::imwrite, filepath.toStdString(), img.img,std::vector<int>());

    if(stereoMode && (img.type == CameraImageType::STEREO_IMAGE_FILE || img.type == CameraImageType::LIVE_STEREO_CAMERA)) {
        QString filepathSecondary = outputDirectorySecondary.filePath(QString::number(img.timestamp) + "." + imageWriterFormat);
        QtConcurrent::run(cv::imwrite, filepathSecondary.toStdString(), img.imgSecondary,std::vector<int>());
    }
}

