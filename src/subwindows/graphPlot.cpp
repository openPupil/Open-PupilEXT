
#include <iostream>
#include "graphPlot.h"
#include "dataTable.h"
#include "./../SVGIconColorAdjuster.h"

uint64 GraphPlot::sharedTimestamp = 0;

// Create a graph plot window showing the given plotvalue in real-time
// QCustomPlot library is used for plotting
GraphPlot::GraphPlot(DataTypes::DataType _plotDataKey, ProcMode procMode, bool legend, QWidget *parent) :
        QWidget(parent),
        customPlot(new QCustomPlot(parent)),
        plotDataKey(_plotDataKey),
        currentProcMode(procMode),
        incrementedTimestamp(0),
//    interaction(false),
//    yinteraction(true),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    setWindowTitle(DataTypes::map.value(plotDataKey));
    setMinimumSize(440, 210);

    // While this works, the scaling of the plot inside the window is wrong, unclear how to fix this
    // TODO any chance to get the opengl qcustomplot scaling to work?
    //customPlot->setOpenGl(true);

    updateDelay = 33; // 30fps

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(customPlot);

    setLayout(layout);

    loadYaxisSettings();

    // GB begin
    /*
    if(stereoMode) {
        customPlot->addGraph();

        QPen penSec(Qt::green, 0, Qt::SolidLine);
        customPlot->graph(1)->setPen(penSec);
    }
    */
    QPen pen1 = QPen(Qt::blue, 0, Qt::SolidLine);
    QPen pen2 = QPen(Qt::green, 0, Qt::SolidLine);
    QPen pen3 = QPen(Qt::cyan, 0, Qt::SolidLine);
    QPen pen4 = QPen(Qt::yellow, 0, Qt::SolidLine);
    std::size_t numCols=1;
    switch(currentProcMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            customPlot->addGraph();
            customPlot->graph(0)->setPen(pen1);
            //numCols=1;
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            customPlot->addGraph();
            customPlot->addGraph();
            customPlot->graph(0)->setPen(pen1);
            customPlot->graph(1)->setPen(pen2);
            numCols=2;
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            customPlot->addGraph();
            customPlot->addGraph();
            customPlot->addGraph();
            customPlot->addGraph();
            customPlot->graph(0)->setPen(pen1);
            customPlot->graph(1)->setPen(pen2);
            customPlot->graph(2)->setPen(pen3);
            customPlot->graph(3)->setPen(pen4);
            numCols=4;
            break;
    }

//    enableInteractions();
//    enableYAxisInteraction();

    bool darkMode = applicationSettings->value("GUIDarkAdaptMode", "0") == "1" || (applicationSettings->value("GUIDarkMode", "0") == "2");
    if(darkMode) {
        customPlot->setBackground(QBrush("#242424"));
    } else {
        customPlot->setBackground(QBrush("white"));
    }

    customPlot->legend->setVisible(legend);

//    QPen pen(Qt::blue, 0, Qt::SolidLine);
//    customPlot->graph(0)->setPen(pen);

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
    return QSize(640, 210);
}

// Resets the graph, removes all data and resets timestamp (x-axis)
void GraphPlot::reset() {

    lastTimestamp = 0;

    incrementedTimestamp = 0;
//    customPlot->graph(0)->data()->clear();
    switch(currentProcMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            customPlot->graph(0)->data()->clear();
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            customPlot->graph(0)->data()->clear();
            customPlot->graph(1)->data()->clear();
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            customPlot->graph(0)->data()->clear();
            customPlot->graph(1)->data()->clear();
            customPlot->graph(2)->data()->clear();
            customPlot->graph(3)->data()->clear();
            break;
    }
    customPlot->update();

    // GB TODO: IMPORTANT TODO: clean all plots
}

