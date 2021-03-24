
#include <pylon/TlFactory.h>
#include "singleCamera.h"
#include "hardwareTriggerConfiguration.h"

SingleCamera::SingleCamera(const String_t &fullname, QObject* parent)
        : SingleCamera(CDeviceInfo().SetFullName(fullname), parent) {

}

SingleCamera::SingleCamera(const CDeviceInfo &di, QObject* parent)
        : Camera(parent), camera(CTlFactory::GetInstance().CreateDevice(di)),
        cameraImageEventHandler(new SingleCameraImageEventHandler(parent)),
        frameCounter(new CameraFrameRateCounter(parent)),
        cameraCalibration(new CameraCalibration()),
        calibrationThread(new QThread()),
        hardwareTriggerEnabled(false),
        lineSource("Line1") {

    settingsDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!settingsDirectory.exists()) {
        settingsDirectory.mkdir(".");
    }

    // calibration worker thread
    cameraCalibration->moveToThread(calibrationThread);
    connect(calibrationThread, SIGNAL (finished()), calibrationThread, SLOT (deleteLater()));
    calibrationThread->start();
    calibrationThread->setPriority(QThread::HighPriority);

    // CTlFactory::GetInstance().CreateDevice( CDeviceInfo().SetFullName( fullname))
    connect(cameraImageEventHandler, SIGNAL(onNewGrabResult(CameraImage)), this, SIGNAL(onNewGrabResult(CameraImage)));
    connect(cameraImageEventHandler, SIGNAL(onNewGrabResult(CameraImage)), frameCounter, SLOT(count(CameraImage)));

    connect(frameCounter, SIGNAL(fps(double)), this, SIGNAL(fps(double)));
    connect(frameCounter, SIGNAL(framecount(int)), this, SIGNAL(framecount(int)));

    if(camera.IsOpen()) {
        camera.Close();
    }

    try {
        camera.RegisterConfiguration(new CAcquireContinuousConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);

        camera.RegisterImageEventHandler(cameraImageEventHandler, RegistrationMode_Append, Cleanup_Delete);

        camera.Open();

        synchronizeTime();
        cameraImageEventHandler->setTimeSynchronization(cameraTime, systemTime);

        camera.PixelFormat.SetValue(PixelFormat_Mono8);

        // load calibration if existing
        if(!cameraCalibration->isCalibrated()) {
            // If we already used this camera before, a config file may exists
            loadCalibrationFile();
        }

        if (camera.CanWaitForFrameTriggerReady()) {

            // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
            // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
            // The GrabStrategy_OneByOne default grab strategy is used.
            camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
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
    }
}

SingleCamera::~SingleCamera() {
    calibrationThread->quit();
    calibrationThread->wait();
}

bool SingleCamera::isOpen() {
    return camera.IsOpen();
}

void SingleCamera::close() {
    std::cout << "SingleCamera: Releasing pylon resources.";
    camera.StopGrabbing();
    camera.Close();
    camera.DeregisterImageEventHandler(cameraImageEventHandler);
}

