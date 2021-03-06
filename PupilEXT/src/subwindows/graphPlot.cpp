
#include <iostream>
#include "graphPlot.h"
#include "dataTable.h"

uint64 GraphPlot::sharedTimestamp = 0;

// Create a graph plot window showing the given plotvalue in real-time
// QCustomPlot library is used for plotting
GraphPlot::GraphPlot(QString plotValue, bool stereoMode, bool legend, QWidget *parent) : QWidget(parent), customPlot(new QCustomPlot(parent)), plotValue(plotValue), incrementedTimestamp(0), interaction(false) {

    setWindowTitle(plotValue);

    // While this works, the scaling of the plot inside the window is wrong, unclear how to fix this
    // TODO any chance to get the opengl qcustomplot scaling to work?
    //customPlot->setOpenGl(true);

    updateDelay = 33; // 30fps

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(customPlot);

    setLayout(layout);

    customPlot->addGraph();

    if(stereoMode) {
        customPlot->addGraph();

        QPen penSec(Qt::green, 0, Qt::SolidLine);
        customPlot->graph(1)->setPen(penSec);
    }

    customPlot->legend->setVisible(legend);

    QPen pen(Qt::blue, 0, Qt::SolidLine);
    customPlot->graph(0)->setPen(pen);

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    customPlot->xAxis->setTicker(timeTicker);
    customPlot->axisRect()->setupFullAxesBox();

    setupPlotAxis();

    // Make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // According to qcustomplot docs, shouldnt be enabled https://www.qcustomplot.com/documentation/performanceimprovement.html
    //setRenderHint(QPainter::Antialiasing);

    customPlot->show();
    timer.start();
}

GraphPlot::~GraphPlot() {

    delete customPlot;
}

QSize GraphPlot::sizeHint() const {
    return QSize(640, 180);
}

// Resets the graph, removes all data and resets timestamp (x-axis)
void GraphPlot::reset() {

    incrementedTimestamp = 0;
    customPlot->graph(0)->data()->clear();
}

// On right click on the plot a context menu is created at the position of the click
void GraphPlot::contextMenuRequest(QPoint pos) {

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction("Clear", this, SLOT(reset()));
    QAction *interactionAct = menu->addAction("Interaction", this, SLOT(enableInteractions()));
    interactionAct->setCheckable(true);
    interactionAct->setChecked(interaction);

    QAction *yinteractionAct = menu->addAction("Scale Y-Axis", this, SLOT(enableYAxisInteraction()));
    yinteractionAct->setCheckable(true);
    yinteractionAct->setChecked(yinteraction);

    menu->popup(customPlot->mapToGlobal(pos));
}

// Enables the interactive plot on all axes
// Enables dragging and zooming on both axis
void GraphPlot::enableInteractions() {
    interaction = !interaction;
    if(interaction) {
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    } else {
        customPlot->setInteractions(QFlags<QCP::Interaction>());
    }
}

// Enables the interactive plot only on the Y axis
// Enables dragging and zooming on the A axis
// The X axis still scales and moves automatically
void GraphPlot::enableYAxisInteraction() {
    yinteraction = !yinteraction;
    if(yinteraction) {
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        customPlot->axisRect(0)->setRangeDrag(Qt::Vertical);
    } else {
        customPlot->setInteractions(QFlags<QCP::Interaction>());
        customPlot->axisRect(0)->setRangeDrag(QFlags<Qt::Orientation>());
    }
}

// Sets Y axis label of the graph plot according to the current plot value
void GraphPlot::setupPlotAxis() {

    if(plotValue == DataTable::FRAME_NUMBER) {
        customPlot->yAxis->setLabel("frame number");
    } else if(plotValue == DataTable::CAMERA_FPS) {
        customPlot->yAxis->setLabel("[fps]");
    } else if(plotValue == DataTable::PUPIL_FPS) {
        customPlot->yAxis->setLabel("[fps]");
    } else if(plotValue == DataTable::PUPIL_CENTER_X) {
        customPlot->yAxis->setLabel("pupil center [px]");
    } else if(plotValue == DataTable::PUPIL_CENTER_Y) {
        customPlot->yAxis->setLabel("pupil center [px]");
    } else if(plotValue == DataTable::PUPIL_MAJOR) {
        customPlot->yAxis->setLabel("pupil major axis [px]");
    } else if(plotValue == DataTable::PUPIL_MINOR) {
        customPlot->yAxis->setLabel("pupil minor axis [px]");
    } else if(plotValue == DataTable::PUPIL_WIDTH) {
        customPlot->yAxis->setLabel("pupil width [px]");
    } else if(plotValue == DataTable::PUPIL_HEIGHT) {
        customPlot->yAxis->setLabel("pupil height [px]");
    } else if(plotValue == DataTable::PUPIL_CONFIDENCE) {
        customPlot->yAxis->setLabel("pupil confidence");
        customPlot->yAxis->setRange(-0.2, 1.2);
    } else if(plotValue == DataTable::PUPIL_OUTLINE_CONFIDENCE) {
        customPlot->yAxis->setLabel("pupil outline confidence");
        customPlot->yAxis->setRange(-0.2, 1.2);
    } else if(plotValue == DataTable::PUPIL_CIRCUMFERENCE) {
        customPlot->yAxis->setLabel("pupil circumference [px]");
    } else if(plotValue == DataTable::PUPIL_RATIO) {
        customPlot->yAxis->setLabel("pupil axis ratio");
    } else if(plotValue == DataTable::PUPIL_DIAMETER) {
        customPlot->yAxis->setLabel("pupil diameter [px]");
    } else if(plotValue == DataTable::PUPIL_UNDIST_DIAMETER) {
        customPlot->yAxis->setLabel("pupil undistorted diameter [px]");
    } else if(plotValue == DataTable::PUPIL_PHYSICAL_DIAMETER) {
        customPlot->yAxis->setLabel("pupil physical diameter [mm]");
    }
}

