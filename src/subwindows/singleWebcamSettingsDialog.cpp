
#include "singleWebcamSettingsDialog.h"

SingleWebcamSettingsDialog::SingleWebcamSettingsDialog(SingleWebcam *singleWebcam, QWidget *parent) :
        QDialog(parent),
        singleWebcam(singleWebcam),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    setMinimumSize(300, 220);

    setWindowTitle(QString("[%1] Camera Settings").arg(singleWebcam->getFriendlyName()));

    createForm();

    connect(gainInputBox, SIGNAL(valueChanged(double)), singleWebcam, SLOT(setGainValue(double)));

    loadSettings();
}

void SingleWebcamSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *adjGroup = new QGroupBox("Image adjustments");
    QFormLayout *adjLayout = new QFormLayout();

    
    QHBoxLayout *fpsInputLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel(tr("FPS:"));
    fpsInputBox = new QSpinBox();
    fpsInputBox->setMinimum(1);
    fpsInputBox->setMaximum(500);
    fpsInputBox->setSingleStep(1);
    fpsInputBox->setValue(30);

    fpsInputLayout->addWidget(fpsInputBox);
    adjLayout->addRow(fpsLabel, fpsInputLayout);

    
    QHBoxLayout *brightnessInputLayout = new QHBoxLayout();
    QLabel *brightnessLabel = new QLabel(tr("Brightness:"));
    brightnessInputBox = new QDoubleSpinBox();
    brightnessInputBox->setMinimum(-1000);
    brightnessInputBox->setMaximum(1000);
    brightnessInputBox->setSingleStep(1);

    brightnessInputLayout->addWidget(brightnessInputBox);
    adjLayout->addRow(brightnessLabel, brightnessInputLayout);

    
    QHBoxLayout *contrastInputLayout = new QHBoxLayout();
    QLabel *contrastLabel = new QLabel(tr("Contrast:"));
    contrastInputBox = new QDoubleSpinBox();
    contrastInputBox->setMinimum(0.01);
    contrastInputBox->setMaximum(1000);
    contrastInputBox->setSingleStep(1);

    contrastInputLayout->addWidget(contrastInputBox);
    adjLayout->addRow(contrastLabel, contrastInputLayout);

    
    QHBoxLayout *gainInputLayout = new QHBoxLayout();
    QLabel *gainLabel = new QLabel(tr("Gain:"));
    gainInputBox = new QDoubleSpinBox();
    gainInputBox->setMinimum(-10000);
    gainInputBox->setMaximum(10000);
    gainInputBox->setSingleStep(1);

    gainInputLayout->addWidget(gainInputBox);
    adjLayout->addRow(gainLabel, gainInputLayout);

    
    QHBoxLayout *exposureInputLayout = new QHBoxLayout();
    QLabel *exposureLabel = new QLabel(tr("Exposure:"));
    exposureInputBox = new QDoubleSpinBox();
    exposureInputBox->setMinimum(-500);
    exposureInputBox->setMaximum(500);
    exposureInputBox->setSingleStep(1);

    exposureInputLayout->addWidget(exposureInputBox);
    adjLayout->addRow(exposureLabel, exposureInputLayout);

    
    QFrame* sep1 = new QFrame;
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    adjLayout->addWidget(sep1);

    
    QHBoxLayout *resizeInputLayout = new QHBoxLayout();
    QLabel *resizeLabel = new QLabel(tr("Resize factor:"));
    resizeInputBox = new QDoubleSpinBox();
    resizeInputBox->setMinimum(0.05);
    resizeInputBox->setMaximum(1.0);
    resizeInputBox->setSingleStep(0.01);

    resizeInputLayout->addWidget(resizeInputBox);
    adjLayout->addRow(resizeLabel, resizeInputLayout);

    //

    adjGroup->setLayout(adjLayout);
    mainLayout->addWidget(adjGroup);

    setLayout(mainLayout);


    connect(fpsInputBox, SIGNAL(valueChanged(int)), singleWebcam, SLOT(setFPSValue(int)));
    connect(brightnessInputBox, SIGNAL(valueChanged(double)), singleWebcam, SLOT(setBrightnessValue(double)));
    connect(contrastInputBox, SIGNAL(valueChanged(double)), singleWebcam, SLOT(setContrastValue(double)));
    connect(gainInputBox, SIGNAL(valueChanged(double)), singleWebcam, SLOT(setGainValue(double)));
    connect(exposureInputBox, SIGNAL(valueChanged(double)), singleWebcam, SLOT(setExposureValue(double)));
    connect(resizeInputBox, SIGNAL(valueChanged(double)), singleWebcam, SLOT(setResizeFactor(double)));

}

// Instead of rejecting the dialog, thus closing it, we only hide it and show it again, so that all settings of the current camera are still in the forms
void SingleWebcamSettingsDialog::reject() {
    saveSettings();
    //QDialog::reject();
    hide();
}

void SingleWebcamSettingsDialog::accept() {
    saveSettings();
    QDialog::accept();
}

void SingleWebcamSettingsDialog::loadSettings() {
    fpsInputBox->setValue(applicationSettings->value("SingleWebcamSettingsDialog.fps", singleWebcam->getFPSValue()).toInt());
    singleWebcam->setFPSValue(fpsInputBox->value());

    brightnessInputBox->setValue(applicationSettings->value("SingleWebcamSettingsDialog.brightness", singleWebcam->getBrightnessValue()).toDouble());
    singleWebcam->setBrightnessValue(brightnessInputBox->value());

    contrastInputBox->setValue(applicationSettings->value("SingleWebcamSettingsDialog.contrast", singleWebcam->getContrastValue()).toDouble());
    singleWebcam->setContrastValue(contrastInputBox->value());

    gainInputBox->setValue(applicationSettings->value("SingleWebcamSettingsDialog.gain", singleWebcam->getGainValue()).toDouble());
    singleWebcam->setGainValue(gainInputBox->value());

    exposureInputBox->setValue(applicationSettings->value("SingleWebcamSettingsDialog.exposure", singleWebcam->getExposureValue()).toDouble());
    singleWebcam->setExposureValue(exposureInputBox->value());

    resizeInputBox->setValue(applicationSettings->value("SingleWebcamSettingsDialog.resizeFactor", singleWebcam->getResizeFactor()).toDouble());
    singleWebcam->setResizeFactor(resizeInputBox->value());
}

void SingleWebcamSettingsDialog::saveSettings() {
    applicationSettings->setValue("SingleWebcamSettingsDialog.fps", fpsInputBox->value());
    applicationSettings->setValue("SingleWebcamSettingsDialog.brightness", brightnessInputBox->value());
    applicationSettings->setValue("SingleWebcamSettingsDialog.contrast", contrastInputBox->value());
    applicationSettings->setValue("SingleWebcamSettingsDialog.gain", gainInputBox->value());
    applicationSettings->setValue("SingleWebcamSettingsDialog.exposure", exposureInputBox->value());
    applicationSettings->setValue("SingleWebcamSettingsDialog.resizeFactor", resizeInputBox->value());
}

void SingleWebcamSettingsDialog::onSettingsChange() {
    loadSettings();
}

SingleWebcamSettingsDialog::~SingleWebcamSettingsDialog() = default;

void SingleWebcamSettingsDialog::setLimitationsWhileTracking(bool state) {
    resizeInputBox->setDisabled(state);
}
