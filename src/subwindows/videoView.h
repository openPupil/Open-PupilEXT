
#ifndef PUPILEXT_VIDEOVIEW_H
#define PUPILEXT_VIDEOVIEW_H

/**
    @author Moritz Lode, Gábor Bényei
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

enum ViewMode {FIT = 0, FULL = 1, ZOOM = 2};

enum ColorFill {NO_FILL = 0, CONFIDENCE = 1, OUTLINE_CONFIDENCE = 2};

/**
    Custom widget representing a live camera view displaying a given camera images in a live-view

    Converts the given camera image to QImage and paints it directly on a QGraphicsView/QGraphicsScene

    Also renders and handles the ROI selection by the user, rendered over the camera image, returns ROI results through a signal

signals:
    void onROISelection(QRectF roi): Signalling a ROI selection by the user, transporting the ROI rect

*/
class VideoView : public QWidget {
    Q_OBJECT

public:

    // GB: added to guide user not to select overlapping areas when used with double ROIs
    enum ROIAllowedArea {ALL = 0, LEFT_HALF = 1, RIGHT_HALF = 2};


    explicit VideoView(bool usingDoubleROI=false, QColor selectionColor1=Qt::blue, QColor selectionColor2=Qt::green, QWidget *parent=0);

    QSize sizeHint() const override;

    // GB added begin
    QRectF getImageSize() {
        return QRectF(imageSize.width, imageSize.height, imageSize.width, imageSize.height);
    }
    // GB added end

    void setImageSize(const float width, const float height){
        imageSize.width = width;
        imageSize.height = height;
    }
    // Source Andy Maloney: https://github.com/asmaloney/asmOpenCV/blob/master/asmOpenCV.h
    static inline QImage cvMatToQImage(const cv::Mat &inMat)
    {
        switch (inMat.type())
        {
            // 8-bit, 4 channel
            case CV_8UC4:
            {
                QImage image(inMat.data,
                             inMat.cols, inMat.rows,
                             static_cast<int>(inMat.step),
                             QImage::Format_ARGB32);

                return image;
            }

                // 8-bit, 3 channel
            case CV_8UC3:
            {
                QImage image(inMat.data,
                             inMat.cols, inMat.rows,
                             static_cast<int>(inMat.step),
                             QImage::Format_RGB888);

                return image.rgbSwapped();
            }

            // 8-bit, 1 channel
            case CV_8U:
            {
#if QT_VERSION >= 0x050500

                // From Qt 5.5
                QImage image(inMat.data, inMat.cols, inMat.rows,
                             static_cast<int>(inMat.step),
                             QImage::Format_Grayscale8);
#else
                static QVector<QRgb>  sColorTable;

                // only create our color table the first time
                if (sColorTable.isEmpty())
                {
                    sColorTable.resize(256);
                    for (int i = 0; i < 256; ++i)
                    {
                        sColorTable[i] = qRgb(i, i, i);
                    }
                }

                QImage image(inMat.data,
                    inMat.cols, inMat.rows,
                    static_cast<int>(inMat.step),
                    QImage::Format_Indexed8);

                image.setColorTable(sColorTable);
#endif
                return image;
            }

            default:
                std::cerr << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type() << std::endl;
                break;
        }

        return QImage();
    }

private:

    QGraphicsView *graphicsView;
    QGraphicsView *roi1GraphicsView; // GB renamed
    QGraphicsScene *graphicsScene;

    ImageGraphicsItem *currentImage;

    bool initialFit;
    cv::Size imageSize;
    double scaleFactor;

    int mode;

    // GB added/modified begin
    std::vector<cv::Rect> tROIs;
    std::vector<Pupil> tPupils;

    ResizableRectItem *roi1Selection; 
    ResizableRectItem *roi2Selection; 
    // GB we need these, because due to some kind of bug (Qt 5.15), the rect item's sceneBoundingRect() returns inappropriate values
    QRectF roi1SelectionRectLastR; // GB the last saved position. necessary in cases when there is no pupil detection going on, and we are setting a custom ROI, which is not yet committed, but moved on the scene. when image play is on, this variable is needed
    QRectF roi2SelectionRectLastR; 
    void updateViewInternal(const cv::Mat &img);
    
