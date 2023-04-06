
#ifndef PUPILEXT_STEREOCAMERASETTINGSDIALOG_H
#define PUPILEXT_STEREOCAMERASETTINGSDIALOG_H

/**
    @author Moritz Lode, Gábor Bényei
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

// BG added
#include "stereoCameraView.h" // BG NOTE: to connect with acquisition image ROI changes to view update

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
    void setCameraConfigurable(bool state); // GB added

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
    
    // BG added (+made global) begin
    QHBoxLayout *framerateInputLayout; 
    QLabel *frameRateLabel;
    QLabel *exposureLabel;

    QLabel *imageROIwidthLabel;
    QLabel *imageROIheightLabel;
    QLabel *imageROIoffsetXLabel;
    QLabel *imageROIoffsetYLabel;
    QLabel *binningLabel;

    QLabel *imageROIwidthMaxLabel;
    QLabel *imageROIheightMaxLabel;
    QLabel *imageROIoffsetXMaxLabel;
    QLabel *imageROIoffsetYMaxLabel;

    QSpinBox *imageROIwidthInputBox;
    QSpinBox *imageROIheightInputBox;
    QSpinBox *imageROIoffsetXInputBox;
    QSpinBox *imageROIoffsetYInputBox;
    QComboBox *binningBox;

    int lastUsedBinningVal = 0;
    //when binning value is 1. Must be divisible by 4 (camera dependent). 4*16
    const int minImageSize = 64; 
    //when binning value is 1. Must be divisible by 4 (camera dependent). 4*16
    const int imageSizeChangeSingleStep = 32;
    // BG added end

public slots:
    // GB added begin
    void setLimitationsWhileTracking(bool state);
    void setLimitationsWhileUnconnected(bool state);
    // GB added end

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

    // BG added begin
    void onSetImageROIwidth(int val);
    void onSetImageROIheight(int val);
    void onSetImageROIoffsetX(int val);
    void onSetImageROIoffsetY(int val);
    void onBinningModeChange(int index);

    void updateImageROISettingsMin(int binningVal);
    void updateImageROISettingsMax();
    void updateImageROISettingsValues();
    // BG added end

public slots:
    // GB added begin
    void openStereoCamera(const QString &camName1, const QString &camName2);
    // GB added end

signals:

    void onSerialConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

};


#endif //PUPILEXT_STEREOCAMERASETTINGSDIALOG_H
