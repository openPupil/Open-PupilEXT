#include <QtWidgets>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/QSpinBox>
#include <iostream>
#include "imagePlaybackControlDialog.h"
#include "timestampSpinBox.h"
#include "../SVGIconColorAdjuster.h"


// TODO: readSettings and updateForm calls necessary when someone changes settings through remote control command, via QSettings

// Create a settings dialog for the general software settings
// Settings are read upon creation from the QT application settings if existing
ImagePlaybackControlDialog::ImagePlaybackControlDialog(FileCamera *fileCamera, PupilDetection *pupilDetection, RecEventTracker *recEventTracker, QWidget *parent) :
        QWidget(parent),
        fileCamera(fileCamera),
        pupilDetection(pupilDetection),
        recEventTracker(recEventTracker),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {
 

    //this->setMinimumSize(500, 400); 
    this->setMinimumSize(600, 200); 
    this->setWindowTitle("Image Playback Control");

    drawDelay = 33; // ~30 fps

    readSettings();

    createForm();

    numImagesTotal = fileCamera->getNumImagesTotal();
    timeTotal = QTime::fromMSecsSinceStartOfDay(fileCamera->getRecordingDuration());
    lastTimestamp = 0;
    startTimestamp = 0;


    updateInfoInternal(0);
    //emit stillImageChange(0); // cannot do it right here, because signals and slots aren't yet connected out in mainwindow code

}

ImagePlaybackControlDialog::~ImagePlaybackControlDialog() {
}

