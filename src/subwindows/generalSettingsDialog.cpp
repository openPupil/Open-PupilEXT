
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
        imageWriterFormat("tiff"),
        imageWriterDataRule("ask"),
        delimiterToUse(","),
        dataWriterDataRule("ask"),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    //this->setMinimumSize(200, 330); 
    this->setMinimumSize(290, 370);
    this->setWindowTitle("Settings");

    readSettings();
    createForm();

    // GB added/modified begin
    updateForm();

    connect(imageWriterFormatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageWriterFormatChange(int)));
    connect(imageWriterDataRuleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageWriterDataRuleChange(int)));

    connect(delimiterBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDelimiterChange(int)));
    connect(dataWriterDataRuleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataWriterDataRuleChange(int)));

    connect(darkAdaptBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDarkAdaptChange(int)));
    connect(metaSnapshotBox, SIGNAL(stateChanged(int)), this, SLOT(setMetaSnapshotEnabled(int)));
    connect(saveOfflineEventLogBox, SIGNAL(stateChanged(int)), this, SLOT(setSaveOfflineEventLog(int)));
    // GB added/modified end

    connect(applyButton, &QPushButton::clicked, this, &GeneralSettingsDialog::apply);
    connect(cancelButton, &QPushButton::clicked, this, &GeneralSettingsDialog::cancel);
}

