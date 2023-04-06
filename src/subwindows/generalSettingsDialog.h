#ifndef PUPILEXT_GENERALSETTINGSDIALOG_H
#define PUPILEXT_GENERALSETTINGSDIALOG_H

/**
    @author Moritz Lode, Gábor Bényei
*/

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QSettings>


/**
    General settings window (dialog) containing all general settings of the pupilext software

    NOTE: Modified by Gábor Bényei, 2023 jan
    GB NOTE:
        Playback speed and playback loop settings were moved to ImagePlaybackControlDialog.
        Other new settings are added.
*/
class GeneralSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit GeneralSettingsDialog(QWidget *parent = nullptr);
    ~GeneralSettingsDialog() override;

    int getPlaybackSpeed() const;
    bool getPlaybackLoop() const;

    QString getWriterFormat() const;

    // GB added begin
    bool getMetaSnapshotsEnabled() const;
    bool getSaveOfflineEventLog() const; 
    // GB added end

private:

    QSettings *applicationSettings;

    QString writerFormat;

    QPushButton *applyButton;
    QPushButton *cancelButton;

    QComboBox *formatBox;
    QSpinBox *playbackSpeedInputBox;
    QCheckBox *playbackLoopBox;

    // GB added begin
    bool metaSnapshotsEnabled;
    bool saveOfflineEventLog; 

    QString delimiterToUse;
    QComboBox *delimiterBox;
    QCheckBox *metaSnapshotBox;
    QCheckBox *saveOfflineEventLogBox;
    // GB added end

    void createForm();
    void saveSettings();
    void updateForm();

private slots:

    void apply();
    void cancel();
    void onFormatChange(int index);
    void readSettings();
    //void setPlaybackSpeed(int playbackSpeed);
    void setWriterFormat(const QString &writerFormat);
    //void setPlaybackLoop(int m_state);

    // GB added begin
    void onDelimiterChange(int index); 
    void setMetaSnapshotEnabled(int m_state);
    void setSaveOfflineEventLog(int m_state);
    // GB added end

signals:

    void onSettingsChange();

};


#endif //PUPILEXT_GENERALSETTINGSDIALOG_H
