
#include "mainwindow.h"


void MainWindow::PRGincrementTrialCounter(const quint64 &timestamp) {
    incrementTrialCounter(timestamp);
}

void MainWindow::PRGforceResetTrialCounter(const quint64 &timestamp) {
    forceResetTrialCounter(timestamp);
}

void MainWindow::PRGlogRemoteMessage(const quint64 &timestamp, const QString &str) {
    logRemoteMessage(timestamp, str);
}

void MainWindow::PRGopenSingleCamera(const QString &camName) {
    if(selectedCamera && selectedCamera->isOpen())
        return;

    Pylon::DeviceInfoList_t lstDevices = enumerateCameraDevices();
    QString temp = "";
    if (!lstDevices.empty()) {
        Pylon::DeviceInfoList_t::const_iterator deviceIt;
        bool foundGiven = false;

        for(deviceIt = lstDevices.begin(); deviceIt != lstDevices.end(); ++deviceIt ) {
            //std::cout << "Found camera full name: " << deviceIt->GetFullName().c_str() << std::endl;
            //std::cout << "Found camera friendly name: " << deviceIt->GetFriendlyName().c_str() << std::endl;
            if(QString::fromStdString(deviceIt->GetFriendlyName().c_str()).toLower() == camName) { // NOTE: we search for friendly name, but connection can be initiated with full name
                temp = QString::fromStdString(deviceIt->GetFullName().c_str());
                foundGiven = true;
            }
        }
        if(!foundGiven) {
            std::cout << "Could not find the specified camera" << std::endl;
            return;
        }
    }
    else {
        std::cout << "Could not find the specified camera. Pylon library sees that there are no Basler cameras connecter currently" << std::endl;
        return;
    }
    cameraAct->setData(temp);
    singleCameraSelected(cameraAct);
}

void MainWindow::PRGopenStereoCamera(const QString &camName1, const QString &camName2) {
    if(selectedCamera && selectedCamera->isOpen())
        return;

    // GB: UNDER DEV
    stereoCameraSettingsDialog->openStereoCamera(camName1, camName2);
}

void MainWindow::PRGcloseCamera() {
    if(selectedCamera)
        onCameraDisconnectClick();
}

void MainWindow::PRGsetOutPath(const QString &str) {
    //std::cout << "received path = " << fullPath.toStdString() << std::endl;
    QString fullPath = SupportFunctions::simplifyPathName(str);
    // NOTE: multiple "/" characters, like "//" are not changed, as probably files are saved using an URI
    if(fullPath == "") 
        return;

    // if path does not end with a "/" character, put it to the end
    if(fullPath[fullPath.length()-1] != '/')
        fullPath = fullPath + '/';

    //std::cout << "reformed path = " << fullPath.toStdString() << std::endl;

    outputDirectory.clear();
    outputDirectory = fullPath;
    //std::cout << "outputDirectory (image dir) after image dir setting = " << outputDirectory.toStdString() << std::endl;

    // NOTE: recentPath is NOT set programmatically, as we can not ensure that the path exists at this point,
    // which could later crash the GUI or whatever
    if(selectedCamera && selectedCamera->getType()!=SINGLE_IMAGE_FILE && selectedCamera->getType()!=STEREO_IMAGE_FILE)
        recordImagesAct->setDisabled(false);
}

void MainWindow::PRGsetCsvPathAndName(const QString &str) {
    //std::cout << "received path = " << fullPathAndName.toStdString() << std::endl;
    QString fullPathAndName = SupportFunctions::simplifyPathName(str);
    if(fullPathAndName == "")
        return;
        
    pupilDetectionDataFile.clear();
    pupilDetectionDataFile = fullPathAndName;

    QFileInfo fileInfo(pupilDetectionDataFile);
    // NOTE: recentPath is NOT set programmatically, as we can not ensure that the path exists at this point,
    // which could later crash the GUI or whatever

    // check if filename has extension
    if(fileInfo.suffix().isEmpty())
        pupilDetectionDataFile = pupilDetectionDataFile + ".csv";
    //std::cout << "saved logfilename (and path) after name-csv setting = " << logFileName.toStdString() << std::endl;

    if(trackingOn)
        recordAct->setDisabled(false);
}

void MainWindow::PRGtrackStart() {
    if(!selectedCamera)
        return;
    if(!trackingOn)
        onTrackActClick();
}

void MainWindow::PRGtrackStop() {
    // order is important
    if(streamOn)
        onStreamClick();
    if(recordOn)
        onRecordClick();
    if(recordImagesOn)
        onRecordImageClick();
    if(trackingOn)
        onTrackActClick();
}