    const float wImg = 640;
    const float sf = 0.3F;

    std::vector<QGraphicsItem*> geBufferROI;
    
    QColor selectionColorCorrect1;// = Qt::blue;
    QColor selectionColorCorrect2;// = Qt::green;
    QColor selectionColorWrong1 = Qt::red;
    QColor selectionColorWrong2 = Qt::darkRed;

    QPen penROIprocessed = Qt::magenta;
    QPen penROIunprocessed1;// = Qt::blue;
    QPen penROIunprocessed2;// = Qt::green;
    QPen penPupilOutline = Qt::red;
    QPen penPupilCenter = Qt::red;

    std::vector<QGraphicsItem*> geBufferAP;
    QPen penAutoParamAccent = QColor(245, 197, 66, 255);
    QPen penAutoParamBack = QColor(144, 66, 245, 255);
    int autoParamPupSizePercent = 50;

    bool usingDoubleROI = false;
    QGraphicsView *roi2GraphicsView; // GB NOTE: shows the zoomed in pupil if needed. overlays the image

    int roi1AllowedArea = ROIAllowedArea::ALL;
    int roi2AllowedArea = ROIAllowedArea::ALL;

    float colorFillLowEnd = 0.0;
    ColorFill pupilColorFill = ColorFill::NO_FILL;
    bool showROI = true;
    bool plotPupilCenter = true;
    bool showAutoParamOverlay = false;
    bool pupilDetectionUsingROI;
    // GB added/modified end

protected:

    void resizeEvent(QResizeEvent *event) override;

public slots:

    // GB added/modified begin
    void updateViewProcessed(const cv::Mat &img, const std::vector<cv::Rect> &ROIs, const std::vector<Pupil> &Pupils);
    void drawUnprocessedOverlay();
    void drawProcessedOverlay();
    void drawOverlay();
    void drawAutoParamOverlay();

    void setSelectionColor1(QColor color);
    void setSelectionColor2(QColor color);
    
    void updatePupilViews(const std::vector<QRect> &rects);
    void enablePupilView(bool value); // GB now for both pupils if needed

    void updateView(const cv::Mat &img);

    void setROI1SelectionR(float roiSize);
    void setROI1SelectionR(QRectF roi);
    void setROI2SelectionR(float roiSize);
    void setROI2SelectionR(QRectF roi);

    void clearProcessedOverlayMemory();

    void setDoubleROI(bool state); 
    bool getDoubleROI();
    void setROI1AllowedArea(int roiAllowedArea);
    void setROI2AllowedArea(int roiAllowedArea);

    void onShowROI(bool value);
    void onChangePupilDetectionUsingROI(bool state);
    void onShowPupilCenter(bool value);
    void onChangePupilColorFill(int colorFill);
    void onChangePupilColorFillThreshold(float value);
    void onChangeShowAutoParamOverlay(bool state);

    void setAutoParamPupSize(int value);

    void showROISelection(bool value); // GB: now for both pupils if needed

    bool saveROI1Selection();
    bool saveROI2Selection();

    void discardROISelection();

    void fitView();
    void showFullView();
    void zoomInView();
    void zoomOutView();

    void onROI1Change();
    void onROI2Change();
    // GB added end

signals:

    // GB NOTE: these emit a QRectF, which has its x, y, width, height expressed as pixels of image width and height
    void onROI1SelectionD(QRectF roiD);
    void onROI2SelectionD(QRectF roiD);

    // GB NOTE: these emit a QRectF, which has its x, y, width, height expressed as ratios of image width and height, expressed as floats between 0.0 and 1.0
    void onROI1SelectionR(QRectF roiR);
    void onROI2SelectionR(QRectF roiR);

};

#endif //PUPILEXT_VIDEOVIEW_H