// Slot that is called upon receiving framecounter signals
// Updates the table columns with current camera fps
// This is called for framecounter classes one time per second
void GraphPlot::appendData(const double &fps) {

    incrementedTimestamp += 1000;
    uint64 m_timestamp = incrementedTimestamp;

    // add data
    if(plotValue == DataTable::CAMERA_FPS || plotValue == DataTable::PUPIL_FPS) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, fps);
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(!interaction) {
            if(!yinteraction && plotValue != DataTable::PUPIL_CONFIDENCE && plotValue != DataTable::PUPIL_OUTLINE_CONFIDENCE) {
                // rescale value (vertical) axis to fit the current data:
                customPlot->graph(0)->rescaleValueAxis(false, true);
            }
            // make key axis range scroll with the data (at a constant range size of 8):
            customPlot->xAxis->setRange(m_timestamp/1000.0, 15, Qt::AlignRight);
        }

        // if the first data is older than 4 minutes, remove 2 minutes of worth
        if((m_timestamp/1000.0) - customPlot->graph()->dataMainKey (0) > 240) {
            customPlot->graph(0)->data()->removeBefore((m_timestamp/1000.0)-120);
        }

        customPlot->replot();
    }
}

// Slot that is called upon receiving framecount signals
// Updates the table columns with current camera framecount
// This is called for framecounter classes one time per second
void GraphPlot::appendData(const int &framecount) {

    incrementedTimestamp += 1000;
    uint64 m_timestamp = incrementedTimestamp;

    // add data
    if(plotValue == DataTable::FRAME_NUMBER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, framecount);
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(!interaction) {
            if(!yinteraction && plotValue != DataTable::PUPIL_CONFIDENCE && plotValue != DataTable::PUPIL_OUTLINE_CONFIDENCE) {
                // rescale value (vertical) axis to fit the current data:
                customPlot->graph(0)->rescaleValueAxis(false, true);
            }
            // make key axis range scroll with the data (at a constant range size of 8):
            customPlot->xAxis->setRange(m_timestamp/1000.0, 15, Qt::AlignRight);
        }

        // if the first data is older than 4 minutes, remove 2 minutes of worth
        if((m_timestamp/1000.0) - customPlot->graph()->dataMainKey (0) > 240) {
            customPlot->graph(0)->data()->removeBefore((m_timestamp/1000.0)-120);
        }

        customPlot->replot();
    }
}