// On right click on the plot a context menu is created at the position of the click
void GraphPlot::contextMenuRequest(QPoint pos) {

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction("Clear", this, SLOT(reset()));
    menu->addSeparator();

    QAction *autoScaleAct = menu->addAction("Auto scroll X, auto scale Y");
    autoScaleAct->setCheckable(true);
    autoScaleAct->setChecked(currentInteractionMode == AUTO_SCROLL_X_AUTO_SCALE_Y);
    // Note: the line below must happen here, not to trigger a toggled event with setChecked
    connect(autoScaleAct, &QAction::toggled, [this]() { setInteractionMode(AUTO_SCROLL_X_AUTO_SCALE_Y); });

    QAction *yinteractionAct = menu->addAction("Auto scroll X, manual scale Y");
    yinteractionAct->setCheckable(true);
    yinteractionAct->setChecked(currentInteractionMode == AUTO_SCROLL_X_MANUAL_SCALE_Y);
    // Note: the line below must happen here, not to trigger a toggled event with setChecked
    connect(yinteractionAct, &QAction::toggled, [this]() { setInteractionMode(AUTO_SCROLL_X_MANUAL_SCALE_Y); });

    QAction *interactionAct = menu->addAction("Manual scale and scroll X and Y");
    interactionAct->setCheckable(true);
    interactionAct->setChecked(currentInteractionMode == MANUAL_SCALE_SCROLL_X_Y);
    // Note: the line below must happen here, not to trigger a toggled event with setChecked
    connect(interactionAct, &QAction::toggled, [this]() { setInteractionMode(MANUAL_SCALE_SCROLL_X_Y); });

    QAction *i4Act = menu->addAction("Auto scroll X, fixed scale Y");
    i4Act->setCheckable(true);
    i4Act->setChecked(currentInteractionMode == AUTO_SCROLL_X_FIXED_SCALE_Y);
    // Note: the line below must happen here, not to trigger a toggled event with setChecked
    connect(i4Act, &QAction::toggled, [this]() { setInteractionMode(AUTO_SCROLL_X_FIXED_SCALE_Y); });

    menu->popup(customPlot->mapToGlobal(pos));
}

void GraphPlot::setInteractionMode(InteractionMode m) {
    if(currentInteractionMode != AUTO_SCROLL_X_AUTO_SCALE_Y) {
        yAxisLimitLow = customPlot->yAxis->range().lower;
        yAxisLimitHigh = customPlot->yAxis->range().upper;
    }

    // ITT nem currentinteractuionmode kéne hanem m!!! most valamiért csak akkor működik a manual scale scroll x y ha előtte épp az auto scroll x manual scale y volt beállitva, lehet emiatt

    if(m == AUTO_SCROLL_X_AUTO_SCALE_Y) {
        customPlot->setInteractions(QFlags<QCP::Interaction>());
        customPlot->axisRect(0)->setRangeDrag(QFlags<Qt::Orientation>()); // csak yinteractionnél volt vmiért
    } else if(m == AUTO_SCROLL_X_MANUAL_SCALE_Y) {
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        customPlot->axisRect(0)->setRangeDrag(Qt::Vertical);
    } else if(m == MANUAL_SCALE_SCROLL_X_Y) {
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    } else if(m == AUTO_SCROLL_X_FIXED_SCALE_Y) {
//        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
        customPlot->setInteractions(QFlags<QCP::Interaction>());
        customPlot->yAxis->setRange(yAxisLimitLow, yAxisLimitHigh);
//        customPlot->yAxis->
    }

    currentInteractionMode = m;
    saveYaxisSettings();
}

//// Enables the interactive plot on all axes
//// Enables dragging and zooming on both axis
//void GraphPlot::enableInteractions() {
//    interaction = !interaction;
//    if(interaction) {
//        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
//    } else {
//        customPlot->setInteractions(QFlags<QCP::Interaction>());
//    }
//}

//// Enables the interactive plot only on the Y axis
//// Enables dragging and zooming on the A axis
//// The X axis still scales and moves automatically
//void GraphPlot::enableYAxisInteraction() {
//    yinteraction = !yinteraction;
//    if(yinteraction) {
//        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//        customPlot->axisRect(0)->setRangeDrag(Qt::Vertical);
//    } else {
//        customPlot->setInteractions(QFlags<QCP::Interaction>());
//        customPlot->axisRect(0)->setRangeDrag(QFlags<Qt::Orientation>());
//    }
//}

void GraphPlot::loadYaxisSettings() {
    yAxisLimitLow = applicationSettings->value("GraphPlot_" + DataTypes::map.value(plotDataKey).simplified().toLower() + "_yLow", QString::number(yAxisLimitLowT)).toDouble();
    yAxisLimitHigh = applicationSettings->value("GraphPlot_" + DataTypes::map.value(plotDataKey).simplified().toLower() + "_yHigh", QString::number(yAxisLimitHighT)).toDouble();
    currentInteractionMode = (InteractionMode)applicationSettings->value("GraphPlot_" + DataTypes::map.value(plotDataKey).simplified().toLower() + "_interactionMode", QString::number((int)AUTO_SCROLL_X_AUTO_SCALE_Y)).toInt();
}

void GraphPlot::saveYaxisSettings() {
    yAxisLimitLow = customPlot->yAxis->range().lower;
    yAxisLimitHigh = customPlot->yAxis->range().upper;
    applicationSettings->setValue("GraphPlot_" + DataTypes::map.value(plotDataKey).simplified().toLower() + "_yLow", yAxisLimitLow);
    applicationSettings->setValue("GraphPlot_" + DataTypes::map.value(plotDataKey).simplified().toLower() + "_yHigh", yAxisLimitHigh);
    applicationSettings->setValue("GraphPlot_" + DataTypes::map.value(plotDataKey).simplified().toLower() + "_interactionMode", currentInteractionMode);
}

