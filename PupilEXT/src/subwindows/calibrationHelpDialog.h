#ifndef PUPILEXT_CALIBRATIONHELPDIALOG_H
#define PUPILEXT_CALIBRATIONHELPDIALOG_H

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
    Helper dialog window displaying information on calibration pattern and how to specify their size.

    Uses image resources located under icons/ directory.
*/
class CalibrationHelpDialog : public QDialog {
    Q_OBJECT

public:

    explicit CalibrationHelpDialog(QWidget *parent = nullptr);
    ~CalibrationHelpDialog() override;

protected:

    void reject() override;

private:

    QPushButton *closeButton;

    void createForm();

};


#endif //PUPILEXT_CALIBRATIONHELPDIALOG_H
