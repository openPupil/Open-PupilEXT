
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
        OPEN_SINGLE_CAMERA = 0,
        OPEN_STEREO_CAMERA = 1,
        OPEN_SINGLE_WEBCAM = 2,
        SET_PD_ALGORITHM = 3,
        SET_PD_USING_ROI = 4,
        SET_PD_COMPUTE_OUTLINE_CONF = 5,
        START_TRACKING = 6,
        SET_DELIMITER = 7,
        SET_IMAGE_OUTPUT_PATH = 8,
        SET_IMAGE_OUTPUT_FORMAT = 9,
        START_IMAGE_RECORDING = 10,
        SET_CSV_OUTPUT_PATH = 11,
        START_RECORDING = 12,
        CONN_STREAM_UDP = 13,
        CONN_STREAM_COM = 14,
        START_STREAMING = 15,
        CONN_REMOTE_UDP = 16,
        CONN_REMOTE_COM = 17
    };

    struct PossArgs {
        PossArgsEnum argType;
        QString argName; 
        short nFollowing;
    };

    // IMPORTANT: the order of this vector is also important!
    // it must follow the possible order of code execution that corresponds to the arguments
    const std::vector<PossArgs> possArgList = {
        {OPEN_SINGLE_CAMERA, "-openSingleCamera", 1},
        {OPEN_STEREO_CAMERA, "-openStereoCamera", 2},
        {OPEN_SINGLE_WEBCAM, "-openSingleWebcam", 1},
        {SET_PD_ALGORITHM, "-setPDAlgorithm", 1},
        {SET_PD_USING_ROI, "-setPDUsingROI", 1},
        {SET_PD_COMPUTE_OUTLINE_CONF, "-setPDComputeOutlineConf", 1},
        {START_TRACKING, "-startTracking", 0},
        {SET_DELIMITER, "-setDelimiter", 1},
        {SET_IMAGE_OUTPUT_PATH, "-setImageOutputPath", 1},
        {SET_IMAGE_OUTPUT_FORMAT, "-setImageOutputFormat", 1},
        {START_IMAGE_RECORDING, "-startImageRecording", 0},
        {SET_CSV_OUTPUT_PATH, "-setDataOutputPath", 1},
        {START_RECORDING, "-startDataRecording", 0},
        {CONN_STREAM_UDP, "-connectStreamUDP", 1},
        {CONN_STREAM_COM, "-connectStreamCOM", 1},
        {START_STREAMING, "-startStreaming", 0},
        {CONN_REMOTE_UDP, "-connectRemoteUDP", 1},
        {CONN_REMOTE_COM, "-connectRemoteCOM", 1}
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


