#include <iostream>
#include <QtCore/qfileinfo.h>
#include "metaSnapshotOrganizer.h"
#include "supportFunctions.h"

// Writes all details of the camera settings and pupil detection settings to a "meta file" in human-readable format
// added by kheki4 on 2022.11.07.


void MetaSnapshotOrganizer::writeMetaSnapshot(QString fileName, Camera *camera, ImageWriter *imageWriter, PupilDetection *pupilDetection, DataWriter *dataWriter, Purpose purpose, QSettings *applicationSettings) {

    qDebug() << fileName;
    QDomDocument document;
    QDomElement root = document.createElement("MetaSnapshot");
    document.appendChild(root);

    addInfoNode(document, root, imageWriter, dataWriter, purpose, fileName);

    addCameraNode(document, root, camera);

    if(pupilDetection && pupilDetection->isTrackingOn())
        addPupilDetectionNode(document, root, pupilDetection, applicationSettings);
    
    QString payload = document.toString();


    bool changedGiven = false;
    QString changedPath;
    bool pathWriteable = SupportFunctions::preparePath(fileName, changedGiven, changedPath);
    //if(changedGiven)
    //    QMessageBox::warning(nullptr, "Path name changed", "The given path/name contained nonstandard characters,\nwhich were changed automatically for the following: a-z, A-Z, 0-9, _");

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

void MetaSnapshotOrganizer::addInfoNode(QDomDocument &document, QDomElement &root, ImageWriter *imageWriter, DataWriter *dataWriter, Purpose purpose, QString fileName) {
    
    QMap<QString, QString> metaSnapshot;
    metaSnapshot["version"] = QString::number(version);
    if(purpose==DATA_REC) {
        metaSnapshot["purpose"] = "datarec";
    } else if(purpose==IMAGE_REC) {
        metaSnapshot["purpose"] = "imagerec";
    }
    metaSnapshot["creationTime"] = QDateTime::currentDateTime().toString("yyyy. MMM dd. hh:mm:ss");
    metaSnapshot["name"] = QString(fileName);
    metaSnapshot["creationTimeUnix"] = QString::number(QDateTime::currentMSecsSinceEpoch());
    metaSnapshot["algorithm"] = "test";
    metaSnapshot["type"] = "test";
    if(imageWriter) {
        metaSnapshot["imageOutputDirectory"] = imageWriter->getOpenableDirectoryName();
    }
    if(dataWriter) {
        metaSnapshot["csvOutputFile"] = dataWriter->getDataFileName();
    }

    addMapToNode(document, metaSnapshot, root);
}

void MetaSnapshotOrganizer::addCameraNode(QDomDocument &document, QDomElement &root, Camera *camera) {
   

    QString cameraIds[] = {"Main", "Secondary"};
    
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

    


    

    
    

    if(camera->getType() == LIVE_SINGLE_CAMERA) {
        SingleCamera *singleCamera = dynamic_cast<SingleCamera *>(camera);
        QMap<QString, QString> cameraMap;
        cameraMap["id"] = cameraIds[0];
        cameraMap["friendlyName"] =  singleCamera->getFriendlyName();
        cameraMap["cameraCalibrationFileName"] = singleCamera->getCalibrationFilename();
        cameraMap["lineSource"] = singleCamera->getLineSource();
        cameraMap["binning"] = singleCamera->getBinningVal();
        QMap<QString, QString> settingsMap;
        settingsMap["cameraImageType"] = cameraImageType;
        settingsMap["width"] = singleCamera->getImageROIwidthMax();
        settingsMap["height"] = singleCamera->getImageROIheightMax();
        settingsMap["gain"] = singleCamera->getGainValue();
        settingsMap["exposureTime"] = singleCamera->getExposureTimeValue();
         
        

        QMap<QString, QString> acqRoiMap;
        acqRoiMap["x"] = singleCamera->getImageROIoffsetX();
        acqRoiMap["y"] = singleCamera->getImageROIoffsetY(); 
        acqRoiMap["width"] = singleCamera->getImageROIwidth(); 
        acqRoiMap["height"] = singleCamera->getImageROIheight(); 
        



        QDomElement actualImageROI = document.createElement("imageAcqRoi");
        addMapToNode(document, acqRoiMap, actualImageROI);

        QDomElement calibrationSettingsNode = document.createElement("calibrationSettings");
        calibrationSettingsNode.appendChild(actualImageROI);
        addMapToNode(document, settingsMap, calibrationSettingsNode);
        QDomElement cameraNode = document.createElement("camera");
        cameraNode.appendChild(calibrationSettingsNode);
         addMapToNode(document, cameraMap, cameraNode);
        QDomElement cameras = document.createElement("cameras");
        cameras.appendChild(cameraNode);
        root.appendChild(cameras);

    } else if(camera->getType() == LIVE_STEREO_CAMERA) {
        StereoCamera *stereoCamera = dynamic_cast<StereoCamera *>(camera);
        QDomElement cameraNode = document.createElement("camera");
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
        QDomElement cameraNode = document.createElement("camera");
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
    //root.appendChild(cameraNode);
}

void MetaSnapshotOrganizer::addPupilDetectionNode(QDomDocument &document, QDomElement &root, PupilDetection *pupilDetection, QSettings *applicationSettings) {
    QDomElement pdNode = document.createElement("pupilDetections");
    root.appendChild(pdNode);

    QString pupilIds[] = { "A", "B" };
    int val = pupilDetection->getCurrentProcMode();
    QString procMode = "UNDETERMINED";
    if(val == SINGLE_IMAGE_ONE_PUPIL) {
        procMode = "SINGLE_IMAGE_ONE_PUPIL";
    } else if(val == SINGLE_IMAGE_TWO_PUPIL) {
        procMode = "SINGLE_IMAGE_TWO_PUPIL";
    } else if(val == STEREO_IMAGE_ONE_PUPIL) {
        procMode = "STEREO_IMAGE_ONE_PUPIL";
    } else if(val == STEREO_IMAGE_TWO_PUPIL) {
        procMode = "STEREO_IMAGE_TWO_PUPIL";
    // } else if(val == MIRR_IMAGE_ONE_PUPIL) {
    //     procMode = "MIRR_IMAGE_ONE_PUPIL";
    } 

    QMap<QString, QString> pupilDetectionMap;
    pupilDetectionMap["procMode"] = procMode;


    //pdNode.setAttribute("ProcMode", procMode);
    //pdNode.setAttribute("Algorithm", QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
    // TODO alg params too, etc

    QDomElement pdROIs = document.createElement("pupilDetection");
    
    QDomElement pupilObjA;
    QDomElement pupilObjB;
    QDomElement viewObjAMain;
    QDomElement viewObjASec;
    QDomElement viewObjBMain;
    QDomElement viewObjBSec;
    QMap<QString, QString> pupilMap;
    QMap<QString, QString> discreteMap;
    QMap<QString, QString> rationalMap;
    QDomElement id = document.createElement("id");
    QRectF rationalROI = applicationSettings->value("SingleCameraView.ROIsingleImageOnePupil.rational", QRectF()).toRectF();
    QVector<QDomElement> pupilDetectionROIs;
    QDomElement discreteROIObj = document.createElement("pupilDetectionROI");
    QDomElement rationalROIObj = document.createElement("pupilDetectionROI");
    switch(val) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:

            
            pupilMap["id"] = pupilIds[0];
            
            
            viewObjAMain = document.createElement("cameraRef");
            
            viewObjAMain.appendChild(id);

            
            discreteMap["x"] = pupilDetection->getROIsingleImageOnePupil().x();
            discreteMap["y"] = pupilDetection->getROIsingleImageOnePupil().y();
            discreteMap["width"] = pupilDetection->getROIsingleImageOnePupil().width();
            discreteMap["height"] = pupilDetection->getROIsingleImageOnePupil().height();

            
            rationalMap["x"] = rationalROI.x();
            rationalMap["y"] = rationalROI.y();
            rationalMap["width"] = rationalROI.width();
            rationalMap["height"] = rationalROI.height();
            
            
            addMapToNode(document, discreteMap, discreteROIObj);
            pupilDetectionROIs.append(discreteROIObj);

            
            addMapToNode(document, rationalMap, rationalROIObj);

            pupilDetectionROIs.append(rationalROIObj);

            for (auto elem: pupilDetectionROIs){
                viewObjAMain.appendChild(elem);
            }
            
            pupilObjA = document.createElement("pupil");
            addMapToNode(document, pupilMap, pupilObjA);

            pdROIs.appendChild(viewObjAMain);
            pdROIs.appendChild(pupilObjA);
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
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     pupilObjA = document.createElement("A");
        //     pdROIs.appendChild(pupilObjA);
        //     viewObjAMain = document.createElement("Main");
        //     viewObjASec = document.createElement("Sec");
            
        //     viewObjAMain.setAttribute("x", pupilDetection->getROImirrImageOnePupil1().x()); 
        //     viewObjAMain.setAttribute("y", pupilDetection->getROImirrImageOnePupil1().y()); 
        //     viewObjAMain.setAttribute("width", pupilDetection->getROImirrImageOnePupil1().width()); 
        //     viewObjAMain.setAttribute("height", pupilDetection->getROImirrImageOnePupil1().height()); 

        //     viewObjASec.setAttribute("x", pupilDetection->getROImirrImageOnePupil2().x()); 
        //     viewObjASec.setAttribute("y", pupilDetection->getROImirrImageOnePupil2().y()); 
        //     viewObjASec.setAttribute("width", pupilDetection->getROImirrImageOnePupil2().width()); 
        //     viewObjASec.setAttribute("height", pupilDetection->getROImirrImageOnePupil2().height()); 

        //     pupilObjA.appendChild(viewObjAMain);
        //     pupilObjA.appendChild(viewObjASec);
        //     break;
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

void MetaSnapshotOrganizer::addMapToNode(QDomDocument &document, QMap<QString, QString> map, QDomElement &parent){
    for (auto k : map.keys()){
        QDomElement elem = document.createElement(k);
        QDomText text = document.createTextNode(map.value(k));
        elem.appendChild(text);
        parent.appendChild(elem);
    }
}