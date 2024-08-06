//
// Created by aboncser on 2024. 03. 16..
//

#ifndef PUPILEXT_PLAYBACKSYNCHRONISER_H
#define PUPILEXT_PLAYBACKSYNCHRONISER_H

/**
    @author Attila Boncser
*/

#include "pupilDetection.h"
#include "devices/fileCamera.h"

class PlaybackSynchroniser : public QObject {
    Q_OBJECT

    PupilDetection* pupilDetection;

    FileCamera* camera;
    ImageReader* imageReader;



public:
    explicit PlaybackSynchroniser(QObject *parent = 0);


    void setPupilDetection(PupilDetection *pupilDetection);

    void setCamera(Camera *camera);

    void setImageReader(ImageReader *imageReader);

    //signals:

    public slots:
    void onPlaybackStarted();
    void onPlaybackStopped();

    void onPupilDetectionStarted();
    void onPupilDetectionStopped();




};


#endif //PUPILEXT_PLAYBACKSYNCHRONISER_H
