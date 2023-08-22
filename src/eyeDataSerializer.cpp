
#include "eyeDataSerializer.h"

void EyeDataSerializer::populatePupilNodeXML(quint64 &timestamp, QDomElement &dObj, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, double temperature) {
    dObj.setAttribute("filename", filename); 
    dObj.setAttribute("timestamp_ms", QString::number(timestamp)); 
    dObj.setAttribute("algorithm", QString::fromStdString(Pupils[idx].algorithmName)); 
    dObj.setAttribute("diameter_px", QString::number(Pupils[idx].diameter())); 
    dObj.setAttribute("undistortedDiameter_px", QString::number(Pupils[idx].undistortedDiameter) ); 
    dObj.setAttribute("physicalDiameter_mm", QString::number(Pupils[idx].physicalDiameter)); 
    dObj.setAttribute("width_px", QString::number(Pupils[idx].width())); 
    dObj.setAttribute("height_px", QString::number(Pupils[idx].height())); 
    dObj.setAttribute("axisRatio_px", QString::number((double)Pupils[idx].width() / Pupils[idx].height())); 
    dObj.setAttribute("center_x", QString::number(Pupils[idx].center.x)); 
    dObj.setAttribute("center_y", QString::number(Pupils[idx].center.y)); 
    dObj.setAttribute("angle_deg", QString::number(Pupils[idx].angle)); 
    dObj.setAttribute("circumference_px", QString::number(Pupils[idx].circumference())); 
    dObj.setAttribute("confidence", QString::number(Pupils[idx].confidence)); 
    dObj.setAttribute("outlineConfidence", QString::number(Pupils[idx].outline_confidence)); 
    dObj.setAttribute("trial", QString::number(trialNum)); 
    dObj.setAttribute("temperature_c", QString::number(temperature));
}

