
#include <QtWidgets/QLabel>
#include <QtWidgets/qboxlayout.h>
#include "gettingsStartedWizard.h"
#include <QtWidgets/QtWidgets>

GettingsStartedWizard::GettingsStartedWizard(QWidget *parent) :
        QWizard(parent), applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    this->setPage(Page_Intro, createIntroPage());
    this->setPage(Page_1, createInfo1Page());
    this->setPage(Page_Conclusion ,createConclusionPage());

    this->setWindowTitle("Getting Started");

    this->setMinimumSize(600,400);

    setWindowIcon(parent->windowIcon());
}

QWizardPage* GettingsStartedWizard::createIntroPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Introduction");

    QLabel *label = new QLabel(tr("<b>%1 Version: %2 </b> is an open source application for pupillometry.<br><br>"
                                  "Babak Zandi, Moritz Lode, Alexander Herzog, Georgios Sakas and Tran Quoc Khanh. (2021). "
                                  "PupilEXT: Flexible Open-Source Platform for High-Resolution Pupil Measurement in Vision Research.</br>"
                                  " Frontiers in Neuroscience. doi:10.3389/fnins.2021.676220."
                                  "<br><br>Consider to cite our work, if you find this tool useful for your research."
                                  "Github: <a href=\"https://github.com/openPupil/Open-PupilEXT\">https://github.com/openPupil/Open-PupilEXT</a><br><br>"
                                  "The software PupilEXT is licensed under <a href=\"https://github.com/openPupil/Open-PupilEXT/blob/main/PupilEXT/LICENSE\">GNU General Public License v.3.0.</a>"
                                  ", Copyright (c) 2021 Technical University of Darmstadt. PupilEXT is for academic and non-commercial use only."
                                  " Note that third-party libraries used in PupilEXT may be distributed under other open-source licenses (see GitHub repository).<br><br>"
                                  "This release was built from the Experimental Community Version branch source code, contributed by Gábor Bényei and Attila Boncsér as of 2024.<br><br>"
                                  "Application settings path: %3<br>"
    ).arg(QCoreApplication::applicationName(),QCoreApplication::applicationVersion(), applicationSettings->fileName()));

    label->setOpenExternalLinks(true);
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingsStartedWizard::createInfo1Page()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("First Steps");
    page->setSubTitle("Your next steps to take to conduct a pupil experiment.");

    QLabel *label = new QLabel("<ol>\n"
                               "  <li>Open a physical camera (single, stereo) or an offline image recording.</li>\n"
                               "  <li>Calibrate the physical camera or load an existing calibration file.</li>\n"
                               "  <li>Select the pupil detection algorithm and set parameters accordingly.</li>\n"
                               "  \n"
                               "    <li>Select an output file and start data recording.</li>\n"
                               "\n"
                               "  <li>Activate pupil detection.</li>\n"
                               "\n"
                               "</ol> ");
    label->setWordWrap(true);

    QLabel *label2= new QLabel("For more information and detailed video instructions visit the project website: <a href=\"https://github.com/openPupil/Open-PupilEXT\">PupilEXT</a>");
    label2->setWordWrap(true);
    label2->setOpenExternalLinks(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    layout->addWidget(label2);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingsStartedWizard::createConclusionPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Conclusion");

    QLabel *label = new QLabel("You are now ready to conduct pupil experiments! <br><br><br>Consider visiting <a href=\"https://openpupil.io\">openPupil.io</a> (under construction) to review openly available pupil datasets and contribute your insights.");
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

