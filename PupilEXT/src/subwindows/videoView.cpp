
#include <QtWidgets/QHBoxLayout>
#include <QResizeEvent>
#include <ctime>
#include <opencv2/imgproc.hpp>
#include "videoView.h"

// Creates a new live-view widget
VideoView::VideoView(QWidget *parent) : QWidget(parent), graphicsScene(new QGraphicsScene(parent)),
    graphicsView(new QGraphicsView(parent)), initialFit(false), mode(FIT), scaleFactor(1.25), ROI(QRect()) {

    roiGraphicsView = new QGraphicsView(graphicsView);
    roiGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    roiGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    roiGraphicsView->setGeometry(static_cast<int>(640 - (640 * 0.2 + 50)), 50, static_cast<int>(640 * 0.2),static_cast<int>(640 * 0.2));

    QColor selectionColor = Qt::blue;
    selectionColor.setAlphaF( 0.3 );
    roiSelection = new ResizableRectItem(QRectF(QPointF(32, 32), QPointF(200, 200)));
    roiSelection->setBrush(selectionColor);
    roiSelection->setFlag(QGraphicsItem::ItemIsMovable);
    roiSelection->setZValue(100);
    connect(roiSelection, SIGNAL(onChange()), this, SLOT(onROIChange()));


    currentImage = new ImageGraphicsItem();
    graphicsScene->addItem(currentImage);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(graphicsView, 0, 0);

    setLayout(layout);

    graphicsView->setScene(graphicsScene);
    roiGraphicsView->setScene(graphicsScene);
    graphicsView->show();
    roiGraphicsView->hide();
}

// Event handler when the widget is resized
// If the widget is currently in FIT mode, scale the live-view according to the window size
// If the lens pupil view is visible, it needs to be resized manually to preserve the size ratio to the large view
void VideoView::resizeEvent(QResizeEvent *event) {

    if(mode==FIT)
        graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);

    if(roiGraphicsView->isVisible()) {
        int msize = std::max(event->size().width(), event->size().height());
        roiGraphicsView->setGeometry(static_cast<int>(event->size().width() - (msize * 0.2 + 50)), 50,
                                     static_cast<int>(msize * 0.2), static_cast<int>(msize * 0.2));
    }
}

QSize VideoView::sizeHint() const {
    return QSize(640, 480);
}

// Slot receiving images to display
// The image received as cv::Mat is first converted to a QImage, then painted to the scene using the custom ImageGraphicsItem
// If fit scaling is activated, the scene is scaled to fit the window size, however, only on the first image as we dont know the image dimensions before
void VideoView::updateView(const cv::Mat &img) {

    if(!img.empty()) {
        cv::Mat bgrFrame = img; // For some reason, qt crashes when trying to paint a grayscale image over qpainter
        if (img.channels() == 1) {
            cv::cvtColor(img, bgrFrame, cv::COLOR_GRAY2BGR);
        }
        QImage qImg = cvMatToQImage(bgrFrame);
        graphicsScene->removeItem(currentImage);
        delete currentImage;
        currentImage = new ImageGraphicsItem(qImg);
        graphicsScene->addItem(currentImage);

        if(!initialFit) {
            imageSize = img.size();
            graphicsScene->setSceneRect(0, 0, img.cols, img.rows);
            if(mode==FIT) {
                graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
            }

            // ML: ADDED START 20.06.21
            // Reason: Prevent error if the ROI is bigger than the image itself
            QRectF roi = roiSelection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
            if(!graphicsScene->sceneRect().contains(roi)) {
                roiSelection->setRect(0, 0, 32, 32);
                saveROISelection();
            }
            // ML: ADDED END

            initialFit = true;
        }
    }
}

