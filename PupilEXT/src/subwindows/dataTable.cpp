
#include <QtWidgets/qboxlayout.h>
#include <QMenu>
#include <QHeaderView>
#include <QtGui/QtGui>
#include <iostream>
#include "dataTable.h"

const QString DataTable::TIME = "time";
const QString DataTable::FRAME_NUMBER = "frame#";
const QString DataTable::CAMERA_FPS = "camera fps";
const QString DataTable::PUPIL_FPS = "pupil tracking fps";
const QString DataTable::PUPIL_CENTER_X = "pupil center x";
const QString DataTable::PUPIL_CENTER_Y = "pupil center y";
const QString DataTable::PUPIL_MAJOR = "pupil major axis";
const QString DataTable::PUPIL_MINOR = "pupil minor axis";
const QString DataTable::PUPIL_WIDTH = "pupil width";
const QString DataTable::PUPIL_HEIGHT = "pupil height";
const QString DataTable::PUPIL_DIAMETER = "pupil diameter";
const QString DataTable::PUPIL_UNDIST_DIAMETER = "pupil undistorted diameter";
const QString DataTable::PUPIL_PHYSICAL_DIAMETER = "pupil physical diameter";
const QString DataTable::PUPIL_CONFIDENCE = "pupil confidence";
const QString DataTable::PUPIL_OUTLINE_CONFIDENCE = "pupil outline confidence";
const QString DataTable::PUPIL_CIRCUMFERENCE = "pupil circumference";
const QString DataTable::PUPIL_RATIO = "pupil axis ratio";


// Create a new DataTable, stereoMode decides wherever two or one column is displayed.
// The different columns of the Datatable are defined in the header file using the constants.
DataTable::DataTable(bool stereoMode, QWidget *parent) : QWidget(parent), stereoMode(stereoMode) {

    setWindowTitle("Data Table");

    updateDelay = 33; // 30fps

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Create table view, for stereo mode the table has two columns
    tableView = new QTableView();

    tableView->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView->verticalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));

    layout->addWidget(tableView);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

    tableContextMenu = new QMenu(this);
    // Caution when adding new actions to this menu, the handler onContextMenuClick depends on the plot value action to be the first
    tableContextMenu->addAction(new QAction("Plot Value", this));
    connect(tableContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(onContextMenuClick(QAction*)));

    tableModel = new QStandardItemModel(17, stereoMode ? 2 : 1, this);

    tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Main Value"));

    if(stereoMode) {
        tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Secondary Value"));
    }

    tableModel->setHeaderData(0, Qt::Vertical, TIME);
    tableModel->setHeaderData(1, Qt::Vertical, FRAME_NUMBER);
    tableModel->setHeaderData(2, Qt::Vertical, CAMERA_FPS);
    tableModel->setHeaderData(3, Qt::Vertical, PUPIL_FPS);
    tableModel->setHeaderData(4, Qt::Vertical, PUPIL_CENTER_X);
    tableModel->setHeaderData(5, Qt::Vertical, PUPIL_CENTER_Y);
    tableModel->setHeaderData(6, Qt::Vertical, PUPIL_MAJOR);
    tableModel->setHeaderData(7, Qt::Vertical, PUPIL_MINOR);
    tableModel->setHeaderData(8, Qt::Vertical, PUPIL_WIDTH);
    tableModel->setHeaderData(9, Qt::Vertical, PUPIL_HEIGHT);
    tableModel->setHeaderData(10, Qt::Vertical, PUPIL_DIAMETER);
    tableModel->setHeaderData(11, Qt::Vertical, PUPIL_UNDIST_DIAMETER);
    tableModel->setHeaderData(12, Qt::Vertical, PUPIL_PHYSICAL_DIAMETER);
    tableModel->setHeaderData(13, Qt::Vertical, PUPIL_CONFIDENCE);
    tableModel->setHeaderData(14, Qt::Vertical, PUPIL_OUTLINE_CONFIDENCE);
    tableModel->setHeaderData(15, Qt::Vertical, PUPIL_CIRCUMFERENCE);
    tableModel->setHeaderData(16, Qt::Vertical, PUPIL_RATIO);

    tableView->setModel(tableModel);
    tableView->resizeRowsToContents();
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    // Color coding the two column header for stereo mode
    QStandardItem *item1 = tableModel->horizontalHeaderItem(0);
    item1->setForeground(Qt::blue);

    if(stereoMode) {
        QStandardItem *item2 = tableModel->horizontalHeaderItem(1);
        item2->setForeground(Qt::green);
    }

    resize(tableView->width(), tableView->height());

    tableView->show();

    timer.start();
}

