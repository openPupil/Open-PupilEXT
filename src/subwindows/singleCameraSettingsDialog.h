#ifndef PUPILEXT_SINGLECAMERASETTINGSDIALOG_H
#define PUPILEXT_SINGLECAMERASETTINGSDIALOG_H

/**
    @author Moritz Lode, Gabor Benyei, Attila Boncser
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
#include "../SVGIconColorAdjuster.h"

// BG added
#include "../devices/singleWebcam.h"
#include "../camImageRegionsWidget.h"

using namespace Pylon;

/**
    Settings widget/window for the configuration of a single camera (Basler)
*/
class SingleCameraSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit SingleCameraSettingsDialog(SingleCamera *cameraPtr, SerialSettingsDialog *serialSetting, QWidget *parent = nullptr);

    ~SingleCameraSettingsDialog() override;

    void accept() override;

protected:

    void reject() override;

private:

    SingleCamera *camera;

    QDir settingsDirectory;
    QSettings *applicationSettings;

    SerialSettingsDialog *serialSettings;

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

    QPushButton *serialConfigButton;
    QFormLayout *HWTgroupLayout;
    QLabel *HWTframerateLabel;
    QLabel *HWTlineSourceLabel;
    QLabel *HWTtimeSpanLabel;
    QComboBox *HWTlineSourceBox;
    QLabel *serialConnDisconnButtonLabel;
    QLabel *HWTstartStopButtonLabel;
    bool HWTrunning = false;
    QRadioButton *HWTradioButton;
    QHBoxLayout *HWTframerateLayout;
    QSpinBox *HWTframerateBox;
    QDoubleSpinBox *HWTtimeSpanBox;

    QGroupBox *serialConnGroup;
    QPushButton *serialConnDisconnButton;

    QGroupBox *triggerGroup;
    QGroupBox *analogGroup;
    QGroupBox *acquisitionGroup;

    void createForm();
    void updateForms();
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

    void enableAcquisitionFrameRate(bool state);
    void setAcquisitionFPSValue(int value);

    void setExposureTimeValue(int value);
    void setGainValue(double value);

private slots:

    void saveButtonClick();
    void loadButtonClick();

    void autoGainOnce();
    void autoExposureOnce();

    void onLineSourceChange(int index);
    void updateFrameRateValue();

    void onHWTenabledChange(bool state);

    void onSettingsChange();

    void onSetImageROIwidth(int val);
    void onSetImageROIheight(int val);
    void onSetImageROIoffsetX(int val);
    void onSetImageROIoffsetY(int val);
    void onBinningModeChange(int index);

    void HWTstartStopButtonClicked();
    void serialConnDisconnButtonClicked();

    void updateImageROISettingsMax();

    void SWTframerateEnabledToggled(bool state);

signals:
    void onSerialConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

    void onImageROIChanged(QRect rect);
    void onSensorSizeChanged(QSize size);

};

#endif //PUPILEXT_SINGLECAMERASETTINGSDIALOG_H
