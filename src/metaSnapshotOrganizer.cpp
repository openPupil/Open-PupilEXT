#include <iostream>
#include <QtCore/qfileinfo.h>
#include "metaSnapshotOrganizer.h"
#include "supportFunctions.h"

// Writes all details of the camera settings and pupil detection settings to a "meta file" in human-readable format
// added by kheki4 on 2022.11.07.
void MetaSnapshotOrganizer::writeMetaSnapshot(QString fileName, Camera *camera, ImageWriter *imageWriter, PupilDetection *pupilDetection, DataWriter *dataWriter) {

    QDomDocument document;
    QDomElement root = document.createElement("MetaSnapshot");
    document.appendChild(root);


    addInfoNode(document, root, fileName);    
    if(imageWriter) {
        root.setAttribute("ImageOutputDirectory", imageWriter->getOpenableDirectoryName());
    }
    if(dataWriter) {
        root.setAttribute("CSVOutputFileName", dataWriter->getDataFileName());
    }
    addCameraNode(document, root, camera);

    if(pupilDetection && pupilDetection->isTrackingOn())
        addPupilDetectionNode(document, root, pupilDetection);
    
    QString payload = document.toString();



    // write to file
    SupportFunctions::preparePath(fileName);
    QFile* dataFile = new QFile(fileName);
    //if(dataFile->exists() == false) {
    //    return;
    //}

    // Open the file in append mode
    if (!dataFile->open(QIODevice::WriteOnly | QIODevice::ReadWrite | QIODevice::Text)) {
        std::cout << "Recording failure. Could not open: " << fileName.toStdString() << std::endl;
        delete dataFile;
        dataFile = nullptr;
    }

    QTextStream *textStream = new QTextStream(dataFile);
    *textStream << payload;

    std::cout<<fileName.toStdString()<<std::endl;

    if (dataFile){
        dataFile->close();
        dataFile->deleteLater();
    }
    delete textStream;
    delete dataFile;
    dataFile = nullptr;
    textStream = nullptr;
    
}

void MetaSnapshotOrganizer::addInfoNode(QDomDocument &document, QDomElement &root, QString fileName) {
    QDomElement infoNode = document.createElement("Info");
    root.appendChild(infoNode);




    QString metaSnapshotVersion = QString::fromStdString("1");
    QDateTime dateTime = QDateTime::currentDateTime();
    //QString humanDate = QLocale::system().toString(dateTime, QLocale::LongFormat);
    QString humanDate = dateTime.toString("yyyy. MMM dd. hh:mm:ss");
    infoNode.setAttribute("MetaSnapshotVersion", QString(metaSnapshotVersion)); 
    infoNode.setAttribute("MetaSnapshotName", QString(fileName));
    infoNode.setAttribute("MetaSnapshotCreationTime", humanDate);
    infoNode.setAttribute("MetaSnapshotCreationTimeUNIXms", QString::number(QDateTime::currentMSecsSinceEpoch()));
}