void ImagePlaybackControlDialog::createForm() {
    QGridLayout *layout = new QGridLayout(this);

    QFormLayout *infoLayout = new QFormLayout;
    infoGroup = new QGroupBox("Playback info");

    QLabel *timestampLabel = new QLabel(tr("Timestamp [ms]:"));
    //timestampVal = new QLineEdit();
    timestampVal = new TimestampSpinBox(fileCamera);
    timestampVal->setReadOnly(false);
    timestampVal->setMaximumWidth(100);
    timestampVal->setMinimum(0);
    uint64_t timestampMax = fileCamera->getTimestampForFrameNumber(fileCamera->getNumImagesTotal()-1);
    timestampVal->setMaximum(timestampMax);
    timestampVal->setWrapping(true);
    infoLayout->addRow(timestampLabel, timestampVal);

    QLabel *frameLabel = new QLabel(tr("Frame:"));
    selectedFrameBox = new QSpinBox();
    selectedFrameBox->setReadOnly(false);
    selectedFrameBox->setMaximumWidth(100);
    selectedFrameBox->setMinimum(1);
    selectedFrameBox->setMaximum(fileCamera->getNumImagesTotal());
    selectedFrameBox->setValue(selectedFrameVal);
    selectedFrameBox->setWrapping(true);
    infoLayout->addRow(frameLabel, selectedFrameBox);

    QLabel *timestampHumanLabel = new QLabel(tr("Date and time:"));
    timestampHumanValLabel = new QLabel("-");
    infoLayout->addRow(timestampHumanLabel, timestampHumanValLabel);

    QLabel *imgNumberLabel = new QLabel(tr("Image number:"));
    imgNumberValLabel = new QLabel("-");
    infoLayout->addRow(imgNumberLabel, imgNumberValLabel);

    // GB: number of (so far) skipped images:
    // Sounds cool, but it is complicated to implement correctly.
    // As there can be quite a delay sometimes between the image reading and the moment 
    // the processed image arrives here (at updateInfo), we either need to keep track of
    // numSkipped in each processed CameraImage or call imageReader to retrieve this number
    // from a list that it keeps track of, for each image... 
    // Both are unnecessarily complex for this simple purpose
    // // gets reset on stop button click
    //QLabel *skippedLabel = new QLabel(tr("Skipped frames:")); 
    //skippedValLabel = new QLabel(tr("0"));
    //infoLayout->addRow(skippedLabel, skippedValLabel);

    QLabel *elapsedTimeLabel = new QLabel(tr("Elapsed time:"));
    elapsedTimeValLabel = new QLabel("-");
    infoLayout->addRow(elapsedTimeLabel, elapsedTimeValLabel);

    QLabel *acqFPSLabel = new QLabel(tr("Acquisition FPS:"));
    acqFPSValLabel = new QLabel("-");
    infoLayout->addRow(acqFPSLabel, acqFPSValLabel);

    QLabel *percentLabel = new QLabel(tr("Playback at [%]:"));
    percentValLabel = new QLabel("-");
    infoLayout->addRow(percentLabel, percentValLabel);

    QLabel *trialLabel = new QLabel(tr("Trial:"));
    trialValLabel = new QLabel("-");
    infoLayout->addRow(trialLabel, trialValLabel);

    infoGroup->setLayout(infoLayout);
    layout->addWidget(infoGroup, 0, 0, 1, 1);

    QGridLayout *controlLayout = new QGridLayout;
    QGroupBox *controlGroup = new QGroupBox("Control");

    slider = new PlaybackSlider();
    slider->setOrientation(Qt::Horizontal);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksAbove);
    //slider->setFixedWidth(300); // 100 px width
    slider->setMinimumWidth(200);
    slider->setTickInterval(10); //max value is 100, and every 10th value gets a visual mark
    slider->setSingleStep(1);
    slider->setMaximum(99);
    slider->setMinimum(0);


    dial = new PlaybackDial();
    dial->setFocusPolicy(Qt::StrongFocus);
    connect(dial, SIGNAL(incremented()), this, SLOT(onDialForward()));
    connect(dial, SIGNAL(decremented()), this, SLOT(onDialBackward()));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(dial, SIGNAL(valueChanged(int)), this, SLOT(on(int)));
    
    controlLayout->addWidget(slider, 0, 0, 1, 3);
    controlLayout->addWidget(dial, 1, 2, 6, 1);

    const QIcon startIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-start.svg"), applicationSettings);
    startPauseButton = new QPushButton();
    startPauseButton->setToolTip("Start/Pause image playback");
    startPauseButton->setIcon(startIcon);

    const QIcon stopIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-stop.svg"), applicationSettings);
    stopButton = new QPushButton();
    stopButton->setToolTip("Stop image playback");
    stopButton->setIcon(stopIcon);

    controlLayout->addWidget(startPauseButton, 2, 0, 1, 1);
    controlLayout->addWidget(stopButton, 2, 1, 1, 1);

    QLabel *playbackFPSLabel = new QLabel(tr("Playback FPS:"));
    playbackFPSLabel->setAccessibleDescription("Speed with which offline recordings are played. Speed of 0 will make it play as fast as possible.");
    playbackFPSVal = new QSpinBox();
    playbackFPSVal->setMinimum(0);
    playbackFPSVal->setMaximum(999);
    playbackFPSVal->setSingleStep(1);
    playbackFPSVal->setValue(playbackSpeed);
    controlLayout->addWidget(playbackFPSLabel, 3, 0, 1, 1);
    controlLayout->addWidget(playbackFPSVal, 3, 1, 1, 1);

    loopBox = new QCheckBox("Loop playback when end is reached");
    loopBox->setChecked(playbackLoop);
    controlLayout->addWidget(loopBox, 4, 0, 1, 2);

    syncRecordCsvBox = new QCheckBox("Start/pause Data Recording in sync");
    syncRecordCsvBox->setChecked(syncRecordCsv);
    controlLayout->addWidget(syncRecordCsvBox, 5, 0, 1, 2);

    syncStreamBox = new QCheckBox("Start/pause Data Streaming in sync");
    syncStreamBox->setChecked(syncStream);
    controlLayout->addWidget(syncStreamBox, 6, 0, 1, 2);

    controlGroup->setLayout(controlLayout);
    //
    layout->addWidget(controlGroup, 0, 1, 1, 1);

    setLayout(layout);


    connect(startPauseButton, SIGNAL(clicked()), this, SLOT(onStartPauseButtonClick()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(onStopButtonClick()));

    //bool succeeded = connect(pupilDetection, SIGNAL(processedImage(CameraImage)), this, SLOT(updateInfo(CameraImage)));
    connect(fileCamera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateInfo(CameraImage)));
    connect(fileCamera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateSliderColorTick(CameraImage)));
    // BREAKPOINT
    connect(pupilDetection, SIGNAL(processingStarted()), this, SLOT(onPupilDetectionStart()));
    connect(pupilDetection, SIGNAL(processingFinished()), this, SLOT(onPupilDetectionStop()));

    connect(playbackFPSVal, SIGNAL(valueChanged(int)), this, SLOT(setPlaybackSpeed(int)));
    connect(loopBox, SIGNAL(stateChanged(int)), this, SLOT(setPlaybackLoop(int)));
    connect(syncRecordCsvBox, SIGNAL(stateChanged(int)), this, SLOT(setSyncRecordCsv(int)));
    connect(syncStreamBox, SIGNAL(stateChanged(int)), this, SLOT(setSyncStream(int)));
    connect(selectedFrameBox, SIGNAL(valueChanged(int)), this, SLOT(onFrameSelected(int)));
    connect(timestampVal, SIGNAL(valueChanged(double)), this, SLOT(onTimestampSelected(double)));
}

