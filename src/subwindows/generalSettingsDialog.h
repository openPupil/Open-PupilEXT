#ifndef PUPILEXT_GENERALSETTINGSDIALOG_H
#define PUPILEXT_GENERALSETTINGSDIALOG_H

/**
    @author Moritz Lode
*/

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QSettings>


/**
    General settings window (dialog) containing all general settings of the pupilext software
*/
class GeneralSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit GeneralSettingsDialog(QWidget *parent = nullptr);
    ~GeneralSettingsDialog() override;

    int getPlaybackSpeed() const;
    bool getPlaybackLoop() const;

    QString getWriterFormat() const;


private:

    QSettings *applicationSettings;

    int playbackSpeed;
    bool playbackLoop;

    QString writerFormat;

    QPushButton *applyButton;
    QPushButton *cancelButton;

    QComboBox *formatBox;
    QSpinBox *playbackSpeedInputBox;
    QCheckBox *playbackLoopBox;

    void createForm();
    void saveSettings();
    void updateForm();

private slots:

    void apply();
    void cancel();
    void onFormatChange(int index);
    void readSettings();
    void setPlaybackSpeed(int playbackSpeed);
    void setWriterFormat(const QString &writerFormat);
    void setPlaybackLoop(int m_state);

signals:

    void onSettingsChange();

};


#endif //PUPILEXT_GENERALSETTINGSDIALOG_H