QString EyeDataSerializer::pupilToXML(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const std::vector<double> &temperatures) {
    
    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    QDomDocument document;
    QDomElement root = document.createElement("EyeData");
    document.appendChild(root);
    
    QDomElement pupilObjA;
    QDomElement pupilObjB;
    QDomElement viewObjAMain;
    QDomElement viewObjASec;
    QDomElement viewObjBMain;
    QDomElement viewObjBSec;

    switch((ProcMode)procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            pupilObjA = document.createElement("A");
            root.appendChild(pupilObjA);
            viewObjAMain = document.createElement("Main");
            populatePupilNodeXML(timestamp, viewObjAMain, SINGLE_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
            pupilObjA.appendChild(viewObjAMain);
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            pupilObjA = document.createElement("A");
            pupilObjB = document.createElement("B");
            root.appendChild(pupilObjA);
            root.appendChild(pupilObjB);
            viewObjAMain = document.createElement("Main");
            viewObjBMain = document.createElement("Main");
            populatePupilNodeXML(timestamp, viewObjAMain, SINGLE_IMAGE_TWO_PUPIL_A, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjBMain, SINGLE_IMAGE_TWO_PUPIL_B, Pupils, filename, trialNum, temperatures[0]);
            pupilObjA.appendChild(viewObjAMain);
            pupilObjB.appendChild(viewObjBMain);
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            pupilObjA = document.createElement("A");
            root.appendChild(pupilObjA);
            viewObjAMain = document.createElement("Main");
            viewObjASec = document.createElement("Sec");
            populatePupilNodeXML(timestamp, viewObjAMain, STEREO_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjASec, STEREO_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[1]);
            pupilObjA.appendChild(viewObjAMain);
            pupilObjA.appendChild(viewObjASec);
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            pupilObjA = document.createElement("A");
            pupilObjB = document.createElement("B");
            root.appendChild(pupilObjA);
            root.appendChild(pupilObjB);
            viewObjAMain = document.createElement("Main");
            viewObjASec = document.createElement("Sec");
            viewObjBMain = document.createElement("Main");
            viewObjBSec = document.createElement("Sec");
            populatePupilNodeXML(timestamp, viewObjAMain, STEREO_IMAGE_TWO_PUPIL_A_MAIN, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjASec, STEREO_IMAGE_TWO_PUPIL_A_SEC, Pupils, filename, trialNum, temperatures[1]);
            populatePupilNodeXML(timestamp, viewObjBMain, STEREO_IMAGE_TWO_PUPIL_B_MAIN, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjBSec, STEREO_IMAGE_TWO_PUPIL_B_SEC, Pupils, filename, trialNum, temperatures[1]);
            pupilObjA.appendChild(viewObjAMain);
            pupilObjA.appendChild(viewObjASec);
            pupilObjB.appendChild(viewObjBMain);
            pupilObjB.appendChild(viewObjBSec);
            break;
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     pupilObjA = document.createElement("A");
        //     root.appendChild(pupilObjA);
        //     viewObjAMain = document.createElement("Main");
        //     viewObjASec = document.createElement("Sec");
        //     populatePupilNodeXML(timestamp, viewObjAMain, MIRR_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
        //     populatePupilNodeXML(timestamp, viewObjASec, MIRR_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[0]);
        //     pupilObjA.appendChild(viewObjAMain);
        //     pupilObjA.appendChild(viewObjASec);
        //     break;

        //default:
            //break;
    }

    return document.toString();
}


void EyeDataSerializer::populatePupilNodeJSON(quint64 &timestamp, QJsonObject &dObj, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, double temperature) {
    dObj["filename"] = filename; 
    dObj["timestamp_ms"] = QString::number(timestamp); 
    dObj["algorithm"] = QString::fromStdString(Pupils[idx].algorithmName); 
    dObj["diameter_px"] = QString::number(Pupils[idx].diameter()); 
    dObj["undistortedDiameter_px"] = QString::number(Pupils[idx].undistortedDiameter) ; 
    dObj["physicalDiameter_mm"] = QString::number(Pupils[idx].physicalDiameter); 
    dObj["width_px"] = QString::number(Pupils[idx].width()); 
    dObj["height_px"] = QString::number(Pupils[idx].height()); 
    dObj["axisRatio_px"] = QString::number((double)Pupils[idx].width() / Pupils[idx].height()); 
    dObj["center_x"] = QString::number(Pupils[idx].center.x); 
    dObj["center_y"] = QString::number(Pupils[idx].center.y); 
    dObj["angle_deg"] = QString::number(Pupils[idx].angle); 
    dObj["circumference_px"] = QString::number(Pupils[idx].circumference()); 
    dObj["confidence"] = QString::number(Pupils[idx].confidence); 
    dObj["outlineConfidence"] = QString::number(Pupils[idx].outline_confidence); 
    dObj["trial"] = QString::number(trialNum); 
    dObj["temperature_c"] = QString::number(temperature);
}

QString EyeDataSerializer::pupilToJSON(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const std::vector<double> &temperatures) {
    
    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    QJsonObject root;
    
    QJsonObject pupilObjA;
    QJsonObject pupilObjB;
    QJsonObject viewObjAMain;
    QJsonObject viewObjASec;
    QJsonObject viewObjBMain;
    QJsonObject viewObjBSec;

    switch((ProcMode)procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, SINGLE_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
            pupilObjA["Main"] = viewObjAMain;
            root["A"] = pupilObjA;
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, SINGLE_IMAGE_TWO_PUPIL_A, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjBMain, SINGLE_IMAGE_TWO_PUPIL_B, Pupils, filename, trialNum, temperatures[0]);
            pupilObjA["Main"] = viewObjAMain;
            root["A"] = pupilObjA;
            pupilObjB["Main"] = viewObjBMain;
            root["B"] = pupilObjB;
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, STEREO_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjASec, STEREO_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[1]);
            pupilObjA["Main"] = viewObjAMain;
            pupilObjA["Sec"] = viewObjASec;
            root["A"] = pupilObjA;
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, STEREO_IMAGE_TWO_PUPIL_A_MAIN, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjASec, STEREO_IMAGE_TWO_PUPIL_A_SEC, Pupils, filename, trialNum, temperatures[1]);
            populatePupilNodeJSON(timestamp, viewObjBMain, STEREO_IMAGE_TWO_PUPIL_B_MAIN, Pupils, filename, trialNum, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjBSec, STEREO_IMAGE_TWO_PUPIL_B_SEC, Pupils, filename, trialNum, temperatures[1]);
            pupilObjA["Main"] = viewObjAMain;
            pupilObjA["Sec"] = viewObjASec;
            root["A"] = pupilObjA;
            pupilObjB["Main"] = viewObjBMain;
            pupilObjB["Sec"] = viewObjBSec;
            root["B"] = pupilObjB;
            break;
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     populatePupilNodeJSON(timestamp, viewObjAMain, MIRR_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
        //     populatePupilNodeJSON(timestamp, viewObjASec, MIRR_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[0]);
        //     pupilObjA["Main"] = viewObjAMain;
        //     pupilObjA["Sec"] = viewObjASec;
        //     root["A"] = pupilObjA;
        //     break;
        
        //default:
            //break;
    }

    QJsonObject dObj;
    dObj["EyeData"] = root;
    QJsonDocument dDoc(dObj);
    QByteArray content = dDoc.toJson();
    return QString(content);
}