void MetaSnapshotOrganizer::addCameraNode(QDomDocument &document, QDomElement &root, Camera *camera) {
    QDomElement cameraNode = document.createElement("Camera");

    QString cameraImageType = "";
    if(camera->getType() == SINGLE_IMAGE_FILE)
        cameraImageType = "SINGLE_IMAGE_FILE";
    else if(camera->getType() == STEREO_IMAGE_FILE)
        cameraImageType = "STEREO_IMAGE_FILE";
    else if(camera->getType() == LIVE_SINGLE_CAMERA)
        cameraImageType = "LIVE_SINGLE_CAMERA";
    else if(camera->getType() == LIVE_STEREO_CAMERA)
        cameraImageType = "LIVE_STEREO_CAMERA";
    else if(camera->getType() == LIVE_SINGLE_WEBCAM)
        cameraImageType = "LIVE_SINGLE_WEBCAM";

    cameraNode.setAttribute("CameraImageType", cameraImageType);
    

    if(camera->getType() == LIVE_SINGLE_CAMERA) {
        SingleCamera *singleCamera = dynamic_cast<SingleCamera *>(camera);

        // only live camera devices have the calibration file name
        cameraNode.setAttribute("CameraCalibrationFileName", singleCamera->getCalibrationFilename());
        cameraNode.setAttribute("LineSource", QString(singleCamera->getLineSource())); 
        cameraNode.setAttribute("Binning", singleCamera->getBinningVal()); 
        QDomElement maxImageSize = document.createElement("MaxImageSize");
        maxImageSize.setAttribute("width", singleCamera->getImageROIwidthMax()); 
        maxImageSize.setAttribute("height", singleCamera->getImageROIheightMax()); 
        cameraNode.appendChild(maxImageSize);
        QDomElement actualImageROI = document.createElement("ImageAcqROI");
        actualImageROI.setAttribute("x", singleCamera->getImageROIoffsetX()); 
        actualImageROI.setAttribute("y", singleCamera->getImageROIoffsetY()); 
        actualImageROI.setAttribute("width", singleCamera->getImageROIwidth()); 
        actualImageROI.setAttribute("height", singleCamera->getImageROIheight()); 
        cameraNode.appendChild(actualImageROI);

        QDomElement cam1 = document.createElement("Main");
        cam1.setAttribute("FriendlyName", singleCamera->getFriendlyName()); 
        cameraNode.appendChild(cam1);

    } else if(camera->getType() == LIVE_STEREO_CAMERA) {
        StereoCamera *stereoCamera = dynamic_cast<StereoCamera *>(camera);

        // only live camera devices have the calibration file name
        cameraNode.setAttribute("CameraCalibrationFileName", stereoCamera->getCalibrationFilename());
        cameraNode.setAttribute("LineSource", QString(stereoCamera->getLineSource())); 
        cameraNode.setAttribute("Binning", stereoCamera->getBinningVal()); 
        QDomElement maxImageSize = document.createElement("MaxImageSize");
        maxImageSize.setAttribute("width", stereoCamera->getImageROIwidthMax()); 
        maxImageSize.setAttribute("height", stereoCamera->getImageROIheightMax()); 
        cameraNode.appendChild(maxImageSize);
        QDomElement actualImageROI = document.createElement("ImageAcqROI");
        actualImageROI.setAttribute("x", stereoCamera->getImageROIoffsetX()); 
        actualImageROI.setAttribute("y", stereoCamera->getImageROIoffsetY()); 
        actualImageROI.setAttribute("width", stereoCamera->getImageROIwidth()); 
        actualImageROI.setAttribute("height", stereoCamera->getImageROIheight()); 
        cameraNode.appendChild(actualImageROI);

        QDomElement cam1 = document.createElement("Main");
        cam1.setAttribute("FriendlyName", stereoCamera->getFriendlyNames()[0]); 
        cameraNode.appendChild(cam1);

        QDomElement cam2 = document.createElement("Secondary");
        cam2.setAttribute("FriendlyName", stereoCamera->getFriendlyNames()[1]); 
        cameraNode.appendChild(cam2);

    } else if(camera->getType() == SINGLE_IMAGE_FILE || camera->getType() == STEREO_IMAGE_FILE) {
        FileCamera *fileCamera = dynamic_cast<FileCamera *>(camera);

        // only live camera devices have the calibration file name
        cameraNode.setAttribute("ImageDirectory", fileCamera->getImageDirectoryName()); 

        QDomElement imageSize = document.createElement("ImageSize");
        imageSize.setAttribute("width", fileCamera->getImageWidth()); 
        imageSize.setAttribute("height", fileCamera->getImageHeight()); 
        cameraNode.appendChild(imageSize);
        // we assume that image width and height is the same for all images, and all filecamera folders

    } /*else if(camera->getType() == LIVE_SINGLE_WEBCAM) {
        SingleWebcam *singleWebcam = dynamic_cast<SingleWebcam *>(camera);

        QDomElement imageSize = document.createElement("ImageSize");
        imageSize.setAttribute("width", singleWebcam->getImageWidth()); 
        imageSize.setAttribute("height", singleWebcam->getImageHeight()); 
        cameraNode.appendChild(imageSize);
        
    }*/
    root.appendChild(cameraNode);
}

