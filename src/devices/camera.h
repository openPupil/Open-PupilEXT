
#ifndef PUPILEXT_CAMERA_H
#define PUPILEXT_CAMERA_H

/**
    @authors Moritz Lode, Gábor Bényei
*/


#include <QtCore/QObject>
#include <opencv2/core/mat.hpp>

/**
    Enum representing the different camera image types, also used for differentiating the respective camera type


    GB: added LIVE_SINGLE_WEBCAM 
*/
enum CameraImageType { LIVE_SINGLE_CAMERA=0, LIVE_STEREO_CAMERA=1, SINGLE_IMAGE_FILE=2, STEREO_IMAGE_FILE=3, LIVE_SINGLE_WEBCAM = 4 };

/**
    Struct representing a camera image returned from a camera and its respective meta-data

    For stereo cameras the struct contains two images
    For file cameras it further contains filename information

*/
struct CameraImage {
    int type;
    cv::Mat img;
    cv::Mat imgSecondary;
    uint64_t timestamp;
    uint64_t frameNumber; // GB NOTE: it was originally already there in beta 0.1.1, and it holds the INDEX of image (not starting from 1)
    std::string filename;
};

Q_DECLARE_METATYPE(CameraImage)

/**
    Abstract class representing a camera device, can represent any device type defined in CameraImageType

    isOpen(): checks wherever the camera is already opened, opening the camera should be conducted in the constructor
    close(): closes the camera device
    getType(): describe the type of camera

signals:
    onNewGrabResult(): transmits a newly arrived camera image
*/
class Camera : public QObject {
    Q_OBJECT

public:

    explicit inline Camera(QObject *parent = 0) : QObject(parent){}

    virtual bool isOpen() = 0;
    virtual void close() = 0;
    virtual CameraImageType getType() = 0;

    virtual int getImageROIwidth() = 0;
    virtual int getImageROIheight() = 0;
    virtual int getImageROIwidthMax() = 0;
    virtual int getImageROIheightMax() = 0;
    virtual int getImageROIoffsetX() = 0; 
    virtual int getImageROIoffsetY() = 0;  
    virtual QRectF getImageROI() = 0;

    virtual void stopGrabbing() = 0;
    virtual void startGrabbing() = 0;

    virtual bool isGrabbing() = 0;

signals:

    void onNewGrabResult(CameraImage grabResult);

};


#endif //PUPILEXT_CAMERA_H
