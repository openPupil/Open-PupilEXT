#pragma once

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
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
#include "MCUSettingsDialog.h"
#include "../SVGIconColorAdjuster.h"
#include "../devices/singleWebcam.h"
#include "../camImageRegionsWidget.h"

using namespace Pylon;

/**
    Settings widget/window for the configuration of a single camera (Basler)
*/
class SingleCameraSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit SingleCameraSettingsDialog(SingleCamera *cameraPtr, MCUSettingsDialog *MCUSettings, QWidget *parent = nullptr);

    ~SingleCameraSettingsDialog() override;

    void accept() override;

protected:

    void reject() override;

private:

    SingleCamera *camera;

    QDir settingsDirectory;
    QSettings *applicationSettings;

    MCUSettingsDialog *MCUSettings;

    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *autoGainOnceButton;
    QPushButton *autoExposureOnceButton;
    QPushButton *HWTstartStopButton;

    QDoubleSpinBox *gainBox;
    QSpinBox *exposureInputBox;

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

    void updateImageROISettingsValues();
    void updateCamImageRegionsWidget();
    void updateSensorSize();

    void startHardwareTrigger();
    void stopHardwareTrigger();
    void setHWTlineSource(int lineSourceNum);
    void setHWTruntime(double runtimeMinutes);
    void setHWTframerate(int fps);

    void setAcquisitionFPSValue(int value);

    void setExposureTimeValue(int value);
    void setGainValue(double value);

    void updateForms();

private slots:

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

    void HWTstartStopButtonClicked();
    void MCUConnDisconnButtonClicked();

    void updateImageROISettingsMax();

    void updateHWTStartStopRelatedWidgets();
    void updateMCUConnDisconnButtonState();

public slots:
    void connectMCU();
    void startHWT();

    void SWTframerateEnabledToggled(bool state);
    void onHWTenabledChange(bool state);

signals:
    void onMCUConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

    void onImageROIChanged(QRect rect);
    void onSensorSizeChanged(QSize size);

};