void MainWindow::PRGstreamStart() {
    if(!selectedCamera || pupilDetectionDataFile.isEmpty())
        return;
    if(!trackingOn) // we can start tracking if only that is needed
        onTrackActClick();
    if(!streamOn)
        onStreamClick();
}

void MainWindow::PRGstreamStop() {
    if(streamOn)
        onStreamClick();
}

void MainWindow::PRGrecordStart() {
    if(!selectedCamera || pupilDetectionDataFile.isEmpty())
        return;
    if(!trackingOn) // we can start tracking if only that is needed
        onTrackActClick();
    if(!recordOn)
        onRecordClick();
}

void MainWindow::PRGrecordStop() {
    if(recordOn)
        onRecordClick();
}

void MainWindow::PRGrecordImageStart() {
    if(selectedCamera && selectedCamera->getType() != SINGLE_IMAGE_FILE && selectedCamera->getType() != STEREO_IMAGE_FILE && !outputDirectory.isEmpty() && !recordImagesOn)
        onRecordImageClick();
}

void MainWindow::PRGrecordImageStop() {
    if(recordImagesOn)
        onRecordImageClick();
}

void MainWindow::PRGopenSingleWebcam(int deviceID) {
    if(selectedCamera && selectedCamera->isOpen())
        return;

    QAction *action = new QAction();
    action->setData(deviceID);
    singleWebcamSelected(action);
}

/*void MainWindow::PRGincrementTrial() {
    if(dataStreamer)
        dataStreamer->incrementTrial();
    if(dataWriter)
        dataWriter->incrementTrial();
    if(offlineEventLogWriter && imageWriter)
        offlineEventLogWriter->incrementTrial();
}*/

void MainWindow::PRGsetGlobalDelimiter(const QString &str) {
    if( dataWriter || imageWriter || dataStreamer )
        return;

    QString value = ",";
    if(str=="," || str=="comma")
        value=",";
    else if(str==";" || str=="semicolon")
        value=";";
    if(str=="\t" || str=="tab" || str=="tabulator" || str == "tabulation")
        value="\t";
    
    applicationSettings->setValue("dataWriterDelimiter", value);
}

void MainWindow::PRGsetImageOutputFormat(QString format) {
    if(imageWriter)
        return;

    format.replace(".", "");
    if(format=="tiff" || format=="tif" || format =="png"  || format =="bmp" || format=="jpeg" || format=="jpg" ||  format=="webp" || format=="pgm") {
        if(format=="tif")
            format="tiff";
        else if(format=="jpg")
            format="jpeg";
        applicationSettings->setValue("imageWriterFormat.chosenFormat", format);
    }
}

void MainWindow::PRGsetPupilDetectionAlgorithm(const QString &alg) {
    if( alg == "else" || alg == "excuse" || alg == "pure" || alg == "purest" || alg == "starburst" || alg == "swirski2d" ) {
        pupilDetectionWorker->setAlgorithm(alg);
        applicationSettings->setValue("PupilDetectionSettingsDialog.algorithm", alg);
    }
}

void MainWindow::PRGsetPupilDetectionUsingROI(const QString &state) {
    if(state == "true" || state == "1") {
        pupilDetectionWorker->enableROIPreProcessing(true);
        applicationSettings->setValue("PupilDetectionSettingsDialog.processROI", true);
    } else if(state == "false" || state == "0") {
        pupilDetectionWorker->enableROIPreProcessing(false);
        applicationSettings->setValue("PupilDetectionSettingsDialog.processROI", false);
    }
}

void MainWindow::PRGsetPupilDetectionCompOutlineConf(const QString &state) {
    if(state == "true" || state == "1") {
        pupilDetectionWorker->enableOutlineConfidence(true);
        applicationSettings->setValue("PupilDetectionSettingsDialog.outlineConfidence", true);
    } else if(state == "false" || state == "0") {
        pupilDetectionWorker->enableOutlineConfidence(false);
        applicationSettings->setValue("PupilDetectionSettingsDialog.outlineConfidence", false);
    }
}

