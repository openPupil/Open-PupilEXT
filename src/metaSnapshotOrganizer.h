#pragma once

/**
    @author Gábor Bényei
*/

#include <ctime>

#include "pupilDetection.h"
#include "imageWriter.h"
#include "dataWriter.h"
#include "devices/singleCamera.h"
#include "devices/stereoCamera.h"
#include "devices/fileCamera.h"
#include "devices/singleWebcam.h"

/**
    
    This stores camera-related and pupil-detection related data in certain files that can be later
    used alongside with image recordings or pupil detection recordings for analytical or dev purposes.
    
    Writes all details of the camera settings and pupil detection settings to a "meta file" in human-readable format.
    XML format and extension is used right now.

*/
class MetaSnapshotOrganizer : public QObject {
    Q_OBJECT

    static const int version = 1;

public:

    enum Purpose {IMAGE_REC = 1, DATA_REC = 2};

    static void addInfoNode(QDomDocument &document, QDomElement &root, ImageWriter *imageWriter, DataWriter *dataWriter, Purpose purpose, QString fileName);
    static void addCameraNode(QDomDocument &document, QDomElement &root, Camera *camera);
    static void addPupilDetectionNode(QDomDocument &document, QDomElement &root, PupilDetection *pupilDetection, QSettings *applicationSettings);
    
    static void writeMetaSnapshot(QString fileName, Camera *camera, ImageWriter *imageWriter, PupilDetection *pupilDetection, DataWriter *dataWriter, Purpose purpose, QSettings *applicationSettings);

    static void addMapToNode(QDomDocument &document, QMap<QString, QString> map, QDomElement &parent);
};