DataTable::~DataTable() {

}

QSize DataTable::sizeHint() const {
    return QSize(330, 500);
}

// On right-click in the table header
// Shows a context menu at the click position
void DataTable::customMenuRequested(QPoint pos){
    int row = tableView->verticalHeader()->logicalIndexAt(pos);

    tableContextMenu->actions().first()->setData(tableModel->verticalHeaderItem(row)->text());
    tableContextMenu->popup(tableView->verticalHeader()->viewport()->mapToGlobal(pos));
}

// Slot handler that receives new pupil data from the pupil detection process
// New data is only updated in the table at a lower interval defined by updateDelay, to not overload/block the GUI process
void DataTable::onPupilData(quint64 timestamp, const Pupil &pupil, const QString &filename) {

    if(timer.elapsed() > updateDelay && pupil.valid(-2)) {
        timer.restart();

        // QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
        QDateTime date = QDateTime::fromMSecsSinceEpoch(timestamp);
        // Display the date/time in the system specific locale format
        QStandardItem *item = new QStandardItem(QLocale::system().toString(date));
        tableModel->setItem(0, 0, item);

        setPupilData(pupil, 0);
    }
}

// Slot handler that receives new stereo pupil data from the pupil detection process
// New data is only updated in the table at a lower interval defined by updateDelay, to not overload/block the GUI process
void DataTable::onStereoPupilData(quint64 timestamp, const Pupil &pupil, const Pupil &pupilSec, const QString &filename) {

    if(timer.elapsed() > updateDelay && pupil.valid(-2)) {
        timer.restart();

        // QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
        QDateTime date = QDateTime::fromMSecsSinceEpoch(timestamp);
        // Display the date/time in the system specific locale format
        QStandardItem *item = new QStandardItem(QLocale::system().toString(date));
        tableModel->setItem(0, 0, item);

        setPupilData(pupil, 0);
        setPupilData(pupilSec, 1);
    }
}

// Updates the table column entries given pupil data and a column index (0, 1)
void DataTable::setPupilData(const Pupil &pupil, int column) {

    if(column > 1) {
        return;
    }

    QStandardItem *item = new QStandardItem(QString::number(pupil.center.x));
    tableModel->setItem(4, column, item);

    item = new QStandardItem(QString::number(pupil.center.y));
    tableModel->setItem(5, column, item);

    item = new QStandardItem(QString::number(pupil.majorAxis()));
    tableModel->setItem(6, column, item);

    item = new QStandardItem(QString::number(pupil.minorAxis()));
    tableModel->setItem(7, column, item);

    item = new QStandardItem(QString::number(pupil.width()));
    tableModel->setItem(8, column, item);

    item = new QStandardItem(QString::number(pupil.height()));
    tableModel->setItem(9, column, item);

    item = new QStandardItem(QString::number(pupil.diameter()));
    tableModel->setItem(10, column, item);

    item = new QStandardItem(QString::number(pupil.undistortedDiameter));
    tableModel->setItem(11, column, item);

    item = new QStandardItem(QString::number(pupil.physicalDiameter));
    tableModel->setItem(12, column, item);

    item = new QStandardItem(QString::number(pupil.confidence));
    tableModel->setItem(13, column, item);

    item = new QStandardItem(QString::number(pupil.outline_confidence));
    tableModel->setItem(14, column, item);

    item = new QStandardItem(QString::number(pupil.circumference()));
    tableModel->setItem(15, column, item);

    item = new QStandardItem(QString::number((double)pupil.majorAxis() / pupil.minorAxis()));
    tableModel->setItem(16, column, item);
}

// Slot handler receiving FPS data from a camera framecounter
void DataTable::onCameraFPS(double fps) {
    QStandardItem *item = new QStandardItem(QString::number(fps));
    tableModel->setItem(2, item);
}

// Slot handler receiving FPS data from a camera framecounter
void DataTable::onCameraFramecount(int framecount) {
    QStandardItem *item = new QStandardItem(QString::number(framecount));
    tableModel->setItem(1, item);
}

// Slot handler receiving processing FPS data from the pupil detection process
void DataTable::onProcessingFPS(double fps) {
    QStandardItem *item = new QStandardItem(QString::number(fps));
    tableModel->setItem(3, item);
}

// Event handler that is called on click of an action in the context menu of the table
// Currently only a single action is in that menu, plot value
// Data inside the action which describes the clicked column is read and the corresponding graphplot is created
void DataTable::onContextMenuClick(QAction *action) {

    QString value = action->data().toString();

    emit createGraphPlot(value);
}