void SingleCamera::enableHardwareTrigger(bool enabled) {
    std::cout<< "SingleCamera: Enabling Hardware trigger to line source: " + lineSource << std::endl;

    frameCounter->reset();

    try {

        if(camera.IsOpen()) {
            camera.StopGrabbing();
            camera.Close();
        }

        if(enabled) {
            camera.RegisterConfiguration(new HardwareTriggerConfiguration(lineSource), RegistrationMode_ReplaceAll, Cleanup_Delete);
            hardwareTriggerEnabled = true;
        } else {
            camera.RegisterConfiguration(new CAcquireContinuousConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
            hardwareTriggerEnabled = false;
        }

        camera.Open();

        synchronizeTime();
        cameraImageEventHandler->setTimeSynchronization(cameraTime, systemTime);

        if (camera.CanWaitForFrameTriggerReady()) {

            // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
            // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
            // The GrabStrategy_OneByOne default grab strategy is used.
            camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
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
    }
}

/*
 * Taken from the Basler C++ Documentation Examples
 */
void SingleCamera::autoGainOnce() {
    try {

        if(!camera.IsOpen()) {
            camera.Open();
        }

        camera.StopGrabbing();

        // Turn test image off.
        camera.TestImageSelector.TrySetValue(TestImageSelector_Off);
        camera.TestPattern.TrySetValue(TestPattern_Off);

        // Only area scan cameras support auto functions.
        if (camera.DeviceScanType.GetValue() == DeviceScanType_Areascan) {
            // Cameras based on SFNC 2.0 or later, e.g., USB cameras
            if (camera.GetSfncVersion() >= Sfnc_2_0_0) {
                // All area scan cameras support luminance control.
                // Carry out luminance control by using the "once" gain auto function.
                // For demonstration purposes only, set the gain to an initial value.

                std::cout << "Starting AutoGain..." << std::endl;

                //camera.Gain.SetToMaximum();
                //camera.Gain.TrySetToMaximum();

                if (!camera.GainAuto.IsWritable()) {
                    std::cout << "The camera does not support Gain Auto." << std::endl;
                    return;
                }

                // Maximize the grabbed image area of interest (Image AOI).
                camera.OffsetX.TrySetToMinimum();
                camera.OffsetY.TrySetToMinimum();
                camera.Width.TrySetToMaximum();
                camera.Height.TrySetToMaximum();

                if (camera.AutoFunctionROISelector.IsWritable()) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    // Set the Auto Function ROI for luminance statistics.
                    // We want to use ROI1 for gathering the statistics

                    camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);
                    camera.AutoFunctionROIUseBrightness.TrySetValue(true);   // ROI 1 is used for brightness control
                    camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI2);
                    camera.AutoFunctionROIUseBrightness.TrySetValue(false);   // ROI 2 is not used for brightness control

                    // Set the ROI (in this example the complete sensor is used)
                    camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);  // configure ROI 1
                    camera.AutoFunctionROIOffsetX.SetValue(camera.OffsetX.GetMin());
                    camera.AutoFunctionROIOffsetY.SetValue(camera.OffsetY.GetMin());
                    camera.AutoFunctionROIWidth.SetValue(camera.Width.GetMax());
                    camera.AutoFunctionROIHeight.SetValue(camera.Height.GetMax());
                }

                if (camera.GetSfncVersion() >= Sfnc_2_0_0) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    // Set the target value for luminance control.
                    // A value of 0.3 means that the target brightness is 30 % of the maximum brightness of the raw pixel value read out from the sensor.
                    // A value of 0.4 means 40 % and so forth.
                    camera.AutoTargetBrightness.SetValue(0.3);

                    // We are going to try GainAuto = Once.

                    std::cout << "Trying 'GainAuto = Once'." << std::endl;
                    std::cout << "Initial Gain = " << camera.Gain.GetValue() << std::endl;

                    // Set the gain ranges for luminance control.
                    camera.AutoGainLowerLimit.SetValue(camera.Gain.GetMin());
                    camera.AutoGainUpperLimit.SetValue(camera.Gain.GetMax());
                }

                camera.GainAuto.SetValue(GainAuto_Once);

                // When the "once" mode of operation is selected,
                // the parameter values are automatically adjusted until the related image property
                // reaches the target value. After the automatic parameter value adjustment is complete, the auto
                // function will automatically be set to "off" and the new parameter value will be applied to the
                // subsequently grabbed images.

                int n = 0;
                while (camera.GainAuto.GetValue() != GainAuto_Off) {
                    CBaslerUniversalGrabResultPtr ptrGrabResult;
                    camera.GrabOne( 5000, ptrGrabResult);
                    ++n;
                    //Make sure the loop is exited.
                    if (n > 100) {
                        throw TIMEOUT_EXCEPTION( "The adjustment of auto gain did not finish.");
                    }
                }

                std::cout << "GainAuto went back to 'Off' after " << n << " frames." << std::endl;
                if(camera.Gain.IsReadable()) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    std::cout << "Final Gain = " << camera.Gain.GetValue() << std::endl;
                }

                camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
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

/*
 * Taken from the Basler C++ Documentation Examples
 */
