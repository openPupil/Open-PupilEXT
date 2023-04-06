#include "camTempMonitor.h"

CamTempMonitor::CamTempMonitor(Camera *camera) : camera(camera) {
    stereo=false;
    if(camera->getType()==LIVE_SINGLE_CAMERA) {
        //stereo=false;
        std::cerr << "started camTempMonitor with single camera" << std::endl;
    } else if(camera->getType()==LIVE_STEREO_CAMERA) {
        stereo=true;
        std::cerr << "started camTempMonitor with stereo camera" << std::endl;
    } //else {
    //    return;
    //}
}
 
bool CamTempMonitor::running() const {
    std::cerr << "camTempMonitor running" << std::endl;
    return m_running;
}

void CamTempMonitor::run() {
    std::cerr << "camTempMonitor thread: RUN()" << std::endl;

    m_running = true;

    while(m_running) {
        QThread::msleep(5000); // TODO: this time should be configurable in general settings dialog, as well as the option to track camera temperature or not
        if(!camera || (camera && !camera->isOpen())) 
            continue;
        
        if(!stereo) {
            emit camTempChecked(std::vector<double>{static_cast<SingleCamera*>(camera)->getTemperature(), 0});
            // recEventTracker->addTemperatureCheck( std::vector<double>{static_cast<SingleCamera*>(camera)->getTemperature(), 0} );
        } else {
            //std::vector<double> d = stereoCamera->getTemperatures();
            emit camTempChecked(static_cast<StereoCamera*>(camera)->getTemperatures());
            //recEventTracker->addTemperatureCheck( static_cast<StereoCamera*>(camera)->getTemperatures() );
        }
    }

    std::cerr << "camTempMonitor thread FINISHING" << std::endl;

    emit finished();
}
 
void CamTempMonitor::setRunning(bool running)
{
    std::cerr << "camTempMonitor thread setRunning()" << std::endl;

    if (m_running == running)
        return;
 
    m_running = running;
    emit runningChanged(running);
}