// Reads the settings from the QT application setting, if the entries were found
void GeneralSettingsDialog::readSettings() {
    const QString m_imageWriterFormat = applicationSettings->value("imageWriterFormat", QByteArray()).toString();
    if (!m_imageWriterFormat.isEmpty()) {
        imageWriterFormat = m_imageWriterFormat;
    }

    // GB begin
    const QString m_imageWriterDataRule = applicationSettings->value("imageWriterDataRule", QByteArray()).toString();
    if (!m_imageWriterDataRule.isEmpty()) {
        imageWriterDataRule = m_imageWriterDataRule;
    }

    const QString m_delimiterToUse = applicationSettings->value("delimiterToUse", "0").toString();
    if (!m_delimiterToUse.isEmpty()) {
        delimiterToUse = m_delimiterToUse;
    }

    const QString m_dataWriterDataRule = applicationSettings->value("dataWriterDataRule", QByteArray()).toString();
    if (!m_dataWriterDataRule.isEmpty()) {
        dataWriterDataRule = m_dataWriterDataRule;
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

    const int m_darkAdaptMode = applicationSettings->value("GUIDarkAdaptMode", "2").toInt();
    darkAdaptMode = m_darkAdaptMode;
    // GUIDarkAdaptMode: 0 = no, 1 = yes, 2 = let PupilEXT guess
        
    // GB end
}

void GeneralSettingsDialog::updateForm() {

    // GB modified begin
    // NOTE: thiw line below somehow does not work (always sets the index 0 element of the combobox)
    // imageWriterFormatBox->setCurrentText(imageWriterFormat);
    if(imageWriterFormat == "tiff")
        imageWriterFormatBox->setCurrentIndex(0);
    else if(imageWriterFormat == "jpg")
        imageWriterFormatBox->setCurrentIndex(1);
    else if(imageWriterFormat == "png")
        imageWriterFormatBox->setCurrentIndex(2);
    else if(imageWriterFormat == "bmp")
        imageWriterFormatBox->setCurrentIndex(3);
    //qDebug() << imageWriterFormat << "\n";

    if(imageWriterDataRule == "ask")
        imageWriterDataRuleBox->setCurrentIndex(0);
    else if(imageWriterDataRule == "append")
        imageWriterDataRuleBox->setCurrentIndex(1);
    else if(imageWriterDataRule == "new")
        imageWriterDataRuleBox->setCurrentIndex(2);

    //delimiterBox->setCurrentText(delimiterToUse);
    if(delimiterToUse == ",")
        delimiterBox->setCurrentIndex(0);
    else if(delimiterToUse == ";")
        delimiterBox->setCurrentIndex(1);
    else if(delimiterToUse == "\t")
        delimiterBox->setCurrentIndex(2);
    //qDebug() << delimiterToUse << "\n";

    if(dataWriterDataRule == "ask")
        dataWriterDataRuleBox->setCurrentIndex(0);
    else if(dataWriterDataRule == "append")
        dataWriterDataRuleBox->setCurrentIndex(1);
    else if(dataWriterDataRule == "new")
        dataWriterDataRuleBox->setCurrentIndex(2);

    darkAdaptBox->setCurrentIndex(darkAdaptMode);

    metaSnapshotBox->setChecked(metaSnapshotsEnabled);
    saveOfflineEventLogBox->setChecked(saveOfflineEventLog);
    // GB modified end
}

// Saved the settings selected in the dialog to the QT application settings
void GeneralSettingsDialog::saveSettings() {
    applicationSettings->setValue("imageWriterFormat", imageWriterFormat);
    // GB begin
    applicationSettings->setValue("imageWriterDataRule", imageWriterDataRule);
    applicationSettings->setValue("delimiterToUse", delimiterToUse );
    applicationSettings->setValue("dataWriterDataRule", dataWriterDataRule);
    applicationSettings->setValue("GUIDarkAdaptMode", darkAdaptMode );
    applicationSettings->setValue("metaSnapshotsEnabled", metaSnapshotsEnabled );
    applicationSettings->setValue("saveOfflineEventLog", saveOfflineEventLog );
    // GB end
}

void GeneralSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    dataWriterGroup = new QGroupBox("General Data Output");
    QFormLayout *dataOutLayout = new QFormLayout;

    QLabel *delimiterLabel = new QLabel(tr("Delimiter Character"));
    delimiterBox = new QComboBox();
    delimiterBox->addItem(QString("Comma [,]"), QString(","));
    delimiterBox->addItem(QString("Semicolon [;]"), QString(";"));
    delimiterBox->addItem(QString("Tabulation"), QString("\t"));
    delimiterBox->setCurrentText(delimiterToUse);
    dataOutLayout->addRow(delimiterLabel, delimiterBox);


    QLabel *dataWriterDataRuleLabel = new QLabel(tr("Action when output recording already exists:"));
    dataOutLayout->addRow(dataWriterDataRuleLabel);

    dataWriterDataRuleBox = new QComboBox();
    dataWriterDataRuleBox->addItem(QString("Ask every time"), QString("ask"));
    dataWriterDataRuleBox->addItem(QString("Append to found recording"), QString("append"));
    dataWriterDataRuleBox->addItem(QString("Keep existing and save new one too"), QString("new"));
    dataWriterDataRuleBox->setCurrentText(dataWriterDataRule);
    dataOutLayout->addRow(dataWriterDataRuleBox);

    metaSnapshotBox = new QCheckBox("Generate metadata snapshot files");
    metaSnapshotBox->setChecked(getMetaSnapshotsEnabled());
    dataOutLayout->addRow(metaSnapshotBox);

    dataWriterGroup->setLayout(dataOutLayout);
    mainLayout->addWidget(dataWriterGroup);



    imageWriterGroup = new QGroupBox("Image Writer");
    QFormLayout *writerLayout = new QFormLayout;

    QLabel *formatLabel = new QLabel(tr("Image Format"));

    imageWriterFormatBox = new QComboBox();
    imageWriterFormatBox->addItem(QString("tiff [very CPU heavy]"), QString("tiff"));
    imageWriterFormatBox->addItem(QString("jpg [CPU heavy]"), QString("jpg"));
    imageWriterFormatBox->addItem(QString("png [CPU heavy]"), QString("png"));
    imageWriterFormatBox->addItem(QString("bmp [fastest]"), QString("bmp"));
    imageWriterFormatBox->setCurrentText(imageWriterFormat);
    writerLayout->addRow(formatLabel, imageWriterFormatBox);


    QLabel *imageWriterDataRuleLabel = new QLabel(tr("Action when output recording already exists:"));
    writerLayout->addRow(imageWriterDataRuleLabel);

    imageWriterDataRuleBox = new QComboBox();
    imageWriterDataRuleBox->addItem(QString("Ask every time"), QString("ask"));
    imageWriterDataRuleBox->addItem(QString("Append to found recording"), QString("append"));
    imageWriterDataRuleBox->addItem(QString("Keep existing and save new one too"), QString("new"));
    imageWriterDataRuleBox->setCurrentText(imageWriterDataRule);
    writerLayout->addRow(imageWriterDataRuleBox);

    saveOfflineEventLogBox = new QCheckBox("Save trials/event log for offline analyses");
    saveOfflineEventLogBox->setChecked(getSaveOfflineEventLog());
    writerLayout->addRow(saveOfflineEventLogBox);

    imageWriterGroup->setLayout(writerLayout);
    mainLayout->addWidget(imageWriterGroup);

    // GB NOTE: removed playback speed and playback loop settings, as these are yet in ImagePlaybackSettingsDialog

    QGroupBox *appearanceGroup = new QGroupBox("Appearance");
    QFormLayout *appearanceLayout = new QFormLayout;

    QLabel *darkAdaptLabel = new QLabel(tr("GUI dark mode:"));

    darkAdaptBox = new QComboBox();
    darkAdaptBox->addItem(QString("Light"));
    darkAdaptBox->addItem(QString("Dark"));
    darkAdaptBox->addItem(QString("Auto-detect"));
    darkAdaptBox->setCurrentIndex(darkAdaptMode);
    appearanceLayout->addRow(darkAdaptLabel, darkAdaptBox);

    // TODO: add "always on top" checkbox

    appearanceGroup->setLayout(appearanceLayout);
    mainLayout->addWidget(appearanceGroup);



    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    applyButton = new QPushButton(tr("Apply and Close"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
}

void GeneralSettingsDialog::setLimitationsWhileImageWriting(bool state) {
    readSettings();
    updateForm();
    imageWriterGroup->setDisabled(state);
}

void GeneralSettingsDialog::setLimitationsWhileDataWriting(bool state) {
    readSettings();
    updateForm();
    dataWriterGroup->setDisabled(state);
}

void GeneralSettingsDialog::open() {
    onSettingsChangedElsewhere();
}

void GeneralSettingsDialog::onSettingsChangedElsewhere() {
    readSettings();
    updateForm();
}

// On apply button click
// Save the settings, send signal that settings have changed
void GeneralSettingsDialog::apply() {
    saveSettings();
    emit onSettingsChange();
    
    // this->parentWidget()->repaint(); // todo: maybe do this via receiving onSettingChange from mainwindow?

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
QString GeneralSettingsDialog::getImageWriterFormat() const {
    return imageWriterFormat;
}

QString GeneralSettingsDialog::getImageWriterDataRule() const {
    return imageWriterDataRule;
}

QString GeneralSettingsDialog::getDataWriterDataRule() const {
    return dataWriterDataRule;
}

void GeneralSettingsDialog::setMetaSnapshotEnabled(int m_state) {
    metaSnapshotsEnabled = (bool) m_state;
}
void GeneralSettingsDialog::setSaveOfflineEventLog(int m_state) {
    saveOfflineEventLog = (bool) m_state;
}

// Set the image writer format, all formats supported by OpenCV's imwrite can be specified
// Choices in the settings window are tiff, jpg, and bmp
void GeneralSettingsDialog::setImageWriterFormat(const QString &m_imageWriterFormat) {
    imageWriterFormat = m_imageWriterFormat;
}

void GeneralSettingsDialog::setImageWriterDataRule(const QString &m_imageWriterDataRule) {
    imageWriterDataRule = m_imageWriterDataRule;
}

void GeneralSettingsDialog::setDataWriterDataRule(const QString &m_dataWriterDataRule) {
    dataWriterDataRule = m_dataWriterDataRule;
}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onImageWriterFormatChange(int index) {
    imageWriterFormat = imageWriterFormatBox->itemData(index).toString();
}

void GeneralSettingsDialog::onImageWriterDataRuleChange(int index) {
    imageWriterDataRule = imageWriterDataRuleBox->itemData(index).toString();
}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onDelimiterChange(int index) {
    delimiterToUse = delimiterBox->itemData(index).toString();
}

void GeneralSettingsDialog::onDataWriterDataRuleChange(int index) {
    dataWriterDataRule = dataWriterDataRuleBox->itemData(index).toString();
}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onDarkAdaptChange(int index) {
    darkAdaptMode = index;
}

GeneralSettingsDialog::~GeneralSettingsDialog() = default;
