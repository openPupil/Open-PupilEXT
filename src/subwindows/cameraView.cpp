
#include "cameraView.h"

void CameraView::onPlotMenuClick() {
    // Fix to open submenu in the camera menu
    plotMenuAct->menu()->exec(QCursor::pos());
}

void CameraView::onPupilDetectionMenuClick()
{
    pupilDetectionMenuAct->menu()->exec(QCursor::pos());
}

void CameraView::onPlotPupilCenterClick(bool value)
{
    plotPupilCenter = value;
    applicationSettings->setValue("SingleCameraView.plotPupilCenter", plotPupilCenter);

    emit onShowPupilCenter(plotPupilCenter);
}

void CameraView::onPlotROIClick(bool value)
{
    plotROIContour = value;
    applicationSettings->setValue("SingleCameraView.plotROIContour", plotROIContour);
    showAutoParamAct->setEnabled(value);
    emit onShowAutoParamOverlay(showAutoParamOverlay);
    emit onShowROI(plotROIContour);
}

void CameraView::onPupilDetectionStart()
{
    statusBar->insertPermanentWidget(1, statusProcessingFPSWidget);
    statusProcessingFPSWidget->show();
    processingConfigLabel->setText(pupilDetection->getCurrentConfigLabel());
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title())); // GB NOTE: But when do we hide it??

    updateProcModeLabel();

    disconnect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage)), this, SLOT(updateView(CameraImage)));
    
    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
}

void CameraView::onPupilDetectionStop()
{
    statusBar->removeWidget(statusProcessingFPSWidget);

    disconnect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updateView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));
    disconnect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)), this, SLOT(updatePupilView(CameraImage, int, std::vector<cv::Rect>, std::vector<Pupil>)));

    connect(pupilDetection, SIGNAL(processedImageLowFPS(CameraImage)), this, SLOT(updateView(CameraImage)));
    onPupilDetectionStopInternal();
}

void CameraView::onSettingsChange()
{
    loadSettings();
}

void CameraView::onPupilDetectionConfigChanged(QString config)
{
    processingConfigLabel->setText(config);
    autoParamMenu->setEnabled(isAutoParamModificationEnabled());
}

bool CameraView::isAutoParamModificationEnabled()
{
    return pupilDetection->isAutoParamSettingsEnabled();
}

void CameraView::updateCameraFPS(double fps)
{ 
    currentCameraFPS = fps;
    //cameraFPSValue->setText(QString::number(fps));

    if(fps == 0)
        cameraFPSValue->setText("-");
    else if(fps > 0 && fps < 1)
        cameraFPSValue->setText(QString::number(fps,'f',4));
    else
        cameraFPSValue->setText(QString::number(round(fps)));
}

void CameraView::updateProcessingFPS(double fps)
{
    
    if(fps < currentCameraFPS*0.9) {
        processingFPSValue->setStyleSheet("color: red;");
    } else {
        processingFPSValue->setStyleSheet("color: black;");
    }
    //processingFPSValue->setText(QString::number(fps));

    if(fps == 0)
        processingFPSValue->setText("-");
    else if(fps > 0 && fps < 1)
        processingFPSValue->setText(QString::number(fps,'f',4));
    else
        processingFPSValue->setText(QString::number(round(fps)));
}

void CameraView::updateAlgorithmLabel()
{
    processingAlgorithmLabel->setText(QString::fromStdString(pupilDetection->getCurrentMethod1()->title()));
}

void CameraView::onPupilColorFillChanged(int itemIndex)
{
    pupilColorFill = (ColorFill)itemIndex;
    applicationSettings->setValue("SingleCameraView.pupilColorFill", pupilColorFill);
    
    emit onChangePupilColorFill(pupilColorFill);
}

void CameraView::onPupilColorFillThresholdChanged(double value)
{
    pupilColorFillThreshold = value;
    applicationSettings->setValue("StereoCameraView.pupilColorFillThreshold", pupilColorFillThreshold);

    emit onChangePupilColorFillThreshold(pupilColorFillThreshold);
}

void CameraView::onShowAutoParamOverlay(bool state)
{
    showAutoParamOverlay = state;
    applicationSettings->setValue("SingleCameraView.showAutoParamOverlay", showAutoParamOverlay);
    emit onChangeShowAutoParamOverlay(showAutoParamOverlay);
}

void CameraView::onFreezeClicked()
{
    emit cameraPlaybackChanged();
}

void CameraView::onCameraPlaybackChanged()
{
    playbackFrozen = !playbackFrozen;
    if (playbackFrozen)
        freezeText = "Unfreeze";
    else 
        freezeText = "Freeze";

    freezeAct->setText(freezeText);

    
}
