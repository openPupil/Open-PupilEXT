
#include "stereoCamera.h"
#include "hardwareTriggerConfiguration.h"
#include <pylon/TlFactory.h>

// Creates a new stereo camera
// The stereo camera is implemented using Pylon's CBaslerUniversalInstantCameraArray
// A stereo camera consists of two cameras configured to receive hardware trigger signals
// Stereo camera images handled using a StereoCameraImageEventHandler
StereoCamera::StereoCamera(QObject* parent) : Camera(parent),
            cameras(2),
            cameraImageEventHandler(new StereoCameraImageEventHandler(parent)),
            frameCounter(new CameraFrameRateCounter(parent)),
            cameraCalibration(new StereoCameraCalibration()),
            calibrationThread(new QThread()),
            lineSource("Line1") {

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    // Calibration thread working
    cameraCalibration->moveToThread(calibrationThread);
    connect(calibrationThread, SIGNAL (finished()), calibrationThread, SLOT (deleteLater()));
    calibrationThread->start();
    calibrationThread->setPriority(QThread::HighPriority);

    connect(cameraImageEventHandler, SIGNAL(onNewGrabResult(CameraImage)), this, SIGNAL(onNewGrabResult(CameraImage)));
    connect(cameraImageEventHandler, SIGNAL(onNewGrabResult(CameraImage)), frameCounter, SLOT(count(CameraImage)));
    //connect(cameraImageEventHandler, SIGNAL(needsTimeSynchronization()), this, SLOT(resynchronizeTime()));

    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    connect(frameCounter, SIGNAL(framecount(int)), this, SIGNAL(framecount(int)));
}

// Creates a stereo camera and attaches the two given Pylon camera device information
StereoCamera::StereoCamera(const CDeviceInfo &diMain, const CDeviceInfo &diSecondary, QObject* parent)
        : StereoCamera(parent) {

    cameras[0].Attach(CTlFactory::GetInstance().CreateDevice(diMain));
    cameras[1].Attach(CTlFactory::GetInstance().CreateDevice(diSecondary));
}

// Creates a stereo camera and attaches the two given Pylon device names (fullnames)
StereoCamera::StereoCamera(const String_t &fullnameMain, const String_t &fullnameSecondary, QObject* parent)
        : StereoCamera(CDeviceInfo().SetFullName(fullnameMain), CDeviceInfo().SetFullName(fullnameSecondary), parent) {

}

// Destroys the stereo camera
// Closes the camera array
StereoCamera::~StereoCamera() {
    if(cameras.IsOpen()) {
        if(cameras.IsGrabbing())
            cameras.StopGrabbing();
        cameras.Close();
    }
    delete cameraImageEventHandler;
}

// Attaches the main and secondary cameras to the camera array, based on their given device information
void StereoCamera::attachCameras(const CDeviceInfo &diMain, const CDeviceInfo &diSecondary) {

    // If cameras are already attached to the array, remove them
    if(cameras.GetSize() > 0) {
        if(cameras.IsGrabbing())
            cameras.StopGrabbing();
        cameras.Close();
        cameras.DetachDevice();
        cameras.DestroyDevice();
    }

    cameras[0].Attach(CTlFactory::GetInstance().CreateDevice(diMain));
    cameras[1].Attach(CTlFactory::GetInstance().CreateDevice(diSecondary));

    std::cout<<"Attached Camera0:" << cameras[0].GetDeviceInfo().GetFriendlyName() << std::endl;
    std::cout<<"Attached Camera1:" << cameras[1].GetDeviceInfo().GetFriendlyName() << std::endl << std::endl;
}

// Opens the stereo camera through its corresponding camera array
// If the stereo camera is already open, it is first closed and then reopened again
// CAUTION: Its important for the stereo cameras to be in sync, that the stereo camera is first opened, and only then the hardware trigger source is started
// If the hardware triggers are started before opening the camera, the camera images will not be in sync due to the sequential opening of the camera
// (one camera will receive a trigger signal before the other)
void StereoCamera::open() {

    if(cameras.GetSize() < 2) {
        std::cerr << "StereoCamera: must have two cameras connected."<< std::endl;
        return;
    }

    if(cameras.IsOpen()) {
        if(cameras.IsGrabbing())
            cameras.StopGrabbing();
        cameras.Close();
    }

    try {
        // Register the configurations of the cameras, setting both to receive hardware trigger signals on the given line source
        //cameras[0].RegisterConfiguration(new HardwareTriggerConfiguration(lineSource), RegistrationMode_ReplaceAll, Cleanup_Delete);
        //cameras[1].RegisterConfiguration(new HardwareTriggerConfiguration(lineSource), RegistrationMode_ReplaceAll, Cleanup_Delete);

        // Register the image event handler
        // IMPORTANT: For both cameras the same handler object is registered, as the handler must receive both main and secondary images to create a single stereo camera image
        cameras[0].RegisterImageEventHandler(cameraImageEventHandler, RegistrationMode_ReplaceAll, Cleanup_None); // Cleanup_None as its deleted in unregister
        cameras[1].RegisterImageEventHandler(cameraImageEventHandler, RegistrationMode_ReplaceAll, Cleanup_None);

        cameras.Open();

        // Synchronize the camera time to the system time
        synchronizeTime();
        
        cameraImageEventHandler->setTimeSynchronization(cameraMainTime, cameraSecondaryTime, systemTime);

        cameras[0].PixelFormat.SetValue(PixelFormat_Mono8);
        cameras[1].PixelFormat.SetValue(PixelFormat_Mono8);

        // Load calibration if existing
        if(!cameraCalibration->isCalibrated()) {
            // If we already used this camera before, a config file may exists
            loadCalibrationFile();
        }

        if (cameras[0].CanWaitForFrameTriggerReady() && cameras[1].CanWaitForFrameTriggerReady()) {

            // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
            // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
            // The GrabStrategy_OneByOne default grab strategy is used.
            cameras.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
        } else {
            // See the documentation of CInstantCamera::CanWaitForFrameTriggerReady() for more information.
            std::cout << std::endl;
            std::cout << "Error: This sample can only be used with cameras that can be queried whether they are ready to accept the next frame trigger.";
            std::cout << std::endl;
            std::cout << std::endl;
        }
    }
    catch (const GenericException &e) {
        std::cerr << "A Pylon exception occurred." << std::endl<< e.GetDescription() << std::endl;
        cameras.Close();
        cameras.DetachDevice();
        cameras.DestroyDevice();
    }
}

