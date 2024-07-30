
#include <QtWidgets/QHBoxLayout>
#include <QResizeEvent>
#include <ctime>
#include <opencv2/imgproc.hpp>
#include "videoView.h"
#include "../supportFunctions.h"

// Creates a new live-view widget
VideoView::VideoView(bool usingDoubleROI, QColor selectionColor1, QColor selectionColor2, QWidget *parent) : 
    QWidget(parent), 
    graphicsScene(new QGraphicsScene(this)),
    graphicsView(new QGraphicsView(this)),
    initialFit(false), 
    mode(FIT), 
    scaleFactor(1.25), 
    usingDoubleROI(usingDoubleROI),
    selectionColorCorrect1(selectionColor1),
    selectionColorCorrect2(selectionColor2)  {


    roi1GraphicsView = new QGraphicsView(graphicsView);
    roi1GraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    roi1GraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    int msize = std::max(graphicsView->width(), graphicsView->height());
    roi1GraphicsView->setGeometry(
        static_cast<int>(graphicsView->width() - (msize * 0.99)), 
        static_cast<int>(msize * 0.01), 
        static_cast<int>(msize * 0.2), 
        static_cast<int>(msize * 0.2));

    // GB modified begin
    if(usingDoubleROI) {
        roi2GraphicsView = new QGraphicsView(graphicsView);
        roi2GraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        roi2GraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        roi2GraphicsView->setGeometry(
            static_cast<int>(graphicsView->width() - (msize * 0.21)), 
            static_cast<int>(msize * 0.01), 
            static_cast<int>(msize * 0.2), 
            static_cast<int>(msize * 0.2));
    }

    penROIprocessed.setWidth(2);
    penROIunprocessed1.setWidth(2);
    penROIunprocessed2.setWidth(2);
    penPupilOutline.setWidth(2);
    penPupilCenter.setWidth(2);
    // GB modified end

    penROIunprocessed1 = QPen(selectionColorCorrect1);
    penROIunprocessed2 = QPen(selectionColorCorrect2);
    //selectionColorCorrect1 = Qt::blue;
    selectionColorCorrect1.setAlphaF( 0.3 );
    //selectionColorCorrect2 = Qt::green;
    selectionColorCorrect2.setAlphaF( 0.3 );
    //selectionColorWrong1 = Qt::red;
    selectionColorWrong1.setAlphaF( 0.3 );
    //selectionColorWrong2 = Qt::darkRed;
    selectionColorWrong2.setAlphaF( 0.3 );
    penAutoParamAccent.setStyle(Qt::DashLine);
    penAutoParamBack.setStyle(Qt::SolidLine);

    //roi1Selection = new ResizableRectItem(QRectF(QPointF(32, 32), QPointF(200, 200)));
    roi1Selection = new ResizableRectItem(QRectF(QPointF(20, 20), QPointF(20 + 200, 20 + 150)), QSizeF(4,3) );
    roi1Selection->setMinSize(QSizeF(30,30)); // GB
    roi1Selection->setBrush(selectionColorCorrect1);
    roi1Selection->setFlag(QGraphicsItem::ItemIsMovable);
    roi1Selection->setZValue(100);
    connect(roi1Selection, SIGNAL(onChange()), this, SLOT(onROI1Change()));
    roi1Selection->setVisible(false);
    graphicsScene->addItem(roi1Selection);
    // GB modified begin
    if(usingDoubleROI) {
        //roi2Selection = new ResizableRectItem(QRectF(QPointF(32, 32), QPointF(200, 300))); // DIFFERENT POSITION
        roi2Selection = new ResizableRectItem(QRectF(QPointF(220, 20), QPointF(220 + 200, 20 + 150)), QSizeF(4,3) );
        roi2Selection->setMinSize(QSizeF(30,30)); // GB
        roi2Selection->setBrush(selectionColorCorrect2);
        roi2Selection->setFlag(QGraphicsItem::ItemIsMovable);
        roi2Selection->setZValue(100);
        connect(roi2Selection, SIGNAL(onChange()), this, SLOT(onROI2Change()));
        roi2Selection->setVisible(false);
        graphicsScene->addItem(roi2Selection);
    }
    // GB modified end


    currentImage = new ImageGraphicsItem();
    graphicsScene->addItem(currentImage);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(graphicsView, 0, 0);

    setLayout(layout);

    graphicsView->setScene(graphicsScene);
    roi1GraphicsView->setScene(graphicsScene);
    graphicsView->show();
    roi1GraphicsView->hide();

    // GB added begin
    if(usingDoubleROI) {
        roi2GraphicsView->setScene(graphicsScene); 
        roi2GraphicsView->hide();
    }
    // GB added end
}

void VideoView::setSelectionColor1(QColor color) {
    selectionColorCorrect1 = color;
    penROIunprocessed1 = QPen(selectionColorCorrect1);
}

void VideoView::setSelectionColor2(QColor color) {
    selectionColorCorrect2 = color;
    penROIunprocessed2 = QPen(selectionColorCorrect2);
}

// Event handler when the widget is resized
// If the widget is currently in FIT mode, scale the live-view according to the window size
// If the lens pupil view is visible, it needs to be resized manually to preserve the size ratio to the large view
void VideoView::resizeEvent(QResizeEvent *event) {

    // GB: changed numbers to let both pupil views fit

    if(mode==FIT)
        graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);

    int msize;
    if(roi1GraphicsView->isVisible()) {
        msize = std::max(event->size().width(), event->size().height());
        roi1GraphicsView->setGeometry(
            static_cast<int>(event->size().width() - (msize * 0.99)), 
            static_cast<int>(msize * 0.01), 
            static_cast<int>(msize * 0.2), 
            static_cast<int>(msize * 0.2));
    }

    if(usingDoubleROI && roi2GraphicsView->isVisible()) {
        msize = std::max(event->size().width(), event->size().height());
        roi2GraphicsView->setGeometry(
            static_cast<int>(event->size().width() - (msize * 0.21)), 
            static_cast<int>(msize * 0.01), 
            static_cast<int>(msize * 0.2), 
            static_cast<int>(msize * 0.2));
    }
}