// Sets Y axis label of the graph plot according to the current plot value
// GB: capitalized first letter for fancy look
// TODO: update for the data types map use
void GraphPlot::setupPlotAxis() {

    if(plotDataKey == DataTypes::DataType::CAMERA_FPS) {
        customPlot->yAxis->setLabel("Camera/Image read FPS");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 200.0;
        spinBoxStep = 10.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_FPS) {
        customPlot->yAxis->setLabel("Processing FPS");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 200.0;
        spinBoxStep = 10.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_CENTER_X) {
        customPlot->yAxis->setLabel("Pupil center [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 2040.0;
        spinBoxStep = 10.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_CENTER_Y) {
        customPlot->yAxis->setLabel("Pupil center [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 2040.0;
        spinBoxStep = 10.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_MAJOR) {
        customPlot->yAxis->setLabel("Pupil major axis [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500.0;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_MINOR) {
        customPlot->yAxis->setLabel("Pupil minor axis [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500.0;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_WIDTH) {
        customPlot->yAxis->setLabel("Pupil width [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500.0;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_HEIGHT) {
        customPlot->yAxis->setLabel("Pupil height [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500.0;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_CONFIDENCE) {
        customPlot->yAxis->setLabel("Pupil confidence");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500;
        spinBoxStep = 0.1;
        customPlot->yAxis->setRange(-0.2, 1.2);
    } else if(plotDataKey == DataTypes::DataType::PUPIL_OUTLINE_CONFIDENCE) {
        customPlot->yAxis->setLabel("Pupil outline confidence");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 1.0;
        spinBoxStep = 0.1;
        customPlot->yAxis->setRange(-0.2, 1.2);
    } else if(plotDataKey == DataTypes::DataType::PUPIL_CIRCUMFERENCE) {
        customPlot->yAxis->setLabel("Pupil circumference [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_RATIO) {
        customPlot->yAxis->setLabel("Pupil axis ratio");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 1.0;
        spinBoxStep = 0.1;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_DIAMETER) {
        customPlot->yAxis->setLabel("Pupil diameter [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_UNDIST_DIAMETER) {
        customPlot->yAxis->setLabel("Pupil undistorted diameter [px]");
        yAxisLimitLowT = 0.0;
        yAxisLimitHighT = 500;
        spinBoxStep = 1.0;
    } else if(plotDataKey == DataTypes::DataType::PUPIL_PHYSICAL_DIAMETER) {
        customPlot->yAxis->setLabel("Pupil physical diameter [mm]");
        yAxisLimitLowT = 1.0;
        yAxisLimitHighT = 9.0;
        spinBoxStep = 0.1;
    } else {
        yAxisLimitLowT = -2000.0;
        yAxisLimitHighT = -65000.0;
        spinBoxStep = 0.1;
    }

    updateYaxisRange();
}

// Slot that is called upon receiving framecounter signals
// Updates the table columns with current camera fps
// This is called for framecounter classes one time per second
void GraphPlot::appendData(const double &fps) {

    incrementedTimestamp += 1000;
    uint64 m_timestamp = incrementedTimestamp;

    // Note: we currently only plot it for the main camera main view, as all their FPS's are equal
    if(plotDataKey == DataTypes::DataType::CAMERA_FPS || plotDataKey == DataTypes::DataType::PUPIL_FPS) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, fps);
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(currentInteractionMode != InteractionMode::MANUAL_SCALE_SCROLL_X_Y) {
            if((currentInteractionMode != InteractionMode::AUTO_SCROLL_X_MANUAL_SCALE_Y) &&
               (currentInteractionMode != InteractionMode::AUTO_SCROLL_X_FIXED_SCALE_Y) &&
               plotDataKey != DataTypes::DataType::PUPIL_CONFIDENCE && plotDataKey != DataTypes::DataType::PUPIL_OUTLINE_CONFIDENCE) {

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
    if(plotDataKey == DataTypes::DataType::FRAME_NUMBER) {
        customPlot->graph(0)->addData(m_timestamp/1000.0, framecount);
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(currentInteractionMode != InteractionMode::MANUAL_SCALE_SCROLL_X_Y) {
            if((currentInteractionMode != InteractionMode::AUTO_SCROLL_X_MANUAL_SCALE_Y) &&
               (currentInteractionMode != InteractionMode::AUTO_SCROLL_X_FIXED_SCALE_Y) &&
               plotDataKey != DataTypes::DataType::PUPIL_CONFIDENCE && plotDataKey != DataTypes::DataType::PUPIL_OUTLINE_CONFIDENCE) {

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


// Slot that is called upon receiving a new stereo pupil detection
// Updates the table columns with current pupil data i.e. all meta information of both pupil detections
// This is called from the pupil detection process, potentially 120 times per second, however only data is appended in that rate
// Plot replots are executed in rates defined by updateDelay i.e. 30 fps
// GB: made it work with vector of pupils for different Proc modes
void GraphPlot::appendData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename) {

    // TODO: something better?
    if(currentProcMode != procMode) {
        qDebug() << "Processing mode has changed. Cannot plot data this way";
        return;
    }

    if(sharedTimestamp==0)
        sharedTimestamp = timestamp;
    uint64 m_timestamp = timestamp - sharedTimestamp;

    // If the looping playback has restarted in case of fileCamera
    if(lastTimestamp != 0 && lastTimestamp > m_timestamp)
        reset();

    lastTimestamp = m_timestamp;

    std::size_t numCols=1;
    switch(currentProcMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            //numCols=1;
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            numCols=2;
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            numCols=4;
            break;
    }

    // add data
    // GB: TODO: physical diameter is the same for two views, but is added as two different curves now..
    for(int i=0; i<numCols; i++) {
        if(plotDataKey == DataTypes::DataType::PUPIL_CENTER_X) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].center.x);
        } else if(plotDataKey == DataTypes::DataType::PUPIL_CENTER_Y) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].center.y);
        } else if(plotDataKey == DataTypes::DataType::PUPIL_MAJOR) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].majorAxis());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_MINOR) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].minorAxis());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_WIDTH) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].width());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_HEIGHT) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].height());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_CONFIDENCE) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].confidence);
        } else if(plotDataKey == DataTypes::DataType::PUPIL_OUTLINE_CONFIDENCE) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].outline_confidence);
        } else if(plotDataKey == DataTypes::DataType::PUPIL_CIRCUMFERENCE) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].circumference());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_RATIO) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, (double)Pupils[i].majorAxis() / Pupils[i].minorAxis());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_DIAMETER) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].diameter());
        } else if(plotDataKey == DataTypes::DataType::PUPIL_UNDIST_DIAMETER) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].undistortedDiameter);
        } else if(plotDataKey == DataTypes::DataType::PUPIL_PHYSICAL_DIAMETER) {
            customPlot->graph(i)->addData(m_timestamp/1000.0, Pupils[i].physicalDiameter);
        }
    }

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        if(currentInteractionMode != InteractionMode::MANUAL_SCALE_SCROLL_X_Y) {
            
            if((currentInteractionMode != InteractionMode::AUTO_SCROLL_X_MANUAL_SCALE_Y) &&
               (currentInteractionMode != InteractionMode::AUTO_SCROLL_X_FIXED_SCALE_Y) &&
               plotDataKey != DataTypes::DataType::PUPIL_CONFIDENCE && plotDataKey != DataTypes::DataType::PUPIL_OUTLINE_CONFIDENCE) {

                // rescale value (vertical) axis to fit the current data:
            //    customPlot->graph(0)->rescaleValueAxis(false, true);
            //    customPlot->graph(1)->rescaleValueAxis(false, true);
                std::set<double> possibleMinima;
                std::set<double> possibleMaxima;
                bool ok;
                QCPRange possibleRange;
                for(int i=0; i<numCols; i++) {
                    ok = false;
                    possibleRange = customPlot->graph(0)->data()->valueRange(ok);
                    if (ok) {
                        possibleMinima.insert(possibleRange.lower);
                        possibleMaxima.insert(possibleRange.upper);
                    }
                }
                QCPRange commonRange(yAxisLimitLowT, yAxisLimitHighT);
                if(!possibleMinima.empty())
                    commonRange.lower = *possibleMinima.begin();
                if(!possibleMaxima.empty())
                    commonRange.upper = *possibleMaxima.rbegin();

                customPlot->yAxis->setRange(commonRange);

//                for(int i=0; i<numCols; i++) {
//                    customPlot->graph(i)->rescaleValueAxis(false, true);
//                }
            }
            
            // make key axis range scroll with the data (at a constant range size of 15secs):
            customPlot->xAxis->setRange(m_timestamp/1000.0, 15, Qt::AlignRight);
        }


        // if the first data is older than 4 minutes, remove 2 minutes of worth
        if((m_timestamp/1000.0) - customPlot->graph(0)->dataMainKey (0) > 240) {
//            customPlot->graph(0)->data()->removeBefore((m_timestamp/1000.0)-120);
//            customPlot->graph(1)->data()->removeBefore((m_timestamp/1000.0)-120);
            for(int i=0; i<numCols; i++) {
                customPlot->graph(i)->data()->removeBefore((m_timestamp/1000.0)-120);
            }
        }

        customPlot->replot();
    }
}

void GraphPlot::updateYaxisRange() {
    customPlot->yAxis->setRange(yAxisLimitLow,yAxisLimitHigh);
}

void GraphPlot::onPlaybackSafelyStopped() {
    reset();
}
