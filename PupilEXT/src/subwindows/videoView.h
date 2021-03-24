
#ifndef PUPILEXT_VIDEOVIEW_H
#define PUPILEXT_VIDEOVIEW_H

/**
    @author Moritz Lode
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

enum ViewMode {FIT = 0, FULL = 1, ZOOM = 2};

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

    explicit VideoView(QWidget *parent=0);

    QSize sizeHint() const override;

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
    QGraphicsView *roiGraphicsView;
    QGraphicsScene *graphicsScene;

    ImageGraphicsItem *currentImage;

    ResizableRectItem *roiSelection;
    QRect ROI;

    bool initialFit;
    cv::Size imageSize;
    double scaleFactor;

    int mode;

protected:

    void resizeEvent(QResizeEvent *event) override;

public slots:

    void updateView(const cv::Mat &img);

    void updatePupilView(const QRect &rect);
    void enablePupilView(bool value);

    void setROISelection(float roiSize);
    void setROISelection(QRectF roi);

    void showROISelection(bool value);

    bool saveROISelection();
    void discardROISelection();

    void fitView();
    void showFullView();
    void zoomInView();
    void zoomOutView();

    void onROIChange();

signals:

    void onROISelection(QRectF roi);

};

#endif //PUPILEXT_VIDEOVIEW_H
