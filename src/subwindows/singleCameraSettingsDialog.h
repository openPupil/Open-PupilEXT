#ifndef PUPILEXT_SINGLECAMERASETTINGSDIALOG_H
#define PUPILEXT_SINGLECAMERASETTINGSDIALOG_H

/**
    @author Moritz Lode, Gábor Bényei
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

    // BG added (+made global) begin
    QGroupBox *hwTriggerGroup;
    QGroupBox *analogGroup;
    QGroupBox *acquisitionGroup;

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

    CamImageRegionsWidget *camImageRegionsWidget;

    int lastUsedBinningVal = 0;
    //when binning value is 1. Must be divisible by 4 (camera dependent). 4*16
    const int minImageSize = 64; 
    //when binning value is 1. Must be divisible by 4 (camera dependent). 4*16
    const int imageSizeChangeSingleStep = 32;
    // BG added end

public slots:
    void setLimitationsWhileTracking(bool state); // GÁBOR

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

    // BG added begin
    void onSetImageROIwidth(int val);
    void onSetImageROIheight(int val);
    void onSetImageROIoffsetX(int val);
    void onSetImageROIoffsetY(int val);
    void onBinningModeChange(int index);

    void updateImageROISettingsMin(int binningVal);
    void updateImageROISettingsMax();
    void updateImageROISettingsValues();
    void updateCamImageRegionsWidget();

    void setExposureTimeValue(int value);
    void setAcquisitionFPSValue(int value);
    void enableAcquisitionFrameRate(bool value);
    // BG added end

signals:

    void onSerialConfig();
    void onHardwareTriggerStart(QString cmd);
    void onHardwareTriggerStop(QString cmd);

    void onHardwareTriggerEnable();
    void onHardwareTriggerDisable();

};

#endif //PUPILEXT_SINGLECAMERASETTINGSDIALOG_H
