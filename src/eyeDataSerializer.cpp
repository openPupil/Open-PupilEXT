
#include "eyeDataSerializer.h"

void EyeDataSerializer::populatePupilNodeXML(quint64 &timestamp, QDomElement &dObj, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, const QString& message, double temperature) {
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
    dObj.setAttribute("message", message);
    dObj.setAttribute("temperature_c", QString::number(temperature));
}

QString EyeDataSerializer::pupilToXML(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const QString& message, const std::vector<double> &temperatures) {
    
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
            populatePupilNodeXML(timestamp, viewObjAMain, SINGLE_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            pupilObjA.appendChild(viewObjAMain);
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            pupilObjA = document.createElement("A");
            pupilObjB = document.createElement("B");
            root.appendChild(pupilObjA);
            root.appendChild(pupilObjB);
            viewObjAMain = document.createElement("Main");
            viewObjBMain = document.createElement("Main");
            populatePupilNodeXML(timestamp, viewObjAMain, SINGLE_IMAGE_TWO_PUPIL_A, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjBMain, SINGLE_IMAGE_TWO_PUPIL_B, Pupils, filename, trialNum, message, temperatures[0]);
            pupilObjA.appendChild(viewObjAMain);
            pupilObjB.appendChild(viewObjBMain);
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            pupilObjA = document.createElement("A");
            root.appendChild(pupilObjA);
            viewObjAMain = document.createElement("Main");
            viewObjASec = document.createElement("Sec");
            populatePupilNodeXML(timestamp, viewObjAMain, STEREO_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjASec, STEREO_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, message, temperatures[1]);
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
            populatePupilNodeXML(timestamp, viewObjAMain, STEREO_IMAGE_TWO_PUPIL_A_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjASec, STEREO_IMAGE_TWO_PUPIL_A_SEC, Pupils, filename, trialNum, message, temperatures[1]);
            populatePupilNodeXML(timestamp, viewObjBMain, STEREO_IMAGE_TWO_PUPIL_B_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeXML(timestamp, viewObjBSec, STEREO_IMAGE_TWO_PUPIL_B_SEC, Pupils, filename, trialNum, message, temperatures[1]);
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
        //     populatePupilNodeXML(timestamp, viewObjAMain, MIRR_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0], message);
        //     populatePupilNodeXML(timestamp, viewObjASec, MIRR_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[0], message);
        //     pupilObjA.appendChild(viewObjAMain);
        //     pupilObjA.appendChild(viewObjASec);
        //     break;

        //default:
            //break;
    }

    return document.toString();
}


void EyeDataSerializer::populatePupilNodeJSON(quint64 &timestamp, QJsonObject &dObj, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, const QString& message, double temperature) {
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
    dObj["message"] = message;
    dObj["temperature_c"] = QString::number(temperature);
}

QString EyeDataSerializer::pupilToJSON(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const QString& message, const std::vector<double> &temperatures) {
    
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
            populatePupilNodeJSON(timestamp, viewObjAMain, SINGLE_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            pupilObjA["Main"] = viewObjAMain;
            root["A"] = pupilObjA;
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, SINGLE_IMAGE_TWO_PUPIL_A, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjBMain, SINGLE_IMAGE_TWO_PUPIL_B, Pupils, filename, trialNum, message, temperatures[0]);
            pupilObjA["Main"] = viewObjAMain;
            root["A"] = pupilObjA;
            pupilObjB["Main"] = viewObjBMain;
            root["B"] = pupilObjB;
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, STEREO_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjASec, STEREO_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, message, temperatures[1]);
            pupilObjA["Main"] = viewObjAMain;
            pupilObjA["Sec"] = viewObjASec;
            root["A"] = pupilObjA;
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            populatePupilNodeJSON(timestamp, viewObjAMain, STEREO_IMAGE_TWO_PUPIL_A_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjASec, STEREO_IMAGE_TWO_PUPIL_A_SEC, Pupils, filename, trialNum, message, temperatures[1]);
            populatePupilNodeJSON(timestamp, viewObjBMain, STEREO_IMAGE_TWO_PUPIL_B_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            populatePupilNodeJSON(timestamp, viewObjBSec, STEREO_IMAGE_TWO_PUPIL_B_SEC, Pupils, filename, trialNum, message, temperatures[1]);
            pupilObjA["Main"] = viewObjAMain;
            pupilObjA["Sec"] = viewObjASec;
            root["A"] = pupilObjA;
            pupilObjB["Main"] = viewObjBMain;
            pupilObjB["Sec"] = viewObjBSec;
            root["B"] = pupilObjB;
            break;
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     populatePupilNodeJSON(timestamp, viewObjAMain, MIRR_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0], message);
        //     populatePupilNodeJSON(timestamp, viewObjASec, MIRR_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[0], message);
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

QString EyeDataSerializer::getHeaderCSV(int procMode, QChar delim, DataWriterDataStyle dataStyle) {

    QString result; // TODO: .reserve() ?

    switch(procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % "filename" % delim;
            }
            result = result % "timestamp_ms" % delim;
            result = result % "algorithm" % delim;
            result = result % "diameter_px" % delim;
            result = result % "undistortedDiameter_px" % delim;
            result = result % "physicalDiameter_mm" % delim;
            result = result % "width_px" % delim;
            result = result % "height_px" % delim;
            result = result % "axisRatio" % delim;
            result = result % "center_x" % delim;
            result = result % "center_y" % delim;
            result = result % "angle_deg" % delim;
            result = result % "circumference_px" % delim;
            result = result % "confidence" % delim;
            result = result % "outlineConfidence";
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % "trial" % delim;
                result = result % "message" % delim;
                result = result % "temperature_c";
            }
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % "filename" % delim;
            }
            result = result % "timestamp_ms" % delim;
            result = result % "algorithm" % delim;
            result = result % "diameterA_px" % delim;
            result = result % "diameterB_px" % delim;
            result = result % "undistortedDiameterA_px" % delim;
            result = result % "undistortedDiameterB_px" % delim;
            result = result % "physicalDiameterA_mm" % delim;
            result = result % "physicalDiameterB_mm" % delim; // the different line
            result = result % "widthA_px" % delim;
            result = result % "heightA_px" % delim;
            result = result % "axisRatioA" % delim;
            result = result % "widthB_px" % delim;
            result = result % "heightB_px" % delim;
            result = result % "axisRatioB" % delim;
            result = result % "centerA_x" % delim;
            result = result % "centerA_y" % delim;
            result = result % "centerB_x" % delim;
            result = result % "centerB_y" % delim;
            result = result % "angleA_deg" % delim;
            result = result % "angleB_deg" % delim;
            result = result % "circumferenceA_px" % delim;
            result = result % "circumferenceB_px" % delim;
            result = result % "confidenceA" % delim;
            result = result % "outlineConfidenceA" % delim;
            result = result % "confidenceB" % delim;
            result = result % "outlineConfidenceB";
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % "trial" % delim;
                result = result % "message" % delim;
                result = result % "temperature_c";
            }
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % "filename" % delim;
            }
            result = result % "timestamp_ms" % delim;
            result = result % "algorithm" % delim;
            result = result % "diameterMain_px" % delim;
            result = result % "diameterSec_px" % delim;
            result = result % "undistortedDiameterMain_px" % delim;
            result = result % "undistortedDiameterSec_px" % delim;
            result = result % "physicalDiameter_mm" % delim;
            result = result % "widthMain_px" % delim;
            result = result % "heightMain_px" % delim;
            result = result % "axisRatioMain" % delim;
            result = result % "widthSec_px" % delim;
            result = result % "heightSec_px" % delim;
            result = result % "axisRatioSec" % delim;
            result = result % "centerMain_x" % delim;
            result = result % "centerMain_y" % delim;
            result = result % "centerSec_x" % delim;
            result = result % "centerSec_y" % delim;
            result = result % "angleMain_deg" % delim;
            result = result % "angleSec_deg" % delim;
            result = result % "circumferenceMain_px" % delim;
            result = result % "circumferenceSec_px" % delim;
            result = result % "confidenceMain" % delim;
            result = result % "outlineConfidenceMain" % delim;
            result = result % "confidenceSec" % delim;
            result = result % "outlineConfidenceSec";
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % "trial" % delim;
                result = result % "message" % delim;
                result = result % "temperatureMain_c" % delim;
                result = result % "temperatureSec_c";
            }
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % "filename" % delim;
            }
            result = result % "timestamp_ms" % delim;
            result = result % "algorithm" % delim; //
            result = result % "diameterAMain_px" % delim;
            result = result % "diameterASec_px" % delim; //
            result = result % "diameterBMain_px" % delim;
            result = result % "diameterBSec_px" % delim; //
            result = result % "undistortedDiameterAMain_px" % delim;
            result = result % "undistortedDiameterASec_px" % delim; //
            result = result % "undistortedDiameterBMain_px" % delim;
            result = result % "undistortedDiameterBSec_px" % delim; //
            result = result % "physicalDiameterA_mm" % delim; //
            result = result % "physicalDiameterB_mm" % delim; //
            result = result % "widthAMain_px" % delim;
            result = result % "heightAMain_px" % delim;
            result = result % "axisRatioAMain" % delim;
            result = result % "widthASec_px" % delim;
            result = result % "heightASec_px" % delim;
            result = result % "axisRatioASec" % delim; //
            result = result % "widthBMain_px" % delim;
            result = result % "heightBMain_px" % delim;
            result = result % "axisRatioBMain" % delim;
            result = result % "widthBSec_px" % delim;
            result = result % "heightBSec_px" % delim;
            result = result % "axisRatioBSec" % delim; //
            result = result % "centerAMain_x" % delim;
            result = result % "centerAMain_y" % delim;
            result = result % "centerASec_x" % delim;
            result = result % "centerASec_y" % delim; //
            result = result % "centerBMain_x" % delim;
            result = result % "centerBMain_y" % delim;
            result = result % "centerBSec_x" % delim;
            result = result % "centerBSec_y" % delim; //
            result = result % "angleAMain_deg" % delim;
            result = result % "angleASec_deg" % delim; //
            result = result % "angleBMain_deg" % delim;
            result = result % "angleBSec_deg" % delim; //
            result = result % "circumferenceAMain_px" % delim;
            result = result % "circumferenceASec_px" % delim; //
            result = result % "circumferenceBMain_px" % delim;
            result = result % "circumferenceBSec_px" % delim; //
            result = result % "confidenceAMain" % delim;
            result = result % "outlineConfidenceAMain" % delim;
            result = result % "confidenceASec" % delim;
            result = result % "outlineConfidenceASec" % delim; //
            result = result % "confidenceBMain" % delim;
            result = result % "outlineConfidenceBMain" % delim;
            result = result % "confidenceBSec" % delim;
            result = result % "outlineConfidenceBSec"; //
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % "trial" % delim; //
                result = result % "message" % delim;
                result = result % "temperatureMain_c" % delim;
                result = result % "temperatureSec_c";
            }
            break;
        
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        //     // NOTE: even though mirr image data comes from one camera, now we have different fields for temperature checks, 
        //     // no problem, just use the same value
        
        default:
            result = QString("PROCESSING MODE UNDETERMINED");
    }
    return result;
}

