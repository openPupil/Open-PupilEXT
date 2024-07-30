//
// Created by epresleves on 2024. 04. 17..
//

#ifndef PUPILEXT_OUTPUTDATARULEDIALOG_H
#define PUPILEXT_OUTPUTDATARULEDIALOG_H

#include <QtWidgets>
#include <QDialog>
#include <QCheckBox>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

class OutputDataRuleDialog : public QDialog {
Q_OBJECT

public:

    enum OutputDataRuleResponse {APPEND = 1, KEEP_AND_SAVE_NEW = 2};


    explicit OutputDataRuleDialog(const QString windowTitle, QWidget *parent = nullptr);
    ~OutputDataRuleDialog() override;
    OutputDataRuleResponse getResponse();
    bool getRememberChoice();

private slots:
    void onAppendClicked();
    void onKeepAndSaveNewClicked();

private:
    OutputDataRuleResponse outputDataRuleResponse;
    bool rememberChoice = false;
    QCheckBox *rememberChoiceBox;
    QPushButton *buttonAppend;
    QPushButton *buttonKeepAndSaveNew;
};

#endif //PUPILEXT_OUTPUTDATARULEDIALOG_H