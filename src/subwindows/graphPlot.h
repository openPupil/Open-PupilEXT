#ifndef PUPILEXT_GRAPHPLOT_H
#define PUPILEXT_GRAPHPLOT_H

/**
    @author Moritz Lode, Gábor Bényei
*/

#include <QtWidgets/QWidget>
#include <QtCore/qobjectdefs.h>

#include "qcustomplot/qcustomplot.h"
#include "../pupil-detection-methods/Pupil.h"

#include "../pupilDetection.h"

/**
    Custom lineplot graph widget employing the QCustomPlot library for plotting

    NOTE: Modified by Gábor Bényei, 2023 jan
    GB NOTE:
        Reorganized code to let it handle an std::vector of Pupils, in order to comply with new signal-slot strategy, which
        I introduced to manage different pupil detection processing modes (procModes)

    GraphPlot(): create graph window and define window title

slots:
    appendData(): Slot for receiving respective data, depends on which data value is selected
*/
class GraphPlot : public QWidget {
    Q_OBJECT

public:

    static uint64 sharedTimestamp; // timestamp that shares every graph so the times match

    explicit GraphPlot(QString plotValue, ProcMode procMode=ProcMode::SINGLE_IMAGE_ONE_PUPIL, bool legend=false, QWidget *parent=0);
    ~GraphPlot() override;

    QSize sizeHint() const override;
    void setupPlotAxis();

private slots:

    void reset();
    void contextMenuRequest(QPoint pos);
    void enableInteractions();
    void enableYAxisInteraction();

public slots:

    //void appendData(quint64 timestamp, const Pupil &pupil, const QString &filename);
    //void appendData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);

    void appendData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename); // GB

    void appendData(const double &fps);
    void appendData(const int &framecount);

private:

    QString plotValue;

    QCustomPlot *customPlot;
    QCPGraph *graph;

    QElapsedTimer timer;
    uint64 incrementedTimestamp;

    bool interaction;
    bool yinteraction;

    int updateDelay;

};

#endif //PUPILEXT_GRAPHPLOT_H