QString EyeDataSerializer::getHeaderCSV(int procMode, QChar delim) {
    switch(procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            return
                QString::fromStdString("filename") + delim +
                QString::fromStdString("timestamp_ms") + delim +
                QString::fromStdString("algorithm") + delim +
                QString::fromStdString("diameter_px") + delim +
                QString::fromStdString("undistortedDiameter_px") + delim +
                QString::fromStdString("physicalDiameter_mm") + delim +
                QString::fromStdString("width_px") + delim +
                QString::fromStdString("height_px") + delim +
                QString::fromStdString("axisRatio") + delim +
                QString::fromStdString("center_x") + delim +
                QString::fromStdString("center_y") + delim +
                QString::fromStdString("angle_deg") + delim +
                QString::fromStdString("circumference_px") + delim +
                QString::fromStdString("confidence") + delim +
                QString::fromStdString("outlineConfidence") + delim +
                QString::fromStdString("trial") + delim +
                QString::fromStdString("temperature_c") 
            ;
    //        break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            return
                QString::fromStdString("filename") + delim +
                QString::fromStdString("timestamp_ms") + delim +
                QString::fromStdString("algorithm") + delim +
                QString::fromStdString("diameterA_px") + delim +
                QString::fromStdString("diameterB_px") + delim +
                QString::fromStdString("undistortedDiameterA_px") + delim +
                QString::fromStdString("undistortedDiameterB_px") + delim +
                QString::fromStdString("physicalDiameterA_mm") + delim +
                QString::fromStdString("physicalDiameterB_mm") + delim + // the different line
                QString::fromStdString("widthA_px") + delim +
                QString::fromStdString("heightA_px") + delim +
                QString::fromStdString("axisRatioA") + delim +
                QString::fromStdString("widthB_px") + delim +
                QString::fromStdString("heightB_px") + delim +
                QString::fromStdString("axisRatioB") + delim +
                QString::fromStdString("centerA_x") + delim +
                QString::fromStdString("centerA_y") + delim +
                QString::fromStdString("centerB_x") + delim +
                QString::fromStdString("centerB_y") + delim +
                QString::fromStdString("angleA_deg") + delim +
                QString::fromStdString("angleB_deg") + delim +
                QString::fromStdString("circumferenceA_px") + delim +
                QString::fromStdString("circumferenceB_px") + delim +
                QString::fromStdString("confidenceA") + delim +
                QString::fromStdString("outlineConfidenceA") + delim +
                QString::fromStdString("confidenceB") + delim +
                QString::fromStdString("outlineConfidenceB") + delim +
                QString::fromStdString("trial") + delim +
                QString::fromStdString("temperature_c")
            ; // GB TODO: check temp
    //        //break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            return
                QString::fromStdString("filename") + delim +
                QString::fromStdString("timestamp_ms") + delim +
                QString::fromStdString("algorithm") + delim +
                QString::fromStdString("diameterMain_px") + delim +
                QString::fromStdString("diameterSec_px") + delim +
                QString::fromStdString("undistortedDiameterMain_px") + delim +
                QString::fromStdString("undistortedDiameterSec_px") + delim +
                QString::fromStdString("physicalDiameter_mm") + delim +
                QString::fromStdString("widthMain_px") + delim +
                QString::fromStdString("heightMain_px") + delim +
                QString::fromStdString("axisRatioMain") + delim +
                QString::fromStdString("widthSec_px") + delim +
                QString::fromStdString("heightSec_px") + delim +
                QString::fromStdString("axisRatioSec") + delim +
                QString::fromStdString("centerMain_x") + delim +
                QString::fromStdString("centerMain_y") + delim +
                QString::fromStdString("centerSec_x") + delim +
                QString::fromStdString("centerSec_y") + delim +
                QString::fromStdString("angleMain_deg") + delim +
                QString::fromStdString("angleSec_deg") + delim +
                QString::fromStdString("circumferenceMain_px") + delim +
                QString::fromStdString("circumferenceSec_px") + delim +
                QString::fromStdString("confidenceMain") + delim +
                QString::fromStdString("outlineConfidenceMain") + delim +
                QString::fromStdString("confidenceSec") + delim +
                QString::fromStdString("outlineConfidenceSec") + delim +
                QString::fromStdString("trial") + delim +
                QString::fromStdString("temperatureMain_c") + delim +
                QString::fromStdString("temperatureSec_c")
            ;
    //        //break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            return
                QString::fromStdString("filename") + delim +
                QString::fromStdString("timestamp_ms") + delim +
                QString::fromStdString("algorithm") + delim +
                //
                QString::fromStdString("diameterAMain_px") + delim +
                QString::fromStdString("diameterASec_px") + delim +
                //
                QString::fromStdString("diameterBMain_px") + delim +
                QString::fromStdString("diameterBSec_px") + delim +
                //
                QString::fromStdString("undistortedDiameterAMain_px") + delim +
                QString::fromStdString("undistortedDiameterASec_px") + delim +
                //
                QString::fromStdString("undistortedDiameterBMain_px") + delim +
                QString::fromStdString("undistortedDiameterBSec_px") + delim +
                //
                QString::fromStdString("physicalDiameterA_mm") + delim +
                //
                QString::fromStdString("physicalDiameterB_mm") + delim +
                //
                QString::fromStdString("widthAMain_px") + delim +
                QString::fromStdString("heightAMain_px") + delim +
                QString::fromStdString("axisRatioAMain") + delim +
                QString::fromStdString("widthASec_px") + delim +
                QString::fromStdString("heightASec_px") + delim +
                QString::fromStdString("axisRatioASec") + delim +
                //
                QString::fromStdString("widthBMain_px") + delim +
                QString::fromStdString("heightBMain_px") + delim +
                QString::fromStdString("axisRatioBMain") + delim +
                QString::fromStdString("widthBSec_px") + delim +
                QString::fromStdString("heightBSec_px") + delim +
                QString::fromStdString("axisRatioBSec") + delim +
                //
                QString::fromStdString("centerAMain_x") + delim +
                QString::fromStdString("centerAMain_y") + delim +
                QString::fromStdString("centerASec_x") + delim +
                QString::fromStdString("centerASec_y") + delim +
                //
                QString::fromStdString("centerBMain_x") + delim +
                QString::fromStdString("centerBMain_y") + delim +
                QString::fromStdString("centerBSec_x") + delim +
                QString::fromStdString("centerBSec_y") + delim +
                //
                QString::fromStdString("angleAMain_deg") + delim +
                QString::fromStdString("angleASec_deg") + delim +
                //
                QString::fromStdString("angleBMain_deg") + delim +
                QString::fromStdString("angleBSec_deg") + delim +
                //
                QString::fromStdString("circumferenceAMain_px") + delim +
                QString::fromStdString("circumferenceASec_px") + delim +
                //
                QString::fromStdString("circumferenceBMain_px") + delim +
                QString::fromStdString("circumferenceBSec_px") + delim +
                //
                QString::fromStdString("confidenceAMain") + delim +
                QString::fromStdString("outlineConfidenceAMain") + delim + 
                QString::fromStdString("confidenceASec") + delim +
                QString::fromStdString("outlineConfidenceASec") + delim +
                //
                QString::fromStdString("confidenceBMain") + delim +
                QString::fromStdString("outlineConfidenceBMain") + delim + 
                QString::fromStdString("confidenceBSec") + delim +
                QString::fromStdString("outlineConfidenceBSec") + delim +
                //
                QString::fromStdString("trial") + delim +
                //
                QString::fromStdString("temperatureMain_c") + delim +
                QString::fromStdString("temperatureSec_c")
            ;
            // GB TODO: TEMP CHECK
    //        break;
        
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     // NOTE: even though mirr image data comes from one camera, now we have different fields for temperature checks, 
        //     // no problem, just use the same value
        
        default:
            return QString("PROCESSING MODE UNDETERMINED");
    }
}

