
#include <QtWidgets>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/QSpinBox>
#include <iostream>
#include "generalSettingsDialog.h"

// Create a settings dialog for the general software settings
// Settings are read upon creation from the QT application settings if existing
GeneralSettingsDialog::GeneralSettingsDialog(QWidget *parent) :
        QDialog(parent),
        playbackSpeed(30),
        writerFormat("tiff"),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setMinimumSize(200, 330);
    this->setWindowTitle("Settings");

    readSettings();
    createForm();

    connect(playbackSpeedInputBox, SIGNAL(valueChanged(int)), this, SLOT(setPlaybackSpeed(int)));
    connect(playbackLoopBox, SIGNAL(stateChanged(int)), this, SLOT(setPlaybackLoop(int)));

    connect(formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatChange(int)));

    connect(applyButton, &QPushButton::clicked, this, &GeneralSettingsDialog::apply);
    connect(cancelButton, &QPushButton::clicked, this, &GeneralSettingsDialog::cancel);
}

// Reads the settings from the QT application setting, if the entries were found
void GeneralSettingsDialog::readSettings() {

    const QByteArray m_playbackSpeed = applicationSettings->value("playbackSpeed", QByteArray()).toByteArray();

    if (!m_playbackSpeed.isEmpty()) {
        playbackSpeed = m_playbackSpeed.toInt();
    }

    const QByteArray m_playbackLoop = applicationSettings->value("playbackLoop", QByteArray()).toByteArray();

    if (!m_playbackLoop.isEmpty()) {
        playbackLoop = (bool) m_playbackLoop.toInt();
    }

    const QString m_writerFormat = applicationSettings->value("writerFormat", QByteArray()).toString();

    if (!m_writerFormat.isEmpty()) {
        writerFormat = m_writerFormat;
    }
}

void GeneralSettingsDialog::updateForm() {

    playbackSpeedInputBox->setValue(playbackSpeed);
    formatBox->setCurrentText(writerFormat);
}

// Saved the settings selected in the dialog to the QT application settings
void GeneralSettingsDialog::saveSettings() {

    applicationSettings->setValue("playbackSpeed", playbackSpeed);
    applicationSettings->setValue("playbackLoop", playbackLoop);

    applicationSettings->setValue("writerFormat", writerFormat);
}

void GeneralSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *writerGroup = new QGroupBox("Image Writer");
    QFormLayout *writerLayout = new QFormLayout;

    QLabel *formatLabel = new QLabel(tr("Image Format"));

    formatBox = new QComboBox();
    formatBox->addItem(QString("tiff [very CPU heavy]"), QString("tiff"));
    formatBox->addItem(QString("jpg [CPU heavy]"), QString("jpg"));
    formatBox->addItem(QString("bmp [fastest]"), QString("bmp"));
    formatBox->setCurrentText(writerFormat);

    writerLayout->addRow(formatLabel, formatBox);
    writerGroup->setLayout(writerLayout);
    mainLayout->addWidget(writerGroup);

    QGroupBox *playerGroup = new QGroupBox("Image Player");
    QFormLayout *playerLayout = new QFormLayout;

    QLabel *playbackSpeedLabel = new QLabel(tr("Playback Speed [fps]"));
    playbackSpeedLabel->setAccessibleDescription("Speed with which offline recordings are played.");
    playbackSpeedInputBox = new QSpinBox();
    playbackSpeedInputBox->setMinimum(0);
    playbackSpeedInputBox->setMaximum(999);
    playbackSpeedInputBox->setSingleStep(1);
    playbackSpeedInputBox->setValue(playbackSpeed);

    QLabel *playSpeedHintLabel = new QLabel(tr("Speed of 0 will make it play as fast as possible."));
    playSpeedHintLabel->setStyleSheet("color: gray;");

    playerLayout->addRow(playbackSpeedLabel, playbackSpeedInputBox);
    playerLayout->addRow(playSpeedHintLabel);

    QLabel *playbackLoopLabel = new QLabel(tr("Loop offline playback (infinite)"));
    playbackLoopBox = new QCheckBox();

    playerLayout->addRow(playbackLoopLabel, playbackLoopBox);

    playerGroup->setLayout(playerLayout);
    mainLayout->addWidget(playerGroup);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    applyButton = new QPushButton(tr("Apply and Close"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
}

// On apply button click
// Save the settings, send signal that settings have changed
void GeneralSettingsDialog::apply() {
    saveSettings();
    emit onSettingsChange();

    close();
}

// On cancel button click and discards the settings and close the dialog
// Reset the settings so on the next dialog open the saved settings will be loaded again
void GeneralSettingsDialog::cancel() {

    readSettings();
    updateForm();

    close();
}

// Returns the current playback speed setting
int GeneralSettingsDialog::getPlaybackSpeed() const {
    return playbackSpeed;
}

// Returns the setting if playback is looped infinitely
bool GeneralSettingsDialog::getPlaybackLoop() const {
    return playbackLoop;
}

// Returns the current writer format setting i.e. tiff, jpg, bmp
QString GeneralSettingsDialog::getWriterFormat() const {
    return writerFormat;
}

// Set the playback speed in frames per second
void GeneralSettingsDialog::setPlaybackSpeed(int m_playbackSpeed) {
    playbackSpeed = m_playbackSpeed;
}

// Set that playback is looped infinitely
void GeneralSettingsDialog::setPlaybackLoop(int m_state) {
    playbackLoop = (bool) m_state;
}

// Set the image writer format, all formats supported by OpenCV's imwrite can be specified
// Choices in the settings window are tiff, jpg, and bmp
void GeneralSettingsDialog::setWriterFormat(const QString &m_writerFormat) {
    writerFormat = m_writerFormat;
}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onFormatChange(int index) {
    writerFormat = formatBox->itemData(index).toString();
}

GeneralSettingsDialog::~GeneralSettingsDialog() = default;
