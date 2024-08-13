#pragma once

/**
    @author Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/qtableview.h>
#include "../pupil-detection-methods/Pupil.h"
#include "../devices/camera.h"
#include "../pupilDetection.h"
#include "../dataTypes.h"


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

    explicit DataTable(ProcMode procMode = ProcMode::SINGLE_IMAGE_ONE_PUPIL, QWidget *parent=0);
    ~DataTable() override;

    QSize sizeHint() const override;
    void fitForTableSize();

private:

    QSettings *applicationSettings;

    QTableView *tableView;
    QStandardItemModel *tableModel;

    QMenu *tableContextMenu;

    ProcMode procMode;
    int numCols = 1;

    void setPupilData(const Pupil &pupil, int column=0);

public slots:

    void onPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename);

    void onCameraFPS(double fps);
    void onCameraFramecount(int framecount);
    void onProcessingFPS(double fps);

    void customMenuRequested(QPoint pos);
    void onContextMenuClick(QAction* action);

    void onTableRowDoubleClick(const QModelIndex &modelIndex);

signals:

    void createGraphPlot(DataTypes::DataType value);

};