void ImagePlaybackControlDialog::updateSliderColorTick(const CameraImage &cimg) {
    // GB: even though this gets called really often, the slider's setter doesnt invalidate the widget, 
    // so even fast updating does not slow the GUI thread
    if(numImagesTotal>=1)
        slider->setColorTickPos((cimg.frameNumber+1)/(float)numImagesTotal);
}

void ImagePlaybackControlDialog::onPupilDetectionStart() {
    disconnect(fileCamera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateInfo(CameraImage)));
    connect(pupilDetection, SIGNAL(processedPlaybackImage(quint64, int)), this, SLOT(updateInfo(quint64, int)));
}

void ImagePlaybackControlDialog::onPupilDetectionStop() {
    disconnect(pupilDetection, SIGNAL(processedPlaybackImage(quint64, int)), this, SLOT(updateInfo(quint64, int)));
    connect(fileCamera, SIGNAL(onNewGrabResult(CameraImage)), this, SLOT(updateInfo(CameraImage)));
}

// when someone seeks, but there is no playback going on
void ImagePlaybackControlDialog::updateInfoInternal(int frameNumber) {
    uint64_t currTimestamp = fileCamera->getTimestampForFrameNumber(frameNumber);

    QDateTime date = QDateTime::fromMSecsSinceEpoch(currTimestamp);
    //date.toString("yyyy-MMM-dd hh:mm:ss");

    acqFPS = 0.0;
    if(lastTimestamp != 0)
        acqFPS = 1/(float)(currTimestamp-lastTimestamp) *1000; //timestamps are in millisecond

    elapsedMs = 0.0;
    if(startTimestamp != 0)
        elapsedMs = (currTimestamp-startTimestamp);

    QTime timeElapsed = QTime::fromMSecsSinceStartOfDay(elapsedMs);

    //timestampValLabel->setText(QString::number(img.timestamp));
    timestampVal->setValue(frameNumber+1);
    timestampHumanValLabel->setText(date.toString("yyyy. MMM dd. hh:mm:ss"));
    //timestampHumanValLabel->setText(QLocale::system().toString(date));
    imgNumberValLabel->setText(QString::number(frameNumber+1) + "\t/ " + QString::number(numImagesTotal));
    acqFPSValLabel->setText("-");
    //elapsedTimeValLabel->setText(QString::number(elapsedMs));
    elapsedTimeValLabel->setText(timeElapsed.toString("hh:mm:ss") + "\t/ " + timeTotal.toString("hh:mm:ss"));
    percentValLabel->setText(QString::number((float)(frameNumber+1)/(float)numImagesTotal*(float)100,'f',1)); 
    if(recEventTracker)
        trialValLabel->setText(QString::number(recEventTracker->getTrialIncrement(currTimestamp).trialNumber)); 

    lastTimestamp = currTimestamp;
    if(startTimestamp == 0)
        startTimestamp = currTimestamp;

}

void ImagePlaybackControlDialog::updateInfo(const CameraImage &img) {
    // NOTE: this img.frameNumber field existed already/originally in pupilEXT beta 0.1.1 and is used in calibration code
    // Pay attention, it is an INDEX, so it starts from 0
    updateInfo((quint64)img.timestamp, (int)img.frameNumber);
}