void SingleCamera::autoExposureOnce() {
    try {

        if(!camera.IsOpen()) {
            camera.Open();
        }

        camera.StopGrabbing();

        // Turn test image off.
        camera.TestImageSelector.TrySetValue(TestImageSelector_Off);
        camera.TestPattern.TrySetValue(TestPattern_Off);

        // Only area scan cameras support auto functions.
        if (camera.DeviceScanType.GetValue() == DeviceScanType_Areascan) {
            // Cameras based on SFNC 2.0 or later, e.g., USB cameras
            if (camera.GetSfncVersion() >= Sfnc_2_0_0) {
                // For demonstration purposes only, set the exposure time to an initial value.
                //camera.ExposureTime.SetToMinimum();
                // Carry out luminance control by using the "once" exposure auto function.


                if (!camera.ExposureAuto.IsWritable())
                {
                    std::cout << "The camera does not support Exposure Auto." << std::endl;
                    return;
                }

                // Maximize the grabbed area of interest (Image AOI).
                camera.OffsetX.TrySetToMinimum();
                camera.OffsetY.TrySetToMinimum();
                camera.Width.SetToMaximum();
                camera.Height.SetToMaximum();

                if (camera.AutoFunctionROISelector.IsWritable())
                {
                    // Set the Auto Function ROI for luminance statistics.
                    // We want to use ROI1 for gathering the statistics
                    camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);
                    camera.AutoFunctionROIUseBrightness.TrySetValue(true);   // ROI 1 is used for brightness control
                    camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI2);
                    camera.AutoFunctionROIUseBrightness.TrySetValue(false);   // ROI 2 is not used for brightness control

                    // Set the ROI (in this example the complete sensor is used)
                    camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);  // configure ROI 1
                    camera.AutoFunctionROIOffsetX.SetValue(camera.OffsetX.GetMin());
                    camera.AutoFunctionROIOffsetY.SetValue(camera.OffsetY.GetMin());
                    camera.AutoFunctionROIWidth.SetValue(camera.Width.GetMax());
                    camera.AutoFunctionROIHeight.SetValue(camera.Height.GetMax());
                }

                if (camera.GetSfncVersion() >= Sfnc_2_0_0) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    // Set the target value for luminance control.
                    // A value of 0.3 means that the target brightness is 30 % of the maximum brightness of the raw pixel value read out from the sensor.
                    // A value of 0.4 means 40 % and so forth.
                    camera.AutoTargetBrightness.SetValue(0.3);

                    // Try ExposureAuto = Once.
                    std::cout << "Trying 'ExposureAuto = Once'." << std::endl;
                    std::cout << "Initial exposure time = ";
                    std::cout << camera.ExposureTime.GetValue() << " us" << std::endl;

                    // Set the exposure time ranges for luminance control.
                    camera.AutoExposureTimeLowerLimit.SetValue(camera.AutoExposureTimeLowerLimit.GetMin());
                    camera.AutoExposureTimeUpperLimit.SetValue(camera.AutoExposureTimeLowerLimit.GetMax());

                    camera.ExposureAuto.SetValue(ExposureAuto_Once);
                }

                // When the "once" mode of operation is selected,
                // the parameter values are automatically adjusted until the related image property
                // reaches the target value. After the automatic parameter value adjustment is complete, the auto
                // function will automatically be set to "off", and the new parameter value will be applied to the
                // subsequently grabbed images.
                int n = 0;
                while (camera.ExposureAuto.GetValue() != ExposureAuto_Off)
                {
                    CBaslerUniversalGrabResultPtr ptrGrabResult;
                    camera.GrabOne(5000, ptrGrabResult);
                    ++n;

                    //Make sure the loop is exited.
                    if (n > 100) {
                        throw TIMEOUT_EXCEPTION( "The adjustment of auto exposure did not finish.");
                    }
                }

                std::cout << "ExposureAuto went back to 'Off' after " << n << " frames." << std::endl;
                std::cout << "Final exposure time = ";

                if (camera.ExposureTime.IsReadable()) // Cameras based on SFNC 2.0 or later, e.g., USB cameras
                {
                    std::cout << camera.ExposureTime.GetValue() << " us" << std::endl;
                }

                camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
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

QString SingleCamera::getFriendlyName() {

    return QString(camera.GetDeviceInfo().GetFriendlyName().c_str());
}

QString SingleCamera::getFullName() {

    return QString(camera.GetDeviceInfo().GetFullName().c_str());
}

QString SingleCamera::getDeviceID() {

    return QString(camera.GetDeviceInfo().GetDeviceID().c_str());
}

int SingleCamera::getExposureTimeValue() {
    if (camera.ExposureTime.IsReadable()) {
        return static_cast<int>(camera.ExposureTime.GetValue());
    }
    return 0;
}

int SingleCamera::getExposureTimeMin() {
    if (camera.ExposureTime.IsReadable()) {
        return static_cast<int>(camera.ExposureTime.GetMin());
    }
    return 0;
}

int SingleCamera::getExposureTimeMax() {
    if (camera.ExposureTime.IsReadable()) {
        return static_cast<int>(camera.ExposureTime.GetMax());
    }
    return 0;
}

double SingleCamera::getGainValue() {
    if (camera.Gain.IsReadable()) {
        return camera.Gain.GetValue();
    }
    return 0;
}

double SingleCamera::getGainMin() {
    if (camera.Gain.IsReadable()) {
        return camera.Gain.GetMin();
    }
    return 0;
}

double SingleCamera::getGainMax() {
    if (camera.Gain.IsReadable()) {
        return camera.Gain.GetMax();
    }
    return 0;
}

void SingleCamera::setGainValue(double value) {
    if (camera.Gain.IsWritable()) {
        camera.Gain.TrySetValue(value);
    }
}

