#ifndef PUPILEXT_SINGLECAMERASETTINGSDIALOG_H
#define PUPILEXT_SINGLECAMERASETTINGSDIALOG_H

/**
    @author Moritz Lode
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
#include "../devices/singleCamera.h"
#include "serialSettingsDialog.h"

using namespace Pylon;

/**
    Settings widget/window for the configuration of a single camera (Basler)
*/
class SingleCameraSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit SingleCameraSettingsDialog(SingleCamera *singleCamera, SerialSettingsDialog *serialSetting, QWidget *parent = nullptr);

    ~SingleCameraSettingsDialog() override;

    void accept() override;

protected:

    void reject() override;

private:

    SingleCamera *singleCamera;

    QDir settingsDirectory;
    QSettings *applicationSettings;

    SerialSettingsDialog *serialSettings;

    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *gainAutoOnceButton;
    QPushButton *exposureAutoOnceButton;
    QPushButton *startHWButton;
    QPushButton *stopHWButton;

    QDoubleSpinBox *gainInputBox;
    QSpinBox *exposureInputBox;

    QLabel *frameRateValueLabel;
    QCheckBox *framerateEnabled;
    QSpinBox *framerateInputBox;

    QCheckBox *hwTriggerEnabled;
    QPushButton *serialConfigButton;
    QComboBox *lineSourceBox;

    QSpinBox *triggerFramerateInputBox;
    QDoubleSpinBox *triggerTimeSpanInputBox;

    void createForm();
    void updateForms();
    void loadSettings();
    void saveSettings();

private slots:

    void saveButtonClick();
    void loadButtonClick();

    void autoGainOnce();
    void autoExposureOnce();

    void onLineSourceChange(int index);
    void onSerialConnect();
    void onSerialDisconnect();

    void updateFrameRateValue();
    void startHardwareTrigger();
    void stopHardwareTrigger();

    void onHardwareTriggerCheckbox(bool value);

    void onSettingsChange();

signals:

    void onSerialConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

};


#endif //PUPILEXT_SINGLECAMERASETTINGSDIALOG_H
