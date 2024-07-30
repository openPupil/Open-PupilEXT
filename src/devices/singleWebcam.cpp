
#include "singleWebcam.h"


GrabberDummy::GrabberDummy(int deviceID) : deviceID(deviceID), m_camera(new cv::VideoCapture()) {
    //std::cout << "GrabberDummy::GrabberDummy(cv::VideoCapture* camera)" << std::endl;
}
 
bool GrabberDummy::running() const {
    //std::cout << "GrabberDummy::running()" << std::endl;
    return m_running;
}

cv::Size GrabberDummy::getImageSize() const {
    return image.size();
}

std::string uint64_to_string( uint64 value ) {
    std::ostringstream os;
    os << value;
    return os.str();
}

void GrabberDummy::run() {
    //std::cout << "GrabberDummy::run() started" << std::endl;
    m_running = true;

    if(m_camera->isOpened())
        m_camera->release();

    m_camera->open(deviceID);

    while(m_running) {
    //while(true) {
        *m_camera >> image;
        if (image.channels() > 1)
            cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        if(image.cols > 0 && image.rows > 0 && resizeFactor<1.0 && resizeFactor >0.0) 
            cv::resize(image, image, cv::Size(), resizeFactor, resizeFactor);
        emit GrabberDummyEvent(image);

        QThread::msleep((int)round(1000/queryFPS));
    }

    m_camera->release();

    //std::cout << "GrabberDummy::run() finishing" << std::endl;
    emit finished();
}

bool GrabberDummy::registerEventHandler(SingleWebcamImageEventHandler *handler, const char *method) {
    return connect(this, SIGNAL(GrabberDummyEvent(cv::Mat)), handler, method );
}
 
void GrabberDummy::stop() {
    std::cerr << "GrabberDummy::stop()" << std::endl;
    m_running = false;
    //emit runningChanged(running);
}

bool GrabberDummy::isOpen() {
    return m_camera->isOpened();
}


int GrabberDummy::getFPSValue() {
    return (int)round(m_camera->get(CV_CAP_PROP_FPS));
}

double GrabberDummy::getBrightnessValue() {
    return m_camera->get(CV_CAP_PROP_BRIGHTNESS);
}

double GrabberDummy::getContrastValue() {
    return m_camera->get(CV_CAP_PROP_CONTRAST);
}

double GrabberDummy::getGainValue() {
    return m_camera->get(CV_CAP_PROP_GAIN);
}

double GrabberDummy::getExposureValue() {
    return m_camera->get(CV_CAP_PROP_EXPOSURE);
}

double GrabberDummy::getResizeFactor() {
    return resizeFactor;
}


bool GrabberDummy::setFPSValue(int value) {
    //std::cout << "setting FPS value " << std::endl;
    queryFPS = value;
    return m_camera->set(CV_CAP_PROP_FPS, value);
}

bool GrabberDummy::setBrightnessValue(double value) {
    //std::cout << "setting brightness value " << std::endl;
    return m_camera->set(CV_CAP_PROP_BRIGHTNESS, value);
}

bool GrabberDummy::setContrastValue(double value) {
    //std::cout << "setting contrast value " << std::endl;
    return m_camera->set(CV_CAP_PROP_CONTRAST, value);
}

bool GrabberDummy::setGainValue(double value) {
    //std::cout << "setting gain value " << std::endl;
    return m_camera->set(CV_CAP_PROP_GAIN, value);
}

bool GrabberDummy::setExposureValue(double value) {
    //std::cout << "setting exposure value " << std::endl;
    return m_camera->set(CV_CAP_PROP_EXPOSURE, value);
}