// Synchronize the camera to system time
// At the given point in time, both camera and system time are recorded, which is then used to convert between them for later image timestamps
void StereoCamera::synchronizeTime() {

    if(cameras.GetSize() < 2) {
        std::cerr << "StereoCamera: must have two cameras connected."<< std::endl;
        return;
    }

    if (!isEmulated()){
        cameras[0].TimestampLatch.Execute();
        cameras[1].TimestampLatch.Execute();
    }
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> epoche = std::chrono::time_point<std::chrono::system_clock>{};


    if (!isEmulated()){
        cameraMainTime = static_cast<uint64>(cameras[0].TimestampLatchValue.GetValue());
        cameraSecondaryTime = static_cast<uint64>(cameras[1].TimestampLatchValue.GetValue());
    }
    else {
        cameraMainTime = static_cast<uint64>(start.time_since_epoch().count());
        cameraSecondaryTime = static_cast<uint64>(start.time_since_epoch().count());
    }
    

    systemTime  = std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch()).count();
    std::time_t startTime = std::chrono::system_clock::to_time_t(start);
    std::time_t epochTime = std::chrono::system_clock::to_time_t(epoche);

    std::cout << "Camera Synchronize Time" << std::endl << "=========================" << std::endl;
    std::cout << "Timestamp Camera Main: " << cameraMainTime << std::endl;
    std::cout << "Timestamp Camera Secondary: " << cameraSecondaryTime << std::endl;
    std::cout << "Timestamp System: " << systemTime << std::endl;
    std::cout << "System Epoch: " << std::ctime(&epochTime) << std::endl;
    std::cout << "System Time: " << std::ctime(&startTime) << std::endl;
    std::cout << "Time from Epoch (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count() << std::endl;
    std::cout << "Time from Epoch (us): " << std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch()).count() << std::endl;
    std::cout << "Time from Epoch (ns): " << std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch()).count() << std::endl;
    std::cout << "=========================" << std::endl;
}


bool StereoCamera::isOpen() {
    return cameras.IsOpen();
}

// Close the stereo camera and release all Pylon resources
void StereoCamera::close() {

    std::cout << "StereoCamera: Releasing pylon resources.";
    cameras.StopGrabbing();

    for(int i = 0; i<cameras.GetSize(); i++) {
        cameras[i].DeregisterImageEventHandler(cameraImageEventHandler);
    }

    cameras.Close();
}

// Current exposure time value of the main camera
int StereoCamera::getExposureTimeValue() {
    if (cameras.GetSize() > 0 && cameras[0].ExposureTime.IsReadable()) {
        return cameras[0].ExposureTime.GetValue();
    }
    if (cameras.GetSize() > 0 && cameras[0].ExposureTimeAbs.IsReadable()) {
        return cameras[0].ExposureTimeAbs.GetValue();
    }
    return 0;
}

// Minimal possible exposure time value of the main camera
int StereoCamera::getExposureTimeMin() {
    if (cameras.GetSize() > 0 && cameras[0].ExposureTime.IsReadable()) {
        return cameras[0].ExposureTime.GetMin();
    }
    if (cameras.GetSize() > 0 && cameras[0].ExposureTimeAbs.IsReadable()) {
        return cameras[0].ExposureTimeAbs.GetMin();
    }
    return 0;
}

// Maximal possible exposure time value of the main camera
int StereoCamera::getExposureTimeMax() {
    if (cameras.GetSize() > 0 && cameras[0].ExposureTime.IsReadable()) {
        return cameras[0].ExposureTime.GetMax();
    }
    if (cameras.GetSize() > 0 && cameras[0].ExposureTimeAbs.IsReadable()) {
        return cameras[0].ExposureTimeAbs.GetMax();
    }
    return 0;
}

// Current Gain value of the main camera
double StereoCamera::getGainValue() {
    if (cameras.GetSize() > 0 && cameras[0].Gain.IsReadable()) {
        return cameras[0].Gain.GetValue();
    }
    if (cameras.GetSize() > 0 && cameras[0].GainRaw.IsReadable()) {
        return cameras[0].GainRaw.GetValue();
    }
    return 0;
}

// Minimal possible Gain value of the main camera
double StereoCamera::getGainMin() {
    if (cameras.GetSize() > 0 && cameras[0].Gain.IsReadable()) {
        return cameras[0].Gain.GetMin();
    }
    if (cameras.GetSize() > 0 && cameras[0].GainRaw.IsReadable()) {
        return cameras[0].GainRaw.GetMin();
    }
    return 0;
}

// Maximal possible Gain value of the main camera
double StereoCamera::getGainMax() {
    if (cameras.GetSize() > 0 && cameras[0].Gain.IsReadable()) {
        return cameras[0].Gain.GetMax();
    }
    if (cameras.GetSize() > 0 && cameras[0].GainRaw.IsReadable()) {
        return cameras[0].GainRaw.GetMax();
    }
    return 0;
}

// Sets the Gain value of the main and secondary camera
void StereoCamera::setGainValue(double value) {
    if (isEmulated()){
        if (cameras.GetSize() == 2 && cameras[0].GainRaw.IsWritable() && cameras[1].GainRaw.IsWritable() &&  value >= getGainMin() && value <= getGainMax()) {
            qDebug() << cameras[0].GainRaw.GetMin();
            qDebug() << cameras[1].GainRaw.GetMin();
            qDebug() << cameras[0].GainRaw.GetMax();
            qDebug() << cameras[1].GainRaw.GetMax();
            int intValue = static_cast<int>(value);
            cameras[0].GainRaw.TrySetValue(intValue);
            cameras[1].GainRaw.TrySetValue(intValue);
        }
    }
    else {
        if (cameras.GetSize() == 2 && cameras[0].Gain.IsWritable() && cameras[1].Gain.IsWritable()) {
            cameras[0].Gain.TrySetValue(value);
            cameras[1].Gain.TrySetValue(value);
        }
    }

}

// Sets the exposure time value of the main and secondary camera
void StereoCamera::setExposureTimeValue(int value) {
    if (isEmulated()){
        if (cameras.GetSize() == 2 && cameras[0].ExposureTimeAbs.IsWritable() && cameras[1].ExposureTimeAbs.IsWritable() && value != 0) {
            std::cout<<"Writing exposure value: " << value <<std::endl;
            cameras[0].ExposureTimeAbs.TrySetValue(value);
            cameras[1].ExposureTimeAbs.TrySetValue(value);
        } 
    }
    else {
        if (cameras.GetSize() == 2 && cameras[0].ExposureTime.IsWritable() && cameras[1].ExposureTime.IsWritable()) {
            std::cout<<"Writing exposure value: " << value <<std::endl;
            cameras[0].ExposureTime.TrySetValue(value);
            cameras[1].ExposureTime.TrySetValue(value);
        }
    }
}

