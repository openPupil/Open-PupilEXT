
#ifndef PUPILEXT_PUPILMETHODSETTING_H
#define PUPILEXT_PUPILMETHODSETTING_H

/**
    @author Moritz Lode
*/

#include <QtWidgets/QWidget>

/**
    Abstract class representing the pupil detection algorithm's individual parameters, this widget will be integrated into the pupil detection settings configuration windows
*/
class PupilMethodSetting : public QWidget {
    Q_OBJECT

public:

    QWidget *infoBox;
    QSettings *applicationSettings;

    explicit PupilMethodSetting(QWidget *parent=0) : QWidget(parent),
             applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

        infoBox = new QWidget();
    }

    //virtual void addSecondary(PupilDetectionMethod *method) = 0;

public slots:

    virtual void loadSettings() {

    }

    virtual void updateSettings() {

    }

signals:

    void onConfigChange(QString configText);

};

#endif //PUPILEXT_PUPILMETHODSETTING_H
