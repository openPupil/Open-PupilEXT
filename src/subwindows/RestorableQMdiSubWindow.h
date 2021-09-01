#ifndef PUPILEXT_RESTORABLEQMDISUBWINDOW_H
#define PUPILEXT_RESTORABLEQMDISUBWINDOW_H

#include <QtWidgets>
#include <QtWidgets/QMdiSubWindow>
#include <QtCore/QSettings>
#include <iostream>

/**
    A helper class that enables the saving and restoring of the window geometry to the application settings.

    All subwindows in pupilext for which the geometry should be saved (non-dialogs) should inherit this class.

    Based on the given window identifier, the geometry is loaded from the appliation settings, and saved when the window is closed.
*/
class RestorableQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT

public:

    explicit RestorableQMdiSubWindow(QWidget *child, const QString &windowID, QWidget *parent=0) :
            QMdiSubWindow(parent),
            windowID(windowID),
            applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), this)) {

        setWidget(child);
        setAttribute(Qt::WA_DeleteOnClose);
    }

    void restoreGeometry() {
        const QRect geometry = applicationSettings->value(windowID+".geometry", QRect()).toRect();

        if(!geometry.isEmpty()) {
            setGeometry(geometry);
        }
    }

    void saveGeometry() {
        applicationSettings->setValue(windowID+".geometry", geometry());
    }

    void resetGeometry() {

        QSize hint = sizeHint();
        setGeometry(QRect(QPoint(0, 0), hint));
    }

protected:

    void closeEvent(QCloseEvent *event) override {
        saveGeometry();

        emit onCloseSubWindow();

        QMdiSubWindow::closeEvent(event);
    }

private:

    QString windowID;
    QSettings *applicationSettings;


signals:

    void onCloseSubWindow();

};


#endif //PUPILEXT_RESTORABLEQMDISUBWINDOW_H
