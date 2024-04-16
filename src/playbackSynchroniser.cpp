//
// Created by aboncser on 2024. 03. 16..
//

#include "playbackSynchroniser.h"

PlaybackSynchroniser::PlaybackSynchroniser(QObject *parent) : QObject(parent) {}

void PlaybackSynchroniser::setPupilDetection(PupilDetection *pupilDetection) {
    PlaybackSynchroniser::pupilDetection = pupilDetection;
}

void PlaybackSynchroniser::setImageReader(ImageReader *imageReader) {
    PlaybackSynchroniser::imageReader = imageReader;
}

void PlaybackSynchroniser::onPlaybackStarted() {
    if (pupilDetection && pupilDetection->isTrackingOn()){
        pupilDetection->setSynchronised(true);
        imageReader->setSynchronised(true);
    }
}

void PlaybackSynchroniser::onPlaybackStopped() {
    if (pupilDetection && pupilDetection->isTrackingOn()){
        imageReader->setSynchronised(false);
        pupilDetection->setSynchronised(false);
    }
}

void PlaybackSynchroniser::onPupilDetectionStarted() {
    if (imageReader && imageReader->isPlaying()){
        imageReader->setSynchronised(true);
        pupilDetection->setSynchronised(true);
    }
}

void PlaybackSynchroniser::onPupilDetectionStopped() {
    if (imageReader && imageReader->isPlaying()){
        pupilDetection->setSynchronised(false);
        imageReader->setSynchronised(false);
    }
}

void PlaybackSynchroniser::setCamera(Camera *camera) {
    PlaybackSynchroniser::camera = dynamic_cast<FileCamera*>(camera);
    if (PlaybackSynchroniser::camera) {
        PlaybackSynchroniser::setImageReader(PlaybackSynchroniser::camera->getImageReader());
    }
    else{
        qDebug() << "Invalid camera object. Camera is not FileCamera.";
    }
}
