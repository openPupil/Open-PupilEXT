
#include <QtWidgets>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/QSpinBox>
#include <iostream>
#include "generalSettingsDialog.h"
#include "../supportFunctions.h"

// Create a settings dialog for the general software settings
// Settings are read upon creation from the QT application settings if existing
GeneralSettingsDialog::GeneralSettingsDialog(QWidget *parent) :
        QDialog(parent),
        //playbackSpeed(30),
        imageWriterFormat("tiff"),
        imageWriterDataRule("ask"),
        dataWriterDelimiter(","),
        dataWriterDataRule("ask"),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    //this->setMinimumSize(200, 330); 
    this->setMinimumSize(380, 580);
    this->setWindowTitle("Settings");

    readSettings();
    createForm();

    updateForm();

    connect(imageWriterFormatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageWriterFormatChange(int)));
    connect(imageWriterDataRuleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageWriterDataRuleChange(int)));

    connect(formatPngCompressionBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageWriterFormatPngCompressionChange(int)));
    connect(formatJpegQualityBox, SIGNAL(valueChanged(int)), this, SLOT(onImageWriterFormatJpegQualityChange(int)));
    connect(formatWebpQualityBox, SIGNAL(valueChanged(int)), this, SLOT(onImageWriterFormatWebpQualityChange(int)));

    connect(dataWriterDelimiterBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataWriterDelimiterChange(int)));
    connect(dataWriterDataStyleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataWriterDataStyleChange(int)));
    connect(dataWriterDataRuleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataWriterDataRuleChange(int)));

    connect(darkAdaptBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDarkAdaptChange(int)));
    connect(metaSnapshotBox, SIGNAL(stateChanged(int)), this, SLOT(setMetaSnapshotEnabled(int)));
    connect(saveOfflineEventLogBox, SIGNAL(stateChanged(int)), this, SLOT(setSaveOfflineEventLog(int)));
    connect(alwaysOnTopBox, SIGNAL(stateChanged(int)), this, SLOT(setAlwaysOnTop(int)));

    connect(applyButton, &QPushButton::clicked, this, &GeneralSettingsDialog::apply);
    connect(cancelButton, &QPushButton::clicked, this, &GeneralSettingsDialog::cancel);
}

// Reads the settings from the QT application setting, if the entries were found
void GeneralSettingsDialog::readSettings() {
    const QString m_imageWriterFormat = applicationSettings->value("imageWriterFormat.chosenFormat", "tiff").toString();
    if (!m_imageWriterFormat.isEmpty()) {
        imageWriterFormat = m_imageWriterFormat;
    }

    imageWriterFormatPngCompression = applicationSettings->value("imageWriterFormat.png.compression", "0").toInt();
    imageWriterFormatJpegQuality = applicationSettings->value("imageWriterFormat.jpeg.quality", "100").toInt();
    imageWriterFormatWebpQuality = applicationSettings->value("imageWriterFormat.webp.quality", "100").toInt();

    const QString m_imageWriterDataRule = applicationSettings->value("imageWriterDataRule", QByteArray()).toString();
    if (!m_imageWriterDataRule.isEmpty()) {
        imageWriterDataRule = m_imageWriterDataRule;
    }

    const QString m_dataWriterDelimiter = applicationSettings->value("dataWriterDelimiter", ",").toString();
    if (!m_dataWriterDelimiter.isEmpty()) {
        dataWriterDelimiter = m_dataWriterDelimiter;
    }

    const QString m_dataWriterDataStyle = applicationSettings->value("dataWriterDataStyle", "PupilEXT-0-1-2").toString();
    if (!m_dataWriterDataStyle.isEmpty()) {
        dataWriterDataStyle = m_dataWriterDataStyle;
    }

    const QString m_dataWriterDataRule = applicationSettings->value("dataWriterDataRule", QByteArray()).toString();
    if (!m_dataWriterDataRule.isEmpty()) {
        dataWriterDataRule = m_dataWriterDataRule;
    }

    metaSnapshotsEnabled = SupportFunctions::readBoolFromQSettings("metaSnapshotsEnabled", true, applicationSettings);
    saveOfflineEventLog = SupportFunctions::readBoolFromQSettings("saveOfflineEventLog", true, applicationSettings);
    alwaysOnTop = SupportFunctions::readBoolFromQSettings("alwaysOnTop", false, applicationSettings);

    darkAdaptMode = applicationSettings->value("GUIDarkAdaptMode", "2").toInt();
    // GUIDarkAdaptMode: 0 = no, 1 = yes, 2 = let PupilEXT guess

}

