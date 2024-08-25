#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QtCore/QMap>

class DataTypes : public QObject {
    Q_OBJECT

public:
    enum struct DataType {
        TIME_RAW_TIMESTAMP,
        TIME,
        FRAME_NUMBER,
        CAMERA_FPS,
        PUPIL_FPS,
        PUPIL_CENTER_X,
        PUPIL_CENTER_Y,
        PUPIL_MAJOR,
        PUPIL_MINOR,
        PUPIL_WIDTH,
        PUPIL_HEIGHT,
        PUPIL_DIAMETER,
        PUPIL_UNDIST_DIAMETER,
        PUPIL_PHYSICAL_DIAMETER,
        PUPIL_CONFIDENCE,
        PUPIL_OUTLINE_CONFIDENCE,
        PUPIL_CIRCUMFERENCE,
        PUPIL_RATIO
    };

    static QMap<DataType, QString> map;
};
