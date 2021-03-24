
#include <iostream>
#include "subjectSelectionDialog.h"


const QString SubjectSelectionDialog::SAVE_ACTION = "Save Current Config";
const QString SubjectSelectionDialog::LOAD_ACTION = "Load Subject Config";
const QString SubjectSelectionDialog::DELETE_ACTION = "Remove Subject Config";


// Creates a new widget showing the subject selection, a table view with subject entries which be saved and load
// Subject configurations are saved to disk to the specified path
SubjectSelectionDialog::SubjectSelectionDialog(QWidget *parent): QDialog(parent),
        subjectConfigs(QList<QPair<QString, QString>>()),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    setWindowTitle("Subject Configuration Files");

    this->setMinimumSize(600, 300);
    this->setWindowTitle("Subject Configurations");

    // The actual entries of the table view are saved in the application settings, containing the name and path of the specified settings files
    subjectConfigs = applicationSettings->value("SubjectSelectionDialog.subjectConfigs", QVariant::fromValue(subjectConfigs)).value<QList<QPair<QString, QString>>>();

    QVBoxLayout* layout = new QVBoxLayout(this);

    tableView = new QTableView();

    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));

    layout->addWidget(tableView);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

    tableContextMenu = new QMenu(this);
    tableContextMenu->addAction(new QAction(SAVE_ACTION, this));
    tableContextMenu->addAction(new QAction(LOAD_ACTION, this));
    tableContextMenu->addAction(new QAction(DELETE_ACTION, this));

    // React on the click of an action of the menu
    connect(tableContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(onContextMenuClick(QAction*)));

    tableModel = new QStandardItemModel(subjectConfigs.size(), 2, this);

    tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Subject"));
    tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Path"));

    for(int i=0; i<subjectConfigs.size();i++) {
        QPair<QString, QString> var = subjectConfigs[i];

        QStandardItem* item = new QStandardItem(var.first);
        tableModel->setItem(i, 0, item);

        item = new QStandardItem(var.second);
        tableModel->setItem(i, 1, item);
        item->setFlags(item->flags() &= ~Qt::ItemIsEditable);
    }

    tableView->setModel(tableModel);
    //tableView->resizeColumnsToContents();

    // Event handler on actions done on the table view
    // Itemchanged for changes in the name column (renaming)
    // DoubleClick for selection of a table entry (loading)
    connect(tableModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onCellChange(QStandardItem*)));
    connect(tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onCellDoubleClick(const QModelIndex &)));

    tableView->resizeRowsToContents();
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->show();

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(11,8,11,11);
    buttonsLayout->setSpacing(8);

    addButton = new QPushButton(tr("Add Subject"));
    buttonsLayout->addWidget(addButton);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addButtonClick()));

    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    layout->addLayout(buttonsLayout);
}

SubjectSelectionDialog::~SubjectSelectionDialog() {

}

// On right click in the table view, show the contextmenu at the click position
// For each action in the contextmenu, the clicked row index is set at the data field to identify the clicked row in the actions event handler
void SubjectSelectionDialog::customMenuRequested(QPoint pos){
    QModelIndex index = tableView->indexAt(pos);

    for(auto &act: tableContextMenu->actions()) {
        act->setData(index.row());
    }
    tableContextMenu->popup(tableView->verticalHeader()->viewport()->mapToGlobal(pos));
}

// On close, saved table entries
void SubjectSelectionDialog::reject() {
    saveSettings();

    QDialog::reject();
}

// Event handler when a cell entry changes
void SubjectSelectionDialog::onCellChange(QStandardItem *item) {
    // We are only interested in changes in the name column
    if(item->column()==0) {
        int i = item->row();
        if(i<subjectConfigs.size()) {
            QPair<QString, QString> old = subjectConfigs[i];
            std::cout<<"Cell changed:"<<item->row()<<" "<<item->column()<<item->text().toStdString()<<std::endl;

            subjectConfigs[i] = QPair<QString, QString>(item->text(), old.second);

            saveSettings();
        }
    }
}

