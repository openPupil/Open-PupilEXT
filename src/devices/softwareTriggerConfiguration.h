#pragma once

#include <pylon/Platform.h>

#include <pylon/InstantCamera.h>
#include <pylon/ParameterIncludes.h>

using namespace Pylon;

/**
    Basler camera configuration taken from the Basler codebase and modified to configure a camera for Software trigger acquisition

    The linesource used for the camera is defined as a constructor parameter
*/
class SoftwareTriggerConfiguration : public CConfigurationEventHandler
{

public:
    String_t lineSource;

    explicit SoftwareTriggerConfiguration(const String_t &lineSource) : CConfigurationEventHandler(), lineSource(lineSource)
    {
    }

    // Apply Software trigger configuration.
    void ApplyConfiguration(GENAPI_NAMESPACE::INodeMap &nodemap)
    {
        using namespace GENAPI_NAMESPACE;

        // Disable all trigger types except the trigger type used for triggering the acquisition of frames.
        {
            // Get required enumerations.
            CEnumParameter triggerSelector(nodemap, "AcquisitionStart");
            CEnumParameter triggerMode(nodemap, "TriggerMode");
            CEnumParameter triggerSource(nodemap, "TriggerSource");
            //CEnumParameter (nodemap, "Tri");

            // Check the available camera trigger mode(s) to select the appropriate one: acquisition start trigger mode
            // (used by older cameras, i.e. for cameras supporting only the legacy image acquisition control mode;
            // do not confuse with acquisition start command) or frame start trigger mode
            // (used by newer cameras, i.e. for cameras using the standard image acquisition control mode;
            // equivalent to the acquisition start trigger mode in the legacy image acquisition control mode).
            String_t triggerName("FrameStart");
            if (!triggerSelector.CanSetValue(triggerName))
            {
                triggerName = "AcquisitionStart";
                if (!triggerSelector.CanSetValue(triggerName))
                {
                    throw RUNTIME_EXCEPTION("Could not select trigger. Neither FrameStart nor AcquisitionStart is available.");
                }
            }

            String_t triggerModeValue("Off")
            if(!triggerMode.canSetValue(triggerModeValue)){
                throw RUNTIME_EXCEPTION("Could not set trigger mode.");
            }
            String_t triggerSourceValue("TriggerSoftware")

            if(!triggerSource.canSetValue(triggerSourceValue)){
                throw RUNTIME_EXCEPTION("Could not set trigger source");
            }
        }

        // Set acquisition mode to "continuous"
        CEnumParameter(nodemap, "AcquisitionMode").SetValue("Continuous");
    }

    // Set basic camera settings.
    virtual void OnOpened(CInstantCamera &camera)
    {
        try
        {
            ApplyConfiguration(camera.GetNodeMap());
        }
        catch (const GenericException &e)
        {
            throw RUNTIME_EXCEPTION("Could not apply configuration. Pylon::GenericException caught in OnOpened method: =%hs", e.what());
        }
        catch (const std::exception &e)
        {
            throw RUNTIME_EXCEPTION("Could not apply configuration. std::exception caught in OnOpened method: %hs", e.what());
        }
        catch (...)
        {
            throw RUNTIME_EXCEPTION("Could not apply configuration. Unknown exception caught in OnOpened method.");
        }
    }
};