void GeneralSettingsDialog::updateForm() {

    imageWriterFormatBox->setCurrentIndex(imageWriterFormatBox->findData(imageWriterFormat));

    formatPngCompressionBox->setCurrentIndex(imageWriterFormatPngCompression);
    formatJpegQualityBox->setValue(imageWriterFormatJpegQuality);
    formatWebpQualityBox->setValue(imageWriterFormatWebpQuality);

    formatPngCompressionWidget->setVisible(imageWriterFormat == "png");
    formatJpegQualityWidget->setVisible(imageWriterFormat == "jpeg");
    formatWebpQualityWidget->setVisible(imageWriterFormat == "webp");

    if(imageWriterDataRule == "ask")
        imageWriterDataRuleBox->setCurrentIndex(0);
    else if(imageWriterDataRule == "append")
        imageWriterDataRuleBox->setCurrentIndex(1);
    else // if(imageWriterDataRule == "new")
        imageWriterDataRuleBox->setCurrentIndex(2);

    //dataWriterDelimiterBox->setCurrentText(delimiterToUse);
    if(dataWriterDelimiter == ";")
        dataWriterDelimiterBox->setCurrentIndex(1);
    else if(dataWriterDelimiter == "\t")
        dataWriterDelimiterBox->setCurrentIndex(2);
    else //if(dataWriterDelimiter == ",")
        dataWriterDelimiterBox->setCurrentIndex(0);
//    qDebug() << "Data writer delimiter read as: " << dataWriterDelimiter << "\n";

    if(dataWriterDataStyle == "PupilEXT-0-1-1")
        dataWriterDataStyleBox->setCurrentIndex(0);
    else // if(dataWriterDataStyle == "PupilEXT-0-1-2")
        dataWriterDataStyleBox->setCurrentIndex(1);

    if(dataWriterDataRule == "ask")
        dataWriterDataRuleBox->setCurrentIndex(0);
    else if(dataWriterDataRule == "append")
        dataWriterDataRuleBox->setCurrentIndex(1);
    else // if(dataWriterDataRule == "new")
        dataWriterDataRuleBox->setCurrentIndex(2);

    darkAdaptBox->setCurrentIndex(darkAdaptMode);

    metaSnapshotBox->setChecked(metaSnapshotsEnabled);
    saveOfflineEventLogBox->setChecked(saveOfflineEventLog);
    alwaysOnTopBox->setChecked(alwaysOnTop);
}

// Saved the settings selected in the dialog to the QT application settings
void GeneralSettingsDialog::saveSettings() {
    applicationSettings->setValue("imageWriterFormat.chosenFormat", imageWriterFormat);
    applicationSettings->setValue("imageWriterFormat.png.compression", imageWriterFormatPngCompression);
    applicationSettings->setValue("imageWriterFormat.jpeg.quality", imageWriterFormatJpegQuality);
    applicationSettings->setValue("imageWriterFormat.webp.quality", imageWriterFormatWebpQuality);
    applicationSettings->setValue("imageWriterDataRule", imageWriterDataRule);
    applicationSettings->setValue("dataWriterDelimiter", dataWriterDelimiter );
    applicationSettings->setValue("dataWriterDataStyle", dataWriterDataStyle );
    applicationSettings->setValue("dataWriterDataRule", dataWriterDataRule);
    applicationSettings->setValue("GUIDarkAdaptMode", darkAdaptMode );
    applicationSettings->setValue("metaSnapshotsEnabled", metaSnapshotsEnabled );
    applicationSettings->setValue("saveOfflineEventLog", saveOfflineEventLog );
    applicationSettings->setValue("alwaysOnTop", alwaysOnTop );
}

void GeneralSettingsDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout();


    dataWriterGroup = new QGroupBox("General Data Output");
    QFormLayout *dataOutLayout = new QFormLayout();

    QLabel *dataWriterDelimiterLabel = new QLabel(tr("Delimiter Character"));
    dataWriterDelimiterBox = new QComboBox();
    dataWriterDelimiterBox->addItem(QString("Comma [,]"), QString(","));
    dataWriterDelimiterBox->addItem(QString("Semicolon [;]"), QString(";"));
    dataWriterDelimiterBox->addItem(QString("Tabulation"), QString("\t"));
    dataWriterDelimiterBox->setCurrentText(dataWriterDelimiter);
    dataOutLayout->addRow(dataWriterDelimiterLabel, dataWriterDelimiterBox);

    QLabel *dataWriterDataStyleLabel = new QLabel(tr("Data Style*: "));
    dataWriterDataStyleBox = new QComboBox();
    dataWriterDataStyleBox->addItem(QString("PupilEXT v0.1.1"), QString("PupilEXT-0-1-1"));
    dataWriterDataStyleBox->addItem(QString("PupilEXT v0.1.2"), QString("PupilEXT-0-1-2"));
    dataWriterDataStyleBox->setCurrentText(dataWriterDataStyle);
    dataOutLayout->addRow(dataWriterDataStyleLabel, dataWriterDataStyleBox);
    QLabel *dataWriterDataStyleWarnLabel = new QLabel(tr("*Older version will not save trial numbering."));
    SupportFunctions::setSmallerLabelFontSize(dataWriterDataStyleWarnLabel);
    dataWriterDataStyleWarnLabel->setAlignment(Qt::AlignRight);
    dataOutLayout->addRow(dataWriterDataStyleWarnLabel);


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
    QFormLayout *writerLayout = new QFormLayout();

    QLabel *formatLabel = new QLabel(tr("Image Format**"));
    imageWriterFormatBox = new QComboBox();
    imageWriterFormatBox->addItem(QString("tiff [small files]"), QString("tiff"));
    imageWriterFormatBox->addItem(QString("png [configurable]"), QString("png"));
    imageWriterFormatBox->addItem(QString("bmp [large files]"), QString("bmp"));
    imageWriterFormatBox->addItem(QString("jpeg [configurable]"), QString("jpeg"));
    imageWriterFormatBox->addItem(QString("webp [configurable]"), QString("webp"));
    imageWriterFormatBox->addItem(QString("pgm"), QString("pgm"));
    int hahaha = imageWriterFormatBox->findData(imageWriterFormat);
    imageWriterFormatBox->setCurrentIndex(imageWriterFormatBox->findData(imageWriterFormat));
    writerLayout->addRow(formatLabel, imageWriterFormatBox);

    QLabel *formatNoteLabel = new QLabel(tr("**Please consider the file size vs. CPU load tradeoff!\nAlso, jpeg and webp can be lossy, thus not recommended."));
    SupportFunctions::setSmallerLabelFontSize(formatNoteLabel);
    writerLayout->addRow(formatNoteLabel);

    formatPngCompressionWidget = new QWidget();
    QHBoxLayout *formatPngCompressionLayout = new QHBoxLayout();
    formatPngCompressionLayout->setContentsMargins(0,0,0,0);
    QLabel *formatPngCompressionLabel = new QLabel(tr("PNG compression level:"));
    formatPngCompressionBox = new QComboBox();
    formatPngCompressionBox->addItem(QString("0 (large files, fast)"), 0);
    formatPngCompressionBox->addItem(QString("1"), 1);
    formatPngCompressionBox->addItem(QString("2"), 2);
    formatPngCompressionBox->addItem(QString("3"), 3);
    formatPngCompressionBox->addItem(QString("4"), 4);
    formatPngCompressionBox->addItem(QString("5"), 5);
    formatPngCompressionBox->addItem(QString("6"), 6);
    formatPngCompressionBox->addItem(QString("7"), 7);
    formatPngCompressionBox->addItem(QString("8"), 8);
    formatPngCompressionBox->addItem(QString("9 (small files, slow)"), 9);
    formatPngCompressionBox->setCurrentIndex(formatPngCompressionBox->findData(imageWriterFormatPngCompression));
    formatPngCompressionLayout->addWidget(formatPngCompressionLabel);
    formatPngCompressionLayout->addWidget(formatPngCompressionBox);
    formatPngCompressionWidget->setLayout(formatPngCompressionLayout);
    writerLayout->addRow(formatPngCompressionWidget);

    formatJpegQualityWidget = new QWidget();
    QHBoxLayout *formatJpegQualityLayout = new QHBoxLayout();
    formatJpegQualityLayout->setContentsMargins(0,0,0,0);
    QLabel *formatJpegQualityLabel = new QLabel(tr("JPEG quality:"));
    formatJpegQualityBox = new QSpinBox();
    formatJpegQualityBox->setMinimum(50);
    formatJpegQualityBox->setMaximum(100);
    formatJpegQualityBox->setSingleStep(1);
    formatJpegQualityBox->setValue(imageWriterFormatJpegQuality);
    formatJpegQualityLayout->addWidget(formatJpegQualityLabel);
    formatJpegQualityLayout->addWidget(formatJpegQualityBox);
    formatJpegQualityWidget->setLayout(formatJpegQualityLayout);
    writerLayout->addRow(formatJpegQualityWidget);

    formatWebpQualityWidget = new QWidget();
    QHBoxLayout *formatWebpQualityLayout = new QHBoxLayout();
    formatWebpQualityLayout->setContentsMargins(0,0,0,0);
    QLabel *formatWebpQualityLabel = new QLabel(tr("WEBP quality:"));
    formatWebpQualityBox = new QSpinBox();
    formatWebpQualityBox->setMinimum(50);
    formatWebpQualityBox->setMaximum(100);
    formatWebpQualityBox->setSingleStep(1);
    formatWebpQualityBox->setValue(imageWriterFormatWebpQuality);
    formatWebpQualityLayout->addWidget(formatWebpQualityLabel);
    formatWebpQualityLayout->addWidget(formatWebpQualityBox);
    formatWebpQualityWidget->setLayout(formatWebpQualityLayout);
    writerLayout->addRow(formatWebpQualityWidget);

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
    QFormLayout *appearanceLayout = new QFormLayout();

    QLabel *darkAdaptLabel = new QLabel(tr("GUI dark mode (needs restart):"));

    darkAdaptBox = new QComboBox();
    darkAdaptBox->addItem(QString("Light"));
    darkAdaptBox->addItem(QString("Dark"));
    darkAdaptBox->addItem(QString("Auto-detect"));
    darkAdaptBox->setCurrentIndex(darkAdaptMode);
    appearanceLayout->addRow(darkAdaptLabel, darkAdaptBox);

    alwaysOnTopBox = new QCheckBox("Keep application always on top (needs restart)");
    alwaysOnTopBox->setChecked(getAlwaysOnTop());
    appearanceLayout->addRow(alwaysOnTopBox);

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

    bool alwaysOnTopBeforeSave = SupportFunctions::readBoolFromQSettings("alwaysOnTop", false, applicationSettings);
    int darkAdaptModeBeforeSave = applicationSettings->value("GUIDarkAdaptMode", "2").toInt();

    saveSettings();
    emit onSettingsChange();
    
    // this->parentWidget()->repaint(); // todo: maybe do this via receiving onSettingChange from mainwindow?

    // If there is any settings change that may need application restart to take effect,
    // tell MainWindow to offer the user a restart in a dialog
    if((alwaysOnTopBeforeSave != alwaysOnTop) || (darkAdaptModeBeforeSave != darkAdaptMode)) {
        emit onSettingsChangeNeedingRestart();
    }

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
bool GeneralSettingsDialog::getAlwaysOnTop() const {
    return alwaysOnTop;
}