// Converts a pupil detection to a string row that is written to file
// CAUTION: This must exactly reproduce the format defined by the header fields
QString EyeDataSerializer::pupilToRowCSV(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, QChar delim, const std::vector<double> &temperatures) {

    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    switch((ProcMode)procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            return 
                filename + delim + 
                QString::number(timestamp) + delim + 
                QString::fromStdString(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].algorithmName) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].diameter()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].undistortedDiameter)  + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].physicalDiameter) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].width()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].height()) + delim + 
                QString::number((double)Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].width() / Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].height()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].center.x) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].center.y) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].angle) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].circumference()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].confidence) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].outline_confidence) + delim + 
                QString::number(trialNum) + delim + 
                QString::number(temperatures[0])
            ;
            //break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            return 
                filename + delim + 
                QString::number(timestamp) + delim + 
                QString::fromStdString(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].algorithmName) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].diameter()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].diameter()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].undistortedDiameter) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].undistortedDiameter) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].physicalDiameter) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].physicalDiameter) + delim + // HERE ONLY THIS LINE IS THE DIFFERENCE
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].width()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].height()) + delim + 
                QString::number((double)Pupils[SINGLE_IMAGE_TWO_PUPIL_A].width() / Pupils[SINGLE_IMAGE_TWO_PUPIL_A].height()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].width()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].height()) + delim + 
                QString::number((double)Pupils[SINGLE_IMAGE_TWO_PUPIL_B].width() / Pupils[SINGLE_IMAGE_TWO_PUPIL_B].height()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].center.x) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].center.y) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].center.x) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].center.y) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].angle) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].angle) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].circumference()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].circumference()) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].confidence) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].outline_confidence) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].confidence) + delim + 
                QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].outline_confidence) + delim + 
                QString::number(trialNum) + delim +
                QString::number(temperatures[0])
            ;
            // GB TODO: TEMP CHECK!
            //break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            return 
                filename + delim + 
                QString::number(timestamp) + delim + 
                QString::fromStdString(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].algorithmName) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].diameter()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].diameter()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].undistortedDiameter) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].undistortedDiameter) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].physicalDiameter) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].width()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].height()) + delim + 
                QString::number((double)Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].width() / Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].height()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].width()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].height()) + delim + 
                QString::number((double)Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].width() / Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].height()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].center.x) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].center.y) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].center.x) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].center.y) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].angle) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].angle) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].circumference()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].circumference()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].outline_confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].outline_confidence) + delim + 
                QString::number(trialNum) + delim +
                QString::number(temperatures[0]) + delim +
                QString::number(temperatures[1])
            ;
            // GB TODO: TEMP CHECK!
            //break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            return 
                filename + delim + 
                QString::number(timestamp) + delim + 
                QString::fromStdString(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].algorithmName) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].diameter()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].diameter()) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].diameter()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].diameter()) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].undistortedDiameter) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].undistortedDiameter) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].undistortedDiameter) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].undistortedDiameter) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].physicalDiameter) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].physicalDiameter) + delim +
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].width()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].height()) + delim + 
                QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].height()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].width()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].height()) + delim + 
                QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].height()) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].width()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].height()) + delim + 
                QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].height()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].width()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].height()) + delim + 
                QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].height()) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].center.x) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].center.y) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].center.x) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].center.y) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].center.x) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].center.y) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].center.x) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].center.y) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].angle) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].angle) + delim + 

                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].angle) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].angle) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].circumference()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].circumference()) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].circumference()) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].circumference()) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].outline_confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].outline_confidence) + delim + 
                
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].outline_confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].confidence) + delim + 
                QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].outline_confidence) + delim + 
                
                QString::number(trialNum) + delim +
                QString::number(temperatures[0]) + delim +
                QString::number(temperatures[1])
            ;
            // GB TODO: TEMP CHECK
            //break;
        
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     return 
        //         filename + delim + 
        //         QString::number(timestamp) + delim + 
        //         QString::fromStdString(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].algorithmName) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].diameter()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].diameter()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].undistortedDiameter) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].undistortedDiameter) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].physicalDiameter) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].width()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].height()) + delim + 
        //         QString::number((double)Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].width() / Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].height()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].width()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].height()) + delim + 
        //         QString::number((double)Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].width() / Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].height()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].center.x) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].center.y) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].center.x) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].center.y) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].angle) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].angle) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].circumference()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].circumference()) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].confidence) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_MAIN].outline_confidence) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].confidence) + delim + 
        //         QString::number(Pupils[MIRR_IMAGE_ONE_PUPIL_SEC].outline_confidence) + delim + 
        //         QString::number(trialNum) + delim +
        //         QString::number(temperatures[0]) + delim +
        //         QString::number(temperatures[0])
        //     ;
        //     // GB TODO: TEMP CHECK!
        //     //break;
        
        default:
            return QString(" ");
    }

}