QSize VideoView::sizeHint() const {
    return QSize(640, 480);
}

void VideoView::clearProcessedOverlayMemory() {
    tROIs.clear();
    tPupils.clear();
}

void VideoView::drawOverlay() {

    drawPositioningGuide();
    drawAutoParamOverlay();

    if(tROIs.size()>=1)
        drawProcessedOverlay();
    else
        drawUnprocessedOverlay();
}

void VideoView::setAutoParamPupSize(int value) {
    autoParamPupSizePercent = value;
    //drawAutoParamOverlay();
}

void VideoView::onShowROI(bool value) {
    showROI = value;
    drawOverlay();
}

void VideoView::onShowPupilCenter(bool value) {
    plotPupilCenter = value;
    drawOverlay();
}

void VideoView::onChangePupilColorFill(int value) {
    pupilColorFill = (ColorFill)value;
    drawOverlay();
}

void VideoView::onChangePupilColorFillThreshold(float value) {
    colorFillLowEnd = value;
    drawOverlay();
}

void VideoView::onChangeShowAutoParamOverlay(bool state) {
    showAutoParamOverlay = state;
    drawOverlay();
}

void VideoView::onChangeShowPositioningGuide(bool state) {
    showPositioningGuide = state;
    drawOverlay();
}

void VideoView::onChangePupilDetectionUsingROI(bool state) {
    pupilDetectionUsingROI = state;
    drawOverlay();
}

// GB: This method paints the "overlaying" ROI and pupil outline on the graphicsScene, over the image
void VideoView::updateViewProcessed(const cv::Mat &img, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils) {

    // NOTE: the reason we are painting ROI on these images, based on ROI values originating from pupilDetection
    // is that like so we can be the best sure of what ROI we have used when pupilDetection was done

    tROIs.clear();
    tPupils.clear();
    for(std::size_t z=0; z<Pupils.size(); z++) {
        tROIs.push_back(ROIs[z]);
        tPupils.push_back(Pupils[z]);
    }

    drawOverlay();
    //drawProcessedOverlay();
    
    updateViewInternal(img);
}

void VideoView::setImageROI(const QRect& ROI) {
    // image ROI width and height is redundant, as it is already contained in the last known (received) actual
    // image width and height values.. but we keep them to use as safety checks.. is they do not match,
    // we have some GUI thread lag and we should not draw the positioning guide
    imageROI = ROI;
}

void VideoView::setSensorSize(const QSize& size) {
    // we need this to calculate the center of the sensor (map it) on the images for showin the positioning guide
    sensorSize = size;
}

void VideoView::drawPositioningGuide() {

    // to prevent memory leaks and lagging GUI
    for(std::size_t c=0; c<geBufferPG.size(); c++) {
        graphicsScene->removeItem(geBufferPG[c]);
        delete geBufferPG[c];
    }
    if(geBufferPG.size()>0)
        geBufferPG.clear();

    if(!showPositioningGuide ||
       sensorSize.isNull() || !sensorSize.isValid() || sensorSize.isEmpty() ||
       imageROI.isNull() || !imageROI.isValid() || imageROI.isEmpty() ||
       imageSize.width<=0 || imageSize.height<=0 ||
       imageSize.width!=imageROI.width() || imageSize.height!=imageROI.height()) {

        return;
    }

    float lineWidth = (float)imageSize.width / (float)graphicsView->width();
    penPositioningGuide.setWidthF(lineWidth);

    // truncation occurs due to operations done in integer type, but this precision is enough here
    int centerMappedX = sensorSize.width()/2-imageROI.x();
    int centerMappedY = sensorSize.height()/2-imageROI.y();

    int circleRadius;

    circleRadius = sensorSize.width()/3.5;
    geBufferPG.push_back( graphicsScene->addEllipse(QRect(centerMappedX-circleRadius, centerMappedY-circleRadius, circleRadius*2, circleRadius*2), penPositioningGuide) );

    circleRadius = sensorSize.width()/7.5;
    geBufferPG.push_back( graphicsScene->addEllipse(QRect(centerMappedX-circleRadius, centerMappedY-circleRadius, circleRadius*2, circleRadius*2), penPositioningGuide) );

    geBufferPG.push_back( graphicsScene->addLine(0-imageROI.x(), 0-imageROI.y(),sensorSize.width()-imageROI.x(), sensorSize.height()-imageROI.y(), penPositioningGuide) );
    geBufferPG.push_back( graphicsScene->addLine(0-imageROI.x(), sensorSize.height()-imageROI.y(), sensorSize.width()-imageROI.x(), 0-imageROI.y(), penPositioningGuide) );
    geBufferPG.push_back( graphicsScene->addLine(sensorSize.width()/2-imageROI.x(), 0-imageROI.y(), sensorSize.width()/2-imageROI.x(), sensorSize.height()-imageROI.y(), penPositioningGuide) );
    geBufferPG.push_back( graphicsScene->addLine(0-imageROI.x(), sensorSize.height()/2-imageROI.y(), sensorSize.width()-imageROI.x(), sensorSize.height()/2-imageROI.y(), penPositioningGuide) );

    geBufferPG.push_back( graphicsScene->addLine(0-imageROI.x(), 0-imageROI.y(),sensorSize.width()-imageROI.x(), 0-imageROI.y(), penPositioningGuide) );
    geBufferPG.push_back( graphicsScene->addLine(0-imageROI.x(), 0-imageROI.y(), 0-imageROI.x(), sensorSize.height()-imageROI.y(), penPositioningGuide) );
    geBufferPG.push_back( graphicsScene->addLine(sensorSize.width()-imageROI.x(), 0-imageROI.y(), sensorSize.width()-imageROI.x(), sensorSize.height()-imageROI.y(), penPositioningGuide) );
    geBufferPG.push_back( graphicsScene->addLine(0-imageROI.x(), sensorSize.height()-imageROI.y(), sensorSize.width()-imageROI.x(), sensorSize.height()-imageROI.y(), penPositioningGuide) );

    for(std::size_t c=0; c<geBufferPG.size(); c++) {
        geBufferPG[c]->setZValue(90);
    }
}