// Converts a pupil detection to a string row that is written to file
// CAUTION: This must exactly reproduce the format defined by the header fields
QString EyeDataSerializer::pupilToRowCSV(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, QChar delim, DataWriterDataStyle dataStyle, const QString& message, const std::vector<double> &temperatures) {

    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    QString result; // TODO: .reserve() ?

    switch((ProcMode)procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % filename % delim;
            }
            result = result % QString::number(timestamp) % delim;
            result = result % QString::fromStdString(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].algorithmName) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].diameter()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].undistortedDiameter)  % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].physicalDiameter) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].width()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].height()) % delim;
            result = result % QString::number((double)Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].width() / Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].height()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].center.x) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].center.y) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].angle) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].circumference()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].confidence) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_ONE_PUPIL_MAIN].outline_confidence);
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result %  delim % QString::number(trialNum) % delim;
                result = result % message % delim;
                result = result % QString::number(temperatures[0]);
            }
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % filename % delim;
            }
            result = result % QString::number(timestamp) % delim;
            result = result % QString::fromStdString(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].algorithmName) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].diameter()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].diameter()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].undistortedDiameter) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].undistortedDiameter) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].physicalDiameter) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].physicalDiameter) % delim; // HERE ONLY THIS LINE IS THE DIFFERENCE
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].width()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].height()) % delim;
            result = result % QString::number((double)Pupils[SINGLE_IMAGE_TWO_PUPIL_A].width() / Pupils[SINGLE_IMAGE_TWO_PUPIL_A].height()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].width()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].height()) % delim;
            result = result % QString::number((double)Pupils[SINGLE_IMAGE_TWO_PUPIL_B].width() / Pupils[SINGLE_IMAGE_TWO_PUPIL_B].height()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].center.x) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].center.y) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].center.x) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].center.y) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].angle) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].angle) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].circumference()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].circumference()) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].confidence) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_A].outline_confidence) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].confidence) % delim;
            result = result % QString::number(Pupils[SINGLE_IMAGE_TWO_PUPIL_B].outline_confidence);
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % QString::number(trialNum) % delim;
                result = result % message % delim;
                result = result % QString::number(temperatures[0]);
            }
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % filename % delim;
            }
            result = result % QString::number(timestamp) % delim;
            result = result % QString::fromStdString(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].algorithmName) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].diameter()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].diameter()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].undistortedDiameter) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].undistortedDiameter) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].physicalDiameter) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].width()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].height()) % delim;
            result = result % QString::number((double)Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].width() / Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].height()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].width()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].height()) % delim;
            result = result % QString::number((double)Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].width() / Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].height()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].center.x) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].center.y) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].center.x) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].center.y) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].angle) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].angle) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].circumference()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].circumference()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_MAIN].outline_confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_ONE_PUPIL_SEC].outline_confidence);
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % QString::number(trialNum) % delim;
                result = result % message % delim;
                result = result % QString::number(temperatures[0]) % delim;
                result = result % QString::number(temperatures[1]);
            }
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_1) {
                result = result % filename % delim;
            }
            result = result % QString::number(timestamp) % delim;
            result = result % QString::fromStdString(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].algorithmName) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].diameter()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].diameter()) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].diameter()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].diameter()) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].undistortedDiameter) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].undistortedDiameter) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].undistortedDiameter) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].undistortedDiameter) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].physicalDiameter) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].physicalDiameter) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].width()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].height()) % delim;
            result = result % QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].height()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].width()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].height()) % delim;
            result = result % QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].height()) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].width()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].height()) % delim;
            result = result % QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].height()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].width()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].height()) % delim;
            result = result % QString::number((double)Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].width() / Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].height()) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].center.x) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].center.y) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].center.x) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].center.y) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].center.x) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].center.y) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].center.x) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].center.y) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].angle) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].angle) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].angle) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].angle) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].circumference()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].circumference()) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].circumference()) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].circumference()) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_MAIN].outline_confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_A_SEC].outline_confidence) % delim; //
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_MAIN].outline_confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].confidence) % delim;
            result = result % QString::number(Pupils[STEREO_IMAGE_TWO_PUPIL_B_SEC].outline_confidence); //
            if(dataStyle == DataWriterDataStyle::PUPILEXT_V0_1_2) {
                result = result % delim % QString::number(trialNum) % delim;
                result = result % message % delim;
                result = result % QString::number(temperatures[0]) % delim;
                result = result % QString::number(temperatures[1]);
            }
            break;
        
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
        //     //break;
        
        default:
            result = QString(" ");
    }
    return result;
}


