#ifndef PUPILEXT_SIGNALPUBSUBHANDLER_H
#define PUPILEXT_SIGNALPUBSUBHANDLER_H

/**
    @author Moritz Lode
*/

#include <QtCore/QObject>
#include "devices/camera.h"
#include "pupil-detection-methods/Pupil.h"

/**
    A signal handler that distributes signals from the selected camera object to independently windows.

    Signals are connected to the camera signals directly to forward them.

    Problem which is solved is that the camera object may change while windows stay open, invalidating signal connections.
*/
class SignalPubSubHandler : public QObject {
    Q_OBJECT

public:

    explicit SignalPubSubHandler(QObject *parent = 0) : QObject(parent) {

    }

    ~SignalPubSubHandler() override = default;

signals:

    void onNewGrabResult(CameraImage grabResult);

    void cameraFPS(double fps);
    void cameraFramecount(int framecount);

};


#endif //PUPILEXT_SIGNALPUBSUBHANDLER_H
