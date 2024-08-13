
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
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    imageWriterFormatString = applicationSettings->value("imageWriterFormat.chosenFormat", "tiff").toString();

    qDebug() << "-------------------------------------";
    if(imageWriterFormatString == "png") {
        int pngCompression = applicationSettings->value("imageWriterFormat.png.compression", "0").toInt();
        writeParams = {cv::IMWRITE_PNG_COMPRESSION , pngCompression};
        qDebug() << "-------------------------------------";
        qDebug() << pngCompression;
    } else if(imageWriterFormatString == "jpeg") {
        int jpegQuality = applicationSettings->value("imageWriterFormat.jpeg.quality", "100").toInt();
        writeParams = {cv::IMWRITE_JPEG_QUALITY, jpegQuality};
        qDebug() << "-------------------------------------";
        qDebug() << jpegQuality;
    } else if(imageWriterFormatString == "webp") {
        int webpQuality = applicationSettings->value("imageWriterFormat.webp.quality", "100").toInt();
        writeParams = {cv::IMWRITE_WEBP_QUALITY, webpQuality};
        qDebug() << "-------------------------------------";
        qDebug() << webpQuality;
    } else {
        writeParams = std::vector<int>();
    }

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

    // std::cout<<"Saving image: " << filepath.toStdString() << std::endl;

    // Write every image over a thread pool managed by QT, this way nothing blocks and we can write images very fast (cpu heavy)
    QString filepath = outputDirectory.filePath(QString::number(img.timestamp) + "." + imageWriterFormatString);
    QtConcurrent::run(cv::imwrite, filepath.toStdString(), img.img, writeParams);

//    if(stereoMode && (img.type == CameraImageType::STEREO_IMAGE_FILE || img.type == CameraImageType::LIVE_STEREO_CAMERA)) {
    if(stereoMode) {
        QString filepathSecondary = outputDirectorySecondary.filePath(QString::number(img.timestamp) + "." + imageWriterFormatString);
        QtConcurrent::run(cv::imwrite, filepathSecondary.toStdString(), img.imgSecondary, writeParams);
    }
}

