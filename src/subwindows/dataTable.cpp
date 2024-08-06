
#include <QtWidgets/qboxlayout.h>
#include <QMenu>
#include <QHeaderView>
#include <QtGui/QtGui>
#include <iostream>
#include "dataTable.h"
#include "./../SVGIconColorAdjuster.h"


// Create a new DataTable, stereoMode decides wherever two or one column is displayed.
// The different columns of the Datatable are defined in the header file using the constants.
DataTable::DataTable(ProcMode procMode, QWidget *parent) : QWidget(parent), procMode(procMode), applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    setWindowTitle("Data Table");

    updateDelay = 33; // 30fps

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Create table view, for stereo mode the table has two columns
    tableView = new QTableView();

//    tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents); // Does not work yet. TODO: track the height of the table, accounting for nr. of rows

    tableView->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView->verticalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));

    layout->addWidget(tableView);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

    tableContextMenu = new QMenu(this);
    // Caution when adding new actions to this menu, the handler onContextMenuClick depends on the plot value action to be the first
    tableContextMenu->addAction(new QAction(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/actions/22/labplot-xy-interpolation-curve.svg"), applicationSettings),"Plot Value", this));
    connect(tableContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(onContextMenuClick(QAction*)));

    // TODO: make this menu for header items too (bit complicated), and for each row (cell) make a contextmenu too
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTableRowDoubleClick(QModelIndex)));


    // GB added/modified begin
//    tableModel = new QStandardItemModel(17, stereoMode ? 2 : 1, this);
    //int numCols=1;
    switch(procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            numCols=1;
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
    tableModel = new QStandardItemModel(17, numCols, this);


    switch(procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Main"));
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
            tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Eye A"));
            tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Eye B"));
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Main"));
            tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Sec."));
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Eye A Main"));
            tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Eye A Sec."));
            tableModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Eye B Main"));
            tableModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Eye B Sec."));
            break;
    }

    for(int v = 0; v < DataTypes::map.size(); v++) {
        tableModel->setHeaderData(v, Qt::Vertical, DataTypes::map.value((DataTypes::DataType)v));
    }

    // Make the table contents read-only in the GUI
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->setModel(tableModel);
    tableView->resizeRowsToContents();
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Items are now made here, not on each pupil detection arrival
    for(int r = 0; r < tableModel->rowCount(); r++) {
        for (int c = 0; c < tableModel->columnCount(); c++)
            tableModel->setItem(r, c, new QStandardItem(""));
    }

    // Marking every second row with different background for better readibility, also adapting to dark mode
    QBrush cellColor1;
    QBrush cellColor2;
    bool darkMode = applicationSettings->value("GUIDarkAdaptMode", "0") == "1" || (applicationSettings->value("GUIDarkMode", "0") == "2");
    if(darkMode) {
        cellColor1 = QBrush("#3d3d3d");
        cellColor2 = QBrush("#696969");
    } else {
        cellColor1 = QBrush("#f5f4f2");
        cellColor2 = QBrush("#e6e6e6");
    }
    QBrush cellColor;
    for(int r = 0; r < tableModel->rowCount(); r++) {
        for (int c = 0; c < tableModel->columnCount(); c++) {
            cellColor = (r % 2 != 0) ? cellColor1 : cellColor2;
            tableModel->item(r, c)->setBackground(cellColor);
        }
    }

    switch(procMode) {
        case ProcMode::SINGLE_IMAGE_ONE_PUPIL:
            tableModel->horizontalHeaderItem(0)->setForeground(Qt::blue);
            break;
        case ProcMode::SINGLE_IMAGE_TWO_PUPIL:
        // case ProcMode::MIRR_IMAGE_ONE_PUPIL:
        case ProcMode::STEREO_IMAGE_ONE_PUPIL:
            tableModel->horizontalHeaderItem(0)->setForeground(Qt::blue);
            tableModel->horizontalHeaderItem(1)->setForeground(Qt::green);
            break;
        case ProcMode::STEREO_IMAGE_TWO_PUPIL:
            tableModel->horizontalHeaderItem(0)->setForeground(Qt::blue);
            tableModel->horizontalHeaderItem(1)->setForeground(Qt::green);
            tableModel->horizontalHeaderItem(2)->setForeground(QColor(144, 55, 212, 255)); // purple
            tableModel->horizontalHeaderItem(3)->setForeground(QColor(214, 140, 49,255)); // orange
            break;
    }

    tableView->show();

    //resize(tableView->width(), tableView->height());
//    resize(160 + numCols*80, 500);

//    // TODO: Make this work. Somehow there is no effect of this
//    int tableHeight = tableView->height(); // Seems to return correct value
////    int oneItemHeight = tableModel->item(0)->sizeHint().height(); // returns -1 for some reason
//    int minimumHeight = tableHeight + 60;
//    int minimumWidth = 120 * (tableModel->columnCount() + 1) + 20;
//    parent->setMinimumSize(minimumWidth, minimumHeight);

    timer.start();
}