void SingleCamera::setExposureTimeValue(int value) {
    if (camera.ExposureTime.IsWritable()) {
        camera.ExposureTime.TrySetValue(value);
    }
}

void SingleCamera::loadFromFile(const String_t &filename) {
    try {
        CFeaturePersistence::Load( filename, &camera.GetNodeMap(), true );
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
    }

    if(camera.TriggerMode.GetValueOrDefault("Off") == "On") {
        lineSource = camera.TriggerSource.GetValueOrDefault("Line1");
        enableHardwareTrigger(true);
    }
}

void SingleCamera::saveToFile(const String_t &filename) {
    try {
        CFeaturePersistence::Save(filename, &camera.GetNodeMap() );
    } catch (const GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
    }
}

bool SingleCamera::isEnabledAcquisitionFrameRate() {
    if (camera.AcquisitionFrameRateEnable.IsReadable()) {
        return camera.AcquisitionFrameRateEnable.GetValue();
    }
    return false;
}

void SingleCamera::enableAcquisitionFrameRate(bool enabled) {
    if (camera.AcquisitionFrameRateEnable.IsWritable()) {
        camera.AcquisitionFrameRateEnable.TrySetValue(enabled);
    }
}

void SingleCamera::setAcquisitionFPSValue(int value) {
    if (camera.AcquisitionFrameRate.IsWritable()) {
        camera.AcquisitionFrameRate.TrySetValue(value);
    }
}

int SingleCamera::getAcquisitionFPSValue() {
    if (camera.AcquisitionFrameRate.IsReadable()) {
        return static_cast<int>(camera.AcquisitionFrameRate.GetValue());
    }
    return 0;
}

int SingleCamera::getAcquisitionFPSMin() {
    if (camera.AcquisitionFrameRate.IsReadable()) {
        return static_cast<int>(camera.AcquisitionFrameRate.GetMin());
    }
    return 0;
}

int SingleCamera::getAcquisitionFPSMax() {
    if (camera.AcquisitionFrameRate.IsReadable()) {
        return static_cast<int>(camera.AcquisitionFrameRate.GetMax());
    }
    return 0;
}

double SingleCamera::getResultingFrameRateValue() {
    if (camera.ResultingFrameRate.IsReadable()) {
        return camera.ResultingFrameRate.GetValue();
    }
    return 0;
}

CameraCalibration *SingleCamera::getCameraCalibration() {
    return cameraCalibration;
}

bool SingleCamera::isHardwareTriggerEnabled() {
    return hardwareTriggerEnabled;
}

/*
 * Synchronizes system and camera time and sets corresponding variables, should be executed before grabbing starts
 */
void SingleCamera::synchronizeTime() {

    camera.TimestampLatch.Execute();
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> epoche = std::chrono::time_point<std::chrono::system_clock>{};

    cameraTime = static_cast<uint64>(camera.TimestampLatchValue.GetValue());
    systemTime  = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
    std::time_t startTime = std::chrono::system_clock::to_time_t(start);
    std::time_t epochTime = std::chrono::system_clock::to_time_t(epoche);

    std::cout << "Camera Synchronize Time" << std::endl << "=========================" << std::endl;
    std::cout << "Timestamp Camera: " << cameraTime << std::endl;
    std::cout << "Timestamp System: " << systemTime << std::endl;
    std::cout << "System Epoch: " << std::ctime(&epochTime) << std::endl;
    std::cout << "System Time: " << std::ctime(&startTime) << std::endl;
    std::cout << "Time from Epoch (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count() << std::endl;
    std::cout << "Time from Epoch (us): " << std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch()).count() << std::endl;
    std::cout << "=========================" << std::endl;
}

String_t SingleCamera::getLineSource() {
    return lineSource;
}

void SingleCamera::setLineSource(String_t value) {
    lineSource = value;
}

CameraImageType SingleCamera::getType() {
    return CameraImageType::LIVE_SINGLE_CAMERA;
}

QString SingleCamera::getCalibrationFilename() {

    return settingsDirectory.filePath(getFriendlyName() + "_calibration_" +
            QString::number(cameraCalibration->getSquareSize()) + "_" +
            QString::number(cameraCalibration->getBoardSize().width+1) + "x" +
            QString::number(cameraCalibration->getBoardSize().height+1) + ".xml");
}

void SingleCamera::loadCalibrationFile() {
    QString configFile = getCalibrationFilename();
    configFile.replace(" ", "");

    if (QFile::exists(configFile)) {
        std::cout << "Found calibration file in settings directory. Loading: " << configFile.toStdString() << std::endl;
        cameraCalibration->loadFromFile(configFile.toStdString().c_str());
    }
}