void VideoView::drawAutoParamOverlay() {

    // to prevent memory leaks and lagging GUI
    for(std::size_t c=0; c<geBufferAP.size(); c++) {
        graphicsScene->removeItem(geBufferAP[c]);
        delete geBufferAP[c];
    }
    if(geBufferAP.size()>0)
        geBufferAP.clear();

    if(!showAutoParamOverlay || !showROI ||
        imageSize.width<=0 || imageSize.height<=0) {

        return;
    }

    float lineWidth = (float)imageSize.width / (float)graphicsView->width();
    //qDebug() << "graphicsView->width()" << graphicsView->width();
    //qDebug() << "imageSize.width" << imageSize.width;
    //qDebug() << "graphicsScene->width()" << graphicsScene->width();
    //qDebug() << "graphicsScene->sceneRect().width()" << graphicsScene->sceneRect().width();
    //qDebug() << "lineWidth" << lineWidth;
    penAutoParamAccent.setWidthF(lineWidth);
    penAutoParamBack.setWidthF(lineWidth);

    //QRectF roi1;
//    if(graphicsScene->items().contains(roi1Selection))
//        roi1 = roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
//    else 
//        roi1 = roi1Selection->getRect();
    QRectF roi1R;
    if(pupilDetectionUsingROI)
        roi1R = roi1SelectionRectLastR; 
    else
        roi1R = QRectF(0,0,1,1); 
    QRectF roi1D = QRectF(roi1R.x()*imageSize.width, roi1R.y()*imageSize.height, roi1R.width()*imageSize.width, roi1R.height()*imageSize.height);

    float minDim1 = (roi1D.width()<=roi1D.height()) ? roi1D.width() : roi1D.height();

    float pxDiaInner1 = minDim1/100*(float)autoParamPupSizePercent *0.2f;
    float pxDiaOuter1 = minDim1/100*(float)autoParamPupSizePercent;
    QPainterPath pp1;
    pp1.addEllipse(roi1D.x()+(roi1D.width()/2)-pxDiaOuter1/2, roi1D.y()+(roi1D.height()/2)-pxDiaOuter1/2, pxDiaOuter1, pxDiaOuter1);
    pp1.addEllipse(roi1D.x()+(roi1D.width()/2)-pxDiaInner1/2, roi1D.y()+(roi1D.height()/2)-pxDiaInner1/2, pxDiaInner1, pxDiaInner1);
    QGraphicsItem *ppointer1 = graphicsScene->addPath(pp1, penAutoParamAccent, QBrush(QColor(10,255,10,90)));
    ppointer1->setZValue(98);
    //graphicsScene->addItem(ppointer1);
    geBufferAP.push_back(ppointer1);

    if(!usingDoubleROI) {
        return;
    }

    //QRectF roi2;
//    if(graphicsScene->items().contains(roi2Selection))
//        roi2 = roi2Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
//    else
//        roi2 = roi2Selection->getRect();
    QRectF roi2R;
    if(pupilDetectionUsingROI)
        roi2R = roi2SelectionRectLastR;
    else
        roi2R = QRectF(0,0,1,1);
    QRectF roi2D = QRectF(roi2R.x()*imageSize.width, roi2R.y()*imageSize.height, roi2R.width()*imageSize.width, roi2R.height()*imageSize.height);

    float pxDiaInner2 = minDim1/100*(float)autoParamPupSizePercent *0.15f;
    float pxDiaOuter2 = minDim1/100*(float)autoParamPupSizePercent;
    QPainterPath pp2;
    pp2.addEllipse(roi2D.x()+(roi2D.width()/2)-pxDiaOuter2/2, roi2D.y()+(roi2D.height()/2)-pxDiaOuter2/2, pxDiaOuter2, pxDiaOuter2);
    pp2.addEllipse(roi2D.x()+(roi2D.width()/2)-pxDiaInner2/2, roi2D.y()+(roi2D.height()/2)-pxDiaInner2/2, pxDiaInner2, pxDiaInner2);
    QGraphicsItem *ppointer2 = graphicsScene->addPath(pp2, penAutoParamAccent, QBrush(QColor(10,255,10,90)));
    ppointer2->setZValue(98);
    //graphicsScene->addItem(ppointer1);
    geBufferAP.push_back(ppointer2);
}