DataTable::~DataTable() {

}

QSize DataTable::sizeHint() const {
    return QSize(330, 500);

//    return QSize(160 + numCols*80, 500);
}

// On right-click in the table header
// Shows a context menu at the click position
void DataTable::customMenuRequested(QPoint pos){
    int row = tableView->verticalHeader()->logicalIndexAt(pos);

    tableContextMenu->actions().first()->setData(row);
    tableContextMenu->popup(tableView->verticalHeader()->viewport()->mapToGlobal(pos));
}

// Slot handler that receives new pupil data from the pupil detection process
// New data is only updated in the table at a lower interval defined by updateDelay, to not overload/block the GUI process
void DataTable::onPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename) {
    // std::cout << "timestamp = " << QString::number(timestamp).toStdString() << "; filename = " << filename.toStdString() << std::endl;

    if(timer.elapsed() > updateDelay) {
        timer.restart();

        // QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
        QDateTime date = QDateTime::fromMSecsSinceEpoch(timestamp);
        // Display the date/time in the system specific locale format
        tableModel->item(0,0)->setText(QLocale::system().toString(date));

        // GB: modified to work with new Pupil vector signals
        for(int i=0; i<Pupils.size(); i++)
            if(Pupils[i].valid(-2))
                setPupilData(Pupils[i], i);
        // GB NOTE: this only works, because we defined the columns in the exact same order 
        // as in what pupil vector elements are, when they arrive
    }
}

// Updates the table column entries given pupil data and a column index (0, 1)
// TODO: Figure out something cleaner using the key-value map we yet have
void DataTable::setPupilData(const Pupil &pupil, int column) {

    tableModel->item(4, column)->setText(QString::number(pupil.center.x));
    tableModel->item(5, column)->setText(QString::number(pupil.center.y));
    tableModel->item(6, column)->setText(QString::number(pupil.majorAxis()));
    tableModel->item(7, column)->setText(QString::number(pupil.minorAxis()));
    tableModel->item(8, column)->setText(QString::number(pupil.width()));
    tableModel->item(9, column)->setText(QString::number(pupil.height()));
    tableModel->item(10, column)->setText(QString::number(pupil.diameter()));
    tableModel->item(11, column)->setText(QString::number(pupil.undistortedDiameter));
    tableModel->item(12, column)->setText(QString::number(pupil.physicalDiameter));
    tableModel->item(13, column)->setText(QString::number(pupil.confidence));
    tableModel->item(14, column)->setText(QString::number(pupil.outline_confidence));
    tableModel->item(15, column)->setText(QString::number(pupil.circumference()));
    tableModel->item(16, column)->setText(QString::number((double)pupil.majorAxis() / pupil.minorAxis()));

}

// Slot handler receiving FPS data from a camera framecounter
void DataTable::onCameraFPS(double fps) {
    tableModel->item(2)->setText(QString::number(fps));
}

// Slot handler receiving FPS data from a camera framecounter
void DataTable::onCameraFramecount(int framecount) {
    tableModel->item(1)->setText(QString::number(framecount));
}

// Slot handler receiving processing FPS data from the pupil detection process
void DataTable::onProcessingFPS(double fps) {
    tableModel->item(3)->setText(QString::number(fps));
}

// Event handler that is called on click of an action in the context menu of the table
// Currently only a single action is in that menu, plot value
// Data inside the action which describes the clicked column is read and the corresponding graphplot is created
void DataTable::onContextMenuClick(QAction *action) {

    DataTypes::DataType value = (DataTypes::DataType)action->data().toInt();

    if(value == DataTypes::DataType::TIME || value == DataTypes::DataType::FRAME_NUMBER)
        return;

    emit createGraphPlot(value);
}

void DataTable::onTableRowDoubleClick(const QModelIndex &modelIndex) {

    DataTypes::DataType value = (DataTypes::DataType)modelIndex.row();

//    QString value = modelIndex.data().toString();
//    // nemtudom jó-e

    if(value == DataTypes::DataType::TIME || value == DataTypes::DataType::FRAME_NUMBER)
        return;

    emit createGraphPlot(value);
}


