
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
        //playbackSpeed(30),
        writerFormat("tiff"),
        delimiterToUse(","),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    //this->setMinimumSize(200, 330); 
    this->setMinimumSize(280, 330); 
    this->setWindowTitle("Settings");

    readSettings();
    createForm();

    // GB added/modified begin
    updateForm();

    connect(formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatChange(int)));

    connect(delimiterBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDelimiterChange(int)));
    connect(metaSnapshotBox, SIGNAL(stateChanged(int)), this, SLOT(setMetaSnapshotEnabled(int)));
    connect(saveOfflineEventLogBox, SIGNAL(stateChanged(int)), this, SLOT(setSaveOfflineEventLog(int)));
    // GB added/modified end

    connect(applyButton, &QPushButton::clicked, this, &GeneralSettingsDialog::apply);
    connect(cancelButton, &QPushButton::clicked, this, &GeneralSettingsDialog::cancel);
}

// Reads the settings from the QT application setting, if the entries were found
void GeneralSettingsDialog::readSettings() {
    const QString m_writerFormat = applicationSettings->value("writerFormat", QByteArray()).toString();
    if (!m_writerFormat.isEmpty()) {
        writerFormat = m_writerFormat;
    }

    // GB begin
    const QString m_delimiterToUse = applicationSettings->value("delimiterToUse", "0").toString();
    if (!m_delimiterToUse.isEmpty()) {
        delimiterToUse = m_delimiterToUse;
    }
    const QByteArray m_metaSnapshotsEnabled = applicationSettings->value("metaSnapshotsEnabled", "1").toByteArray();
    //std::cout << m_metaSnapshotsEnabled.toStdString() << std::endl; //
    if (!m_metaSnapshotsEnabled.isEmpty()) {
        if(m_metaSnapshotsEnabled == "1" || m_metaSnapshotsEnabled == "true")
            metaSnapshotsEnabled = true;
        else
            metaSnapshotsEnabled = false;
    }
    const QByteArray m_saveOfflineEventLog = applicationSettings->value("saveOfflineEventLog", "1").toByteArray();
    //std::cout << m_saveOfflineEventLog.toStdString() << std::endl; //
    if (!m_saveOfflineEventLog.isEmpty()) {
        if(m_saveOfflineEventLog == "1" || m_saveOfflineEventLog == "true")
            saveOfflineEventLog = true;
        else
            saveOfflineEventLog = false;
    }
    // GB end
}

void GeneralSettingsDialog::updateForm() {

    // GB modified begin
    // NOTE: thiw line below somehow does not work (always sets the index 0 element of the combobox)
    // formatBox->setCurrentText(writerFormat); 
    if(writerFormat == "tiff")
        formatBox->setCurrentIndex(0);
    else if(writerFormat == "jpg")
        formatBox->setCurrentIndex(1);
    else if(writerFormat == "png")
        formatBox->setCurrentIndex(2);
    //qDebug() << writerFormat << "\n";

    //delimiterBox->setCurrentText(delimiterToUse);
    if(delimiterToUse == ",")
        delimiterBox->setCurrentIndex(0);
    else if(delimiterToUse == ";")
        delimiterBox->setCurrentIndex(1);
    else if(delimiterToUse == "\t")
        delimiterBox->setCurrentIndex(2);
    //qDebug() << delimiterToUse << "\n";

    metaSnapshotBox->setChecked(metaSnapshotsEnabled);
    saveOfflineEventLogBox->setChecked(saveOfflineEventLog);
    // GB modified end
}

// Saved the settings selected in the dialog to the QT application settings
void GeneralSettingsDialog::saveSettings() {
    applicationSettings->setValue("writerFormat", writerFormat);
    // GB begin
    applicationSettings->setValue("delimiterToUse", delimiterToUse );
    applicationSettings->setValue("metaSnapshotsEnabled", metaSnapshotsEnabled );
    applicationSettings->setValue("saveOfflineEventLog", saveOfflineEventLog );
    // GB end
}

void GeneralSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    QGroupBox *dataOutGroup = new QGroupBox("General Data Output");
    QFormLayout *dataOutLayout = new QFormLayout;

    QLabel *delimiterLabel = new QLabel(tr("Delimiter Character"));
    delimiterBox = new QComboBox();
    delimiterBox->addItem(QString("Comma [,]"), QString(","));
    delimiterBox->addItem(QString("Semicolon [;]"), QString(";"));
    delimiterBox->addItem(QString("Tabulation"), QString("\t"));
    delimiterBox->setCurrentText(delimiterToUse);
    dataOutLayout->addRow(delimiterLabel, delimiterBox);

    metaSnapshotBox = new QCheckBox("Generate metadata snapshot files");
    metaSnapshotBox->setChecked(getMetaSnapshotsEnabled());
    dataOutLayout->addRow(metaSnapshotBox);

    dataOutGroup->setLayout(dataOutLayout);
    mainLayout->addWidget(dataOutGroup);



    QGroupBox *writerGroup = new QGroupBox("Image Writer");
    QFormLayout *writerLayout = new QFormLayout;

    QLabel *formatLabel = new QLabel(tr("Image Format"));

    formatBox = new QComboBox();
    formatBox->addItem(QString("tiff [very CPU heavy]"), QString("tiff"));
    formatBox->addItem(QString("jpg [CPU heavy]"), QString("jpg"));
    formatBox->addItem(QString("bmp [fastest]"), QString("bmp"));
    formatBox->setCurrentText(writerFormat);
    writerLayout->addRow(formatLabel, formatBox);

    saveOfflineEventLogBox = new QCheckBox("Save trials/event log for offline analyses");
    saveOfflineEventLogBox->setChecked(getSaveOfflineEventLog());
    writerLayout->addRow(saveOfflineEventLogBox);

    writerGroup->setLayout(writerLayout);
    mainLayout->addWidget(writerGroup);

    // GB NOTE: removed playback speed and playback loop settings, as these are yet in ImagePlaybackSettingsDialog

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

bool GeneralSettingsDialog::getMetaSnapshotsEnabled() const {
    return metaSnapshotsEnabled;
}
bool GeneralSettingsDialog::getSaveOfflineEventLog() const {
    return saveOfflineEventLog;
}


// Returns the current writer format setting i.e. tiff, jpg, bmp
QString GeneralSettingsDialog::getWriterFormat() const {
    return writerFormat;
}

void GeneralSettingsDialog::setMetaSnapshotEnabled(int m_state) {
    metaSnapshotsEnabled = (bool) m_state;
}
void GeneralSettingsDialog::setSaveOfflineEventLog(int m_state) {
    saveOfflineEventLog = (bool) m_state;
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

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onDelimiterChange(int index) {
    delimiterToUse = delimiterBox->itemData(index).toString();
}

GeneralSettingsDialog::~GeneralSettingsDialog() = default;
