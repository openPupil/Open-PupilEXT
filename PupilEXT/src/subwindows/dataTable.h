
#ifndef PUPILEXT_DATATABLE_H
#define PUPILEXT_DATATABLE_H

/**
    @author Moritz Lode
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/qtableview.h>
#include "../pupil-detection-methods/Pupil.h"
#include "../devices/camera.h"


/**
    Table widget as part of the main GUI showing the live data of the pupil measurements, upon click on a row entry, the option for visualization of the data is given.

    Upon visualization, a signal is send (createGraphPlot), which is received in the main window and used to create a new graphplot window.

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

    explicit DataTable(bool stereoMode=false, QWidget *parent=0);
    ~DataTable() override;

    QSize sizeHint() const override;

private:

    bool stereoMode;

    QTableView *tableView;
    QStandardItemModel *tableModel;

    QElapsedTimer timer;
    int updateDelay;

    QMenu *tableContextMenu;

    void setPupilData(const Pupil &pupil, int column=0);

public slots:

    void onPupilData(quint64 timestamp, const Pupil &pupil, const QString &filename);
    void onStereoPupilData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename);

    void onCameraFPS(double fps);
    void onCameraFramecount(int framecount);
    void onProcessingFPS(double fps);

    void customMenuRequested(QPoint pos);
    void onContextMenuClick(QAction* action);

signals:

    void createGraphPlot(const QString &value);

};


#endif //PUPILEXT_DATATABLE_H
