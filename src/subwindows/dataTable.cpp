
#include <QtWidgets/qboxlayout.h>
#include <QMenu>
#include <QHeaderView>
#include <QtGui/QtGui>
#include <iostream>
#include "dataTable.h"
#include "./../SVGIconColorAdjuster.h"
#include "RestorableQMdiSubWindow.h"


// Create a new DataTable, stereoMode decides wherever two or one column is displayed.
// The different columns of the Datatable are defined in the header file using the constants.
DataTable::DataTable(ProcMode procMode, QWidget *parent) : QWidget(parent), procMode(procMode), applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    // IMPORTANT NOTE: This name should never be altered, it is used for checking if the window exists in the MDI window space
    setWindowTitle("Data Table");

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
    tableModel = new QStandardItemModel(18, numCols, this);

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
        for (int c = 0; c < tableModel->columnCount(); c++) {
            tableModel->setItem(r, c, new QStandardItem(""));
        }
    }

    // Marking every second row with different background for better readibility, also adapting to dark mode
    QString cellColor1;
    QString cellColor2;
    QString gridColor;
    bool darkMode = applicationSettings->value("GUIDarkAdaptMode", "0") == "1" || (applicationSettings->value("GUIDarkMode", "0") == "2");
    if(darkMode) {
        cellColor1 = "#3d3d3d";
        cellColor2 = "#696969";
        gridColor = "#757575";
    } else {
        cellColor1 = "#f5f4f2";
        cellColor2 = "#e6e6e6";
        gridColor = "#262626";
    }
    QString cellColor;
    for(int r = 0; r < tableModel->rowCount(); r++) {
        cellColor = (r % 2 != 0) ? cellColor1 : cellColor2;
        for (int c = 0; c < tableModel->columnCount(); c++) {
            tableModel->item(r, c)->setBackground(QBrush(QColor(cellColor)));
        }
//        tableModel->verticalHeaderItem(r)->setBackground(QBrush(QColor(cellColor))); // It should, but somehow does not work
    }
    tableView->setStyleSheet("QTableView::section { background-color: " + cellColor2 + "; gridline-color: " + gridColor + "; }");
    tableView->setStyleSheet("QHeaderView::section { background-color: " + cellColor2 + "; gridline-color: " + gridColor + "; }");
    tableView->setStyleSheet("QTableCornerButton::section { background-color: " + cellColor2 + "; gridline-color: " + gridColor + "; }");

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
}

DataTable::~DataTable() {

}

QSize DataTable::sizeHint() const {
    return QSize(330, 500);

//    return QSize(160 + numCols*80, 500);
}

void DataTable::fitForTableSize() {
    // TODO: Make this work. Somehow there is no effect of any of these
//    int tableHeight = tableView->height(); // Seems to return correct value, however it gives a smaller value when the widget is squeezed
////    int oneItemHeight2 = tableModel->item(0)->sizeHint().height(); // returns -1 for some reason
//    int minimumHeight = tableHeight; // + 60;

    int minimumWidth = 120 * (tableModel->columnCount() + 1) + 80;
    int minimumHeight = 26 * (tableModel->rowCount() + 1) + 20;
//    parent->setMinimumSize(minimumWidth, minimumHeight);
    dynamic_cast<RestorableQMdiSubWindow*>(parent())->resize(minimumWidth, minimumHeight);
}

// On right-click in the table header
// Shows a context menu at the click position
void DataTable::customMenuRequested(QPoint pos){
    int row = tableView->verticalHeader()->logicalIndexAt(pos);

    tableContextMenu->actions().first()->setData(row);
    tableContextMenu->popup(tableView->verticalHeader()->viewport()->mapToGlobal(pos));
}

