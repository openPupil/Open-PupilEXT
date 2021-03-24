
#ifndef PUPILEXT_PUPILDETECTIONSETTINGSDIALOG_H
#define PUPILEXT_PUPILDETECTIONSETTINGSDIALOG_H

/**
    @author Moritz Lode
*/

#include <QDialog>
#include <QtWidgets/QComboBox>
#include <QtCore/qdir.h>
#include <QtWidgets/QPushButton>
#include "../pupilDetection.h"
#include "pupil-detection-methods/PupilMethodSetting.h"


/**
    Settings window (dialog) showing all pupil detection settings, including the algorithm respective parameters

    Allows the change of algorithm parameter and the selection of different algorithm parameter presets according to ROI sizes
    Further allows the loading of a parameter file to loading custom algorithm parameter

    All algorithm parameter specific things and its complete interface are defined in PupilMethodSetting interface and its implemented classes as each
    algorithm has different parameter settings.

*/
class PupilDetectionSettingsDialog : public QDialog {
    Q_OBJECT

public:

    explicit PupilDetectionSettingsDialog(PupilDetection *pupilDetection, QWidget *parent = nullptr);

    ~PupilDetectionSettingsDialog() override;

protected:

    void reject() override;

private:

    PupilDetection *pupilDetection;

    QSettings *applicationSettings;

    std::vector<PupilMethodSetting*> pupilMethodSettings;

    QPushButton *applyButton;
    QPushButton *cancelButton;
    QPushButton *applyCloseButton;

    QComboBox *algorithmBox;
    QCheckBox *outlineConfidenceBox;
    QCheckBox *roiPreprocessingBox;
    QCheckBox *pupilUndistortionBox;
    QCheckBox *imageUndistortionBox;

    void createForm();
    void updateForm();
    void loadSettings();
    void saveSettings();

private slots:

    void onAlgorithmSelection(int idx);
    void applyButtonClick();
    void cancelButtonClick();
    void applyCloseButtonClick();
    void onShowHelpDialog();

    void onPupilUndistortionClick(int state);
    void onImageUndistortionClick(int state);

public slots:

    void onSettingsChange();

};


#endif //PUPILEXT_PUPILDETECTIONSETTINGSDIALOG_H