void VideoView::drawUnprocessedOverlay() {

    // to prevent memory leaks and lagging GUI
    for(std::size_t c=0; c<geBufferROI.size(); c++) {
        graphicsScene->removeItem(geBufferROI[c]);
        delete geBufferROI[c];
    }
    if(geBufferROI.size()>0)
        geBufferROI.clear();

    if(!showROI || imageSize.width<=0 || imageSize.height<=0) {
        return;
    }

    float lineWidth = (float)imageSize.width / (float)graphicsView->width();
    //qDebug() << "graphicsView->width()" << graphicsView->width();
    //qDebug() << "imageSize.width" << imageSize.width;
    //qDebug() << "graphicsScene->width()" << graphicsScene->width();
    //qDebug() << "graphicsScene->sceneRect().width()" << graphicsScene->sceneRect().width();
    //qDebug() << "lineWidth" << lineWidth;
    penROIunprocessed1.setWidthF(lineWidth);
    penROIunprocessed2.setWidthF(lineWidth);

    //qDebug() << "roi1Selection->rect() = " << roi1Selection->rect();
    //qDebug() << "roi1Selection->getRect() = " << roi1Selection->getRect();
    //qDebug() << "roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5) = " << roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);

    QRectF roi1R;
//    if(graphicsScene->items().contains(roi1Selection))
//        roi1 = roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
//    else
//        roi1 = roi1Selection->getRect();
    roi1R = roi1SelectionRectLastR;

    QGraphicsRectItem *ROIrect1 = new QGraphicsRectItem(roi1R.x()*imageSize.width, roi1R.y()*imageSize.height, roi1R.width()*imageSize.width, roi1R.height()*imageSize.height);
    ROIrect1->setPen(penROIunprocessed1);
    ROIrect1->setZValue(90);

    graphicsScene->addItem(ROIrect1);
    geBufferROI.push_back(ROIrect1);

    if(!usingDoubleROI) {
        return;
    }

    QRectF roi2R;
//    if(graphicsScene->items().contains(roi2Selection))
//        roi2 = roi2Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
//    else
//        roi2 = roi2Selection->getRect();
    roi2R = roi2SelectionRectLastR;

    QGraphicsRectItem *ROIrect2 = new QGraphicsRectItem(roi2R.x()*imageSize.width, roi2R.y()*imageSize.height, roi2R.width()*imageSize.width, roi2R.height()*imageSize.height);
    ROIrect2->setPen(penROIunprocessed2);
    ROIrect2->setZValue(90);

    graphicsScene->addItem(ROIrect2);
    geBufferROI.push_back(ROIrect2);
}

void VideoView::drawProcessedOverlay() {

    // GB NOTE: all ROIs that arrive from pupilDetection are yet in DISCRETE px dimensions, not 0-1 floats

    // to prevent memory leaks and lagging GUI
    for(std::size_t c=0; c<geBufferROI.size(); c++) {
        graphicsScene->removeItem(geBufferROI[c]);
        delete geBufferROI[c];
    }
    if(geBufferROI.size()>0)
        geBufferROI.clear();

    if(imageSize.width<=0 || imageSize.height<=0)
        return;

    float lineWidth = (float)imageSize.width / (float)graphicsView->width();
    //qDebug() << "graphicsView->width()" << graphicsView->width();
    //qDebug() << "imageSize.width" << imageSize.width;
    //qDebug() << "graphicsScene->width()" << graphicsScene->width();
    //qDebug() << "graphicsScene->sceneRect().width()" << graphicsScene->sceneRect().width();
    //qDebug() << "lineWidth" << lineWidth;
    penROIprocessed.setWidthF(lineWidth);
    penPupilOutline.setWidthF(lineWidth);
    penPupilCenter.setWidthF(lineWidth);

    for(std::size_t z=0; z<tPupils.size(); z++) {
        if(showROI) {
            QGraphicsRectItem *ROIrect = new QGraphicsRectItem(tROIs[z].x, tROIs[z].y, tROIs[z].width, tROIs[z].height);
            ROIrect->setPen(penROIprocessed);
            ROIrect->setZValue(90);

            graphicsScene->addItem(ROIrect);
            geBufferROI.push_back(ROIrect);
        }
        if(tPupils[z].valid(-2.0)) {
            QGraphicsEllipseItem *pupil = new QGraphicsEllipseItem(tPupils[z].center.x-tPupils[z].size.width/2, tPupils[z].center.y-tPupils[z].size.height/2, tPupils[z].size.width, tPupils[z].size.height);
            pupil->setTransformOriginPoint(tPupils[z].center.x, tPupils[z].center.y);
            pupil->setRotation(tPupils[z].angle);

            pupil->setPen(penPupilOutline);
            if(pupilColorFill == ColorFill::CONFIDENCE && tPupils[z].confidence >= 0) {
                //qDebug() << tPupils[z].confidence;
                float t = (tPupils[z].confidence-colorFillLowEnd)*(1/(1-colorFillLowEnd));
                if(t<0)
                    t=0;
                pupil->setBrush(QColor(255-t*255, t*255, 0, 255));
            } else if(pupilColorFill == ColorFill::OUTLINE_CONFIDENCE && tPupils[z].outline_confidence >= 0) {
                //qDebug() << tPupils[z].outline_confidence;
                float t = (tPupils[z].outline_confidence-colorFillLowEnd)*(1/(1-colorFillLowEnd));
                if(t<0)
                    t=0;
                pupil->setBrush(QColor(255-t*255, t*255, 0, 255));
            }
            // qDebug() << tPupils[z].confidence-tPupils[z].outline_confidence;
            pupil->setZValue(90);
            graphicsScene->addItem(pupil);
            geBufferROI.push_back(pupil);

            if(plotPupilCenter) {
                QGraphicsRectItem *center = new QGraphicsRectItem(tPupils[z].center.x -(lineWidth), tPupils[z].center.y -(lineWidth), lineWidth*2, lineWidth*2);
                //QGraphicsRectItem *center = new QGraphicsRectItem(tPupils[z].center.x -1, tPupils[z].center.y -1, 2, 2);
                center->setPen(penPupilCenter);
                center->setZValue(95);

                graphicsScene->addItem(center);
                geBufferROI.push_back(center);
            }
        } else {
            QGraphicsTextItem *notfound = new QGraphicsTextItem();
            notfound->setPos(tROIs[z].x +round(tROIs[z].width*0.1), tROIs[z].y +round(tROIs[z].height*0.1));
            notfound->setDefaultTextColor(Qt::red);
            QFont f;
            f.setPixelSize(round(tROIs[z].height/7));
            notfound->setFont(f);
            notfound->setPlainText("NO PUPIL FOUND");
            notfound->setZValue(98);
            geBufferROI.push_back(notfound);
            graphicsScene->addItem(notfound);
        }
    }
}