void MainWindow::PRGconnectRemoteUDP(QString conf) {
    if(remoteCCDialog->isUDPConnected())
        return;

    QString ip;
    quint16 port;
    ConnPoolUDPInstanceSettings p;
    bool valid;

    conf.replace(" ", "");
    //QStringList subStrings = conf.split(',');
    QRegExp separator("[,|;|*|&|#|:]");
    QStringList subStrings = conf.split(separator);
    if(subStrings.length() < 2)
        return;

    if(subStrings[0].isEmpty())
        return;
    ip = subStrings[0];
        
    valid = false;
    port = (quint16)subStrings[1].toInt(&valid, 10); 
    if(!valid)
        return;

    p.ipAddress = QHostAddress(ip);
    p.portNumber = port;

    // TODO: This way the GUI displayed values will not always be congruent, until they get refreshed/updated deliberately...
    // we could solve this in different ways
    remoteCCDialog->connectUDP(p);
}
void MainWindow::PRGconnectRemoteCOM(QString conf) {
    if(remoteCCDialog->isCOMConnected())
        return;

    ConnPoolCOMInstanceSettings p;
    bool valid;

    conf.replace(" ", "");
    //QStringList subStrings = conf.split(',');
    QRegExp separator("[,|;|*|&|#|:]");
    QStringList subStrings = conf.split(separator);
    if(subStrings.length() < 6)
        return;

    const auto infos = QSerialPortInfo::availablePorts();
    valid = false;
    QString portName = "";
    for(int h=1; h < infos.length(); h++) {
        if(infos[h].portName() == subStrings[0]) {
            portName=subStrings[h];
            valid = true;
            break;
        }
    }
    if(!valid || subStrings[0].isEmpty())
        return;
        
    valid = false;
    qint32 baudRate = subStrings[1].toInt(&valid, 10); 
    if(!valid)
        return;

    valid = false;
    qint32 dataBits = subStrings[2].toInt(&valid, 10); 
    if(!valid)
        return;

    valid = false;
    qint32 parity = subStrings[3].toInt(&valid, 10); 
    if(!valid)
        return;

    // NOTE: only integer values supported now
    valid = false;
    qint32 stopBits = subStrings[4].toInt(&valid, 10); 
    if(!valid)
        return;

    valid = false;
    qint32 flowControl = subStrings[5].toInt(&valid, 10); 
    if(!valid)
        return;

    p.name = portName;
    p.baudRate = baudRate;
    p.stringBaudRate = QString::number(baudRate);
    p.dataBits = (QSerialPort::DataBits)dataBits;
    p.stringDataBits = QString::number(baudRate);
    p.parity = (QSerialPort::Parity)parity;
    p.stringParity = QString::number(parity);
    p.stopBits = (QSerialPort::StopBits)stopBits;
    p.stringStopBits = QString::number(stopBits);
    p.flowControl = (QSerialPort::FlowControl)flowControl;
    p.stringFlowControl = QString::number(flowControl);
    p.localEchoEnabled = true; // always on now

    remoteCCDialog->connectCOM(p);
}
void MainWindow::PRGconnectStreamUDP(QString conf) {
    qDebug() << "streamingSettingsDialog->isUDPConnected()" << streamingSettingsDialog->isUDPConnected();
    if(streamingSettingsDialog->isUDPConnected())
        return;

    QString ip;
    quint16 port;
    ConnPoolUDPInstanceSettings p;
    bool valid;

    conf.replace(" ", "");
    //QStringList subStrings = conf.split(',');
    QRegExp separator("[,|;|*|&|#|:]");
    QStringList subStrings = conf.split(separator);
    if(subStrings.length() < 3)
        return;

    if(subStrings[0].isEmpty())
        return;
    ip = subStrings[0];
        
    valid = false;
    port = (quint16)subStrings[1].toInt(&valid, 10); 
    if(!valid)
        return;

    DataStreamer::DataContainer dataContainer;
    if(subStrings[2].toUpper() == "CSV")
        dataContainer = DataStreamer::DataContainer::CSV;
    else if(subStrings[2].toUpper() == "JSON")
        dataContainer = DataStreamer::DataContainer::JSON;
    else if(subStrings[2].toUpper() == "XML")
        dataContainer = DataStreamer::DataContainer::XML;
    else if(subStrings[2].toUpper() == "YAML")
        dataContainer = DataStreamer::DataContainer::YAML;
    else 
        return;

    p.ipAddress = QHostAddress(ip);
    p.portNumber = port;

    // TODO: This way the GUI displayed values will not always be congruent, until they get refreshed/updated deliberately...
    // we could solve this in different ways
    applicationSettings->setValue("StreamingSettings.UDP.dataContainer", dataContainer);
//    applicationSettings->setValue("StreamingSettings.UDP.ip", ip);
//    applicationSettings->setValue("StreamingSettings.UDP.port", port);

    streamingSettingsDialog->connectUDP(p);
}
void MainWindow::PRGconnectStreamCOM(QString conf) {
    if(streamingSettingsDialog->isCOMConnected())
        return;

    ConnPoolCOMInstanceSettings p;
    bool valid;

    conf.replace(" ", "");
    //QStringList subStrings = conf.split(',');
    QRegExp separator("[,|;|*|&|#|:]");
    QStringList subStrings = conf.split(separator);
    if(subStrings.length() < 7)
        return;

    const auto infos = QSerialPortInfo::availablePorts();
    valid = false;
    QString portName = "";
    for(int h=1; h < infos.length(); h++) {
        if(infos[h].portName() == subStrings[0]) {
            portName=subStrings[h];
            valid = true;
            break;
        }
    }
    if(!valid || subStrings[0].isEmpty())
        return;
        
    valid = false;
    qint32 baudRate = subStrings[1].toInt(&valid, 10); 
    if(!valid)
        return;

    valid = false;
    qint32 dataBits = subStrings[2].toInt(&valid, 10); 
    if(!valid)
        return;

    valid = false;
    qint32 parity = subStrings[3].toInt(&valid, 10); 
    if(!valid)
        return;

    // NOTE: only integer values supported now
    valid = false;
    qint32 stopBits = subStrings[4].toInt(&valid, 10); 
    if(!valid)
        return;

    valid = false;
    qint32 flowControl = subStrings[5].toInt(&valid, 10); 
    if(!valid)
        return;

    DataStreamer::DataContainer dataContainer;
    if(subStrings[6].toUpper() == "CSV")
        dataContainer = DataStreamer::DataContainer::CSV;
    else if(subStrings[6].toUpper() == "JSON")
        dataContainer = DataStreamer::DataContainer::JSON;
    else if(subStrings[6].toUpper() == "XML")
        dataContainer = DataStreamer::DataContainer::XML;
    else if(subStrings[6].toUpper() == "YAML")
        dataContainer = DataStreamer::DataContainer::YAML;
    else 
        return;

    p.name = portName;
    p.baudRate = baudRate;
    p.stringBaudRate = QString::number(baudRate);
    p.dataBits = (QSerialPort::DataBits)dataBits;
    p.stringDataBits = QString::number(baudRate);
    p.parity = (QSerialPort::Parity)parity;
    p.stringParity = QString::number(parity);
    p.stopBits = (QSerialPort::StopBits)stopBits;
    p.stringStopBits = QString::number(stopBits);
    p.flowControl = (QSerialPort::FlowControl)flowControl;
    p.stringFlowControl = QString::number(flowControl);
    p.localEchoEnabled = true; // always on now

    applicationSettings->setValue("StreamingSettings.COM.dataContainer", dataContainer);

    streamingSettingsDialog->connectCOM(p);
}
void MainWindow::PRGconnectMicrocontrollerUDP(QString conf) {
    if(MCUSettingsDialogInst->isConnected())
        return;

    QString ip;
    quint16 port;
    ConnPoolUDPInstanceSettings p;
    bool valid;

    conf.replace(" ", "");
    //QStringList subStrings = conf.split(',');
    QRegExp separator("[,|;|*|&|#|:]");
    QStringList subStrings = conf.split(separator);
    if(subStrings.length() < 3)
        return;

    if(subStrings[0].isEmpty())
        return;
    ip = subStrings[0];

    valid = false;
    port = (quint16)subStrings[1].toInt(&valid, 10);
    if(!valid)
        return;

    p.ipAddress = QHostAddress(ip);
    p.portNumber = port;



    MCUSettingsDialogInst->selectConnectionMethod(MCUSettingsDialog::ConnectionMethod::UDP);
//    MCUSettingsDialogInst->connectUDP(p);
    if(stereoCameraSettingsDialog) {
        singleCameraSettingsDialog->connectMCU();
    }
    if(stereoCameraSettingsDialog) {
        stereoCameraSettingsDialog->connectMCU();
    }
}
void MainWindow::PRGconnectMicrocontrollerCOM(QString conf) {
    if(MCUSettingsDialogInst->isConnected())
        return;

    ConnPoolCOMInstanceSettings p;
    bool valid;

    conf.replace(" ", "");
    //QStringList subStrings = conf.split(',');
    QRegExp separator("[,|;|*|&|#|:]");
    QStringList subStrings = conf.split(separator);
    if(subStrings.length() < 7)
        return;

    const auto infos = QSerialPortInfo::availablePorts();
    valid = false;
    QString portName = "";
    for(int h=1; h < infos.length(); h++) {
        if(infos[h].portName() == subStrings[0]) {
            portName=subStrings[h];
            valid = true;
            break;
        }
    }
    if(!valid || subStrings[0].isEmpty())
        return;

    valid = false;
    qint32 baudRate = subStrings[1].toInt(&valid, 10);
    if(!valid)
        return;

    valid = false;
    qint32 dataBits = subStrings[2].toInt(&valid, 10);
    if(!valid)
        return;

    valid = false;
    qint32 parity = subStrings[3].toInt(&valid, 10);
    if(!valid)
        return;

    // NOTE: only integer values supported now
    valid = false;
    qint32 stopBits = subStrings[4].toInt(&valid, 10);
    if(!valid)
        return;

    valid = false;
    qint32 flowControl = subStrings[5].toInt(&valid, 10);
    if(!valid)
        return;

    p.name = portName;
    p.baudRate = baudRate;
    p.stringBaudRate = QString::number(baudRate);
    p.dataBits = (QSerialPort::DataBits)dataBits;
    p.stringDataBits = QString::number(baudRate);
    p.parity = (QSerialPort::Parity)parity;
    p.stringParity = QString::number(parity);
    p.stopBits = (QSerialPort::StopBits)stopBits;
    p.stringStopBits = QString::number(stopBits);
    p.flowControl = (QSerialPort::FlowControl)flowControl;
    p.stringFlowControl = QString::number(flowControl);
    p.localEchoEnabled = true; // always on now

    MCUSettingsDialogInst->selectConnectionMethod(MCUSettingsDialog::ConnectionMethod::COM);
//    MCUSettingsDialogInst->connectCOM(p);
    if(stereoCameraSettingsDialog) {
        singleCameraSettingsDialog->connectMCU();
    }
    if(stereoCameraSettingsDialog) {
        stereoCameraSettingsDialog->connectMCU();
    }
}
void MainWindow::PRGdisconnectRemoteUDP() {
    if(remoteCCDialog->isUDPConnected())
        remoteCCDialog->disconnectUDP();
}
void MainWindow::PRGdisconnectRemoteCOM() {
    if(remoteCCDialog->isCOMConnected())
        remoteCCDialog->disconnectCOM();
}
void MainWindow::PRGdisconnectStreamUDP() {
    if(streamingSettingsDialog->isUDPConnected())
        streamingSettingsDialog->disconnectUDP();
}
void MainWindow::PRGdisconnectStreamCOM() {
    if(streamingSettingsDialog->isCOMConnected())
        streamingSettingsDialog->disconnectCOM();
}
void MainWindow::PRGdisconnectMicrocontroller() {
    if(MCUSettingsDialogInst->isConnected())
        MCUSettingsDialogInst->doDisconnect();
}