// Loads the camera settings of the main camera and sets its values to both the main and secondary camera
// Reads the Basler specific camera setting file format
// Camera settings are automatically saved in the applications settings directory (Path visible in the about window)
void StereoCamera::loadMainFromFile(const String_t &filename) {

    bool wasOpen = cameras.IsOpen();
    if(!wasOpen) {
        cameras.Open();
    }

    try {
        CFeaturePersistence::Load(filename, &cameras[0].GetNodeMap(), true);
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
    }

    // Set the main and second camera with same settings
    setExposureTimeValue(cameras[0].ExposureTime.GetValue());
    setGainValue(cameras[0].Gain.GetValue());

    if(isEnabledAcquisitionFrameRate()) {
        setAcquisitionFPSValue(cameras[0].AcquisitionFrameRate.GetValue());
    }
    if(!wasOpen) {
        cameras.Close();
    }
}

// Saves the main camera settings to file
// Uses the Basler specific file format
void StereoCamera::saveMainToFile(const String_t &filename) {
    try {
        CFeaturePersistence::Save(filename, &cameras[0].GetNodeMap());
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
    }
}

// Reads if a image acquisition frame rate is enabled
// The value of the image acquisition may overwrite hardware trigger framerates
// Assumes that main and secondary camera have the same settings
bool StereoCamera::isEnabledAcquisitionFrameRate() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRateEnable.IsReadable()) {
        return cameras[0].AcquisitionFrameRateEnable.GetValue();
    }
    return false;
}

bool StereoCamera::isEmulated()
{
    if (cameras.GetSize() == 2){
        String_t device1_name = cameras[0].GetDeviceInfo().GetModelName();
        String_t device2_name = cameras[1].GetDeviceInfo().GetModelName();
        qDebug() << QString(device1_name) << QString(device2_name);
        return ((device1_name.find("Emu") != String_t::npos) || (device2_name.find("Emu") != String_t::npos));    
    }
    else return false;
    
}

// Enables image acquisition frame rate for both cameras
void StereoCamera::enableAcquisitionFrameRate(bool enabled) {
    if (cameras.GetSize() == 2 && cameras[0].AcquisitionFrameRateEnable.IsWritable() && cameras[1].AcquisitionFrameRateEnable.IsWritable()) {
        cameras[0].AcquisitionFrameRateEnable.TrySetValue(enabled);
        cameras[1].AcquisitionFrameRateEnable.TrySetValue(enabled);
    }
}

// Sets the value of the image acquisition frame rate for both cameras
void StereoCamera::setAcquisitionFPSValue(int value) {
    if (isEmulated()){
        if (cameras.GetSize() == 2 && cameras[0].AcquisitionFrameRateAbs.IsWritable() && cameras[1].AcquisitionFrameRateAbs.IsWritable()) {
            if (value <= 0 )
                value = 10;
            cameras[0].AcquisitionFrameRateAbs.TrySetValue(value);
            cameras[1].AcquisitionFrameRateAbs.TrySetValue(value);
        }
    } else{
        if (cameras.GetSize() == 2 && cameras[0].AcquisitionFrameRate.IsWritable() && cameras[1].AcquisitionFrameRate.IsWritable()) {
            cameras[0].AcquisitionFrameRate.TrySetValue(value);
            cameras[1].AcquisitionFrameRate.TrySetValue(value);
        }
    }
    
}

// Reads and returns the value of the image acquisition frame rate for the main camera
// Assumes that both cameras are set the same through the above function setAcquisitionFPSValue
int StereoCamera::getAcquisitionFPSValue() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRate.IsReadable()) {
        return cameras[0].AcquisitionFrameRate.GetValue();
    }
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRateAbs.IsReadable()) {
        return cameras[0].AcquisitionFrameRateAbs.GetValue();
    }
    return 0;
}

// Minimal possible image acquisition frame rate of the main camera
int StereoCamera::getAcquisitionFPSMin() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRate.IsReadable()) {
        return cameras[0].AcquisitionFrameRate.GetMin();
    }
    else if(cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRateAbs.IsReadable()) {
        return cameras[0].AcquisitionFrameRateAbs.GetMin();
    }
    return 0;
}

// Maximal possible image acquisition frame rate of the main camera
// This may be influenced by the current camera settings and its value may change
int StereoCamera::getAcquisitionFPSMax() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRate.IsReadable()) {
        return cameras[0].AcquisitionFrameRate.GetMax();
    }
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRateAbs.IsReadable()) {
        return cameras[0].AcquisitionFrameRateAbs.GetMax();
    }
    return 0;
}

// Camera frame rate resulting by the current camera settings
double StereoCamera::getResultingFrameRateValue() {
    if (cameras.GetSize() > 0 && cameras[0].ResultingFrameRate.IsReadable()) {
        return cameras[0].ResultingFrameRate.GetValue();
    }
        if (cameras.GetSize() > 0 && cameras[0].ResultingFrameRateAbs.IsReadable()) {
        return cameras[0].ResultingFrameRateAbs.GetValue();
    }
    return 0;
}

StereoCameraCalibration *StereoCamera::getCameraCalibration() {
    return cameraCalibration;
}

