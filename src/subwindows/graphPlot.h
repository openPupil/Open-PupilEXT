#ifndef PUPILEXT_GRAPHPLOT_H
#define PUPILEXT_GRAPHPLOT_H

/**
    @author Moritz Lode
*/

#include <QtWidgets/QWidget>
#include <QtCore/qobjectdefs.h>

#include "qcustomplot/qcustomplot.h"
#include "../pupil-detection-methods/Pupil.h"

/**
    Custom lineplot graph widget employing the QCustomPlot library for plotting

    GraphPlot(): create graph window and define window title

slots:
    appendData(): Slot for receiving respective data, depends on which data value is selected
*/
class GraphPlot : public QWidget {
    Q_OBJECT

public:

    static uint64 sharedTimestamp; // timestamp that shares every graph so the times match

    explicit GraphPlot(QString plotValue, bool stereoMode=false, bool legend=false, QWidget *parent=0);
    ~GraphPlot() override;

    QSize sizeHint() const override;
    void setupPlotAxis();

private slots:

    void reset();
    void contextMenuRequest(QPoint pos);
    void enableInteractions();
    void enableYAxisInteraction();

public slots:

    void appendData(quint64 timestamp, const Pupil &pupil, const QString &filename);
    void appendData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);

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
