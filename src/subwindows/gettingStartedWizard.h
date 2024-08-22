#pragma once

#include <QtWidgets/qwizard.h>
#include <QtCore/QSettings>
#include <opencv2/core/version.hpp>

class GettingStartedWizard : public QWizard {
    Q_OBJECT

public:

    enum WizardPurpose {
        ABOUT_AND_USERGUIDE,
        ABOUT_ONLY,
        USERGUIDE_ONLY
    };

    explicit GettingStartedWizard(WizardPurpose purpose, QWidget *parent = nullptr);

    QWizardPage *createIntro01();
    QWizardPage *createIntro02();
    QWizardPage *createPreUserGuide01();
    QWizardPage *createPreUserGuide02();
    QWizardPage *createUserGuide01();
    QWizardPage *createUserGuide02();
    QWizardPage *createUserGuide03();
    QWizardPage *createUserGuide04();
    QWizardPage *createUserGuide05();
    QWizardPage *createUserGuide06();
    QWizardPage *createUserGuide07();
    QWizardPage *createUserGuide08();
    QWizardPage *createUserGuide09();
    QWizardPage *createUserGuide10();
    QWizardPage *createUserGuide11();
    QWizardPage *createConclusion01();

private:

    QSettings *applicationSettings;

    enum {
        Page_Intro_01,
        Page_Intro_02,
        Page_Pre_User_Guide_01,
        Page_Pre_User_Guide_02,
        Page_User_Guide_01,
        Page_User_Guide_02,
        Page_User_Guide_03,
        Page_User_Guide_04,
        Page_User_Guide_05,
        Page_User_Guide_06,
        Page_User_Guide_07,
        Page_User_Guide_08,
        Page_User_Guide_09,
        Page_User_Guide_10,
        Page_User_Guide_11,
        Page_Conclusion_01, };

    WizardPurpose purpose;

};
