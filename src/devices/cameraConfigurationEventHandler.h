#ifndef PUPILEXT_CAMERACONFIGURATIONEVENTHANDLER_H
#define PUPILEXT_CAMERACONFIGURATIONEVENTHANDLER_H

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QDebug>
#include <pylon/PylonIncludes.h>
#include "camera.h"

using namespace Pylon;

class CameraConfigurationEventHandler : public QObject, public Pylon::CConfigurationEventHandler {
Q_OBJECT

    public:

        void OnCameraDeviceRemoved( CInstantCamera& camera ) override {
            qDebug() << "CameraConfigurationEventHandler::OnCameraDeviceRemoved called.";

            // NOTE: TODO: this does not work somehow, though it is implemented as provided in the official sample code
            // heartbeat interval is 1000, but this handler does never get called, only a catch deals with the unplugged device, in the camera class
            emit cameraDeviceRemoved();
        };

    signals:
        void cameraDeviceRemoved();

};


#endif //PUPILEXT_CAMERACONFIGURATIONEVENTHANDLER_H
