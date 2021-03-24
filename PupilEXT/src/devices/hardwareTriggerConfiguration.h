
#ifndef PUPILEXT_HARDWARETRIGGERCONFIGURATION_H
#define PUPILEXT_HARDWARETRIGGERCONFIGURATION_H

//-----------------------------------------------------------------------------
//  Basler pylon SDK
//  Copyright (c) 2010-2019 Basler AG
//  http://www.baslerweb.com
//  Author:  Andreas Gau, Modified: Moritz Lode (2020)
//-----------------------------------------------------------------------------

#include <pylon/Platform.h>

#include <pylon/InstantCamera.h>
#include <pylon/ParameterIncludes.h>

using namespace Pylon;

/**
    Basler camera configuration taken from the Basler codebase and modified to configure a camera for hardware trigger acquisition

    The linesource used for the camera is defined as a constructor parameter
*/
class HardwareTriggerConfiguration : public CConfigurationEventHandler
{

public:

    String_t lineSource;

    explicit HardwareTriggerConfiguration(const String_t& lineSource) : CConfigurationEventHandler(), lineSource(lineSource) {

    }

    // Apply hardware trigger configuration.
    void ApplyConfiguration( GENAPI_NAMESPACE::INodeMap& nodemap)
    {
        using namespace GENAPI_NAMESPACE;

        // Disable all trigger types except the trigger type used for triggering the acquisition of frames.
        {
            // Get required enumerations.
            CEnumParameter triggerSelector(nodemap, "TriggerSelector");
            CEnumParameter triggerMode(nodemap, "TriggerMode");

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

            // Set the line source as Input mode
            CEnumParameter(nodemap, "LineSelector").SetValue(lineSource);
            CEnumParameter(nodemap, "LineMode").SetValue("Input");

            // Get all enumeration entries of trigger selector.
            StringList_t triggerSelectorEntries;
            triggerSelector.GetSettableValues(triggerSelectorEntries);

            // Turn trigger mode off for all trigger selector entries except for the frame trigger given by triggerName.
            for(StringList_t::const_iterator it = triggerSelectorEntries.begin(); it != triggerSelectorEntries.end(); ++it)
            {
                // Set trigger mode to off.
                triggerSelector.SetValue(*it);
                if (triggerName == *it)
                {
                    // Activate trigger.
                    triggerMode.SetValue("On");

                    //// Alternative hardware trigger configuration:
                    //// The camera user's manual contains more information about available configurations.
                    //// The Basler pylon Viewer tool can be used to test the selected settings first.

                    //// The trigger source must be set to the trigger input, e.g. 'Line1'.
                    CEnumParameter(nodemap, "TriggerSource").SetValue(lineSource);

                    ////The trigger activation must be set to e.g. 'RisingEdge'.
                    CEnumParameter(nodemap, "TriggerActivation").SetValue("RisingEdge");
                }
                else
                {
                    triggerMode.SetValue("Off");
                }
            }
            // Finally select the frame trigger type (resp. acquisition start type
            // for older cameras). Issuing a software trigger will now trigger
            // the acquisition of a frame.
            triggerSelector.SetValue(triggerName);
        }


        //Set acquisition mode to "continuous"
        CEnumParameter(nodemap, "AcquisitionMode").SetValue("Continuous");
    }

    // Set basic camera settings.
    virtual void OnOpened( CInstantCamera& camera)
    {
        try
        {
            ApplyConfiguration( camera.GetNodeMap());
        }
        catch (const GenericException& e)
        {
            throw RUNTIME_EXCEPTION( "Could not apply configuration. Pylon::GenericException caught in OnOpened method: =%hs", e.what());
        }
        catch (const std::exception& e)
        {
            throw RUNTIME_EXCEPTION( "Could not apply configuration. std::exception caught in OnOpened method: %hs", e.what());
        }
        catch (...)
        {
            throw RUNTIME_EXCEPTION( "Could not apply configuration. Unknown exception caught in OnOpened method.");
        }
    }
};


#endif //PUPILEXT_HARDWARETRIGGERCONFIGURATION_H