// Performs automatically setting of the Gain value based on the current camera image
// Main camera is used to automatically find a Gain value, this value is then applied to the secondary camera
void StereoCamera::autoGainOnce() {

    if(cameras.GetSize() < 2) {
        return;
    }

    // We set Gain value of both cameras by first auto Gain once for the main camera and then set the resulting value for the second
    try {

        if(!cameras.IsOpen()) {
            cameras.Open();
        }

        cameras.StopGrabbing();

        // Turn test image off.
        cameras[0].TestImageSelector.TrySetValue(TestImageSelector_Off);
        cameras[0].TestPattern.TrySetValue(TestPattern_Off);

        // Only area scan cameras support auto functions.
        if (cameras[0].DeviceScanType.GetValue() == DeviceScanType_Areascan) {
            // Cameras based on SFNC 2.0 or later, e.g., USB cameras
            if (cameras[0].GetSfncVersion() >= Sfnc_2_0_0) {
                // All area scan cameras support luminance control.
                // Carry out luminance control by using the "once" gain auto function.
                // For demonstration purposes only, set the gain to an initial value. TODO

                std::cout << "Starting AutoGain..." << std::endl;

                //camera.Gain.SetToMaximum();
                cameras[0].Gain.TrySetToMaximum();


                if (!cameras[0].GainAuto.IsWritable()) {
                    std::cout << "The camera does not support Gain Auto." << std::endl;
                    return;
                }

                // Maximize the grabbed image area of interest (Image AOI).
                cameras[0].OffsetX.TrySetToMinimum();
                cameras[0].OffsetY.TrySetToMinimum();
                cameras[0].Width.TrySetToMaximum();
                cameras[0].Height.TrySetToMaximum();

                if (cameras[0].AutoFunctionROISelector.IsWritable()) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    // Set the Auto Function ROI for luminance statistics.
                    // We want to use ROI1 for gathering the statistics

                    cameras[0].AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);
                    cameras[0].AutoFunctionROIUseBrightness.TrySetValue(true);   // ROI 1 is used for brightness control
                    cameras[0].AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI2);
                    cameras[0].AutoFunctionROIUseBrightness.TrySetValue(false);   // ROI 2 is not used for brightness control

                    // Set the ROI (in this example the complete sensor is used)
                    cameras[0].AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);  // configure ROI 1
                    cameras[0].AutoFunctionROIOffsetX.SetValue(cameras[0].OffsetX.GetMin());
                    cameras[0].AutoFunctionROIOffsetY.SetValue(cameras[0].OffsetY.GetMin());
                    cameras[0].AutoFunctionROIWidth.SetValue(cameras[0].Width.GetMax());
                    cameras[0].AutoFunctionROIHeight.SetValue(cameras[0].Height.GetMax());
                }

                if (cameras[0].GetSfncVersion() >= Sfnc_2_0_0) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    // Set the target value for luminance control.
                    // A value of 0.3 means that the target brightness is 30 % of the maximum brightness of the raw pixel value read out from the sensor.
                    // A value of 0.4 means 40 % and so forth.
                    cameras[0].AutoTargetBrightness.SetValue(0.3);

                    // We are going to try GainAuto = Once.

                    std::cout << "Trying 'GainAuto = Once'." << std::endl;
                    std::cout << "Initial Gain = " << cameras[0].Gain.GetValue() << std::endl;

                    // Set the gain ranges for luminance control.
                    cameras[0].AutoGainLowerLimit.SetValue(cameras[0].Gain.GetMin());
                    cameras[0].AutoGainUpperLimit.SetValue(cameras[0].Gain.GetMax());
                }

                cameras[0].GainAuto.SetValue(GainAuto_Once);

                // When the "once" mode of operation is selected,
                // the parameter values are automatically adjusted until the related image property
                // reaches the target value. After the automatic parameter value adjustment is complete, the auto
                // function will automatically be set to "off" and the new parameter value will be applied to the
                // subsequently grabbed images.

                int n = 0;
                while (cameras[0].GainAuto.GetValue() != GainAuto_Off) {
                    CBaslerUniversalGrabResultPtr ptrGrabResult;
                    cameras[0].GrabOne( 5000, ptrGrabResult);
                    ++n;
                    //Make sure the loop is exited.
                    if (n > 100) {
                        throw TIMEOUT_EXCEPTION( "The adjustment of auto gain did not finish.");
                    }
                }

                std::cout << "GainAuto went back to 'Off' after " << n << " frames." << std::endl;
                if(cameras[0].Gain.IsReadable()) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    std::cout << "Final Gain = " << cameras[0].Gain.GetValue() << std::endl;
                }

                // set Gain value for second camera
                cameras[1].Gain.TrySetValue(cameras[0].Gain.GetValue());

                cameras.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
            }
        } else {
            std::cerr << "Only area scan cameras support auto functions." << std::endl;
        }
    }
    catch (const TimeoutException &e)
    {
        // Auto functions did not finish in time.
        // Maybe the cap on the lens is still on or there is not enough light.
        std::cerr << "A timeout has occurred: " << std::endl << e.GetDescription() << std::endl;
        std::cerr << "Please make sure you remove the cap from the camera lens before running auto gain." << std::endl;
    }
    catch (const GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
    }
}