//// Returns the current writer format setting i.e. tiff, jpeg, bmp
//QString GeneralSettingsDialog::getImageWriterFormat() const {
//    return imageWriterFormat;
//}
//QString GeneralSettingsDialog::getImageWriterDataRule() const {
//    return imageWriterDataRule;
//}
//QString GeneralSettingsDialog::getDataWriterDataRule() const {
//    return dataWriterDataRule;
//}
//QString GeneralSettingsDialog::getDataWriterDataStyle() const {
//    return dataWriterDataStyle;
//}

void GeneralSettingsDialog::setMetaSnapshotEnabled(int m_state) {
    metaSnapshotsEnabled = (bool) m_state;
}
void GeneralSettingsDialog::setSaveOfflineEventLog(int m_state) {
    saveOfflineEventLog = (bool) m_state;
}
void GeneralSettingsDialog::setAlwaysOnTop(int m_state) {
    alwaysOnTop = (bool) m_state;
}

//// Set the image writer format, all formats supported by OpenCV's imwrite can be specified
//// Choices in the settings window are tiff, jpeg, and bmp
//void GeneralSettingsDialog::setImageWriterFormat(const QString &m_imageWriterFormat) {
//    imageWriterFormat = m_imageWriterFormat;
//}
//void GeneralSettingsDialog::setImageWriterDataRule(const QString &m_imageWriterDataRule) {
//    imageWriterDataRule = m_imageWriterDataRule;
//}
//void GeneralSettingsDialog::setDataWriterDataRule(const QString &m_dataWriterDataRule) {
//    dataWriterDataRule = m_dataWriterDataRule;
//}
//void GeneralSettingsDialog::setDataWriterDataStyle(const QString &m_dataWriterDataStyle) {
//    dataWriterDataStyle = m_dataWriterDataStyle;
//}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onImageWriterFormatChange(int index) {
    imageWriterFormat = imageWriterFormatBox->itemData(index).toString();

    formatPngCompressionWidget->setVisible(imageWriterFormat == "png");
    formatJpegQualityWidget->setVisible(imageWriterFormat == "jpeg");
    formatWebpQualityWidget->setVisible(imageWriterFormat == "webp");
}