// GB: grabbed imaged without pupilDetection land here (called from camera views)
// This method paints ROI onto images in cases when pupil detection is on, but the image did not make its way through pupilDetection
void VideoView::updateView(const cv::Mat &img) {
    imageSize = img.size(); // GB: moved here to let it provide image size even on the first (still) frame

    tROIs.clear();
    tPupils.clear();
    drawOverlay();
    //drawUnprocessedOverlay();

    updateViewInternal(img);
}

// Slot receiving images to display
// The image received as cv::Mat is first converted to a QImage, then painted to the scene using the custom ImageGraphicsItem
// If fit scaling is activated, the scene is scaled to fit the window size, however, only on the first image as we dont know the image dimensions before
// GB NOTES: simplified, and moved color conversion out of this method. 
// This method is now also used for cases when we update the videoView just because the user switched on/off pupildetection, 
// thus the existing currentImage is redrawn, with different ROI overlay.
void VideoView::updateViewInternal(const cv::Mat &img) {
    if(img.empty())
        return;

    // For some reason, qt crashes when trying to paint a grayscale image over qpainter
    cv::Mat bgrFrame = img; 
    if (img.channels() == 1)
        cv::cvtColor(img, bgrFrame, cv::COLOR_GRAY2BGR);
    QImage qImg = cvMatToQImage(bgrFrame);

    // added by Gabor Benyei (kheki4) on 2022.11.07, 
    // Reason: when camera binning is changed, the Pylon library sends a request to the camera to send new images considering the newly set binning value
    // however, there is no guarantee at all, that the next acquired image will come with the newly set binning. 
    // Then, GUI will show a strangely small or large image, not fitted. 
    // This can only be effectively worked around, if we always check image size, and  fit it, if differs from the previous one
    if(currentImage->boundingRect() != qImg.rect()) {
        initialFit = false;
        //std::cout << "OLD IMAGE BOUNDING RECT: width = " << currentImage->boundingRect().width() << " ; height = " << currentImage->boundingRect().height() << std::endl;
        //std::cout << "NEW IMAGE BOUNDING RECT: width = " << qImg.rect().width() << " ; height = " << qImg.rect().height() << std::endl;
    }
    // end of add by Gabor Benyei (kheki4)

    graphicsScene->removeItem(currentImage);
    delete currentImage;
    currentImage = new ImageGraphicsItem(qImg);
    graphicsScene->addItem(currentImage);

    if(!initialFit) {
        //imageSize = img.size(); // GB: moved  to updateView
        //std::cout << "SETTING SCENE RECT: width = " << img.cols << " ; height = " << img.rows << std::endl;

        graphicsScene->setSceneRect(0, 0, img.cols, img.rows);
        if(mode==FIT) {
            graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
        }

        // GB modified begin
        /*
        // GB: this should never cause a problem from now on, as ROIs during setting are expressed as ratios of image width and height
        // ML: ADDED START 20.06.21
        // Reason: Prevent error if the ROI is bigger than the image itself
        QRectF roi1 = roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
        if(!graphicsScene->sceneRect().contains(roi1)) {
            // GB begin
            roi1SelectionRectLastR = QRectF(0.1, 0.35, 0.3, 0.3); 
            roi1Selection->setRect(QRect(roi1SelectionRectLastR.x()*img.cols,roi1SelectionRectLastR.y()*img.rows,roi1SelectionRectLastR.width()*img.cols,roi1SelectionRectLastR.height()*img.rows));
            // GB end
            saveROI1Selection();
        }
        // ML: ADDED END

        // GB added begin
        if(usingDoubleROI) {
            QRectF roi2 = roi2Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
            if(!graphicsScene->sceneRect().contains(roi2)) {
                roi2SelectionRectLastR = QRectF(0.6, 0.35, 0.3, 0.3); 
                roi2Selection->setRect(QRect(roi2SelectionRectLastR.x()*img.cols,roi2SelectionRectLastR.y()*img.rows,roi2SelectionRectLastR.width()*img.cols,roi2SelectionRectLastR.height()*img.rows));
                saveROI2Selection();
            }
        }
        // GB added end
        */
        // GB modified end

        initialFit = true;
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

void VideoView::updatePupilViews(const std::vector<QRect> &rects) {
    if(rects[0].isValid() && rects[0].width()>1) {
        roi1GraphicsView->resetTransform();
        roi1GraphicsView->fitInView(rects[0]);
    }
    if(usingDoubleROI && rects[1].isValid() && rects[1].width()>1) {
        roi2GraphicsView->resetTransform();
        roi2GraphicsView->fitInView(rects[1]);
    }
}

/*
// Updates the region of the pupil lens view
void VideoView::updatePupil1View(const QRect &rect) {
    if(rect.isValid()) {
        //roiGraphicsView->setSceneRect(rect);
        roi1GraphicsView->resetTransform();
        roi1GraphicsView->fitInView(rect);
    }
}

// Updates the region of the pupil lens view
void VideoView::updatePupil2View(const QRect &rect) {
    if(usingDoubleROI && rect.isValid()) {
        //roiGraphicsView->setSceneRect(rect);
        roi2GraphicsView->resetTransform();
        roi2GraphicsView->fitInView(rect);
    }
}
*/



// Activates the pupil lens view, which is an overlay on the original scene showing the pupil detection in detail
void VideoView::enablePupilView(bool value) {
    if(value) {
        roi1GraphicsView->show();
        if(usingDoubleROI)
            roi2GraphicsView->show();
    } else {
        roi1GraphicsView->hide();
        if(usingDoubleROI)
            roi2GraphicsView->hide();
    }
}

// Show the ROI selection on top of the scene
void VideoView::showROISelection(bool value) {
    roi1Selection->setVisible(value);
    if(usingDoubleROI)
        roi2Selection->setVisible(value);
    // GB NOTE: This must stay here, before removing roi1Selection from the graphics scene
    //drawOverlay();
}

// Saves the current selected rect by the ROI selection (GB: ROI nr 1)
// Sends the selection as a signal
// If the selected ROI is outside of image bounds nothing is transmitted and false returned i.e. no new ROI is set
bool VideoView::saveROI1Selection() {

    // GB modified begin
    //QRectF roi = roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
    //QRectF roiDBoundingRect = roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
    QRectF roiD = roi1Selection->sceneBoundingRect();
    //QRectF roiD = roi1Selection->getRect();
    QRectF roiR;
    if(imageSize.width>0 && imageSize.height>0)
        roiR = QRectF( roiD.x()/imageSize.width, roiD.y()/imageSize.height, roiD.width()/imageSize.width, roiD.height()/imageSize.height );
    else
        roiR = QRectF(0,0,1,1);

    roi1SelectionRectLastR = roiR;

    std::cout << "imageSize.width=" << imageSize.width<<"; roiR.x()="<<roiR.x()<<"; roiR.width()="<<roiR.width()<<std::endl;
    std::cout << "imageSize.height=" << imageSize.height<<"; roiR.y()="<<roiR.y()<<"; roiR.height()="<<roiR.height()<<std::endl;
    std::cout << "imageSize.width=" << imageSize.width<<"; roiD.x()="<<roiD.x()<<"; roiD.width()="<<roiD.width()<<std::endl;
    std::cout << "imageSize.height=" << imageSize.height<<"; roiD.y()="<<roiD.y()<<"; roiD.height()="<<roiD.height()<<std::endl;

    // GB modified: removed initialFit restriction
    if(roiR.size().width() > 1 ||  roiR.size().height() > 1) {
        std::cout<<"Saving ROI1 Selection: out of image bounds."<<std::endl;
        return false;
    }

    bool left = false;
    if (roi1AllowedArea == ROIAllowedArea::RIGHT_HALF && roiR.left() < 0.5){
    left = true;
    }
    bool right = false;
    if (roi1AllowedArea == ROIAllowedArea::LEFT_HALF && roiR.right() > 0.5){
        right = true;
    }
    QRectF sceneRect = graphicsScene->sceneRect();
    bool contains = graphicsScene->sceneRect().contains(roiD);
    if( !graphicsScene->sceneRect().contains(roiD) ||
            (left) ||
            (right) ) {
        std::cout<<"Saving ROI1 Selection: out of scene bounds."<<std::endl;
        return false;
    }
    //std::cout<<"ROI selected contained:" << graphicsScene->sceneRect().contains(roi) << " size: " << roi.topLeft().x() << ":" << roi.topLeft().x() << " - " << roi.height() << std::endl;
    emit onROI1SelectionR(roiR);
    emit onROI1SelectionD(roiD);

    return true;
    // GB modified end
}

// Saves the current selected rect by the ROI selection (GB: ROI nr 2)
// Sends the selection as a signal
// If the selected ROI is outside of image bounds nothing is transmitted and false returned i.e. no new ROI is set
bool VideoView::saveROI2Selection() {

    if(!usingDoubleROI)
        return false;

    //QRectF roi = roi1Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
    QRectF roiD = roi2Selection->sceneBoundingRect() - QMarginsF(0.5,0.5,0.5,0.5);
    QRectF roiR;
    if(imageSize.width>0 && imageSize.height>0)
        roiR = QRectF( roiD.x()/imageSize.width, roiD.y()/imageSize.height, roiD.width()/imageSize.width, roiD.height()/imageSize.height );
    else
        roiR = QRectF(0,0,1,1);

    std::cout << "imageSize.width=" << imageSize.width<<"; roiR.x()="<<roiR.x()<<"; roiR.width()="<<roiR.width()<<std::endl;
    std::cout << "imageSize.height=" << imageSize.height<<"; roiR.y()="<<roiR.y()<<"; roiR.height()="<<roiR.height()<<std::endl;
    std::cout << "imageSize.width=" << imageSize.width<<"; roiD.x()="<<roiD.x()<<"; roiD.width()="<<roiD.width()<<std::endl;
    std::cout << "imageSize.height=" << imageSize.height<<"; roiD.y()="<<roiD.y()<<"; roiD.height()="<<roiD.height()<<std::endl;

    // GB modified: removed initialFit restriction
    if(roiR.size().width() > 1 ||  roiR.size().height() > 1) {
        std::cout<<"Saving ROI2 Selection: out of image bounds."<<std::endl;
        return false;
    }

    if( !graphicsScene->sceneRect().contains(roiD) ||
            (roi1AllowedArea == ROIAllowedArea::RIGHT_HALF && roiR.left() < 0.5) ||
            (roi1AllowedArea == ROIAllowedArea::LEFT_HALF && roiR.right() > 0.5) ) {
        std::cout<<"Saving ROI2 Selection: out of scene bounds."<<std::endl;
        return false;
    }
    //std::cout<<"ROI selected contained:" << graphicsScene->sceneRect().contains(roi) << " size: " << roi.topLeft().x() << ":" << roi.topLeft().x() << " - " << roi.height() << std::endl;
    emit onROI2SelectionR(roiR);
    emit onROI2SelectionD(roiD);

    return true;
}

// Discards the current ROI selection (GB: ROI nr 1 and 2), meaning no new ROI is set and the ROI selection is rest to a default size
void VideoView::resetROISelection() {

    QRectF roiR;

    float defSf = 0.7f;

    // GB modified begin
    // GB NOTE: modified to work with rational number size ROIs. ALso added code to use roi1SelectionRectLastR and roi2SelectionRectLastR
    float minSize = std::min(imageSize.width, imageSize.height);
    if(!usingDoubleROI) {
        roiR = QRectF( 0.35, 0.35, 0.3, 0.3 );
        roi1Selection->setBrush(selectionColorCorrect1);
        roi1Selection->setPos(0, 0);
        qDebug() << "roi1Selection->setRect() via resetROISelection(): " << roiR; 
        QRectF rect = roi1Selection->getRect();
        roi1SelectionRectLastR = roiR; 
        roi1Selection->setRect(QRect(roi1SelectionRectLastR.x()*imageSize.width,roi1SelectionRectLastR.y()*imageSize.height,roi1SelectionRectLastR.width()*imageSize.width,roi1SelectionRectLastR.height()*imageSize.height));
        
        roi1Selection->update();
        std::cout<<"ROI1 reset (RATIO) contained:" << 
            graphicsScene->sceneRect().contains(QRect(roi1SelectionRectLastR.x()*imageSize.width,roi1SelectionRectLastR.y()*imageSize.height,roi1SelectionRectLastR.width()*imageSize.width,roi1SelectionRectLastR.height()*imageSize.height)) << 
            " size: " << roiR.topLeft().x() << ":" << roiR.topLeft().x() << " - " << roiR.height() << std::endl;

    } else {
        roiR = QRectF( 0.05, 0.35, 0.3, 0.3 );
        roi1Selection->setBrush(selectionColorCorrect1);
        roi1Selection->setPos(0, 0);
        qDebug() << "roi1Selection->setRect() via resetROISelection(): " << roiR; 
        
        roi1SelectionRectLastR = roiR; 
        roi1Selection->setRect(QRect(roi1SelectionRectLastR.x()*imageSize.width,roi1SelectionRectLastR.y()*imageSize.height,roi1SelectionRectLastR.width()*imageSize.width,roi1SelectionRectLastR.height()*imageSize.height));
        
        roi1Selection->update();
        std::cout<<"ROI1 reset (RATIO) contained:" << 
            graphicsScene->sceneRect().contains(QRect(roi1SelectionRectLastR.x()*imageSize.width,roi1SelectionRectLastR.y()*imageSize.height,roi1SelectionRectLastR.width()*imageSize.width,roi1SelectionRectLastR.height()*imageSize.height)) << 
            " size: " << roiR.topLeft().x() << ":" << roiR.topLeft().x() << " - " << roiR.height() << std::endl;

        roiR = QRectF( 0.65, 0.35, 0.3, 0.3 );
        roi2Selection->setBrush(selectionColorCorrect2);
        roi2Selection->setPos(0, 0);
        qDebug() << "roi2Selection->setRect() via resetROISelection(): " << roiR; 
        
        roi2SelectionRectLastR = roiR; 
        roi2Selection->setRect(QRect(roi2SelectionRectLastR.x()*imageSize.width,roi2SelectionRectLastR.y()*imageSize.height,roi2SelectionRectLastR.width()*imageSize.width,roi2SelectionRectLastR.height()*imageSize.height));
        
        roi2Selection->update();
        std::cout<<"ROI2 reset (RATIO) contained:" << 
            graphicsScene->sceneRect().contains(QRect(roi2SelectionRectLastR.x()*imageSize.width,roi2SelectionRectLastR.y()*imageSize.height,roi2SelectionRectLastR.width()*imageSize.width,roi2SelectionRectLastR.height()*imageSize.height)) << 
            " size: " << roiR.topLeft().x() << ":" << roiR.topLeft().x() << " - " << roiR.height() << std::endl;
    }
    // GB modified end
}

// Event handler when the selection of the ROI (GB: ROI nr 1) is changed i.e. the user moved the selection
// To signal a invalid selection, the ROI selection is colored red if it goes outside the scene
// GB: modified that selection color brushes are now stored in global variables
void VideoView::onROI1Change() {
    QRectF roiD = roi1Selection->sceneBoundingRect();

    if( !graphicsScene->sceneRect().contains(roiD) ||
        (roi1AllowedArea == ROIAllowedArea::RIGHT_HALF && roiD.left() < imageSize.width/2) ||
        (roi1AllowedArea == ROIAllowedArea::LEFT_HALF && roiD.right() > imageSize.width/2)
        ) {
        roi1Selection->setBrush(selectionColorWrong1);
    } else {
        roi1Selection->setBrush(selectionColorCorrect1);
    }
}

// Event handler when the selection of the ROI (GB: ROI nr 2) is changed i.e. the user moved the selection
// To signal a invalid selection, the ROI selection is colored red if it goes outside the scene
// GB: added, and now using global selection color brushes
void VideoView::onROI2Change() {
    if(!usingDoubleROI)
        return;

    QRectF roiD = roi2Selection->sceneBoundingRect();

    if(!graphicsScene->sceneRect().contains(roiD) ||
        (roi2AllowedArea == ROIAllowedArea::RIGHT_HALF && roiD.left() < imageSize.width/2) ||
        (roi2AllowedArea == ROIAllowedArea::LEFT_HALF && roiD.right() > imageSize.width/2)
        ) {
        roi2Selection->setBrush(selectionColorWrong2);
    } else {
        roi2Selection->setBrush(selectionColorCorrect2);
    }
}

// Sets a predefined ROI (GB: ROI nr 1) selection based on a given roi size in percentage i.e. 0.3
// The predefined ROI preserved the image ratio
// GB: modified for RATIO ROIs
void VideoView::setROI1SelectionR(float roiSize) {
    // GB: roiSize>0.5 would not fit
    if(roiSize<=0 || (usingDoubleROI && roiSize>0.5))
        return; 

    if(!usingDoubleROI)
        roi1SelectionRectLastR = QRectF(0.5-roiSize/2, 0.5-roiSize/2, roiSize, roiSize);
    else
        roi1SelectionRectLastR = QRectF(0.75-roiSize/2, 0.5-roiSize/2, roiSize, roiSize);

    roi1Selection->setNormalizedRect(SupportFunctions::getRectDiscreteFromRational(QSizeF(imageSize.width, imageSize.height), roi1SelectionRectLastR));
    
    qDebug() << "roi1Selection->setRect() via setROI1Selection(float roiSize) (RATIO): " << roi1SelectionRectLastR; 
}

// Sets a predefined ROI (GB: ROI nr 2) selection based on a given roi size in percentage i.e. 0.3
// The predefined ROI preserved the image ratio
// GB: added, and modified for RATIO ROIs
void VideoView::setROI2SelectionR(float roiSize) {
    // GB: roiSize>0.5 would not fit
    if(roiSize<=0 || !usingDoubleROI || roiSize>0.5)
        return;

    roi2SelectionRectLastR = QRectF(0.75-roiSize/2, 0.5-roiSize/2, roiSize, roiSize); 
    roi2Selection->setRect(QRect(roi2SelectionRectLastR.x()*imageSize.width,roi2SelectionRectLastR.y()*imageSize.height,roi2SelectionRectLastR.width()*imageSize.width,roi2SelectionRectLastR.height()*imageSize.height));
    qDebug() << "roi2Selection->setRect() via setROI1Selection(float roiSize): " << roi2SelectionRectLastR; 
}

// Sets a ROI (GB: ROI nr 1) selection based on a given rectangle
// GB: modified for RATIO ROIs
void VideoView::setROI1SelectionR(QRectF roiR) {
    if(roiR.isEmpty())
        return;

    roi1SelectionRectLastR = roiR;
    roi1Selection->setNormalizedRect(SupportFunctions::getRectDiscreteFromRational(QSizeF(imageSize.width, imageSize.height), roi1SelectionRectLastR));
    
    qDebug() << "roi1Selection->setRect() via setROI1Selection(QRectF roi) (RATIO): " << roiR; 
}

// Sets a ROI (GB: ROI nr 2) selection based on a given rectangle
// GB: added, and modified for RATIO ROIs
void VideoView::setROI2SelectionR(QRectF roiR) {
    if(roiR.isEmpty() || !usingDoubleROI)
        return;

    roi2SelectionRectLastR = roiR; 
    roi2Selection->setNormalizedRect(SupportFunctions::getRectDiscreteFromRational(QSizeF(imageSize.width, imageSize.height), roi2SelectionRectLastR));
    qDebug() << "roi2Selection->setRect() via setROI2Selection(QRectF roi) (RATIO): " << roiR; 
}

bool VideoView::getDoubleROI() {
    return usingDoubleROI;
}

// Sets whether we want to use bouble or single ROIs in this VideoView instance
void VideoView::setDoubleROI(bool state) {
    if(!state && usingDoubleROI) {
        // disable double ROI

        disconnect(roi2Selection, SIGNAL(onChange()), this, SLOT(onROI2Change()));
        delete roi2Selection;
        delete roi2GraphicsView;

        //roi2GraphicsView->hide();

    } else if(state && !usingDoubleROI) {
    //} else if(  state && !usingDoubleROI && 
    //            !roi2GraphicsView && !roi2Selection) {
        // initiate double ROI

        roi2GraphicsView = new QGraphicsView(graphicsView);
        roi2GraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        roi2GraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //const float wImg = 640;
        //const float sf = 0.3;
        int msize = std::max(graphicsView->width(), graphicsView->height());
        roi2GraphicsView->setGeometry(
            static_cast<int>(graphicsView->width() - (msize * 0.21)), 
            static_cast<int>(msize * 0.01), 
            static_cast<int>(msize * 0.2), 
            static_cast<int>(msize * 0.2));

        roi2Selection = new ResizableRectItem(QRectF(QPointF(220, 20), QPointF(220 + 200, 20 + 150)), QSizeF(4,3) );
        roi2Selection->setBrush(selectionColorCorrect2);
        roi2Selection->setFlag(QGraphicsItem::ItemIsMovable);
        roi2Selection->setZValue(100);
        connect(roi2Selection, SIGNAL(onChange()), this, SLOT(onROI2Change()));
    
        roi2GraphicsView->setScene(graphicsScene); 
        roi2GraphicsView->hide();
    }

    usingDoubleROI = state;
}

void VideoView::setROI1AllowedArea(int roiAllowedArea) {
    roi1AllowedArea = roiAllowedArea;
}

void VideoView::setROI2AllowedArea(int roiAllowedArea) {
    roi2AllowedArea = roiAllowedArea;
}

QRectF VideoView::getROI1SelectionR(){
    return roi1SelectionRectLastR;
}

QRectF VideoView::getROI2SelectionR(){
    return roi2SelectionRectLastR;
}