// Performs automatically setting of the exposure time value based on the current main camera image
// Main camera is used to automatically find a exposure time, this value is then applied to the secondary camera
void StereoCamera::autoExposureOnce() {

    if(cameras.GetSize() < 2) {
        return;
    }

    // We set Exposure value of both cameras by first auto Exposure once for the main camera and then set the resulting value for the second
    try {

        if(!cameras.IsOpen()) {
            cameras.Open();
        }

        cameras.StopGrabbing();

        // Turn test image off.
        cameras[0].TestImageSelector.TrySetValue(TestImageSelector_Off);
        cameras[0].TestPattern.TrySetValue(TestPattern_Off);

        // Only area scan cameras support auto functions.
        if (cameras[0].DeviceScanType.GetValue() == DeviceScanType_Areascan) {
            // Cameras based on SFNC 2.0 or later, e.g., USB cameras
            if (cameras[0].GetSfncVersion() >= Sfnc_2_0_0) {
                // For demonstration purposes only, set the exposure time to an initial value.
                cameras[0].ExposureTime.SetToMinimum();
                // Carry out luminance control by using the "once" exposure auto function.


                if (!cameras[0].ExposureAuto.IsWritable())
                {
                    std::cout << "The camera does not support Exposure Auto." << std::endl;
                    return;
                }

                // Maximize the grabbed area of interest (Image AOI).
                cameras[0].OffsetX.TrySetToMinimum();
                cameras[0].OffsetY.TrySetToMinimum();
                cameras[0].Width.SetToMaximum();
                cameras[0].Height.SetToMaximum();

                if (cameras[0].AutoFunctionROISelector.IsWritable())
                {
                    // Set the Auto Function ROI for luminance statistics.
                    // We want to use ROI1 for gathering the statistics
                    cameras[0].AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);
                    cameras[0].AutoFunctionROIUseBrightness.TrySetValue(true);   // ROI 1 is used for brightness control
                    cameras[0].AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI2);
                    cameras[0].AutoFunctionROIUseBrightness.TrySetValue(false);   // ROI 2 is not used for brightness control

                    // Set the ROI (in this example the complete sensor is used)
                    cameras[0].AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);  // configure ROI 1
                    cameras[0].AutoFunctionROIOffsetX.SetValue(cameras[0].OffsetX.GetMin());
                    cameras[0].AutoFunctionROIOffsetY.SetValue(cameras[0].OffsetY.GetMin());
                    cameras[0].AutoFunctionROIWidth.SetValue(cameras[0].Width.GetMax());
                    cameras[0].AutoFunctionROIHeight.SetValue(cameras[0].Height.GetMax());
                }

                if (cameras[0].GetSfncVersion() >= Sfnc_2_0_0) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    // Set the target value for luminance control.
                    // A value of 0.3 means that the target brightness is 30 % of the maximum brightness of the raw pixel value read out from the sensor.
                    // A value of 0.4 means 40 % and so forth.
                    cameras[0].AutoTargetBrightness.SetValue(0.3);

                    // Try ExposureAuto = Once.
                    std::cout << "Trying 'ExposureAuto = Once'." << std::endl;
                    std::cout << "Initial exposure time = ";
                    std::cout << cameras[0].ExposureTime.GetValue() << " us" << std::endl;

                    // Set the exposure time ranges for luminance control.
                    cameras[0].AutoExposureTimeLowerLimit.SetValue(cameras[0].AutoExposureTimeLowerLimit.GetMin());
                    cameras[0].AutoExposureTimeUpperLimit.SetValue(cameras[0].AutoExposureTimeLowerLimit.GetMax());

                    cameras[0].ExposureAuto.SetValue(ExposureAuto_Once);
                }

                // When the "once" mode of operation is selected,
                // the parameter values are automatically adjusted until the related image property
                // reaches the target value. After the automatic parameter value adjustment is complete, the auto
                // function will automatically be set to "off", and the new parameter value will be applied to the
                // subsequently grabbed images.
                int n = 0;
                while (cameras[0].ExposureAuto.GetValue() != ExposureAuto_Off)
                {
                    CBaslerUniversalGrabResultPtr ptrGrabResult;
                    cameras[0].GrabOne(5000, ptrGrabResult);
                    ++n;

                    //Make sure the loop is exited.
                    if (n > 100) {
                        throw TIMEOUT_EXCEPTION( "The adjustment of auto exposure did not finish.");
                    }
                }

                std::cout << "ExposureAuto went back to 'Off' after " << n << " frames." << std::endl;
                std::cout << "Final exposure time = ";

                if (cameras[0].ExposureTime.IsReadable()) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    std::cout << cameras[0].ExposureTime.GetValue() << " us" << std::endl;
                }

                // Set value of second camera
                cameras[1].ExposureTime.TrySetValue(cameras[0].ExposureTime.GetValue());

                cameras.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
            }
        } else {
            std::cerr << "Only area scan cameras support auto functions." << std::endl;
        }
    }
    catch (const TimeoutException &e)
    {
        // Auto functions did not finish in time.
        // Maybe the cap on the lens is still on or there is not enough light.
        std::cerr << "A timeout has occurred: " << e.GetDescription() << std::endl;
        std::cerr << "Please make sure you remove the cap from the camera lens before running this sample." << std::endl;
    }
    catch (const GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
    }
}

// The current used linesource as the hardware trigger source
// The same linesource is used for both cameras
// When the linesource changes, the camera must be closed and opened again
String_t StereoCamera::getLineSource() {
    return lineSource;
}

// Sets the linesource used as the hardware trigger source
// The same linesource is used for both cameras
// When the linesource changes, the camera must be closed and opened again
void StereoCamera::setLineSource(String_t value) {
    lineSource = value;
}

CameraImageType StereoCamera::getType() {
    return CameraImageType::LIVE_STEREO_CAMERA;
}

void StereoCamera::startGrabbing()
{
    if (cameras.IsOpen() && !cameras.IsGrabbing())
        cameras.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
}

void StereoCamera::stopGrabbing()
{
    if (cameras.IsOpen() && cameras.IsGrabbing())
        cameras.StopGrabbing();
}

// Returns a list of the friendly device names of the connected cameras
std::vector<QString> StereoCamera::getFriendlyNames() {
    std::vector<QString> names;

    for(int i = 0; i<cameras.GetSize(); i++) {
        names.push_back(QString(cameras[i].GetDeviceInfo().GetFriendlyName()));
    }

    return names;
}

// Returns the calibration filename describing a calibration setting consisting of the two cameras and the configured calibration pattern
QString StereoCamera::getCalibrationFilename() {
    if(cameras.GetSize() == 2) {
        return settingsDirectory.filePath(
                getFriendlyNames()[0] + "_" + getFriendlyNames()[1] + "_stereo_calibration_" +
                QString::number(cameraCalibration->getPattern()) + "_" +
                QString::number(cameraCalibration->getSquareSize()) + "_" +
                QString::number(cameraCalibration->getBoardSize().width + 1) + "x" +
                QString::number(cameraCalibration->getBoardSize().height + 1) + ".xml");
    }

    return QString();
}

// Loads the calibration file defined by getCalibrationFilename() if it exists
void StereoCamera::loadCalibrationFile() {
    QString configFile = getCalibrationFilename();
    configFile.replace(" ", "");
    if (QFile::exists(configFile)) {
        std::cout << "Found calibration file in settings directory. Loading: " << configFile.toStdString()
                  << std::endl;
        cameraCalibration->loadFromFile(configFile.toStdString().c_str());
    }
}

// Synchronize the camera and system time and update the image event handler
void StereoCamera::resynchronizeTime() {
    if(cameras.IsOpen()) {
        std::cout<<"Resynchronizing Camera Time..."<<std::endl;
        synchronizeTime();
        cameraImageEventHandler->setTimeSynchronization(cameraMainTime, cameraSecondaryTime, systemTime);
    }
}


int StereoCamera::getImageROIwidth() { 
    if (cameras.GetSize() != 2 || !cameras[0].Width.IsReadable() || !cameras[1].Width.IsReadable()) {
        return 0;
    }

    int val0 = (int)cameras[0].Width.GetValue();
    int val1 = (int)cameras[1].Width.GetValue();
    if(val0 != val1) {
        std::cout<<"Image acquisition ROI width of the two cameras are not the same. Now resetting both to the lower value."<<std::endl;
        if(val0 < val1) 
            setImageROIwidth(val0);
        else 
            setImageROIwidth(val1);
    }
    return val0;
}

int StereoCamera::getImageROIheight() { 
    if (cameras.GetSize() != 2 || !cameras[0].Height.IsReadable() || !cameras[1].Height.IsReadable()) {
        return 0;
    }

    int val0 = (int)cameras[0].Height.GetValue();
    int val1 = (int)cameras[1].Height.GetValue();
    if(val0 != val1) {
        std::cout<<"Image acquisition ROI height of the two cameras are not the same. Now resetting both to the lower value."<<std::endl;
        if(val0 < val1)
            setImageROIheight(val0);
        else
            setImageROIheight(val1);
    }
    return val0;
}

