#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <iostream>
#include "mainwindow.h"

/**
    
    This class reads arguments supplied upon executable start, puts them in a rational order, and makes 
    calls to mainwindow to call GUI functions programmatically to accomplish tasks that were set in arguments.
    It could work with a list of signals and slots, but I did not want to connect them for a one-time emit.

*/
class ExecArgParser : public QObject {
    Q_OBJECT
    
public:    
    explicit ExecArgParser(int argc, char *argv[]);
    ~ExecArgParser();
    
    void connectMainWindow(MainWindow *instance);
    void iterThroughDuties();

private: 

    QWidget *mainWindow;

    struct ExecArgs {
        short argID;
        std::vector<QString> argVals; 
    };

    std::vector<ExecArgs> execArgs;

    enum PossArgsEnum {
        CONNECT_MICROCONTROLLER_UDP,
        CONNECT_MICROCONTROLLER_COM,
        OPEN_SINGLE_CAMERA,
        OPEN_STEREO_CAMERA,
        OPEN_SINGLE_WEBCAM,
        SET_EXPOSURE_TIME_MICROSEC,
        SET_ACQUISITION_TRIGGERING_MODE,
        SET_HARDWARE_TRIGGERING_LINE_SOURCE,
        SET_HARDWARE_TRIGGERING_RUNTIME_LENGTH,
        SET_HARDWARE_TRIGGERING_FRAMERATE,
        START_HARDWARE_TRIGGERING,
        SET_SOFTWARE_TRIGGERING_FRAMERATE_LIMITING_ENABLED,
        SET_SOFTWARE_TRIGGERING_FRAMERATE_LIMIT,
        SET_GAIN,
        SET_PD_ALGORITHM,
        SET_PD_USING_ROI,
        SET_PD_COMPUTE_OUTLINE_CONF,
        START_TRACKING,
        SET_IMAGE_OUTPUT_PATH,
        SET_IMAGE_OUTPUT_FORMAT,
        START_IMAGE_RECORDING,
        SET_DATA_OUTPUT_PATH,
        SET_DATA_OUTPUT_DELIMITER,
        START_DATA_RECORDING,
        CONNECT_STREAM_UDP,
        CONNECT_STREAM_COM,
        START_STREAMING,
        CONNECT_REMOTE_UDP,
        CONNECT_REMOTE_COM
    };

    struct PossArgs {
        PossArgsEnum argType;
        QString argName; 
        short nFollowing;
    };

    // IMPORTANT: the order of this vector is also important!
    // it must follow the possible order of code execution that corresponds to the arguments
    const std::vector<PossArgs> possArgList = {
        {CONNECT_MICROCONTROLLER_UDP, "-connectMicrocontrollerUDP", 1},
        {CONNECT_MICROCONTROLLER_COM, "-connectMicrocontrollerCOM", 1},
        {OPEN_SINGLE_CAMERA, "-openSingleCamera", 1},
        {OPEN_STEREO_CAMERA, "-openStereoCamera", 2},
        {OPEN_SINGLE_WEBCAM, "-openSingleWebcam", 1},
        {SET_EXPOSURE_TIME_MICROSEC, "-setExposureTimeMicrosec", 1},
        {SET_ACQUISITION_TRIGGERING_MODE, "-setAcquisitionTriggeringMode", 1},
        {SET_HARDWARE_TRIGGERING_LINE_SOURCE, "-setHWTLineSource", 1},
        {SET_HARDWARE_TRIGGERING_RUNTIME_LENGTH, "-setHWTRuntimeLength", 1},
        {SET_HARDWARE_TRIGGERING_FRAMERATE, "-setHWTFramerate", 1},
        {START_HARDWARE_TRIGGERING, "-startHardwareTriggering", 0},
        {SET_SOFTWARE_TRIGGERING_FRAMERATE_LIMITING_ENABLED, "-setSoftwareTriggeringFramerateLimitingEnabled", 1},
        {SET_SOFTWARE_TRIGGERING_FRAMERATE_LIMIT, "-setSoftwareTriggeringFramerateLimit", 1},
        {SET_GAIN, "-setGain", 1},
        {SET_PD_ALGORITHM, "-setPDAlgorithm", 1},
        {SET_PD_USING_ROI, "-setPDUsingROI", 1},
        {SET_PD_COMPUTE_OUTLINE_CONF, "-setPDComputeOutlineConf", 1},
        {START_TRACKING, "-startTracking", 0},
        {SET_IMAGE_OUTPUT_PATH, "-setImageOutputPath", 1},
        {SET_IMAGE_OUTPUT_FORMAT, "-setImageOutputFormat", 1},
        {START_IMAGE_RECORDING, "-startImageRecording", 0},
        {SET_DATA_OUTPUT_PATH, "-setDataOutputPath", 1},
        {SET_DATA_OUTPUT_DELIMITER, "-setDataOutputDelimiter", 1},
        {START_DATA_RECORDING, "-startDataRecording", 0},
        {CONNECT_STREAM_UDP, "-connectStreamUDP", 1},
        {CONNECT_STREAM_COM, "-connectStreamCOM", 1},
        {START_STREAMING, "-startStreaming", 0},
        {CONNECT_REMOTE_UDP, "-connectRemoteUDP", 1},
        {CONNECT_REMOTE_COM, "-connectRemoteCOM", 1}
    };

    short getArgIdx(PossArgsEnum type) {
        for(short i=0; i<possArgList.size(); i++) {
            if(possArgList[i].argType==type)
                return i;
        }
        return -1;
    }

    void sortArgs();

};


