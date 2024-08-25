#pragma once

/**
    Under construction
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsItem>
#include <opencv2/core/mat.hpp>
#include <iostream>
#include <QMouseEvent>
#include <QtWidgets/qrubberband.h>
#include <QtWidgets/qsizegrip.h>
#include "../devices/camera.h"
#include "imageGraphicsItem.h"
#include "ResizableRectItem.h"
#include <QtWidgets/QHBoxLayout>

#include "../pupilDetection.h"

class SceneImageWidget : public QWidget {
Q_OBJECT

public:

    explicit SceneImageWidget(QWidget *parent=0);

    QSize sizeHint() const override;

    void setSceneImageSize(int w, int h) {
        imageSize = cv::Size(w,h);
    }

    QSizeF getSceneImageSize() {
        return QSizeF(imageSize.width, imageSize.height);
    }

    void setSceneImageSize(const float width, const float height){
        imageSize.width = width;
        imageSize.height = height;
    }

private:

    QSettings *applicationSettings;

    QGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;

    ImageGraphicsItem *currentImage;

    cv::Size imageSize;

    std::vector<std::vector<Pupil>> pupilsForCalibPoints;

    std::vector<QGraphicsItem*> geBufferCalibPoints;
    QPen penCalibPoints = QColor("blue"); // todo: make these reflect the main A1, main A2, stb.. identity of the pupil, as described for each proc.mode
    // TODO: make a QMap data structure for accessing the name, abbreviation, and drawing color associated with each pupil detection vector item and each proc.mode

protected:

    void resizeEvent(QResizeEvent *event) override;

public slots:

    // This class needs to know how many mapped points of regard (gaze mapped "pupils") to draw, and where, and how
    // So, in order to do that it either has to:
    // - always track proc.mode event-based by onProcModeChange slot, or
    // - this class instance should have a pupilDetection instance, from where it can query proc.mode state anytime, or
    // - have the proc.mode supplied with each updateView signal on each frame (safest)
    // Now it works with the third option

    // The view sceneImageWidget is either updated by receiving mapped pupils on screen, to draw/update the points of regard
    // or by receiving an image from a live screen sharing video stream, to update the background.
    // No precise sync needed between these.

    void updateView(const int &procMode, const std::vector<Pupil> &Pupils);
    void updateView(const cv::Mat &img);

    void drawOverlay();
    void clearOverlayMemory();

//signals:
    //...

};