int StereoCamera::getImageROIoffsetX() { 
    if (cameras.GetSize() != 2 || !cameras[0].OffsetX.IsReadable() || !cameras[1].OffsetX.IsReadable()) {
        return 0;
    }

    int val0 = (int)cameras[0].OffsetX.GetValue();
    int val1 = (int)cameras[1].OffsetX.GetValue();
    if(val0 != val1) {
        std::cout<<"Image acquisition ROI offsetX of the two cameras are not the same. Now resetting both to the lower value."<<std::endl;
        if(val0 < val1)
            setImageROIoffsetX(val0);
        else
            setImageROIoffsetX(val1);
    }
    return val0;
}

int StereoCamera::getImageROIoffsetY() { 
    if (cameras.GetSize() != 2 || !cameras[0].OffsetY.IsReadable() || !cameras[1].OffsetY.IsReadable()) {
        return 0;
    }

    int val0 = (int)cameras[0].OffsetY.GetValue();
    int val1 = (int)cameras[1].OffsetY.GetValue();
    if(val0 != val1) {
        std::cout<<"Image acquisition ROI offsetY of the two cameras are not the same. Now resetting both to the lower value."<<std::endl;
        if(val0 < val1)
            setImageROIoffsetY(val0);
        else
            setImageROIoffsetY(val1);
    }
    return val0;
}

// NOTE: Binning affects this
int StereoCamera::getImageROIwidthMax() {
    if (cameras.GetSize() != 2 || !cameras[0].WidthMax.IsReadable() || !cameras[1].WidthMax.IsReadable()) {
        return 0;
    }

    // Classic/U/L GigE cameras
  //  int val0 = (int)cameras[0].Width.GetMax();
  //  int val1 = (int)cameras[1].Width.GetMax();
    // other cameras
    int val0 = (int)cameras[0].WidthMax.GetValue();
    int val1 = (int)cameras[1].WidthMax.GetValue();
    if(val0 != val1) {
        std::cout<<"Image acquisition ROI max width of the two cameras are not the same. Now using the lower (safer) value."<<std::endl;
        if(val0>val1)
            val0=val1;
    }
    return val0;
}

// NOTE: Binning affects this
int StereoCamera::getImageROIheightMax() {
    if (cameras.GetSize() != 2 || !cameras[0].HeightMax.IsReadable() || !cameras[1].HeightMax.IsReadable()) {
        return 0;
    }

    // Classic/U/L GigE cameras
  //  int val0 = (int)cameras[0].Height.GetMax();
  //  int val1 = (int)cameras[1].Height.GetMax();
    // other cameras
    int val0 = (int)cameras[0].HeightMax.GetValue();
    int val1 = (int)cameras[1].HeightMax.GetValue();
    if(val0 != val1) {
        std::cout<<"Image acquisition ROI max height of the two cameras are not the same. Now using the lower (safer) value."<<std::endl;
        if(val0>val1)
            val0=val1;
    }
    return val0;
}

QRectF StereoCamera::getImageROI(){
    return QRectF(getImageROIoffsetX(),getImageROIoffsetY(),getImageROIwidth(), getImageROIheight());
}

int StereoCamera::getBinningVal() {
    if (cameras.GetSize() != 2 || !cameras[0].BinningHorizontal.IsReadable() || !cameras[1].BinningHorizontal.IsReadable()) {
        return 1;
    }

    int b0 = (int)cameras[0].BinningHorizontal.GetValue();
    int b1 = (int)cameras[1].BinningHorizontal.GetValue();
    if(b0 != b1) {
        std::cout<<"Image acquisition horizontal binning of the two cameras are not the same. Now resetting both to the lower value."<<std::endl;
        setBinningVal(b0);
    }

    return b0;
}

std::vector<double> StereoCamera::getTemperatures() {
    std::vector<double> temperatures = {0.0, 0.0};
    if(cameras.GetSize() != 2)
        return temperatures;
    if(!cameras[0].DeviceTemperature.IsReadable() || !cameras[1].DeviceTemperature.IsReadable())
        return temperatures;

    // DEV
    //qDebug() << cameras[0].GetValue(Basler_UniversalCameraParams::PLCamera::DeviceModelName);
    //qDebug() << cameras[1].GetValue(Basler_UniversalCameraParams::PLCamera::DeviceModelName);

    // this line is only needed in ace 2, boost, and dart IMX Cameras
    // NOTE: SENSOR TEMP (and maybe others too) CAN NOT BE MEASURED WHILE GRABBING, but coreboard is OK anytime
    cameras[0].DeviceTemperatureSelector.SetValue(Basler_UniversalCameraParams::DeviceTemperatureSelectorEnums::DeviceTemperatureSelector_Coreboard);
    cameras[1].DeviceTemperatureSelector.SetValue(Basler_UniversalCameraParams::DeviceTemperatureSelectorEnums::DeviceTemperatureSelector_Coreboard);

    temperatures[0] = cameras[0].DeviceTemperature.GetValue();
    temperatures[1] = cameras[1].DeviceTemperature.GetValue();

    return temperatures;
}

bool StereoCamera::isGrabbing()
{
    return cameras.IsGrabbing();
}