// NOTE: now it works with the logic that: whatever gets read by imageReader, must arrive as a CameraImage once later
// no matter if the image is read for just displaying it, or for pupil detection. So if we check the
// timestamp of the last read image from imageReader, we can wait until the image with that timestamp comes
// down the pipeline and arrives at updateInfo method.
// IMPORTANT: Qt doesnt like uint64_t. Connection makes success, but signals never arrive. So I used quint64 instead
void ImagePlaybackControlDialog::updateInfo(quint64 timestamp, int frameNumber) {

    // qDebug() << "arrived frameNumber: " << frameNumber;
    // qDebug() << "arrived timestamp: " << timestamp;

    // NOTES:
    // 1, if it is the last frame in the whole playback, we need to draw it anyhow
    // 2, if we are waiting for the last emitted frame to come with stalledTimestamp we still need to draw it, no matter is drawDelay was not reached yet (fixes disabled-stuck buttons bug)
    if(drawTimer.elapsed() > drawDelay || frameNumber == numImagesTotal || (playbackStalled && stalledTimestamp<=timestamp)) {
        drawTimer.start();

        QDateTime date = QDateTime::fromMSecsSinceEpoch(timestamp);

        acqFPS = 0.0;
        if(lastTimestamp != 0)
            acqFPS = 1/(float)(timestamp-lastTimestamp) *1000; // timestamps are in millisecond

        elapsedMs = 0.0;
        if(startTimestamp != 0)
            elapsedMs = (timestamp-startTimestamp);

        QTime timeElapsed = QTime::fromMSecsSinceStartOfDay(elapsedMs);

        //timestampValLabel->setText(QString::number(img.timestamp));
        timestampVal->setValue(frameNumber+1);
        timestampHumanValLabel->setText(date.toString("yyyy. MMM dd. hh:mm:ss"));
        //timestampHumanValLabel->setText(QLocale::system().toString(date));
        imgNumberValLabel->setText(QString::number(frameNumber+1) + "\t/ " + QString::number(numImagesTotal));
        selectedFrameBox->setValue(frameNumber + 1);
        if(acqFPS == 0)
            acqFPSValLabel->setText("-");
        else if(acqFPS > 0 && acqFPS < 1)
            acqFPSValLabel->setText(QString::number(acqFPS,'f',4));
        else
            acqFPSValLabel->setText(QString::number(round(acqFPS)));
        //elapsedTimeValLabel->setText(QString::number(elapsedMs));
        elapsedTimeValLabel->setText(timeElapsed.toString("hh:mm:ss") + "\t/ " + timeTotal.toString("hh:mm:ss"));
        percentValLabel->setText(QString::number((float)(frameNumber+1)/(float)numImagesTotal*(float)100,'f',1)); 
        if(recEventTracker)
            trialValLabel->setText(QString::number(recEventTracker->getTrialIncrement(timestamp).trialNumber)); 

        // NOTE: workaround to not emit valuechanged signals, so it gets emitted only if the user interacts with it
        slider->blockSignals(true);
        int gg = floor(99*((frameNumber+1)/(float)numImagesTotal));
        slider->setValue( gg );
        slider->blockSignals(false);
    }

    lastTimestamp = timestamp; // GB: need to come before 30 fps drawTime wait
    if(startTimestamp == 0)
        startTimestamp = timestamp;

    if(playbackStalled && stalledTimestamp <= timestamp) {
        playbackStalled = false;
        onFinish(); 
    }
}

void ImagePlaybackControlDialog::onStartPauseButtonClick() {
    emit cameraPlaybackChanged();
}

void ImagePlaybackControlDialog::onStopButtonClick() {
    std::cout<<"Stopping FileCamera Click"<<std::endl;
    fileCamera->stop();
    //playImagesOn = false;

    //stalledTimestamp = fileCamera->getLastCommissionedTimestamp();
    stalledTimestamp = fileCamera->getTimestampForFrameNumber(fileCamera->getLastCommissionedFrameNumber());
    //std::cout<<"Stopping, lastTimestamp: "<< lastTimestamp << std::endl;
    //std::cout<<"Stopping, stalledTimestamp: "<< stalledTimestamp << std::endl;
    if(!playImagesOn || lastTimestamp >= stalledTimestamp) {
        // e.g. when playback automatically finished
        // we can supposedly "safely" reset right now, no need to wait for imageReader to finish
        lastTimestamp = 0;
        startTimestamp = 0;

        // need this here, before slider->setValue(0) because that relies on this bool
        playImagesOn = false;

        slider->blockSignals(false);
        slider->setValue(0);

        dial->setEnabled(true); 
        
        playbackStalled = false;
        waitingForReset = false;

        emit onPlaybackSafelyStopped();
        enableWidgets(false);

    } else if(playImagesOn && lastTimestamp < stalledTimestamp) {

        playImagesOn = false;

        slider->setEnabled(false);
        dial->setEnabled(false);
        startPauseButton->setEnabled(false);
        stopButton->setEnabled(false);
        
        infoGroup->setEnabled(false);

        playbackStalled = true;
        waitingForReset = true;
        enableWidgets(true);
    }

    //stalledFrameNumber = fileCamera->getLastCommissionedFrameNumber();
    //playbackStalled = true;

    const QIcon icon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-start.svg"), applicationSettings);
    startPauseButton->setIcon(icon);

    this->update(); // invalidate 
}