void GeneralSettingsDialog::onImageWriterDataRuleChange(int index) {
    imageWriterDataRule = imageWriterDataRuleBox->itemData(index).toString();
}

void GeneralSettingsDialog::onImageWriterFormatPngCompressionChange(int index) {
    imageWriterFormatPngCompression = formatPngCompressionBox->itemData(index).toInt();
}

void GeneralSettingsDialog::onImageWriterFormatJpegQualityChange(int value) {
    imageWriterFormatJpegQuality = value;
}

void GeneralSettingsDialog::onImageWriterFormatWebpQualityChange(int value) {
    imageWriterFormatWebpQuality = value;
}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onDataWriterDelimiterChange(int index) {
    dataWriterDelimiter = dataWriterDelimiterBox->itemData(index).toString();
}

void GeneralSettingsDialog::onDataWriterDataStyleChange(int index) {
    dataWriterDataStyle = dataWriterDataStyleBox->itemData(index).toString();
}

void GeneralSettingsDialog::onDataWriterDataRuleChange(int index) {
    dataWriterDataRule = dataWriterDataRuleBox->itemData(index).toString();
}

// Event handler on the change of the combobox selection in the dialog
void GeneralSettingsDialog::onDarkAdaptChange(int index) {
    darkAdaptMode = index;
}

GeneralSettingsDialog::~GeneralSettingsDialog() = default;