void MetaSnapshotOrganizer::addPupilDetectionNode(QDomDocument &document, QDomElement &root, PupilDetection *pupilDetection) {
    QDomElement pdNode = document.createElement("PupilDetection");

    int val = pupilDetection->getCurrentProcMode();
    QString procMode = "UNDETERMINED";
    if(val == SINGLE_IMAGE_ONE_PUPIL) {
        procMode = "SINGLE_IMAGE_ONE_PUPIL";
    } else if(val == SINGLE_IMAGE_TWO_PUPIL) {
        procMode = "SINGLE_IMAGE_TWO_PUPIL";
    } else if(val == MIRR_IMAGE_ONE_PUPIL) {
        procMode = "MIRR_IMAGE_ONE_PUPIL";
    } else if(val == STEREO_IMAGE_ONE_PUPIL) {
        procMode = "STEREO_IMAGE_ONE_PUPIL";
    } else if(val == STEREO_IMAGE_TWO_PUPIL) {
        procMode = "STEREO_IMAGE_TWO_PUPIL";
    } 

    pdNode.setAttribute("ProcMode", procMode);
    pdNode.setAttribute("Algorithm", QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
    // TODO alg params too, etc

    QDomElement pdROIs = document.createElement("PupilDetectionROIs");
    
    QDomElement pupilObjA;
    QDomElement pupilObjB;
    QDomElement viewObjAMain;
    QDomElement viewObjASec;
    QDomElement viewObjBMain;
    QDomElement viewObjBSec;
    switch(val) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            pupilObjA = document.createElement("A");
            pdROIs.appendChild(pupilObjA);
            viewObjAMain = document.createElement("Main");
            
            viewObjAMain.setAttribute("x", pupilDetection->getROIsingleImageOnePupil().x()); 
            viewObjAMain.setAttribute("y", pupilDetection->getROIsingleImageOnePupil().y()); 
            viewObjAMain.setAttribute("width", pupilDetection->getROIsingleImageOnePupil().width()); 
            viewObjAMain.setAttribute("height", pupilDetection->getROIsingleImageOnePupil().height()); 

            pupilObjA.appendChild(viewObjAMain);
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            pupilObjA = document.createElement("A");
            pupilObjB = document.createElement("B");
            pdROIs.appendChild(pupilObjA);
            pdROIs.appendChild(pupilObjB);
            viewObjAMain = document.createElement("Main");
            viewObjBMain = document.createElement("Main");
            
            viewObjAMain.setAttribute("x", pupilDetection->getROIsingleImageTwoPupilA().x()); 
            viewObjAMain.setAttribute("y", pupilDetection->getROIsingleImageTwoPupilA().y()); 
            viewObjAMain.setAttribute("width", pupilDetection->getROIsingleImageTwoPupilA().width()); 
            viewObjAMain.setAttribute("height", pupilDetection->getROIsingleImageTwoPupilA().height()); 

            viewObjBMain.setAttribute("x", pupilDetection->getROIsingleImageTwoPupilB().x()); 
            viewObjBMain.setAttribute("y", pupilDetection->getROIsingleImageTwoPupilB().y()); 
            viewObjBMain.setAttribute("width", pupilDetection->getROIsingleImageTwoPupilB().width()); 
            viewObjBMain.setAttribute("height", pupilDetection->getROIsingleImageTwoPupilB().height()); 

            pupilObjA.appendChild(viewObjAMain);
            pupilObjB.appendChild(viewObjBMain);
            break;
        case ProcMode::MIRR_IMAGE_ONE_PUPIL:
            pupilObjA = document.createElement("A");
            pdROIs.appendChild(pupilObjA);
            viewObjAMain = document.createElement("Main");
            viewObjASec = document.createElement("Sec");
            
            viewObjAMain.setAttribute("x", pupilDetection->getROImirrImageOnePupil1().x()); 
            viewObjAMain.setAttribute("y", pupilDetection->getROImirrImageOnePupil1().y()); 
            viewObjAMain.setAttribute("width", pupilDetection->getROImirrImageOnePupil1().width()); 
            viewObjAMain.setAttribute("height", pupilDetection->getROImirrImageOnePupil1().height()); 

            viewObjASec.setAttribute("x", pupilDetection->getROImirrImageOnePupil2().x()); 
            viewObjASec.setAttribute("y", pupilDetection->getROImirrImageOnePupil2().y()); 
            viewObjASec.setAttribute("width", pupilDetection->getROImirrImageOnePupil2().width()); 
            viewObjASec.setAttribute("height", pupilDetection->getROImirrImageOnePupil2().height()); 

            pupilObjA.appendChild(viewObjAMain);
            pupilObjA.appendChild(viewObjASec);
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            pupilObjA = document.createElement("A");
            pdROIs.appendChild(pupilObjA);
            viewObjAMain = document.createElement("Main");
            viewObjASec = document.createElement("Sec");
            
            viewObjAMain.setAttribute("x", pupilDetection->getROIstereoImageOnePupil1().x()); 
            viewObjAMain.setAttribute("y", pupilDetection->getROIstereoImageOnePupil1().y()); 
            viewObjAMain.setAttribute("width", pupilDetection->getROIstereoImageOnePupil1().width()); 
            viewObjAMain.setAttribute("height", pupilDetection->getROIstereoImageOnePupil1().height()); 

            viewObjASec.setAttribute("x", pupilDetection->getROIstereoImageOnePupil2().x()); 
            viewObjASec.setAttribute("y", pupilDetection->getROIstereoImageOnePupil2().y()); 
            viewObjASec.setAttribute("width", pupilDetection->getROIstereoImageOnePupil2().width()); 
            viewObjASec.setAttribute("height", pupilDetection->getROIstereoImageOnePupil2().height()); 

            pupilObjA.appendChild(viewObjAMain);
            pupilObjA.appendChild(viewObjASec);
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            pupilObjA = document.createElement("A");
            pupilObjB = document.createElement("B");
            pdROIs.appendChild(pupilObjA);
            pdROIs.appendChild(pupilObjB);
            viewObjAMain = document.createElement("Main");
            viewObjASec = document.createElement("Sec");
            viewObjBMain = document.createElement("Main");
            viewObjBSec = document.createElement("Sec");
            
            viewObjAMain.setAttribute("x", pupilDetection->getROIstereoImageTwoPupilA1().x()); 
            viewObjAMain.setAttribute("y", pupilDetection->getROIstereoImageTwoPupilA1().y()); 
            viewObjAMain.setAttribute("width", pupilDetection->getROIstereoImageTwoPupilA1().width()); 
            viewObjAMain.setAttribute("height", pupilDetection->getROIstereoImageTwoPupilA1().height()); 

            viewObjASec.setAttribute("x", pupilDetection->getROIstereoImageTwoPupilA2().x()); 
            viewObjASec.setAttribute("y", pupilDetection->getROIstereoImageTwoPupilA2().y()); 
            viewObjASec.setAttribute("width", pupilDetection->getROIstereoImageTwoPupilA2().width()); 
            viewObjASec.setAttribute("height", pupilDetection->getROIstereoImageTwoPupilA2().height()); 

            viewObjBMain.setAttribute("x", pupilDetection->getROIstereoImageTwoPupilB1().x()); 
            viewObjBMain.setAttribute("y", pupilDetection->getROIstereoImageTwoPupilB1().y()); 
            viewObjBMain.setAttribute("width", pupilDetection->getROIstereoImageTwoPupilB1().width()); 
            viewObjBMain.setAttribute("height", pupilDetection->getROIstereoImageTwoPupilB1().height()); 

            viewObjBSec.setAttribute("x", pupilDetection->getROIstereoImageTwoPupilB2().x()); 
            viewObjBSec.setAttribute("y", pupilDetection->getROIstereoImageTwoPupilB2().y()); 
            viewObjBSec.setAttribute("width", pupilDetection->getROIstereoImageTwoPupilB2().width()); 
            viewObjBSec.setAttribute("height", pupilDetection->getROIstereoImageTwoPupilB2().height()); 

            pupilObjA.appendChild(viewObjAMain);
            pupilObjA.appendChild(viewObjASec);
            pupilObjB.appendChild(viewObjBMain);
            pupilObjB.appendChild(viewObjBSec);
            break;
        //default:
            //break;
    }
    pdNode.appendChild(pdROIs);
    root.appendChild(pdNode);
}

