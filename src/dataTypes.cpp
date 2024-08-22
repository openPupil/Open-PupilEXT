
#include "./dataTypes.h"

QMap<DataTypes::DataType, QString> DataTypes::map = {
    {DataType::TIME_RAW_TIMESTAMP,       "Timestamp"},
    {DataType::TIME,                     "Time"},
    {DataType::FRAME_NUMBER,             "Frame #"},
    {DataType::CAMERA_FPS,               "Camera/Image read FPS"},
    {DataType::PUPIL_FPS,                "Processing FPS"},
    {DataType::PUPIL_CENTER_X,           "Pupil center x"},
    {DataType::PUPIL_CENTER_Y,           "Pupil center y"},
    {DataType::PUPIL_MAJOR,              "Pupil major axis"},
    {DataType::PUPIL_MINOR,              "Pupil minor axis"},
    {DataType::PUPIL_WIDTH,              "Pupil width"},
    {DataType::PUPIL_HEIGHT,             "Pupil height"},
    {DataType::PUPIL_DIAMETER,           "Pupil diameter"},
    {DataType::PUPIL_UNDIST_DIAMETER,    "Pupil undistorted diameter"},
    {DataType::PUPIL_PHYSICAL_DIAMETER,  "Pupil physical diameter"},
    {DataType::PUPIL_CONFIDENCE,         "Pupil confidence"},
    {DataType::PUPIL_OUTLINE_CONFIDENCE, "Pupil outline confidence"},
    {DataType::PUPIL_CIRCUMFERENCE,      "Pupil circumference"},
    {DataType::PUPIL_RATIO,              "Pupil axis ratio"}
};

