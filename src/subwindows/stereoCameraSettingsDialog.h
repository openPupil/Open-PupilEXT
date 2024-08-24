#pragma once

/**
    @author Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include <QtCore/qobjectdefs.h>
#include <QtWidgets/QDialog>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include "../devices/camera.h"
#include "../devices/stereoCamera.h"
#include "MCUSettingsDialog.h"
#include "../camImageRegionsWidget.h"
#include "../SVGIconColorAdjuster.h"
#include "stereoCameraView.h"

/**
    Custom widget for configuring a stereo camera setup, main and secondary camera are selected and opened, hardware trigger established and camera settings configured.

    The stereo camera settings is so designed that it setups the stereo camera in the correct order. To guarantee a synchronized stereo image recording, the cameras must be
    opened and started to fetch images BEFORE the hardware trigger is activated. Otherwise, opening and starting image fetching is executed in sequentially and the first
    camera fetches a frame before the second one.
*/
class StereoCameraSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit StereoCameraSettingsDialog(StereoCamera *cameraPtr, MCUSettingsDialog *MCUSettings, QWidget *parent = nullptr);

    ~StereoCameraSettingsDialog() override;

    void accept() override;
    void setCameraConfigurable(bool state);

protected:

    void reject() override;

private:

    StereoCamera *camera;

    Pylon::DeviceInfoList_t lstDevices;

    QDir settingsDirectory;
    QSettings *applicationSettings;

    MCUSettingsDialog *MCUSettings;

    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *autoGainOnceButton;
    QPushButton *autoExposureOnceButton;
    QPushButton *HWTstartStopButton;

    QPushButton *updateDevicesButton;
    QPushButton *cameraOpenCloseButton;

    QDoubleSpinBox *gainBox;
    QSpinBox *exposureInputBox;

    QComboBox *mainCameraBox;
    QComboBox *secondaryCameraBox;

    QLabel *frameRateValueLabel;
    QRadioButton *SWTradioButton;
    QCheckBox *SWTframerateEnabled;
    QSpinBox *SWTframerateBox;

    QPushButton *MCUConfigButton;
    QFormLayout *HWTgroupLayout;
    QLabel *HWTframerateLabel;
    QLabel *HWTlineSourceLabel;
    QLabel *HWTtimeSpanLabel;
    QComboBox *HWTlineSourceBox;
    bool HWTrunning = false;
    QRadioButton *HWTradioButton;
    QHBoxLayout *HWTframerateLayout;
    QSpinBox *HWTframerateBox;
    QDoubleSpinBox *HWTtimeSpanBox;

    QGroupBox *MCUConnGroup;
    QPushButton *MCUConnDisconnButton;

    QGroupBox *triggerGroup;
    QGroupBox *analogGroup;
    QGroupBox *acquisitionGroup;

    void createForm();
    void loadSettings();
    void saveSettings();

    QHBoxLayout *SWTframerateLayout;
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

    CamImageRegionsWidget *camImageRegionsWidget;

    int lastUsedBinningVal = 0;

public slots:
    void setLimitationsWhileTracking(bool state);
    void setLimitationsWhileCameraNotOpen(bool state);

    void updateImageROISettingsValues();
    void updateCamImageRegionsWidget();
    void updateSensorSize();

    void startHardwareTrigger();
    void stopHardwareTrigger();
    void setHWTlineSource(int lineSourceNum);
    void setHWTruntime(double runtimeMinutes);
    void setHWTframerate(int fps);

    void setExposureTimeValue(int value);
    void setGainValue(double value);

    void updateForms();

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

    void onSettingsChange();

    void onSetImageROIwidth(int val);
    void onSetImageROIheight(int val);
    void onSetImageROIoffsetX(int val);
    void onSetImageROIoffsetY(int val);
    void onBinningModeChange(int index);

    void mainCameraBoxCurrentIndexChanged(int);
    void secondaryCameraBoxCurrentIndexChanged(int);
    void HWTstartStopButtonClicked();
    void cameraOpenCloseButtonClicked();
    void MCUConnDisconnButtonClicked();

    void updateImageROISettingsMax();

    void updateHWTStartStopRelatedWidgets();
    void updateMCUConnDisconnButtonState();

public slots:
    void openStereoCamera(const QString &camName1, const QString &camName2);
    void connectMCU();
    void startHWT();

signals:
    void onMCUConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

    void onImageROIChanged(QRect rect);
    void onSensorSizeChanged(QSize size);

    void stereoCamerasOpened();
    void stereoCamerasClosed();

};