// Slot handler that receives new pupil data from the pupil detection process
// New data is only updated in the table at low FPS
void DataTable::onPupilData(quint64 timestamp, int procMode, const std::vector<Pupil> &Pupils, const QString &filename) {
    // std::cout << "timestamp = " << QString::number(timestamp).toStdString() << "; filename = " << filename.toStdString() << std::endl;

    tableModel->item((int)DataTypes::DataType::TIME_RAW_TIMESTAMP,0)->setText(QString::number(timestamp));

    // QDateTime::fromMSecsSinceEpoch converts the UTC timestamp into localtime
    QDateTime date = QDateTime::fromMSecsSinceEpoch(timestamp);
//    // Display the date/time in the system specific locale format
//    tableModel->item(0,0)->setText(QLocale::system().toString(date));
    tableModel->item((int)DataTypes::DataType::TIME,0)->setText(date.toString("hh:mm:ss"));

    for(int i=0; i<Pupils.size(); i++)
        if(Pupils[i].valid(-2))
            setPupilData(Pupils[i], i);
    // NOTE: this only works, because we defined the columns in the exact same order
    // as in what pupil vector elements are, when they arrive
}

// Updates the table column entries given pupil data and a column index (0, 1)
// TODO: Figure out something cleaner using the key-value map we yet have
void DataTable::setPupilData(const Pupil &pupil, int column) {

    tableModel->item((int)DataTypes::DataType::PUPIL_CENTER_X, column)->setText(QString::number(pupil.center.x));
    tableModel->item((int)DataTypes::DataType::PUPIL_CENTER_Y, column)->setText(QString::number(pupil.center.y));
    tableModel->item((int)DataTypes::DataType::PUPIL_MAJOR, column)->setText(QString::number(pupil.majorAxis()));
    tableModel->item((int)DataTypes::DataType::PUPIL_MINOR, column)->setText(QString::number(pupil.minorAxis()));
    tableModel->item((int)DataTypes::DataType::PUPIL_WIDTH, column)->setText(QString::number(pupil.width()));
    tableModel->item((int)DataTypes::DataType::PUPIL_HEIGHT, column)->setText(QString::number(pupil.height()));
    tableModel->item((int)DataTypes::DataType::PUPIL_DIAMETER, column)->setText(QString::number(pupil.diameter()));
    tableModel->item((int)DataTypes::DataType::PUPIL_UNDIST_DIAMETER, column)->setText(QString::number(pupil.undistortedDiameter));
    tableModel->item((int)DataTypes::DataType::PUPIL_PHYSICAL_DIAMETER, column)->setText(QString::number(pupil.physicalDiameter));
    tableModel->item((int)DataTypes::DataType::PUPIL_CONFIDENCE, column)->setText(QString::number(pupil.confidence));
    tableModel->item((int)DataTypes::DataType::PUPIL_OUTLINE_CONFIDENCE, column)->setText(QString::number(pupil.outline_confidence));
    tableModel->item((int)DataTypes::DataType::PUPIL_CIRCUMFERENCE, column)->setText(QString::number(pupil.circumference()));
    tableModel->item((int)DataTypes::DataType::PUPIL_RATIO, column)->setText(QString::number((double)pupil.majorAxis() / pupil.minorAxis()));

}

// Slot handler receiving FPS data from a camera framecounter
void DataTable::onCameraFPS(double fps) {
    tableModel->item((int)DataTypes::DataType::CAMERA_FPS)->setText(QString::number(fps));
}

// Slot handler receiving FPS data from a camera framecounter
void DataTable::onCameraFramecount(int framecount) {
    tableModel->item((int)DataTypes::DataType::FRAME_NUMBER)->setText(QString::number(framecount));
}

// Slot handler receiving processing FPS data from the pupil detection process
void DataTable::onProcessingFPS(double fps) {
    tableModel->item((int)DataTypes::DataType::PUPIL_FPS)->setText(QString::number(fps));
}

// Event handler that is called on click of an action in the context menu of the table
// Currently only a single action is in that menu, plot value
// Data inside the action which describes the clicked column is read and the corresponding graphplot is created
void DataTable::onContextMenuClick(QAction *action) {

    DataTypes::DataType value = (DataTypes::DataType)action->data().toInt();

    if(value == DataTypes::DataType::TIME_RAW_TIMESTAMP || value == DataTypes::DataType::TIME)
        return;

    emit createGraphPlot(value);
}

void DataTable::onTableRowDoubleClick(const QModelIndex &modelIndex) {

    DataTypes::DataType value = (DataTypes::DataType)modelIndex.row();

//    QString value = modelIndex.data().toString();
//    // nemtudom jó-e

    if(value == DataTypes::DataType::TIME_RAW_TIMESTAMP || value == DataTypes::DataType::TIME)
        return;

    emit createGraphPlot(value);
}