// Event handler when a cell was doubleclicked
void SubjectSelectionDialog::onCellDoubleClick(const QModelIndex &index) {
    // We are only interested in clicks on the path column to select a new filepath
    if(index.column()==1) {
        QString name = tableModel->item(index.row(), 0)->text();
        QString oldPath = index.data().toString();
        QFileInfo pathInfo(oldPath);

        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Config File"), pathInfo.dir().path(), tr("INI files (*.ini)"), nullptr, QFileDialog::DontConfirmOverwrite);

        if(!fileName.isEmpty()) {
            QFileInfo fileInfo(fileName);

            recentPath = fileInfo.dir().path();

            // check if filename has extension
            if(fileInfo.suffix().isEmpty()) {
                fileName = fileName + ".ini";
            }

            QStandardItem *item = new QStandardItem(fileName);
            tableModel->setItem(index.row(), index.column(), item);
            item->setFlags(item->flags() &= ~Qt::ItemIsEditable);

            subjectConfigs[index.row()] = QPair<QString, QString>(name, fileName);

            saveSettings();
        }
    }
}

// On the Add button click
// Add a new table entry and directly query the user to select a filepath for the settings to be saved
// The name entry is default to "Subject #" which can then be changed by the user
void SubjectSelectionDialog::addButtonClick() {
    QString name = "Subject " + QString::number(subjectConfigs.size());

    QStandardItem* item = new QStandardItem(name);
    tableModel->setItem(subjectConfigs.size(), 0, item);

    recentPath = applicationSettings->value("RecentOutputPath", "").toString();

    // file dialog
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Config File"), recentPath, tr("INI files (*.ini)"), nullptr, QFileDialog::DontConfirmOverwrite);

    if(!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);

        recentPath = fileInfo.dir().path();

        // check if filename has extension
        if(fileInfo.suffix().isEmpty()) {
            fileName = fileName + ".ini";
        }
    }
    subjectConfigs.append(QPair<QString, QString>(name, fileName));

    item = new QStandardItem(fileName);
    tableModel->setItem(subjectConfigs.size()-1, 1, item);
    item->setFlags(item->flags() &= ~Qt::ItemIsEditable);

    saveSettings();
}

// Saves the table entries into the application wide settings
void SubjectSelectionDialog::saveSettings() {
    applicationSettings->setValue("SubjectSelectionDialog.subjectConfigs", QVariant::fromValue(subjectConfigs));
    applicationSettings->setValue("RecentOutputPath", recentPath);
}

// Loads a subject configuration to the application
// In this process, the application wide settings are overwritten by the values in the loaded settings file
void SubjectSelectionDialog::loadSubjectSettings(int row) {

    QString name = subjectConfigs[row].first;
    QString path = subjectConfigs[row].second;

    QSettings settings(path,QSettings::IniFormat);

    name = settings.value("subjectName").value<QString>();

    QStringList keys = settings.allKeys();

    for(auto &key: keys) {
        applicationSettings->setValue(key, settings.value(key));
    }

    emit onSettingsChange();
    emit onSubjectChange(name);
}

// Saves a settings configuration based on the current application wide settings to a settings file
// Basically, all settings stored at this point in time in the application wide settings are saved, which may include settings not interesting for subjects
void SubjectSelectionDialog::saveSubjectSettings(int row) {

    QString name = subjectConfigs[row].first;
    QString path = subjectConfigs[row].second;

    QSettings settings(path,QSettings::IniFormat);

    settings.setValue("subjectName", name);

    QStringList keys = applicationSettings->allKeys();

    for(auto &key: keys) {
        settings.setValue(key, applicationSettings->value(key));
    }

    settings.sync();

    emit onSubjectChange(name);
}

// Removes a subject entry from the table view and application setting
// This does not remove the actual settings file stored under the filepath
// To restore the entry one may add a new entry and select the existing file again through the file selection dialog
void SubjectSelectionDialog::removeSubject(int row) {

    QMessageBox msgBox;
    msgBox.setText("Only the subject's list entry will be removed.\nLeaving the settings file intact.");
    msgBox.exec();

    tableModel->removeRow(row);
    subjectConfigs.removeAt(row);

    saveSettings();
}

// On click of a contextmenu action
// The actions contain the clicked row under the data field
void SubjectSelectionDialog::onContextMenuClick(QAction *action) {
    int row = action->data().toInt();

    if(action->text() == SAVE_ACTION) {
        std::cout<<"Save Current Config "<<row<<std::endl;
        saveSubjectSettings(row);
    } else if(action->text() == LOAD_ACTION) {
        std::cout<<"Load subject "<<row<<std::endl;
        loadSubjectSettings(row);
    } else if(action->text() == DELETE_ACTION) {
        std::cout<<"remove subject "<<row<<std::endl;
        removeSubject(row);
    }
}
