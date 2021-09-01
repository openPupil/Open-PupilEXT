
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
    connect(cameraImageEventHandler, SIGNAL(needsTimeSynchronization()), this, SLOT(resynchronizeTime()));

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
        cameras[0].RegisterConfiguration(new HardwareTriggerConfiguration(lineSource), RegistrationMode_ReplaceAll, Cleanup_Delete);
        cameras[1].RegisterConfiguration(new HardwareTriggerConfiguration(lineSource), RegistrationMode_ReplaceAll, Cleanup_Delete);

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

    cameras[0].TimestampLatch.Execute();
    cameras[1].TimestampLatch.Execute();

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> epoche = std::chrono::time_point<std::chrono::system_clock>{};

    cameraMainTime = static_cast<uint64>(cameras[0].TimestampLatchValue.GetValue());
    cameraSecondaryTime = static_cast<uint64>(cameras[1].TimestampLatchValue.GetValue());

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
    return 0;
}

// Minimal possible exposure time value of the main camera
int StereoCamera::getExposureTimeMin() {
    if (cameras.GetSize() > 0 && cameras[0].ExposureTime.IsReadable()) {
        return cameras[0].ExposureTime.GetMin();
    }
    return 0;
}

// Maximal possible exposure time value of the main camera
int StereoCamera::getExposureTimeMax() {
    if (cameras.GetSize() > 0 && cameras[0].ExposureTime.IsReadable()) {
        return cameras[0].ExposureTime.GetMax();
    }
    return 0;
}

// Current Gain value of the main camera
double StereoCamera::getGainValue() {
    if (cameras.GetSize() > 0 && cameras[0].Gain.IsReadable()) {
        return cameras[0].Gain.GetValue();
    }
    return 0;
}

// Minimal possible Gain value of the main camera
double StereoCamera::getGainMin() {
    if (cameras.GetSize() > 0 && cameras[0].Gain.IsReadable()) {
        return cameras[0].Gain.GetMin();
    }
    return 0;
}

// Maximal possible Gain value of the main camera
double StereoCamera::getGainMax() {
    if (cameras.GetSize() > 0 && cameras[0].Gain.IsReadable()) {
        return cameras[0].Gain.GetMax();
    }
    return 0;
}

// Sets the Gain value of the main and secondary camera
void StereoCamera::setGainValue(double value) {
    if (cameras.GetSize() == 2 && cameras[0].Gain.IsWritable() && cameras[1].Gain.IsWritable()) {
        cameras[0].Gain.TrySetValue(value);
        cameras[1].Gain.TrySetValue(value);
    }
}

// Sets the exposure time value of the main and secondary camera
void StereoCamera::setExposureTimeValue(int value) {
    if (cameras.GetSize() == 2 && cameras[0].ExposureTime.IsWritable() && cameras[1].ExposureTime.IsWritable()) {
        std::cout<<"Writing exposure value: " << value <<std::endl;
        cameras[0].ExposureTime.TrySetValue(value);
        cameras[1].ExposureTime.TrySetValue(value);
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

// Enables image acquisition frame rate for both cameras
void StereoCamera::enableAcquisitionFrameRate(bool enabled) {
    if (cameras.GetSize() == 2 && cameras[0].AcquisitionFrameRateEnable.IsWritable() && cameras[1].AcquisitionFrameRateEnable.IsWritable()) {
        cameras[0].AcquisitionFrameRateEnable.TrySetValue(enabled);
        cameras[1].AcquisitionFrameRateEnable.TrySetValue(enabled);
    }
}

// Sets the value of the image acquisition frame rate for both cameras
void StereoCamera::setAcquisitionFPSValue(int value) {
    if (cameras.GetSize() == 2 && cameras[0].AcquisitionFrameRate.IsWritable() && cameras[1].AcquisitionFrameRate.IsWritable()) {
        cameras[0].AcquisitionFrameRate.TrySetValue(value);
        cameras[1].AcquisitionFrameRate.TrySetValue(value);
    }
}

// Reads and returns the value of the image acquisition frame rate for the main camera
// Assumes that both cameras are set the same through the above function setAcquisitionFPSValue
int StereoCamera::getAcquisitionFPSValue() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRate.IsReadable()) {
        return cameras[0].AcquisitionFrameRate.GetValue();
    }
    return 0;
}

// Minimal possible image acquisition frame rate of the main camera
int StereoCamera::getAcquisitionFPSMin() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRate.IsReadable()) {
        return cameras[0].AcquisitionFrameRate.GetMin();
    }
    return 0;
}

// Maximal possible image acquisition frame rate of the main camera
// This may be influenced by the current camera settings and its value may change
int StereoCamera::getAcquisitionFPSMax() {
    if (cameras.GetSize() > 0 && cameras[0].AcquisitionFrameRate.IsReadable()) {
        return cameras[0].AcquisitionFrameRate.GetMax();
    }
    return 0;
}

// Camera frame rate resulting by the current camera settings
double StereoCamera::getResultingFrameRateValue() {
    if (cameras.GetSize() > 0 && cameras[0].ResultingFrameRate.IsReadable()) {
        return cameras[0].ResultingFrameRate.GetValue();
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
