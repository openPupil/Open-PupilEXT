#pragma once

/**
    @author Gábor Bényei
*/

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPen>
#include <QLine>

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QSettings>
#include <QCoreApplication>
#include <QString>
#include <QDebug>

/**

    This header is for a custom widget that can be used in camera settings dialogs to position image ROI(s)
    (and possibly to check pupil detection ROIs too, as an enhancement in future)

*/

class CamImageRegionsWidget : public QFrame {
Q_OBJECT

public:
    CamImageRegionsWidget(QWidget *parent = nullptr);
    ~CamImageRegionsWidget(void) override;

    void setImageMaxSize(QSize size);
    void setImageAcqROI1Rect(QRect rect);
    void setImageAcqROI2Rect(QRect rect);
    void setPupilDetectionROI1Rect(QRect rect);
    void setPupilDetectionROI2Rect(QRect rect);
    void setUsingMultipleROI(bool state);

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    
    QSettings *applicationSettings;
    
    void updatePenColors();

    // TODO option 1: 
    // use setScale and translate using Qt things, not QRect and QLine separately but any QGraphicsItem
    // TODO option 2: 
    // discard most of this code and use a QGraphicsScene that is automatically fitted in a view that fills the widget
    QRect toDrawingArea(QRect rect);
    QLine toDrawingArea(QLine line);

    QPen penWidgetBorder;
    QPen penImageAcqROI1;
    QPen penImageAcqROI2;
    QPen penPupilDetectionROI1;
    QPen penPupilDetectionROI2;
    QPen penImageAcqROISplitLine;

    bool usingMultipleROI = false;

    QRect drawingArea;

    QSize imageMaxSize;

    QRect imageAcqROI1Rect;
    QRect imageAcqROI2Rect;
    QRect pupilDetectionROI1Rect;
    QRect pupilDetectionROI2Rect;

};