QString EyeDataSerializer::pupilToYAML(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filepath, uint trialNum, const QString& message, const std::vector<double> &temperatures) {
    

    QString filename = "-1";
    if(!filepath.isEmpty())
        filename = QFileInfo(filepath).fileName();

    QString obj;

    addRowYAML(obj, "EyeData", "", 0, false);

    switch((ProcMode)procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, SINGLE_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, SINGLE_IMAGE_TWO_PUPIL_A, Pupils, filename, trialNum, message, temperatures[0]);
            addRowYAML(obj, "B", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, SINGLE_IMAGE_TWO_PUPIL_B, Pupils, filename, trialNum, message, temperatures[0]);
            break;
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Sec", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, message, temperatures[1]);
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:

            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_A_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            addRowYAML(obj, "A", "", 1, false);
            addRowYAML(obj, "Sec", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_A_SEC, Pupils, filename, trialNum, message, temperatures[1]);

            addRowYAML(obj, "B", "", 1, false);
            addRowYAML(obj, "Main", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_B_MAIN, Pupils, filename, trialNum, message, temperatures[0]);
            addRowYAML(obj, "B", "", 1, false);
            addRowYAML(obj, "Sec", "", 2, false);
            populatePupilNodeYAML(timestamp, obj, 3, STEREO_IMAGE_TWO_PUPIL_B_SEC, Pupils, filename, trialNum, message, temperatures[1]);
            break;
            
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:

        //     addRowYAML(obj, "A", "", 1, false);
        //     addRowYAML(obj, "Main", "", 2, false);
        //     populatePupilNodeYAML(timestamp, obj, 3, MIRR_IMAGE_ONE_PUPIL_MAIN, Pupils, filename, trialNum, temperatures[0], message);
        //     addRowYAML(obj, "A", "", 1, false);
        //     addRowYAML(obj, "Sec", "", 2, false);
        //     populatePupilNodeYAML(timestamp, obj, 3, MIRR_IMAGE_ONE_PUPIL_SEC, Pupils, filename, trialNum, temperatures[0], message);
        //     break;
        
        //default:
            //break;
    }

    return obj;
}

void EyeDataSerializer::populatePupilNodeYAML(quint64 &timestamp, QString &obj, ushort depth, int idx, const std::vector<Pupil> &Pupils, const QString &filename, uint &trialNum, const QString& message, double temperature) {

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
    addRowYAML(obj, "message", message, depth, true);
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