void GrabberDummy::setResizeFactor(double value) {
    resizeFactor = value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool file_exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

SingleWebcam::SingleWebcam(int deviceID, QString friendlyName, QObject* parent)
        : Camera(parent),
        cameraImageEventHandler(new SingleWebcamImageEventHandler(parent)),
        frameCounter(new CameraFrameRateCounter(parent)),
        cameraCalibration(new CameraCalibration()),
        friendlyName(friendlyName),
        calibrationThread(new QThread())
        {

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    // calibration worker thread
    cameraCalibration->moveToThread(calibrationThread);
    calibrationThread->start();
    calibrationThread->setPriority(QThread::HighPriority);

    connect(cameraImageEventHandler, SIGNAL(onNewGrabResult(CameraImage)), this, SIGNAL(onNewGrabResult(CameraImage)));
    connect(cameraImageEventHandler, SIGNAL(onNewGrabResult(CameraImage)), frameCounter, SLOT(count(CameraImage)));

    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    connect(frameCounter, SIGNAL(framecount(int)), this, SIGNAL(framecount(int)));

    try {
        // load calibration if existing
        if(!cameraCalibration->isCalibrated())
            loadCalibrationFile();

        grabbingThread = new QThread();
        grabberDummy = new GrabberDummy(deviceID);

        grabberDummy->registerEventHandler(cameraImageEventHandler, SLOT(OnImageGrabbed(cv::Mat)));
        connect(grabbingThread, &QThread::started, grabberDummy, &GrabberDummy::run);
        grabberDummy->moveToThread(grabbingThread);    
        
        connect(grabbingThread, SIGNAL (finished()), grabbingThread, SLOT (deleteLater()));
        grabbingThread->start();
        grabbingThread->setPriority(QThread::HighPriority);
    }
    catch (const QException &e) {
        std::cerr << "A webcam exception occurred." << std::endl<< e.what() << std::endl;
    }
}

void SingleWebcam::stopGrabbing(){
    grabberDummy->stop();
}

void SingleWebcam::startGrabbing(){
    grabberDummy->run();
}

SingleWebcam::~SingleWebcam() {
    //grabbingThread->terminate();
    grabbingThread->deleteLater();

    if (cameraCalibration != nullptr)
        cameraCalibration->deleteLater();
    if (calibrationThread != nullptr) {
        calibrationThread->quit();
        calibrationThread->deleteLater();
    }
}

bool SingleWebcam::isOpen() {
    return grabberDummy->isOpen();
}

void SingleWebcam::close() {
    std::cout << "SingleWebcam: Releasing resources.";
    //grabbingThread->terminate();
    grabberDummy->stop();
}


CameraCalibration *SingleWebcam::getCameraCalibration() {
    return cameraCalibration;
}

QString SingleWebcam::getFriendlyName() {
    return friendlyName;
}

CameraImageType SingleWebcam::getType() {
    return CameraImageType::LIVE_SINGLE_WEBCAM;
}

QString SingleWebcam::getCalibrationFilename() {

    return settingsDirectory.filePath(friendlyName + "_calibration_" +
            QString::number(cameraCalibration->getSquareSize()) + "_" +
            QString::number(cameraCalibration->getBoardSize().width+1) + "x" +
            QString::number(cameraCalibration->getBoardSize().height+1) + ".xml");
}

bool SingleWebcam::isGrabbing()
{
    return grabberDummy->running();
}

void SingleWebcam::loadCalibrationFile() {
    QString configFile = getCalibrationFilename();
    configFile.replace(" ", "");

    if (QFile::exists(configFile)) {
        std::cout << "Found calibration file in settings directory. Loading: " << configFile.toStdString() << std::endl;
        cameraCalibration->loadFromFile(configFile.toStdString().c_str());
    }
}

int SingleWebcam::getFPSValue() {
    return grabberDummy->getFPSValue();
}

double SingleWebcam::getBrightnessValue() {
    return grabberDummy->getBrightnessValue();
}

double SingleWebcam::getContrastValue() {
    return grabberDummy->getContrastValue();
}

double SingleWebcam::getGainValue() {
    return grabberDummy->getGainValue();
}

double SingleWebcam::getExposureValue() {
    return grabberDummy->getExposureValue();
}

double SingleWebcam::getResizeFactor() {
    return grabberDummy->getResizeFactor();
}

int SingleWebcam::getImageROIwidth(){
    return grabberDummy->getImageSize().width;
}

int SingleWebcam::getImageROIheight(){
    return grabberDummy->getImageSize().height;
}

int SingleWebcam::getImageROIwidthMax(){
    return getImageROIwidth();
}

int SingleWebcam::getImageROIheightMax(){
    return getImageROIheight();
}

int SingleWebcam::getImageROIoffsetX(){
    return 0;
}

int SingleWebcam::getImageROIoffsetY(){
    return 0;
}

QRectF SingleWebcam::getImageROI(){
    return QRectF(0,0,getImageROIwidth(), getImageROIheight());
}


bool SingleWebcam::setFPSValue(int value) {
    std::cout << "setting FPS value " << std::endl;
    return grabberDummy->setFPSValue(value);
}

bool SingleWebcam::setBrightnessValue(double value) {
    std::cout << "setting brightness value " << std::endl;
    return grabberDummy->setBrightnessValue(value);
}

bool SingleWebcam::setContrastValue(double value) {
    std::cout << "setting contrast value " << std::endl;
    return grabberDummy->setContrastValue(value);
}

bool SingleWebcam::setGainValue(double value) {
    std::cout << "setting gain value " << std::endl;
    return grabberDummy->setGainValue(value);
}

bool SingleWebcam::setExposureValue(double value) {
    std::cout << "setting exposure value " << std::endl;
    return grabberDummy->setExposureValue(value);
}

void SingleWebcam::setResizeFactor(double value) {
    grabberDummy->setResizeFactor(value);
}