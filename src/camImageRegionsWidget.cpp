#include "camImageRegionsWidget.h"
#include "supportFunctions.h"

CamImageRegionsWidget::CamImageRegionsWidget(QWidget *parent) : 
    QFrame(parent),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {
    
    QHBoxLayout *vlayout = new QHBoxLayout;

    setLayout(vlayout);
    setFrameStyle(2);

    
    penWidgetBorder.setStyle(Qt::DashLine);
    penImageAcqROI1.setStyle(Qt::DashLine);
    penImageAcqROI2.setStyle(Qt::DashLine);
    penPupilDetectionROI1.setStyle(Qt::SolidLine);
    penPupilDetectionROI2.setStyle(Qt::SolidLine);
    penImageAcqROISplitLine.setStyle(Qt::DashLine);
    updatePenColors();

    drawingArea = QRect(0, 0, size().width(), size().height());
}

CamImageRegionsWidget::~CamImageRegionsWidget(void) {

}

void CamImageRegionsWidget::updatePenColors() {

    // TODO: move these to a separate function as the same lines are used in SVGIconColorAdjuster class too !!
    // -- begin 
    // NOTE: These 4 lines below are from a SO post: https://stackoverflow.com/a/21024983
    QLabel label("something");
    int text_hsv_value = label.palette().color(QPalette::WindowText).value();
    int bg_hsv_value = label.palette().color(QPalette::Window).value();
    // bool dark_theme_found = text_hsv_value > bg_hsv_value;
    //
    // basically the icon set we use is for light theme, so we only need to decide whether to lighten colors or not
    // GUIDarkAdaptMode: 0 = no, 1 = yes, 2 = let PupilEXT guess
    bool doLighten = applicationSettings->value("GUIDarkAdaptMode", "0") == "1" || (applicationSettings->value("GUIDarkMode", "0") == "2" && text_hsv_value > bg_hsv_value);
    // -- end

    penWidgetBorder.setColor(SupportFunctions::changeColors(Qt::black, doLighten, isEnabled()));

    penImageAcqROI1.setColor(SupportFunctions::changeColors(Qt::darkRed, doLighten, isEnabled()));
    penImageAcqROI2.setColor(SupportFunctions::changeColors(Qt::darkRed, doLighten, isEnabled()));

    penPupilDetectionROI1.setColor(SupportFunctions::changeColors(Qt::blue, doLighten, isEnabled()));
    penPupilDetectionROI2.setColor(SupportFunctions::changeColors(Qt::green, doLighten, isEnabled()));
    //penPupilDetectionROI1.setColor(SupportFunctions::changeColors(Qt::cyan, doLighten, isEnabled()));
    //penPupilDetectionROI2.setColor(SupportFunctions::changeColors(Qt::yellow, doLighten, isEnabled()));

    penImageAcqROISplitLine.setColor(SupportFunctions::changeColors(Qt::black, doLighten, isEnabled()));
}

void CamImageRegionsWidget::setImageMaxSize(QSize size) {
    imageMaxSize = size;
    update();
}
void CamImageRegionsWidget::setImageAcqROI1Rect(QRect rect) {
    imageAcqROI1Rect = rect;
    update();
}
void CamImageRegionsWidget::setImageAcqROI2Rect(QRect rect) {
    imageAcqROI2Rect = rect;
    update();
}
void CamImageRegionsWidget::setPupilDetectionROI1Rect(QRect rect) {
    pupilDetectionROI1Rect = rect;
    update();
}
void CamImageRegionsWidget::setPupilDetectionROI2Rect(QRect rect) {
    pupilDetectionROI2Rect = rect;
    update();
}
void CamImageRegionsWidget::setUsingMultipleROI(bool state) {
    usingMultipleROI = state;
    update();
}

QRect CamImageRegionsWidget::toDrawingArea(QRect rect) {
    return QRect(
        (float)rect.x()/imageMaxSize.width() *drawingArea.width() + drawingArea.x(), 
        (float)rect.y()/imageMaxSize.height() *drawingArea.height() + drawingArea.y(), 
        (float)rect.width()/imageMaxSize.width() *drawingArea.width(), 
        (float)rect.height()/imageMaxSize.height() *drawingArea.height()
        );
}
QLine CamImageRegionsWidget::toDrawingArea(QLine line) {
    return QLine(
        (float)line.x1()/imageMaxSize.width() *drawingArea.width() + drawingArea.x(), 
        (float)line.y1()/imageMaxSize.height() *drawingArea.height() + drawingArea.y(), 
        (float)line.x2()/imageMaxSize.width() *drawingArea.width() + drawingArea.x(), 
        (float)line.y2()/imageMaxSize.height() *drawingArea.height() + drawingArea.y()
        );
}


void CamImageRegionsWidget::resizeEvent(QResizeEvent *event) {
    // calculate aspect ratios for "fit in" view of the sensor size (image max size) in the widget
    float widgetAR = (float)size().width()/size().height();
    float sensorAR = (float)imageMaxSize.width()/imageMaxSize.height();

    // -1 is needed because we cannot draw at the rightmost and bottom pixel lines of the widget sometimes
    if(widgetAR >= sensorAR) {
        // up and bottom edges of drawingArea touch the border of the widget
        // shrink the width of drawingArea AND center horizontally
        drawingArea = QRect(
            0 + (float)event->size().width()/2 - ((float)event->size().height() * sensorAR /2), 
            0, 
            (float)event->size().height() * sensorAR -1, 
            event->size().height() -1 );
    } else {
        // left and right edges of drawingArea touch the border of the widget
        // shrink the height of drawingArea AND center vertically
        drawingArea = QRect(
            0, 
            0 + (float)event->size().height()/2 - ((float)event->size().width() / sensorAR /2), 
            event->size().width() -1, 
            (float)event->size().width() / sensorAR -1 );
    }
}

void CamImageRegionsWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    updatePenColors();


    painter.setPen(penWidgetBorder);
    painter.drawRect(toDrawingArea(QRect(0,0,imageMaxSize.width(),imageMaxSize.height())));

    if(!imageAcqROI1Rect.isEmpty()) {
        painter.setPen(penImageAcqROI1);
        painter.drawRect(toDrawingArea(imageAcqROI1Rect));
    }

    if(!imageAcqROI2Rect.isEmpty()) {
        painter.setPen(penImageAcqROI2);
        painter.drawRect(toDrawingArea(imageAcqROI2Rect));
    }

    if(!pupilDetectionROI1Rect.isEmpty()) {
        painter.setPen(penPupilDetectionROI1);
        painter.drawRect(toDrawingArea(pupilDetectionROI1Rect));
    }

    if(!pupilDetectionROI2Rect.isEmpty()) {
        painter.setPen(penPupilDetectionROI2);
        painter.drawRect(toDrawingArea(pupilDetectionROI2Rect));
    }

    if(usingMultipleROI) {
        painter.setPen(penImageAcqROISplitLine);
        painter.drawLine(toDrawingArea(QLine((float)imageMaxSize.width()/2,0,(float)imageMaxSize.width()/2,(float)imageMaxSize.height())));
    }

}