// NOTE: grabbing "pause" is necessary for setting binning
bool StereoCamera::setBinningVal(int value) {
    if (cameras.GetSize() != 2 || !cameras[0].BinningHorizontal.IsReadable() || cameras[1].BinningHorizontal.IsReadable()) {
        return false;
    }

    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // in case of our Basler cameras here, only mode=1,2,4 are only valid values
    if (cameras[0].BinningHorizontal.IsWritable() && cameras[0].BinningVertical.IsWritable() &&
        cameras[1].BinningHorizontal.IsWritable() && cameras[1].BinningVertical.IsWritable()) {
        // "Enable sensor binning"
        // "Note: Available on selected camera models only"
       // camera.BinningSelector.SetValue(BinningSelector_Sensor); // NOTE: found in Basler docs, but no trace of it in Pylon::CBaslerUniversalInstantCamera:: when code tries to compile. What is this?

        // Set "binning mode" of camera
        cameras[0].BinningHorizontalMode.TrySetValue(BinningHorizontalMode_Average);
        //cameras[0].BinningHorizontalMode.SetValue(BinningHorizontalMode_Sum);
        cameras[0].BinningVerticalMode.TrySetValue(BinningVerticalMode_Average);
        //cameras[0].BinningVerticalMode.SetValue(BinningHorizontalMode_Sum);
        //
        cameras[1].BinningHorizontalMode.TrySetValue(BinningHorizontalMode_Average);
        //cameras[1].BinningHorizontalMode.SetValue(BinningHorizontalMode_Sum);
        cameras[1].BinningVerticalMode.TrySetValue(BinningVerticalMode_Average);
        //cameras[1].BinningVerticalMode.SetValue(BinningHorizontalMode_Sum);

        if(value==2 || value==3) {
            cameras[0].BinningHorizontal.TrySetValue(2);
            cameras[0].BinningVertical.TrySetValue(2);
            cameras[1].BinningHorizontal.TrySetValue(2);
            cameras[1].BinningVertical.TrySetValue(2);
            std::cout << "Setting binning to 2 on both axes"<< std::endl;
            success = true;
        } else if(value==4) {
            cameras[0].BinningHorizontal.TrySetValue(4);
            cameras[0].BinningVertical.TrySetValue(4);
            cameras[1].BinningHorizontal.TrySetValue(4);
            cameras[1].BinningVertical.TrySetValue(4);
            std::cout << "Setting binning to 4 on both axes"<< std::endl;
            success = true;
        } else { //if(value==1) {
            cameras[0].BinningHorizontal.TrySetValue(1);
            cameras[0].BinningVertical.TrySetValue(1);
            cameras[1].BinningHorizontal.TrySetValue(1);
            cameras[1].BinningVertical.TrySetValue(1);
            std::cout << "Setting binning to 1 (no binning) on both axes"<< std::endl;
            success = true;
        }
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIwidth(int width) {
    if (cameras.GetSize() != 2 || !cameras[0].BinningHorizontal.IsReadable() || cameras[1].BinningHorizontal.IsReadable()) {
        return false;
    }

    //std::cout << "Setting both cameras Image ROI width=" << std::to_string(width) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    int b0 = cameras[0].BinningHorizontal.GetValue();
    int b1 = cameras[1].BinningHorizontal.GetValue();
    if(b0!=b1) {
        std::cout << "Binning of the two cameras to not match. Now resetting." << std::endl;
        setBinningVal(b0);
    }

    // ace Classic/U/L GigE Cameras
   // int maxWidth = camera.Width.GetMax();
   // int maxHeight = camera.Height.GetMax();
    // other cameras
    int mw0 = cameras[0].WidthMax.GetValue();
    int mw1 = cameras[1].WidthMax.GetValue();
    if(mw0!=mw1) {
        std::cout << "Max px width size of the two cameras to not match." << std::endl;
    }
    int maxWidth = mw0 > mw1 ? mw0 : mw1;

    int offsetX0 = cameras[0].OffsetX.GetValue();
    int offsetX1 = cameras[1].OffsetX.GetValue();

    if(width < 16)
        width=16;

    int modVal=width%4;
    if(modVal != 0)
        width -= modVal;
    
    int wc0, wc1, bestWidth;
    wc0 = wc1 = bestWidth = 16;
    
    if (offsetX0 >= maxWidth-16)
        wc0 = maxWidth-offsetX0;
    if (offsetX1 >= maxWidth-16)
        wc1 = maxWidth-offsetX1;
    bestWidth = wc0 > wc1 ? wc0 : wc1;

    if (bestWidth + offsetX0 <= maxWidth && cameras[0].Width.IsWritable() && cameras[1].Width.IsWritable() ) {
        cameras[0].Width.TrySetValue(bestWidth);
        cameras[1].Width.TrySetValue(bestWidth);
        success = true;
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIheight(int height) {
    if (cameras.GetSize() != 2 || !cameras[0].BinningHorizontal.IsReadable() || cameras[1].BinningHorizontal.IsReadable()) {
        return false;
    }

    //std::cout << "Setting both cameras Image ROI height=" << std::to_string(height) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    int b0 = cameras[0].BinningHorizontal.GetValue();
    int b1 = cameras[1].BinningHorizontal.GetValue();
    if(b0!=b1) {
        std::cout << "Binning of the two cameras to not match. Now resetting." << std::endl;
        setBinningVal(b0);
    }

    // ace Classic/U/L GigE Cameras
   // int maxWidth = camera.Width.GetMax();
   // int maxHeight = camera.Height.GetMax();
    // other cameras
    int mh0 = cameras[0].HeightMax.GetValue();
    int mh1 = cameras[1].HeightMax.GetValue();
    if(mh0!=mh1) {
        std::cout << "Max px height size of the two cameras to not match." << std::endl;
    }
    int maxHeight = mh0 > mh1 ? mh0 : mh1;

    int offsetY0 = cameras[0].OffsetY.GetValue();
    int offsetY1 = cameras[1].OffsetY.GetValue();

    if(height < 16)
        height=16;

    int modVal=height%4;
    if(modVal != 0)
        height -= modVal;
    
    int hc0, hc1, bestHeight;
    hc0 = hc1 = bestHeight = 16;
    
    if (offsetY0 >= maxHeight-16)
        hc0 = maxHeight-offsetY0;
    if (offsetY1 >= maxHeight-16)
        hc1 = maxHeight-offsetY1;
    bestHeight = hc0 > hc1 ? hc0 : hc1;

    if (bestHeight + offsetY0 <= maxHeight && cameras[0].Height.IsWritable() && cameras[1].Height.IsWritable() ) {
        cameras[0].Height.TrySetValue(bestHeight);
        cameras[1].Height.TrySetValue(bestHeight);
        success = true;
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIoffsetX(int offsetX) {
    if (cameras.GetSize() != 2 || !cameras[0].BinningHorizontal.IsReadable() || cameras[1].BinningHorizontal.IsReadable()) {
        return false;
    }

    //std::cout << "Setting Image ROI offsetX=" << std::to_string(offsetX) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // ace Classic/U/L GigE Cameras
   // int maxWidth = cameras[0].Width.GetMax();
   // int maxHeight = cameras[0].Height.GetMax();
    // other cameras
    int maxWidth = cameras[0].WidthMax.GetValue();
    int width = cameras[0].Width.GetValue();

    if(maxWidth - offsetX < 16)
        offsetX = maxWidth - 16;
    //if(width + offsetX > maxWidth)
    //    return;
    
    int modVal=offsetX%4;
    if(modVal != 0)
        offsetX -= modVal;

    if (width + offsetX <= maxWidth && cameras[0].OffsetX.IsWritable() && cameras[1].OffsetX.IsWritable() ) {
        cameras[0].OffsetX.TrySetValue(offsetX);
        cameras[1].OffsetX.TrySetValue(offsetX);
        success = true;
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIoffsetY(int offsetY) {
    if (cameras.GetSize() != 2 || !cameras[0].BinningHorizontal.IsReadable() || cameras[1].BinningHorizontal.IsReadable()) {
        return false;
    }
    
    //std::cout << "Setting Image ROI offsetY=" << std::to_string(offsetY) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // ace Classic/U/L GigE Cameras
   // int maxWidth = cameras[0].Width.GetMax();
   // int maxHeight = cameras[0].Height.GetMax();
    // other cameras
    int maxHeight = cameras[0].HeightMax.GetValue();
    int height = cameras[0].Height.GetValue();

    if(maxHeight - offsetY < 16)
        offsetY = maxHeight - 16;
    //if(height + offsetY > maxHeight)
    //    return;

    int modVal=offsetY%4;
    if(modVal != 0)
        offsetY -= modVal;

    if (height + offsetY <= maxHeight && cameras[0].OffsetY.IsWritable() && cameras[1].OffsetY.IsWritable() ) {
        cameras[0].OffsetY.TrySetValue(offsetY);
        cameras[1].OffsetY.TrySetValue(offsetY);
        success = true;
    } 
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

bool StereoCamera::setImageROIwidthEmu(int width) {
    if (cameras.GetSize() != 2) {
        return false;
    }

    //std::cout << "Setting both cameras Image ROI width=" << std::to_string(width) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // ace Classic/U/L GigE Cameras
   // int maxWidth = camera.Width.GetMax();
   // int maxHeight = camera.Height.GetMax();
    // other cameras
    int mw0 = cameras[0].WidthMax.GetValue();
    int mw1 = cameras[1].WidthMax.GetValue();
    if(mw0!=mw1) {
        std::cout << "Max px width size of the two cameras to not match." << std::endl;
    }
    int maxWidth = mw0 > mw1 ? mw0 : mw1;

    int offsetX0 = cameras[0].OffsetX.GetValue();
    int offsetX1 = cameras[1].OffsetX.GetValue();

    if(width < 16)
        width=16;

    int modVal=width%4;
    if(modVal != 0)
        width -= modVal;
    
    int wc0, wc1, bestWidth;
    wc0 = wc1 = bestWidth = width;
    
    if (offsetX0 >= maxWidth-16)
        wc0 = maxWidth-offsetX0;
    if (offsetX1 >= maxWidth-16)
        wc1 = maxWidth-offsetX1;
    bestWidth = wc0 > wc1 ? wc0 : wc1;

    if (bestWidth + offsetX0 <= maxWidth && cameras[0].Width.IsWritable() && cameras[1].Width.IsWritable() ) {
        cameras[0].Width.TrySetValue(bestWidth);
        cameras[1].Width.TrySetValue(bestWidth);
        success = true;
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIheightEmu(int height) {
    if (cameras.GetSize() != 2) {
        return false;
    }

    //std::cout << "Setting both cameras Image ROI height=" << std::to_string(height) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // ace Classic/U/L GigE Cameras
   // int maxWidth = camera.Width.GetMax();
   // int maxHeight = camera.Height.GetMax();
    // other cameras
    int mh0 = cameras[0].HeightMax.GetValue();
    int mh1 = cameras[1].HeightMax.GetValue();
    if(mh0!=mh1) {
        std::cout << "Max px height size of the two cameras to not match." << std::endl;
    }
    int maxHeight = mh0 > mh1 ? mh0 : mh1;

    int offsetY0 = cameras[0].OffsetY.GetValue();
    int offsetY1 = cameras[1].OffsetY.GetValue();

    if(height < 16)
        height=16;

    int modVal=height%4;
    if(modVal != 0)
        height -= modVal;
    
    int hc0, hc1, bestHeight;
    hc0 = hc1 = bestHeight = height;
    
    if (offsetY0 >= maxHeight-16)
        hc0 = maxHeight-offsetY0;
    if (offsetY1 >= maxHeight-16)
        hc1 = maxHeight-offsetY1;
    bestHeight = hc0 > hc1 ? hc0 : hc1;

    if (bestHeight + offsetY0 <= maxHeight && cameras[0].Height.IsWritable() && cameras[1].Height.IsWritable() ) {
        cameras[0].Height.TrySetValue(bestHeight);
        cameras[1].Height.TrySetValue(bestHeight);
        success = true;
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIoffsetXEmu(int offsetX) {
    if (cameras.GetSize() != 2) {
        return false;
    }

    //std::cout << "Setting Image ROI offsetX=" << std::to_string(offsetX) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // ace Classic/U/L GigE Cameras
   // int maxWidth = cameras[0].Width.GetMax();
   // int maxHeight = cameras[0].Height.GetMax();
    // other cameras
    int maxWidth = cameras[0].WidthMax.GetValue();
    int width = cameras[0].Width.GetValue();

    if(maxWidth - offsetX < 16)
        offsetX = maxWidth - 16;
    //if(width + offsetX > maxWidth)
    //    return;
    
    int modVal=offsetX%4;
    if(modVal != 0)
        offsetX -= modVal;

    if (width + offsetX <= maxWidth && cameras[0].OffsetX.IsWritable() && cameras[1].OffsetX.IsWritable() ) {
        cameras[0].OffsetX.TrySetValue(offsetX);
        cameras[1].OffsetX.TrySetValue(offsetX);
        success = true;
    }
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}

// NOTE: grabbing "pause" is necessary for setting image ROI
bool StereoCamera::setImageROIoffsetYEmu(int offsetY) {
    if (cameras.GetSize() != 2) {
        return false;
    }
    
    //std::cout << "Setting Image ROI offsetY=" << std::to_string(offsetY) << std::endl;
    bool success = false;

    if(cameras[0].IsGrabbing())
        cameras[0].StopGrabbing();
    if(cameras[1].IsGrabbing())
        cameras[1].StopGrabbing();

    // ace Classic/U/L GigE Cameras
   // int maxWidth = cameras[0].Width.GetMax();
   // int maxHeight = cameras[0].Height.GetMax();
    // other cameras
    int maxHeight = cameras[0].HeightMax.GetValue();
    int height = cameras[0].Height.GetValue();

    if(maxHeight - offsetY < 16)
        offsetY = maxHeight - 16;
    //if(height + offsetY > maxHeight)
    //    return;

    int modVal=offsetY%4;
    if(modVal != 0)
        offsetY -= modVal;

    if (height + offsetY <= maxHeight && cameras[0].OffsetY.IsWritable() && cameras[1].OffsetY.IsWritable() ) {
        cameras[0].OffsetY.TrySetValue(offsetY);
        cameras[1].OffsetY.TrySetValue(offsetY);
        success = true;
    } 
    cameras[0].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    cameras[1].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    return success;
}