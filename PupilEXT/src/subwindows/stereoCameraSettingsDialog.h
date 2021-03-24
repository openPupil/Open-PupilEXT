
#ifndef PUPILEXT_STEREOCAMERASETTINGSDIALOG_H
#define PUPILEXT_STEREOCAMERASETTINGSDIALOG_H

/**
    @author Moritz Lode
*/


#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QDialog>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include "../devices/camera.h"
#include "../devices/stereoCamera.h"
#include "serialSettingsDialog.h"

/**
    Custom widget for configuring a stereo camera setup, main and secondary camera are selected and opened, hardware trigger established and camera settings configured.

    The stereo camera settings is so designed that it setups the stereo camera in the correct order. To guarantee a synchronized stereo image recording, the cameras must be
    opened and started to fetch images BEFORE the hardware trigger is activated. Otherwise, opening and starting image fetching is executed in sequentially and the first
    camera fetches a frame before the second one.
*/
class StereoCameraSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit StereoCameraSettingsDialog(StereoCamera *camera, SerialSettingsDialog *serialSettings, QWidget *parent = nullptr);

    ~StereoCameraSettingsDialog() override;

    void accept() override;

protected:

    void reject() override;

private:

    StereoCamera *camera;

    Pylon::DeviceInfoList_t lstDevices;

    QDir settingsDirectory;
    QSettings *applicationSettings;

    SerialSettingsDialog *serialSettings;

    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *gainAutoOnceButton;
    QPushButton *exposureAutoOnceButton;
    QPushButton *startHWButton;
    QPushButton *stopHWButton;
    QPushButton *updateDevicesButton;
    QPushButton *openButton;
    QPushButton *closeButton;

    QDoubleSpinBox *gainInputBox;
    QSpinBox *exposureInputBox;

    QComboBox *mainCameraBox;
    QComboBox *secondaryCameraBox;

    QLabel *frameRateValueLabel;
    QCheckBox *framerateEnabled;
    QSpinBox *framerateInputBox;

    QPushButton *serialConfigButton;
    QComboBox *lineSourceBox;

    QSpinBox *triggerFramerateInputBox;
    QDoubleSpinBox *triggerTimeSpanInputBox;

    QGroupBox *hwTriggerGroup;
    QGroupBox *analogGroup;
    QGroupBox *acquisitionGroup;

    void createForm();
    void updateForms();
    void loadSettings();
    void saveSettings();

private slots:

    void updateDevicesBox();
    void openStereoCamera();
    void closeStereoCamera();

    void saveButtonClick();
    void loadButtonClick();

    void autoGainOnce();
    void autoExposureOnce();

    void onLineSourceChange(int index);
    void updateFrameRateValue();
    void startHardwareTrigger();
    void stopHardwareTrigger();

    void onSerialDisconnect();
    void onSerialConnect();

    void onSettingsChange();

signals:

    void onSerialConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

};


#endif //PUPILEXT_STEREOCAMERASETTINGSDIALOG_H