QString EyeDataSerializer::pupilToYAML(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const std::vector<double> &temperatures) {
    

    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    QString obj;

    addRowYAML(obj, "EyeData", "", 0, false);

    switch((ProcMode)procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, SINGLE_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, SINGLE_IMAGE_TWO_PUPIL_A, Pupils, filename, trialNum, temperatures[0]);
            addRowYAML(obj, "B", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, SINGLE_IMAGE_TWO_PUPIL_B, Pupils, filename, trialNum, temperatures[0]);
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Sec", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[1]);
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_A_MAIN, Pupils, filename, trialNum, temperatures[0]);
            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Sec", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_A_SEC, Pupils, filename, trialNum, temperatures[1]);

            addRowYAML(obj, "B", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_B_MAIN, Pupils, filename, trialNum, temperatures[0]);
            addRowYAML(obj, "B", "", 1, false);
            addRowYAML(obj, "Sec", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_B_SEC, Pupils, filename, trialNum, temperatures[1]);
            break;
            
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:

        //     addRowYAML(obj, "A", "", 1, false);
        //     addRowYAML(obj, "Main", "", 2, false);
        //     populatePupilNodeYAML(timestamp, obj, 3, MIRR_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0]);
        //     addRowYAML(obj, "A", "", 1, false);
        //     addRowYAML(obj, "Sec", "", 2, false);
        //     populatePupilNodeYAML(timestamp, obj, 3, MIRR_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[0]);
        //     break;
        
        //default:
            //break;
    }

    return obj;
}

