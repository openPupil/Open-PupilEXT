
#ifndef PUPILEXT_DATATABLE_H
#define PUPILEXT_DATATABLE_H

/**
    @author Moritz Lode, Gábor Bényei
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/qtableview.h>
#include "../pupil-detection-methods/Pupil.h"
#include "../devices/camera.h"

#include "../pupilDetection.h"


/**
    Table widget as part of the main GUI showing the live data of the pupil measurements, upon click on a row entry, the option for visualization of the data is given.

    Upon visualization, a signal is send (createGraphPlot), which is received in the main window and used to create a new graphplot window.

    NOTE: Modified by Gábor Bényei, 2023 jan
    GB NOTE:
        Reorganized code to let it handle an std::vector of Pupils, in order to comply with new signal-slot strategy, which
        I introduced to manage different pupil detection processing modes (procModes).
        Also, now the width of dataTable window varies depending on how many columns it is showing,
        thus in mainwindow.cpp code I modified code to let their size be stored separately, by distinguishing instances of different columns by other titles.
        Columns are now colored in unison with ROI selection colors.

slots:
    onPupilData(): slot to receive pupil measurement data from the pupil detection
    onCamera*(): slot to receive camera information from the camera device such as fps

signals:
    createGraphPlot(): signal that is send when a visualization is requested by the user, contains which data entry is selected
*/
class DataTable : public QWidget {
    Q_OBJECT

public:

    static const QString TIME;
    static const QString FRAME_NUMBER;
    static const QString CAMERA_FPS;
    static const QString PUPIL_FPS;
    static const QString PUPIL_CENTER_X;
    static const QString PUPIL_CENTER_Y;
    static const QString PUPIL_MAJOR;
    static const QString PUPIL_MINOR;
    static const QString PUPIL_WIDTH;
    static const QString PUPIL_HEIGHT;
    static const QString PUPIL_DIAMETER;
    static const QString PUPIL_UNDIST_DIAMETER;
    static const QString PUPIL_PHYSICAL_DIAMETER;
    static const QString PUPIL_CONFIDENCE;
    static const QString PUPIL_OUTLINE_CONFIDENCE;
    static const QString PUPIL_CIRCUMFERENCE;
    static const QString PUPIL_RATIO;

    explicit DataTable(ProcMode procMode = ProcMode::SINGLE_IMAGE_ONE_PUPIL, QWidget *parent=0);
    ~DataTable() override;

    QSize sizeHint() const override;

private:

    QTableView *tableView;
    QStandardItemModel *tableModel;

    QElapsedTimer timer;
    int updateDelay;

    QMenu *tableContextMenu;

    // GB added begin
    ProcMode procMode;
    int numCols = 1;
    // GB added end

    void setPupilData(const Pupil &pupil, int column=0);

public slots:

    void onPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename); // GB modified for vector of pupils

    void onCameraFPS(double fps);
    void onCameraFramecount(int framecount);
    void onProcessingFPS(double fps);

    void customMenuRequested(QPoint pos);
    void onContextMenuClick(QAction* action);

signals:

    void createGraphPlot(const QString &value);

};


#endif //PUPILEXT_DATATABLE_H