// Fit the image scene to the window size
void VideoView::fitView() {
    if(mode!=FIT) {
        mode = FIT;
        graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void VideoView::showFullView() {
    if(mode!=FULL) {
        mode = FULL;
        graphicsView->resetTransform();
    }
}

void VideoView::zoomInView() {
    if(mode!=ZOOM) {
        mode = ZOOM;
        graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    }
    graphicsView->scale(scaleFactor, scaleFactor);
}

void VideoView::zoomOutView() {
    if(mode!=ZOOM) {
        mode = ZOOM;
        graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    }
    graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}

// Updates the region of the pupil lens view
void VideoView::updatePupilView(const QRect &rect) {
    if(rect.isValid()) {
        //roiGraphicsView->setSceneRect(rect);
        roiGraphicsView->resetTransform();
        roiGraphicsView->fitInView(rect);
    }
}

// Activates the pupil lens view, which is an overlay on the original scene showing the pupil detection in detil
void VideoView::enablePupilView(bool value) {
    if(value) {
        roiGraphicsView->show();
    } else {
        roiGraphicsView->hide();
    }
}

// Show the ROI selection on top of the scene
void VideoView::showROISelection(bool value) {
    if(value) {
        graphicsScene->addItem(roiSelection);
    } else {
        graphicsScene->removeItem(roiSelection);
    }
}

// Saves the current selected rect by the ROI selection
// Sends the selection as a signal
// If the selected ROI is outside of image bounds nothing is transmitted and false returned i.e. no new ROI is set
bool VideoView::saveROISelection() {

    QRectF roi = roiSelection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);

    std::cout<<imageSize.width<<": "<<roi.width()<<std::endl;
    std::cout<<imageSize.height<<": "<<roi.height()<<std::endl;

    if(initialFit && (roi.size().width() > imageSize.width ||  roi.size().height() > imageSize.height)) {
        std::cout<<"Saving ROI Selection: out of image bounds."<<std::endl;
        return false;
    }

    if(initialFit && !graphicsScene->sceneRect().contains(roi)) {
        std::cout<<"Saving ROI Selection: out of scene bounds."<<std::endl;
        return false;
    }
    //std::cout<<"ROI selected contained:" << graphicsScene->sceneRect().contains(roi) << " size: " << roi.topLeft().x() << ":" << roi.topLeft().x() << " - " << roi.height() << std::endl;
    emit onROISelection(roi);

    return true;
}

// Discards the current ROI selection, meaning no new ROI is set and the ROI selection is rest to a default size
void VideoView::discardROISelection() {

    QRectF roi = QRectF(0,0, 0.4*imageSize.width, 0.4*imageSize.height) ; //graphicsScene->sceneRect();
    roiSelection->setPos(0, 0);
    roiSelection->setRect(roi);
    roiSelection->update();
    std::cout<<"ROI reset contained:" << graphicsScene->sceneRect().contains(roi) << " size: " << roi.topLeft().x() << ":" << roi.topLeft().x() << " - " << roi.height() << std::endl;
}

// Event handler when the selection of the ROI is changed i.e. the user moved the selection
// To signal a invalid selection, the ROI selection is colored red if it goes outside the scene
void VideoView::onROIChange() {
    QRectF roi = roiSelection->sceneBoundingRect();

    QColor selectionColor = Qt::blue;

    if(!graphicsScene->sceneRect().contains(roi)) {
        selectionColor = Qt::red;
    } else {
        selectionColor = Qt::blue;
    }
    selectionColor.setAlphaF( 0.3 );
    roiSelection->setBrush(selectionColor);
}

// Sets a predefined ROI selection based on a given roi size in percentage i.e. 0.3
// The predefined ROI preserved the image ratio
void VideoView::setROISelection(float roiSize) {
    if(roiSize > 0) {
        float roiWidth = imageSize.width * roiSize;
        float roiHeight = roiWidth/((float)imageSize.width/imageSize.height);
        roiSelection->setRect(QRectF(0,0, roiWidth, roiHeight));
    }
}

// Sets a ROI selection based on a given rectangle
void VideoView::setROISelection(QRectF roi) {
    //if(!roi.isEmpty() && graphicsScene->sceneRect().contains(roi)) {
    //    roiSelection->setRect(roi);
    //}
    // ML: CHANGED START 20.06.21
    if(!roi.isEmpty()) {
        roiSelection->setRect(roi);
    }
    // ML: CHANGED END
}
