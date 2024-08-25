
#include <QtWidgets/QHBoxLayout>
#include <QResizeEvent>
#include <ctime>
#include <opencv2/imgproc.hpp>
#include "sceneImageWidget.h"
#include "../supportFunctions.h"

// Creates a new live-view widget
SceneImageWidget::SceneImageWidget(QWidget *parent) :
        QWidget(parent),
        graphicsScene(new QGraphicsScene(this)),
        graphicsView(new QGraphicsView(this)),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    int msize = std::max(graphicsView->width(), graphicsView->height());

    bool darkMode = applicationSettings->value("GUIDarkAdaptMode", "0") == "1" || (applicationSettings->value("GUIDarkMode", "0") == "2");
    if(darkMode) {
        graphicsScene->setBackgroundBrush(QBrush("#242424"));
    } else {
        graphicsScene->setBackgroundBrush(QBrush("white"));
    }

////    penROIprocessed.setWidth(2);
////    selectionColorWrong2.setAlphaF( 0.3 );
//    penAutoParamAccent.setStyle(Qt::DashLine);
//    penAutoParamBack.setStyle(Qt::SolidLine);

    currentImage = new ImageGraphicsItem();
    graphicsScene->addItem(currentImage);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(graphicsView, 0, 0);

    setLayout(layout);

    graphicsView->setScene(graphicsScene);
    graphicsView->show();

}

void SceneImageWidget::resizeEvent(QResizeEvent *event) {

    graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);

}

QSize SceneImageWidget::sizeHint() const {
    return QSize(640, 480);
}

void SceneImageWidget::clearOverlayMemory() {
    pupilsForCalibPoints.clear();
}

// This method paints the "overlaying" ROI and pupil outline on the graphicsScene, over the image
void SceneImageWidget::updateView(const int &procMode, const std::vector<Pupil> &Pupils) {

//    pupilsForCalibPoints.clear();
//    for(std::size_t z=0; z<Pupils.size(); z++) {
//        pupilsForCalibPoints.push_back(Pupils[z]);
//    }
    pupilsForCalibPoints.push_back(Pupils);

    drawOverlay();
    // draw Image here again?
}

void SceneImageWidget::drawOverlay() {

    // GB NOTE: all ROIs that arrive from pupilDetection are yet in DISCRETE px dimensions, not 0-1 floats

    // to prevent memory leaks and lagging GUI
    for (std::size_t c = 0; c < geBufferCalibPoints.size(); c++) {
        graphicsScene->removeItem(geBufferCalibPoints[c]);
        delete geBufferCalibPoints[c];
    }
    if (geBufferCalibPoints.size() > 0)
        geBufferCalibPoints.clear();

    if (imageSize.width <= 0 || imageSize.height <= 0)
        return;

    float lineWidth = (float) imageSize.width / (float) graphicsView->width();
    penCalibPoints.setWidthF(lineWidth);

    for (std::size_t c = 0; c < pupilsForCalibPoints.size(); c++) {
        for (std::size_t z = 0; z < pupilsForCalibPoints[c].size(); z++) {

            // dummy test values
            int x = 100 * c + 20 * z; // pupilsForCalibPoints[c][z]. ...
            int y = 100 * c + 20 * z; // pupilsForCalibPoints[c][z]. ...

            QGraphicsLineItem *calibPointCrossHline = new QGraphicsLineItem(x-10, y, x+10, y);
            QGraphicsLineItem *calibPointCrossVline = new QGraphicsLineItem(x, y-10, x, y+10);
            calibPointCrossHline->setPen(penCalibPoints);
            calibPointCrossVline->setPen(penCalibPoints);
//            pupil->setBrush(QColor(255 - t * 255, t * 255, 0, 255)); // if filled shape

            calibPointCrossHline->setZValue(90);
            calibPointCrossVline->setZValue(90);
            graphicsScene->addItem(calibPointCrossHline);
            graphicsScene->addItem(calibPointCrossVline);
            geBufferCalibPoints.push_back(calibPointCrossHline);
            geBufferCalibPoints.push_back(calibPointCrossVline);

        }
    }
}

void SceneImageWidget::updateView(const cv::Mat &img) {

    if(imageSize != img.size()) {
        std::cerr << "arrived image has different size than expected" << std::endl;
        return;
    }

    if(img.empty())
        return;

//    // For some reason, qt crashes when trying to paint a grayscale image over qpainter
//    cv::Mat bgrFrame = img;
//    if (img.channels() == 1)
//        cv::cvtColor(img, bgrFrame, cv::COLOR_GRAY2BGR);
//    QImage qImg = SupportFunctions::cvMatToQImage(bgrFrame);

    QImage qImg = SupportFunctions::cvMatToQImage(img);


    graphicsScene->removeItem(currentImage);
    delete currentImage;
    currentImage = new ImageGraphicsItem(qImg);
    graphicsScene->addItem(currentImage);

    graphicsScene->setSceneRect(0, 0, img.cols, img.rows);
    graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
}

//void SceneImageWidget::fitView() {
//    graphicsView->fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
//}