void MainWindow::PRGenableHWT(bool state) {
    if(!selectedCamera || selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        return;
    if(state)
        singleCameraSettingsDialog->onHardwareTriggerEnable();
    else
        singleCameraSettingsDialog->onHardwareTriggerDisable();
}
void MainWindow::PRGstartHWT() {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->startHardwareTrigger();
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->startHardwareTrigger();
}
void MainWindow::PRGstopHWT() {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->stopHardwareTrigger();
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->stopHardwareTrigger();
}
void MainWindow::PRGsetHWTlineSource(int lineSourceNum) {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->setHWTlineSource(lineSourceNum);
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->setHWTlineSource(lineSourceNum);
}
void MainWindow::PRGsetHWTruntime(float runtimeMinutes) {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->setHWTruntime(runtimeMinutes);
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->setHWTruntime(runtimeMinutes);
}
void MainWindow::PRGsetHWTframerate(int fps) {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->setHWTframerate(fps);
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->setHWTframerate(fps);
}
void MainWindow::PRGenableSWTframerateLimiting(const QString &state) {
    if(!selectedCamera || selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        return;

    if(state == "true" || state == "1") {
        singleCameraSettingsDialog->enableAcquisitionFrameRate(true);
    } else if(state == "false" || state == "0") {
        singleCameraSettingsDialog->enableAcquisitionFrameRate(false);
    }
}
void MainWindow::PRGsetSWTframerate(int fps) {
    if(!selectedCamera || selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        return;

    singleCameraSettingsDialog->setAcquisitionFPSValue(fps);
}
void MainWindow::PRGsetExposure(int value) {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->setExposureTimeValue(value);
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->setExposureTimeValue(value);
}
void MainWindow::PRGsetGain(double value) {
    if(!selectedCamera || (selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA && selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA))
        return;
    if(selectedCamera->getType() != CameraImageType::LIVE_SINGLE_CAMERA)
        singleCameraSettingsDialog->setGainValue(value);
    else /*if(selectedCamera->getType() != CameraImageType::LIVE_STEREO_CAMERA)*/
        stereoCameraSettingsDialog->setGainValue(value);
}

