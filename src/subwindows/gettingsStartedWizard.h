#pragma once

#include <QtWidgets/qwizard.h>
#include <QtCore/QSettings>

class GettingsStartedWizard : public QWizard {
    Q_OBJECT

private:

    QSettings *applicationSettings;
    enum { Page_Intro, Page_1,
        Page_Conclusion };

public:

    explicit GettingsStartedWizard(QWidget *parent = nullptr);

    QWizardPage *createIntroPage();

    QWizardPage *createInfo1Page();

    QWizardPage *createConclusionPage();

};
