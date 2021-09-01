
#ifndef PUPILEXT_SUBJECTSELECTIONDIALOG_H
#define PUPILEXT_SUBJECTSELECTIONDIALOG_H

/**
    @author Moritz Lode
*/

#include <QtWidgets>
#include <QtWidgets/QDialog>


/**
    Widget showing a table view with subject entries which can be saved and load.

    Each subject entry represents a setting state of the application for a specific subject/experiment.

    With this widget, application wide settings can be saved and loaded including pupil detection settings and ROI selections.
*/
class SubjectSelectionDialog : public QDialog {
    Q_OBJECT

public:

    static const QString SAVE_ACTION;
    static const QString LOAD_ACTION;
    static const QString DELETE_ACTION;

    explicit SubjectSelectionDialog(QWidget *parent = nullptr);
    ~SubjectSelectionDialog() override;


protected:

    void reject() override;

private:

    QTableView *tableView;
    QStandardItemModel *tableModel;

    QMenu *tableContextMenu;

    QPushButton *addButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *removeButton;

    QSettings *applicationSettings;
    QString recentPath;

    QList<QPair<QString, QString>> subjectConfigs;

    void saveSettings();

    void loadSubjectSettings(int row);
    void saveSubjectSettings(int row);
    void removeSubject(int row);

private slots:

    void customMenuRequested(QPoint pos);
    void onContextMenuClick(QAction* action);

    void addButtonClick();

    void onCellChange(QStandardItem *item);
    void onCellDoubleClick(const QModelIndex &index);

signals:

    void onSettingsChange();
    void onSubjectChange(QString subject);

};


#endif //PUPILEXT_SUBJECTSELECTIONDIALOG_H