void EyeDataSerializer::populatePupilNodeYAML(quint64 &timestamp, QString &obj, ushort depth, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, double temperature) {

    depth+=1;
    addRowYAML(obj, "filename", filename, depth, true);
    addRowYAML(obj, "timestamp_ms", QString::number(timestamp), depth, true);
    addRowYAML(obj, "algorithm", QString::fromStdString(Pupils[idx].algorithmName), depth, true);
    addRowYAML(obj, "diameter_px", QString::number(Pupils[idx].diameter()), depth, true);
    addRowYAML(obj, "undistortedDiameter_px", QString::number(Pupils[idx].undistortedDiameter), depth, true);
    addRowYAML(obj, "physicalDiameter_mm", QString::number(Pupils[idx].physicalDiameter), depth, true);
    addRowYAML(obj, "width_px", QString::number(Pupils[idx].width()), depth, true);
    addRowYAML(obj, "height_px", QString::number(Pupils[idx].height()), depth, true);
    addRowYAML(obj, "axisRatio_px", QString::number((double)Pupils[idx].width() / Pupils[idx].height()), depth, true);
    addRowYAML(obj, "center_x", QString::number(Pupils[idx].center.x), depth, true);
    addRowYAML(obj, "center_y", QString::number(Pupils[idx].center.y), depth, true);
    addRowYAML(obj, "angle_deg", QString::number(Pupils[idx].angle), depth, true);
    addRowYAML(obj, "circumference_px", QString::number(Pupils[idx].circumference()), depth, true);
    addRowYAML(obj, "confidence", QString::number(Pupils[idx].confidence), depth, true);
    addRowYAML(obj, "outlineConfidence", QString::number(Pupils[idx].outline_confidence), depth, true);
    addRowYAML(obj, "trial", QString::number(trialNum), depth, true);
    addRowYAML(obj, "temperature_c", QString::number(temperature), depth, true);
}

void EyeDataSerializer::addRowYAML(QString &obj, QString key, QString value, ushort depth, bool isLeaf) {
    
    // object:
    //     key: value
    //     array:
    //         - null_value:
    //         - boolean: true
    //         - integer: 1
    //         - alias: value
    
    for(ushort c=0; c<depth; c++) {
        obj = obj % "  ";
    }

    //if(isLeaf)
    //    obj = obj % "- " % key % ": " % value % '\n';
    //else
        obj = obj % key % ": " % value % '\n';
    
}