void ImagePlaybackControlDialog::onFinish() {
    // NOTE: gets called whenever imageReader is finished with reading and sending images
    if(playImagesOn) {
        const QIcon icon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-start.svg"), applicationSettings);
        startPauseButton->setIcon(icon);
        playImagesOn = false;
        enableWidgets(true);
    }

    stalledTimestamp = 0;
    playbackStalled = false;

    slider->setEnabled(true);
    dial->setEnabled(true);
    startPauseButton->setEnabled(true);
    stopButton->setEnabled(true);
    
    infoGroup->setEnabled(true);

    if(waitingForReset) {
        lastTimestamp = 0;
        startTimestamp = 0;
        slider->blockSignals(false);
        slider->setValue(0);
        waitingForReset = false;

        //if(recEventTracker)
        //    recEventTracker->resetReplay();

        emit onPlaybackSafelyStopped();
    } else {
        emit onPlaybackSafelyPaused();
    }

    this->update(); // invalidate 
}

void ImagePlaybackControlDialog::onAutomaticFinish() {
    onStopButtonClick();
}

void ImagePlaybackControlDialog::onDialForward() {
    lastTimestamp = 0;
    fileCamera->step1frameNext();
    selectedFrameVal+= 1;
    selectedFrameBox->setValue(selectedFrameVal);
}

void ImagePlaybackControlDialog::onDialBackward() {
    lastTimestamp = 0;
    fileCamera->step1framePrev();
    selectedFrameVal-= 1;
    selectedFrameBox->setValue(selectedFrameVal);
}

void ImagePlaybackControlDialog::onSliderValueChanged(int val) {
    int frameNumber = floor((float)(val)/(float)slider->maximum()*(float)(numImagesTotal-1));
    //qDebug() << "Seek to frame number (INDEX, starting from 0): " << frameNumber;
    fileCamera->seekToFrame(frameNumber);
    selectedFrameBox->setValue(frameNumber + 1);
    if(!playImagesOn) {
        updateInfoInternal(frameNumber);
        emit stillImageChange(frameNumber);
    }
}


void ImagePlaybackControlDialog::readSettings() {

    const QByteArray m_playbackSpeed = applicationSettings->value("playbackSpeed", QByteArray()).toByteArray();
    if (!m_playbackSpeed.isEmpty())
        setPlaybackSpeed(m_playbackSpeed.toInt());
    else
        setPlaybackSpeed(30);

    const QByteArray m_playbackLoop = applicationSettings->value("playbackLoop", QByteArray()).toByteArray();
    playbackLoop = true;
    // GB: I tried but this does not work, always reads "true" or "false", but not "1" or "0", and Int conversion fails accordingly. I modified the code accordingly
    //playbackLoop = (bool) m_playbackLoop.toInt(); 
    if (!m_playbackLoop.isEmpty() && (m_playbackLoop == "0" || m_playbackLoop == "false"))
        playbackLoop = false;

    const QByteArray m_syncRecordCsv = applicationSettings->value("syncRecordCsv", QByteArray()).toByteArray();
    syncRecordCsv = true;
    if (!m_syncRecordCsv.isEmpty() && (m_syncRecordCsv == "0" || m_syncRecordCsv == "false"))
        syncRecordCsv = false;

    const QByteArray m_syncStream = applicationSettings->value("syncStream", QByteArray()).toByteArray();
    syncStream = true;
    if (!m_syncStream.isEmpty() && (m_syncStream == "0" || m_syncStream == "false"))
        syncStream = false;
}

/*
// Save settings to QT application settings 
void ImagePlaybackControlDialog::saveSettings() {

    applicationSettings->setValue("playbackSpeed", playbackSpeed);
    applicationSettings->setValue("playbackLoop", playbackLoop);
    applicationSettings->setValue("syncRecordCsv", syncRecordCsv);
    applicationSettings->setValue("syncStream", syncStream);
}
*/

void ImagePlaybackControlDialog::updateForm() {
    playbackFPSVal->setValue(playbackSpeed);
    loopBox->setChecked(playbackLoop);
    syncRecordCsvBox->setChecked(syncRecordCsv);
    syncStreamBox->setChecked(syncStream);
    
    this->update();
}

// Set the playback speed in frames per second
void ImagePlaybackControlDialog::setPlaybackSpeed(int m_playbackSpeed) {
    playbackSpeed = m_playbackSpeed;
    applicationSettings->setValue("playbackSpeed", playbackSpeed);

    // NOTE: this was previously done in MainWindow:onGeneralSettingsChange();
    if(fileCamera->getPlaybackSpeed() != playbackSpeed) {
        fileCamera->setPlaybackSpeed(playbackSpeed);
    }
}

