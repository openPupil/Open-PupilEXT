#pragma once

/**
    @author Gábor Bényei
*/


#include <QtCore/QObject>
#include <QDialog>
#include <QtGui/QIntValidator>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QtWidgets>
#include "../devices/singleWebcam.h"
#include "singleCameraView.h" 


class SingleWebcamSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit SingleWebcamSettingsDialog(SingleWebcam *singleWebcam, QWidget *parent = nullptr);

    ~SingleWebcamSettingsDialog() override;

    void accept() override;
    void setLimitationsWhileTracking(bool state); // GB added

protected:

    void reject() override;

private:

    SingleWebcam *singleWebcam;

    QDir settingsDirectory;
    QSettings *applicationSettings;

    /*
    qDebug() << grabberDummy->getResizeFactor();
    qDebug() << grabberDummy->getFPSValue();
    qDebug() << grabberDummy->getBrightnessValue();
    qDebug() << grabberDummy->getContrastValue();
    qDebug() << grabberDummy->getGainValue();
    qDebug() << grabberDummy->getExposureValue();
*/

    QSpinBox *fpsInputBox;
    QDoubleSpinBox *brightnessInputBox;
    QDoubleSpinBox *contrastInputBox;
    QDoubleSpinBox *gainInputBox;
    QDoubleSpinBox *exposureInputBox;
    QDoubleSpinBox *resizeInputBox;

    void createForm();
    void updateForms();
    void loadSettings();
    void saveSettings();

private slots:

    void onSettingsChange();

signals:

    void onResizeFactorChanged(float value);
};