// Slot that is called upon receiving a new pupil detection
// Updates the table columns with current pupil data i.e. all meta information of the pupil
// This is called from the pupil detection process, potentially 120 times per second, however only data is appended in that rate
// Plot replots are executed in rates defined by updateDelay i.e. 30 fps
void GraphPlot::appendData(quint64 timestamp, const Pupil &pupil, const QString &filename) {

    if(sharedTimestamp==0)
        sharedTimestamp = timestamp;
    uint64 m_timestamp = timestamp - sharedTimestamp;

    // add data
    if(plotValue == DataTable::PUPIL_CENTER_X) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.center.x);
    } else if(plotValue == DataTable::PUPIL_CENTER_Y) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.center.y);
    } else if(plotValue == DataTable::PUPIL_MAJOR) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.majorAxis());
    } else if(plotValue == DataTable::PUPIL_MINOR) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.minorAxis());
    } else if(plotValue == DataTable::PUPIL_WIDTH) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.width());
    } else if(plotValue == DataTable::PUPIL_HEIGHT) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.height());
    } else if(plotValue == DataTable::PUPIL_CONFIDENCE) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.confidence);
    } else if(plotValue == DataTable::PUPIL_OUTLINE_CONFIDENCE) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.outline_confidence);
    } else if(plotValue == DataTable::PUPIL_CIRCUMFERENCE) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.circumference());
    } else if(plotValue == DataTable::PUPIL_RATIO) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, (double)pupil.majorAxis() / pupil.minorAxis());
    } else if(plotValue == DataTable::PUPIL_DIAMETER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.diameter());
    } else if(plotValue == DataTable::PUPIL_UNDIST_DIAMETER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.undistortedDiameter);
    } else if(plotValue == DataTable::PUPIL_PHYSICAL_DIAMETER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.physicalDiameter);
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(!interaction) {
            if(!yinteraction && plotValue != DataTable::PUPIL_CONFIDENCE && plotValue != DataTable::PUPIL_OUTLINE_CONFIDENCE) {
                // rescale value (vertical) axis to fit the current data:
                customPlot->graph(0)->rescaleValueAxis(false, true);
            }
            // make key axis range scroll with the data (at a constant range size of 8):
            customPlot->xAxis->setRange(m_timestamp/1000.0, 15, Qt::AlignRight);
        }

        //std::cout<<customPlot->graph()->dataMainKey (0)/1000.0<<std::endl;


        // if the first data is older than 4 minutes, remove 2 minutes of worth
        if((m_timestamp/1000.0) - customPlot->graph()->dataMainKey (0) > 240) {
            customPlot->graph(0)->data()->removeBefore((m_timestamp/1000.0)-120);
        }

        customPlot->replot();
    }
}

// Slot that is called upon receiving a new stereo pupil detection
// Updates the table columns with current pupil data i.e. all meta information of both pupil detections
// This is called from the pupil detection process, potentially 120 times per second, however only data is appended in that rate
// Plot replots are executed in rates defined by updateDelay i.e. 30 fps
void GraphPlot::appendData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename) {

    if(sharedTimestamp==0)
        sharedTimestamp = timestamp;
    uint64 m_timestamp = timestamp - sharedTimestamp;

    // add data
    if(plotValue == DataTable::PUPIL_CENTER_X) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.center.x);
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.center.x);
    } else if(plotValue == DataTable::PUPIL_CENTER_Y) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.center.y);
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.center.y);
    } else if(plotValue == DataTable::PUPIL_MAJOR) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.majorAxis());
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.majorAxis());
    } else if(plotValue == DataTable::PUPIL_MINOR) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.minorAxis());
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.minorAxis());
    } else if(plotValue == DataTable::PUPIL_WIDTH) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.width());
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.width());
    } else if(plotValue == DataTable::PUPIL_HEIGHT) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.height());
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.height());
    } else if(plotValue == DataTable::PUPIL_CONFIDENCE) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.confidence);
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.confidence);
    } else if(plotValue == DataTable::PUPIL_OUTLINE_CONFIDENCE) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.outline_confidence);
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.outline_confidence);
    } else if(plotValue == DataTable::PUPIL_CIRCUMFERENCE) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.circumference());
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.circumference());
    } else if(plotValue == DataTable::PUPIL_RATIO) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, (double)pupil.majorAxis() / pupil.minorAxis());
        customPlot->graph(1)->addData(m_timestamp/1000.0, (double)pupilSec.majorAxis() / pupilSec.minorAxis());
    } else if(plotValue == DataTable::PUPIL_DIAMETER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.diameter());
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.diameter());
    } else if(plotValue == DataTable::PUPIL_UNDIST_DIAMETER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.undistortedDiameter);
        customPlot->graph(1)->addData(m_timestamp/1000.0, pupilSec.undistortedDiameter);
    } else if(plotValue == DataTable::PUPIL_PHYSICAL_DIAMETER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, pupil.physicalDiameter);
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(!interaction) {
            if(!yinteraction && plotValue != DataTable::PUPIL_CONFIDENCE && plotValue != DataTable::PUPIL_OUTLINE_CONFIDENCE) {
                // rescale value (vertical) axis to fit the current data:
                customPlot->graph(0)->rescaleValueAxis(false, true);
                customPlot->graph(1)->rescaleValueAxis(false, true);
            }
            // make key axis range scroll with the data (at a constant range size of 15secs):
            customPlot->xAxis->setRange(m_timestamp/1000.0, 15, Qt::AlignRight);
        }


        // if the first data is older than 4 minutes, remove 2 minutes of worth
        if((m_timestamp/1000.0) - customPlot->graph(0)->dataMainKey (0) > 240) {
            customPlot->graph(0)->data()->removeBefore((m_timestamp/1000.0)-120);
            customPlot->graph(1)->data()->removeBefore((m_timestamp/1000.0)-120);
        }

        customPlot->replot();
    }
}