// Set that playback is looped infinitely
void ImagePlaybackControlDialog::setPlaybackLoop(int m_state) {
    playbackLoop = (bool) m_state;
    applicationSettings->setValue("playbackLoop", playbackLoop);

    // NOTE: this was previously done in MainWindow:onGeneralSettingsChange();
    if(fileCamera->getPlaybackLoop() != static_cast<int>(playbackLoop)) {
        fileCamera->setPlaybackLoop(playbackLoop);
    }
}

bool ImagePlaybackControlDialog::getSyncRecordCsv() {
    return syncRecordCsv;
}

bool ImagePlaybackControlDialog::getSyncStream() {
    return syncStream;
}

bool ImagePlaybackControlDialog::getPlayImagesOn()
{
    return playImagesOn;
}

// Set whether we want csv recording to start/pause on playback start/(pause/stop)
void ImagePlaybackControlDialog::setSyncRecordCsv(int m_state) {
    syncRecordCsv = (bool) m_state;
    applicationSettings->setValue("syncRecordCsv", syncRecordCsv);

    // TODO
}

// Set whether we want streaming to start/pause on playback start/(pause/stop)
void ImagePlaybackControlDialog::setSyncStream(int m_state) {
    syncStream = (bool) m_state;
    applicationSettings->setValue("syncStream", syncStream);

    // TODO
}

void ImagePlaybackControlDialog::onCameraPlaybackChanged()
{
    if(playImagesOn) {
        std::cout<<"Pausing FileCamera Click"<<std::endl;
        fileCamera->pause();

        //stalledTimestamp = fileCamera->getLastCommissionedTimestamp();
        stalledTimestamp = fileCamera->getTimestampForFrameNumber(fileCamera->getLastCommissionedFrameNumber());
        // GB NOTE: I have already added a guiderail in imageReader.cpp, but still, rarely just the image corresponding to the last commissioned frame number (and its timestamp) does never arrive at updateInfo(quint64 timestamp, int frameNumber)
        std::cout<<"Pausing, lastTimestamp: "<< lastTimestamp << std::endl;
        std::cout<<"Pausing, stalledTimestamp: "<< stalledTimestamp << std::endl;
        if(lastTimestamp < stalledTimestamp) {
            slider->setEnabled(false);
            dial->setEnabled(false);
            startPauseButton->setEnabled(false);
            stopButton->setEnabled(false);
            
            infoGroup->setEnabled(false);

            playbackStalled = true;
            waitingForReset = false;

        } else {
            dial->setEnabled(true); 
        }

        const QIcon icon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-start.svg"), applicationSettings);
        startPauseButton->setIcon(icon);
        playImagesOn = false;
        enableWidgets(false);
    } else {
        
        //stalledTimestamp = 0;

        // GB: dial can be buggy when touched during playing is on, so disabled it if play is on. 
        // Can be operated separately, when paused
        dial->setEnabled(false); 

        emit onPlaybackSafelyStarted();

        std::cout<<"Starting FileCamera Click"<<std::endl;
        fileCamera->start();

        const QIcon icon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/media-playback-pause.svg"), applicationSettings);
        startPauseButton->setIcon(icon);
        playImagesOn = true;
        enableWidgets(true);
    }

    this->update(); // invalidate 
}

void ImagePlaybackControlDialog::onFrameSelected(int frameNumber){
    if (!playImagesOn){
        selectedFrameVal = frameNumber;

        slider->blockSignals(true);
        int gg = floor(99*((selectedFrameVal+1)/(float)numImagesTotal));
        slider->setColorTickPos((selectedFrameVal)/(float)numImagesTotal);
        slider->setValue( gg );
        slider->blockSignals(false);
        fileCamera->seekToFrame(selectedFrameVal -1);
        updateInfoInternal(selectedFrameVal - 1);
        emit stillImageChange(selectedFrameVal - 1);
    }
}

void ImagePlaybackControlDialog::enableWidgets(bool enable){
        selectedFrameBox->setReadOnly(enable);
        selectedFrameBox->setDisabled(enable);
        timestampVal->setReadOnly(enable);
        timestampVal->setDisabled(enable);
}

void ImagePlaybackControlDialog::onTimestampSelected(double frameNumber){
        if (!playImagesOn){
            selectedFrameBox->setValue(frameNumber);
        }
}