//
// Created by epresleves on 2024. 04. 17..
//

#include "outputDataRuleDialog.h"

OutputDataRuleDialog::OutputDataRuleDialog(const QString windowTitle, QWidget *parent) :
        QDialog(parent)
{
    this->setWindowTitle(windowTitle);
    this->setMinimumSize(450,150);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *dialogLabel = new QLabel(tr("Existing data was found under the target path/name you specified. Please choose whether you would like to append to the existing recording or keep it and save the new recording with an automatically generated different path/name?"));
    dialogLabel->setWordWrap(true);
    mainLayout->addWidget(dialogLabel);

    rememberChoiceBox = new QCheckBox(tr("Remember this choice"));
    mainLayout->addWidget(rememberChoiceBox);

    QFormLayout *buttonsLayout = new QFormLayout(this);
    buttonAppend = new QPushButton(tr("Append to existing"));
    buttonKeepAndSaveNew = new QPushButton(tr("Keep existing and save new as well"));
    buttonAppend->setFixedWidth(210);
    //buttonKeepAndSaveNew->setFixedWidth(200);
    buttonsLayout->addRow(buttonAppend, buttonKeepAndSaveNew);
    mainLayout->addLayout(buttonsLayout);

    // Set default focus on the safer solution button
    buttonAppend->setDefault(false);
    buttonKeepAndSaveNew->setDefault(true);

    setLayout(mainLayout);

    connect(buttonAppend, &QPushButton::clicked, this, &OutputDataRuleDialog::onAppendClicked);
    connect(buttonKeepAndSaveNew, &QPushButton::clicked, this, &OutputDataRuleDialog::onKeepAndSaveNewClicked);
//    connect(this, &QDialog::close, this, &OutputDataRuleDialog::onKeepAndSaveNewClicked);

}

OutputDataRuleDialog::~OutputDataRuleDialog()  = default;

OutputDataRuleDialog::OutputDataRuleResponse OutputDataRuleDialog::getResponse(){
    return outputDataRuleResponse;
}

bool OutputDataRuleDialog::getRememberChoice(){
    return rememberChoice;
}

void OutputDataRuleDialog::onAppendClicked()
{
    rememberChoice = rememberChoiceBox->isChecked();
    outputDataRuleResponse = OutputDataRuleResponse::APPEND;
    accept();
}

void OutputDataRuleDialog::onKeepAndSaveNewClicked()
{
    rememberChoice = rememberChoiceBox->isChecked();
    outputDataRuleResponse = OutputDataRuleResponse::KEEP_AND_SAVE_NEW;
    accept();
}
