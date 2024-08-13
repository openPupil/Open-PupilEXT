#pragma once

/**
    @author Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include <QtWidgets/QWidget>
#include <QtCore/qobjectdefs.h>
#include "qcustomplot/qcustomplot.h"
#include "../pupil-detection-methods/Pupil.h"
#include "../pupilDetection.h"
#include "../dataTypes.h"

/**
    Custom lineplot graph widget employing the QCustomPlot library for plotting

    GraphPlot(): create graph window and define window title

slots:
    appendData(): Slot for receiving respective data, depends on which data value is selected
*/
class GraphPlot : public QWidget {
    Q_OBJECT

public:

    enum InteractionMode {
        AUTO_SCROLL_X_AUTO_SCALE_Y = 1,
        AUTO_SCROLL_X_MANUAL_SCALE_Y = 2,
        MANUAL_SCALE_SCROLL_X_Y = 3,
        AUTO_SCROLL_X_FIXED_SCALE_Y = 4,
    };

    static uint64 sharedTimestamp; // timestamp that shares every graph so the times match
    uint64 lastTimestamp = 0;

    explicit GraphPlot(DataTypes::DataType plotDataKey, ProcMode procMode=ProcMode::SINGLE_IMAGE_ONE_PUPIL, bool legend=false, QWidget *parent=0);
    ~GraphPlot() override;

    QSize sizeHint() const override;
    void setupPlotAxis();

private slots:

    void reset();
    void contextMenuRequest(QPoint pos);
//    void enableInteractions();
//    void enableYAxisInteraction();

public slots:

    //void appendData(quint64 timestamp, const Pupil &pupil, const QString &filename);
    //void appendData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);

    void appendData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename); // GB

    void appendData(const double &fps);
    void appendData(const int &framecount);

    void onPlaybackSafelyStopped();
    void onPlaybackSafelyStarted();

    void setKnownTimeZero(uint64_t timestamp);

private:

    QSettings *applicationSettings;
    double yAxisLimitLowT;
    double yAxisLimitHighT;
    double yAxisLimitLow;
    double yAxisLimitHigh;

    InteractionMode currentInteractionMode = InteractionMode::AUTO_SCROLL_X_AUTO_SCALE_Y;

    DataTypes::DataType plotDataKey;

    ProcMode currentProcMode;

    QCustomPlot *customPlot;
    QCPGraph *graph;

    uint64 incrementedTimestamp;

//    bool interaction;
//    bool yinteraction;

    void loadYaxisSettings();
    void saveYaxisSettings();

private slots:
    void setInteractionMode(InteractionMode m);
    void updateYaxisRange();
};
