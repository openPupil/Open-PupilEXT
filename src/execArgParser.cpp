
#include "execArgParser.h"

// TODO: remove duplicates in list?
ExecArgParser::ExecArgParser(int argc, char *argv[]) {

    /*
    argv = new char* [] { 
        "PupilEXT.exe",
        "-openSingleCamera",
        "Basler a2A1920-160umBAS (40242221)",
        "-setPDAlgorithm",
        "PuRe", // HIBA TÖRTÉNIK HA SWIRSKI2D, A RUNWITHCONFIDENCE CALL-BAN
        "-setPDUsingROI",
        "1",
        "-setPDComputeOutlineConf",
        "1",
        "-startTracking",
        "-setDelimiter",
        "comma",
        "-setImageOutputPath",
        "C:/_PupilEXT test image output path",
        "-setImageOutputFormat",
        "tiff",
        "-startImageRecording",
        "-setCSVOutputPath",
        "C:/_PupilEXT test csv output file",
        "-startCSVRecording",
        "-connectRemoteUDP",
        "127.0.0.1,6900",
        "-connectRemoteCOM",
        "COM4,9600,mistake"
    };
    argc = 24;
    */

    //std::cout << "First (index 0) argument, a.k.a. executable name: " << argv[0] << std::endl;

    
    //std::cout << "argc: " << argc << std::endl;

    for(size_t i = 1; i < argc; ++i) {
        //std::cout << argv[i] << "\n";
        //std::string str(argv[i]);

        if( i < argc && 
            std::strlen(argv[i])>1 &&
            argv[i][0] == '-' ) {
            
            short currArgIdx = -1;
            for(short h=0; h<possArgList.size(); h++) {
                if(possArgList[h].argName == QString(argv[i])) {
                    currArgIdx = h;
                    break;
                }
            }
            if(currArgIdx == -1)
                continue;

            short nFollowing = possArgList[currArgIdx].nFollowing;
            std::vector<QString> tempStr;
            for(short b=1; b<=nFollowing; b++) {
                //std::cout << argv[i] << std::endl;
                // //if( i < (argc-(1+b)) && std::strlen(argv[i+b])>=1 && argv[i+b][0] != '-')
                if( i < (argc-b) && std::strlen(argv[i+b])>=1 && argv[i+b][0] != '-')
                    tempStr.push_back(argv[i+b]);
            }

            if(tempStr.size() != nFollowing)
                continue;
            else 
                execArgs.push_back(ExecArgs{ currArgIdx, tempStr });
        }
    }

    //std::cout << "Parsed " << execArgs.size() << " arguments" << std::endl;
    //for(size_t h=0; h<execArgs.size(); h++)
    //    std::cout << "\t arg " << h << ": " << possArgList[execArgs[h].argID].argName.toStdString() << std::endl;
    
    if(execArgs.size()<1)
        return;

    sortArgs();

    std::cout << "Sorted arguments:" << std::endl;
    for(size_t h=0; h<execArgs.size(); h++)
        std::cout << "\t arg " << h << ": " << possArgList[execArgs[h].argID].argName.toStdString() << std::endl;
}

ExecArgParser::~ExecArgParser() {

}

// bubble sort the arguments according to the order defined by argIDs
void ExecArgParser::sortArgs() {
    for(size_t i=0; i<execArgs.size(); i++) {
        for(size_t j=0; j<execArgs.size()-(1+i); j++) {
            if(execArgs[j].argID>execArgs[j+1].argID) {
                std::swap(execArgs[j], execArgs[j+1]);
            }
        }
    }
}

void ExecArgParser::connectMainWindow(MainWindow *instance) {
    mainWindow = instance;
}

void ExecArgParser::iterThroughDuties() {
    //qDebug() << "Iterating through arguments supplied on executable start";

    MainWindow *w = dynamic_cast<MainWindow*>(mainWindow);

    for(size_t i=0; i<execArgs.size(); i++) {
    
        //qDebug() << "Now doing argID: " << execArgs[i].argID;
    
        if(execArgs[i].argID == getArgIdx(OPEN_SINGLE_CAMERA) && execArgs[i].argVals.size()>=1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGopenSingleCamera(execArgs[i].argVals[0].toLower());
        }
        if(execArgs[i].argID == getArgIdx(OPEN_SINGLE_CAMERA) && execArgs[i].argVals.size()>=2 && !execArgs[i].argVals[0].isEmpty() && !execArgs[i].argVals[1].isEmpty()) {
            w->PRGopenStereoCamera(execArgs[i].argVals[0].toLower(), execArgs[i].argVals[1].toLower());
        }
        if(execArgs[i].argID == getArgIdx(OPEN_SINGLE_WEBCAM) && execArgs[i].argVals.size()>=1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGopenSingleWebcam(execArgs[i].argVals[0].toInt());
        }
        if(execArgs[i].argID == getArgIdx(START_TRACKING)) {
            w->PRGtrackStart();
        }
        if(execArgs[i].argID == getArgIdx(START_RECORDING)) {
            w->PRGrecordStart();
        }
        if(execArgs[i].argID == getArgIdx(START_STREAMING)) {
            w->PRGstreamStart();
        }
        if(execArgs[i].argID == getArgIdx(START_IMAGE_RECORDING)) {
            w->PRGrecordImageStart();
        }

        if(execArgs[i].argID == getArgIdx(SET_IMAGE_OUTPUT_PATH) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetOutPath(execArgs[i].argVals[0]);
        }
        if(execArgs[i].argID == getArgIdx(SET_CSV_OUTPUT_PATH) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetCsvPathAndName(execArgs[i].argVals[0]);
        }
        if(execArgs[i].argID == getArgIdx(SET_DELIMITER) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetGlobalDelimiter(execArgs[i].argVals[0]);
        }
        if(execArgs[i].argID == getArgIdx(SET_IMAGE_OUTPUT_FORMAT) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetImageOutputFormat(execArgs[i].argVals[0].toLower());
        }

        if(execArgs[i].argID == getArgIdx(SET_PD_ALGORITHM) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetPupilDetectionAlgorithm(execArgs[i].argVals[0].toLower());
        }
        if(execArgs[i].argID == getArgIdx(SET_PD_USING_ROI) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetPupilDetectionUsingROI(execArgs[i].argVals[0].toLower());
        }
        if(execArgs[i].argID == getArgIdx(SET_PD_COMPUTE_OUTLINE_CONF) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGsetPupilDetectionCompOutlineConf(execArgs[i].argVals[0].toLower());
        }

        if(execArgs[i].argID == getArgIdx(CONN_REMOTE_UDP) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGconnectRemoteUDP(execArgs[i].argVals[0].toLower());
        }
        if(execArgs[i].argID == getArgIdx(CONN_REMOTE_COM) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGconnectRemoteCOM(execArgs[i].argVals[0].toLower());
        }
        if(execArgs[i].argID == getArgIdx(CONN_STREAM_UDP) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGconnectStreamUDP(execArgs[i].argVals[0].toLower());
        }
        if(execArgs[i].argID == getArgIdx(CONN_STREAM_COM) && execArgs[i].argVals.size()==1 && !execArgs[i].argVals[0].isEmpty()) {
            w->PRGconnectStreamCOM(execArgs[i].argVals[0].toLower());
        }

    } 

}
