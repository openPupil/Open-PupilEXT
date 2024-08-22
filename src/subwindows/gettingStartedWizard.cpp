
#include <QtWidgets/QLabel>
#include <QtWidgets/qboxlayout.h>
#include "gettingStartedWizard.h"
#include <QtWidgets/QtWidgets>

GettingStartedWizard::GettingStartedWizard(WizardPurpose purpose, QWidget *parent) :
        purpose(purpose),
        QWizard(parent), applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    if(purpose == ABOUT_AND_USERGUIDE || purpose == ABOUT_ONLY) {
        this->setPage(Page_Intro_01, createIntro01());
        this->setPage(Page_Intro_02, createIntro02());
    }

    if(purpose == ABOUT_AND_USERGUIDE) {
        this->setPage(Page_Pre_User_Guide_01, createPreUserGuide01());
        this->setPage(Page_Pre_User_Guide_02, createPreUserGuide02());
    }

    if(purpose == ABOUT_AND_USERGUIDE || purpose == USERGUIDE_ONLY) {
        this->setPage(Page_User_Guide_01, createUserGuide01());
        this->setPage(Page_User_Guide_02, createUserGuide02());
        this->setPage(Page_User_Guide_03, createUserGuide03());
        this->setPage(Page_User_Guide_04, createUserGuide04());
        this->setPage(Page_User_Guide_05, createUserGuide05());
        this->setPage(Page_User_Guide_06, createUserGuide06());
        this->setPage(Page_User_Guide_07, createUserGuide07());
        this->setPage(Page_User_Guide_08, createUserGuide08());
        this->setPage(Page_User_Guide_09, createUserGuide09());
        this->setPage(Page_User_Guide_10, createUserGuide10());
        this->setPage(Page_User_Guide_11, createUserGuide11());

        this->setPage(Page_Conclusion_01, createConclusion01());
    }

    if(purpose == ABOUT_ONLY) {
        this->setWindowTitle("About PupilEXT");
    } else if(purpose == ABOUT_AND_USERGUIDE) {
        this->setWindowTitle("Getting started with PupilEXT");
    } else if(purpose == USERGUIDE_ONLY) {
        this->setWindowTitle("Tutorial");
    }

    setSubTitleFormat(Qt::RichText);

    this->setMinimumSize(750,520);

    setWindowIcon(parent->windowIcon());
}

QWizardPage* GettingStartedWizard::createIntro01() {
    QWizardPage *page = new QWizardPage;
//    if(purpose == ABOUT_AND_USERGUIDE) {
//        page->setTitle("Introduction");
//    }

    QPushButton *logoButton = new QPushButton;
    logoButton->setFlat(true);
    logoButton->setContentsMargins(0,0,0,0);
    logoButton->setAttribute(Qt::WA_NoSystemBackground, true);
    logoButton->setAttribute(Qt::WA_TranslucentBackground, true);
    logoButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px }");
    logoButton->setFixedSize(QSize(190, 65));
    logoButton->setIconSize(QSize(190, 65));
    logoButton->setIcon(QIcon(":/icons/PupilEXT-Logo.png"));

    QLabel *label = new QLabel(tr("<b>%1 Version: %2 </b> is an open source application for pupillometry.<br><br>"
                                  "Babak Zandi, Moritz Lode, Alexander Herzog, Georgios Sakas and Tran Quoc Khanh. (2021). "
                                  "PupilEXT: Flexible Open-Source Platform for High-Resolution Pupil Measurement in Vision Research.</br>"
                                  " Frontiers in Neuroscience. doi:10.3389/fnins.2021.676220."
                                  "<br><br>Consider to cite our work, if you find this tool useful for your research. "
                                  "Github: <a href=\"https://github.com/openPupil/Open-PupilEXT\">https://github.com/openPupil/Open-PupilEXT</a><br><br>"
                                  "The software PupilEXT is licensed under <a href=\"https://github.com/openPupil/Open-PupilEXT/blob/main/PupilEXT/LICENSE\">GNU General Public License v.3.0.</a>"
                                  ", Copyright (c) 2021 Technical University of Darmstadt. PupilEXT is for academic and non-commercial use only."
                                  " Note that third-party libraries used in PupilEXT may be distributed under other open-source licenses (see GitHub repository).<br><br>"
                                  "<b>This release was built from the Experimental Community Version branch source code</b>, "
                                  "contributed by Gábor Bényei and Attila Boncsér as of 2024.<br><br>"
    ).arg(QCoreApplication::applicationName(),QCoreApplication::applicationVersion()));

    QHBoxLayout *appSettingsRow = new QHBoxLayout();
    QLabel *appSettingsLabel = new QLabel("Application settings path: ");
    QLineEdit *appSettingsVal = new QLineEdit(applicationSettings->fileName());
//    appSettingsVal->setMinimumWidth(260);
    appSettingsVal->setReadOnly(true);
    appSettingsRow->addWidget(appSettingsLabel);
    appSettingsRow->addWidget(appSettingsVal);
//    appSettingsRow->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    label->setOpenExternalLinks(true);
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(logoButton);
    layout->addWidget(label);
    layout->addLayout(appSettingsRow);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createIntro02() {
    QWizardPage *page = new QWizardPage;
    page->setTitle("Standing on the shoulders of giants");

    QString qtLabelText =
    "PupilEXT uses the <a href=\"https://www.qt.io/\">Qt framework</a>, version %1"
    "<br><br>"
    "Qt is an application development framework, which enables this application to smoothly run on different operating systems."
    "<br>It is maintained by The Qt Company, but the Qt Project is <a href=\"https://www.qt.io/community/contribute-to-qt\">open for contributors</a>. Feel free to check out their website, if you are interested.</br>";

    QString cvLabelText =
    "PupilEXT relies on the <a href=\"https://opencv.org/\">OpenCV library</a>, version %1"
    "<br><br>"
    "OpenCV is a very useful and efficient, cross-platform library for computer vision, and is employed for image processing everywhere in this application.";

    QLabel *qtLabel = new QLabel(qtLabelText.arg(qVersion()));
    QLabel *cvLabel = new QLabel(cvLabelText.arg(CV_VERSION));

    int iconSizePx = 65;

    QPushButton *qtButton = new QPushButton();
    qtButton->setFlat(true);
    qtButton->setContentsMargins(0,0,0,0);
    qtButton->setAttribute(Qt::WA_NoSystemBackground, true);
    qtButton->setAttribute(Qt::WA_TranslucentBackground, true);
    qtButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px }");
    qtButton->setFixedSize(QSize(iconSizePx, iconSizePx));
    qtButton->setIconSize(QSize(iconSizePx, iconSizePx));
    qtButton->setIcon(QIcon(":/icons/Qt_logo_2016.svg"));

    QPushButton *cvButton = new QPushButton();
    cvButton->setFlat(true);
    cvButton->setContentsMargins(0,0,0,0);
    cvButton->setAttribute(Qt::WA_NoSystemBackground, true);
    cvButton->setAttribute(Qt::WA_TranslucentBackground, true);
    cvButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px }");
    cvButton->setFixedSize(QSize(iconSizePx, iconSizePx));
    cvButton->setIconSize(QSize(iconSizePx, iconSizePx));
    cvButton->setIcon(QIcon(":/icons/OpenCV_Logo_with_text_svg_version.svg"));

    qtLabel->setOpenExternalLinks(true);
    qtLabel->setWordWrap(true);
    qtLabel->setContentsMargins(20,0,0,0);
    cvLabel->setOpenExternalLinks(true);
    cvLabel->setWordWrap(true);
    cvLabel->setContentsMargins(20,0,0,0);

    QHBoxLayout *row1 = new QHBoxLayout;
    QHBoxLayout *row2 = new QHBoxLayout;
    row1->addWidget(qtButton);
    row1->addWidget(qtLabel);
    row2->addWidget(cvButton);
    row2->addWidget(cvLabel);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(row1);
    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout->addLayout(row2);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createPreUserGuide01() {
    QWizardPage *page = new QWizardPage;
    page->setTitle("First Steps");
    page->setSubTitle("<b>The basic steps to take in order to use PupilEXT</b>");

    QString labelText =
            "With PupilEXT, you can:"
            "<ul style=\"margin: 0px\">"
            "<li>Make an image recording for later processing</li>"
            "<li>Perform pupil detection"
            "<ul>"
            "<li>On live image feed, or image recording (latter is advised)</li>"
            "<li>For writing to a \".csv\" data file, or stream data</li>"
            "</ul>"
            "</li>"
            "</ul>"
            "<br>"
            "To make an image recording:"
            "<ol style=\"margin: 0px\">"
            "<li>Open a physical camera (single or stereo setup) and adjust camera settings</li>"
            "<li>Calibrate the physical camera using the calibration pattern, and save it to a file</li>"
            "<li>Specify image recording output directory and <b>Start image recording</b></li>"
            "</ol>";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QFormLayout *layout = new QFormLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createPreUserGuide02() {
    QWizardPage *page = new QWizardPage;
    page->setTitle("First Steps");
    page->setSubTitle("<b>The basic steps to take in order to use PupilEXT</b>");

    QString labelText =
            "To perform pupil detection:"
            "<ol style=\"margin: 0px\">"
            "<li>Open a physical camera (single or stereo setup) and adjust camera settings<br>"
            "&emsp;&nbsp;Or open a previously recorded image set (latter is advised)</li>"
            "<li>Calibrate the physical camera using the calibration pattern<br>"
            "&emsp;&nbsp;Or load an existing camera calibration file to perform an offline calibration</li>"
            "<li>Adjust pupil detection settings using its settings dialog, and the camera view window</li>"
            "<li>&emsp;A, Specify data recording output file name and <b>Start data recording</b><br>"
            "&emsp;&nbsp;In case of an image recording playback, also Start the playback</li>"
            "&emsp;B, Specify streaming target and <b>Start data streaming</b><br>"
            "&emsp;&nbsp;In case of an image recording playback, also Start the playback</li>"
            "</ol>";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QLabel *label2= new QLabel("If you wish to dismiss this dialog right now, just click the Finish Now button below."
                               "<br><br>Alternatively, for a brief guide you can click Next to see the User Guide pages, though they can "
                               "be viewed anytime later as well, by opening the User Guide window from the Help menu.");
    label2->setWordWrap(true);
    label2->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(label2);
    if(purpose == ABOUT_AND_USERGUIDE) {
        QPushButton *finishNowButton = new QPushButton("Finish Now");
        QHBoxLayout *row1 = new QHBoxLayout;
        finishNowButton->setMinimumWidth(120);
        row1->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        row1->addWidget(finishNowButton);
        layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
        layout->addLayout(row1);
        connect(finishNowButton, &QPushButton::clicked, this->button(QWizard::FinishButton), &QPushButton::clicked);
    }
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide01() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Opening a single camera or stereo setup";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "To open a camera device, use the camera icon in the left (camera list is updated automatically). "
    "You can use a single camera setup without an STM Nucleo or Arduino microcontroller for hardware-triggered "
    "image acquisition (though you can, is you choose to). For stereo camera setup, you need to use a "
    "microcontroller to carry out triggering. You need to follow the numbered steps in the camera settings "
    "dialogs, in order to successfully connect to a camera and start image acquisition. "
    "<br>In case of stereo setups, where you have to use hardware-triggered acquisition, it follows as: "
    "<ol>"
    "<li>Connecting to the microcontroller (Can be done via Serial or Ethernet, based on your hardware setup)</li>"
    "<li>Opening the camera pair (Can be connected via USB3 or GigE, based on your hardware setup)</li>"
    "<li>Setting image acquisition properties that will affect you maximum possible FPS rate "
    "<br>(smaller image acquisition ROI, larger binning, and shorter exposition time can be used to increase FPS)</li>"
    "<li>Set the desired FPS rate (lower or equal than the maximum achievable) and start image acquisition triggering.</li>"
    "</ol>";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide02() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Opening an existing image recording";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "To open an image recording from the disk, you need to close any opened camera, and use the "
    "File menu/Open images directory option. You need to navigate into the directory you want to open, "
    "and press Open to open it, or just drag-and-drop an image recording folder (or any of its files inside) "
    "into the main window. PupilEXT supports several image formats for reading, so you can even use recordings that "
    "were made by another program or script, however you need to make sure that all the image files are named with "
    "trailing zeros, and their name equals the timestamp of image acquisition per each frame, in milliseconds. "
    "<br><br>"
    "You can now play back the recording using the graphical interface, which supports frame-by-frame inspection "
    "and automated playback as well. You can also jump to selected frames manually, and see trial numbering and "
    "textual messages annotated for each frame.";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide03() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "After opening a camera or image recording";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "A live eye image will be visible in the camera view window, where you can monitor the state of pupil detection, "
    "set pupil detection ROI (software ROI, which does not affect achievable maximum FPS) and set Automatic "
    "Parametrization expected minimum and maximum pupil size (described later). "
    "<br><br>"
    "To help camera positioning "
    "you can turn on Camera Positioning Guide overlay in the Camera View window, in the Show menu, along with other "
    "useful overlays. The Positioning Guide is particularly helpful if you are using stereo cameras, that need to "
    "be pointed at the same eye or at a common target. This Positioning Guide has its center in the actual center of "
    "the camera sensor, where the lens distortion is the lowest. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide04() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Making an image recording";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "To start image recording, you need to set an image output directory in the left icon bar of the main window. "
    "We advise to save images to a local SATA disk with high write speed. Saved image recording size will "
    "depend on the image size you set in the camera settings dialog. If you choose large image size, "
    "recordings can eat up a very big space, so be cautious."
    "<br>"
    "<br>Also pay attention to the warmup indicator in "
    "the status bar. If it is grey, there is no sufficient data yet (1 minute) to judge whether the camera(s) "
    "have warmed up, it is red if the camera(s) is/are changing temperature, and it is green if the camera(s) "
    "reached a thermal equilibrium with their environment and is/are ready for proper recording. "
    "(Note that the actual warm-up time also depends on the illuminator you use. Right now, PupilEXT only "
    "supports checking the temperature of the camera(s), though in the future we plan to add illuminator "
    "temperature checking too. Usually an LED illuminator and its driver needs half an hour to warm up from "
    "room temperature to their operating temperature.)"
    "<br>"
    "<br>In case a recording exists with the same name, you are "
    "asked whether wou would like PupilEXT to append to the existing recording, or make a new one that is "
    "automatically renamed, in a new folder. You can also set the default behaviour for these cases in Settings. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide05() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Calibrating for camera image undistortion and px-mm mapping";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "Calibration is an optional step that can be made to compensate for the camera lens distortion, and "
    "for proper pixel-to-millimeter mapping the measured pupil size. It is only needed when you are running PupilEXT "
    "to carry out pupil detection, writing data to e.g. a .csv file or streaming the data. "
    "<br>For only image recording, calibration is not a must, though it should be made and saved in order to load it"
    "as an offline calibration later, when the image recording is to be processed in a calibrated manner. "
    "<br><br>"
    "Note that if you alter Image Acquisition ROI or the binning value in the Camera Settings dialog, "
    "you have to make a new calibration. "
    "<br><br>"
    "To calibrate, use a correctly-sized calibration pattern, printed on a sheet of paper, and present it to the "
    "camera(s) on a strictly flat surface (e.g. sticker on a smooth cardboard), placed in the expected eye target "
    "distance to the camera(s). The appropriate calibration routine is accessible after opening the camera(s), with "
    "the Calibrate button in the icon bar of the main window. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide06() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Setting up pupil detection";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "Selecting a pupil detection algorithm is a necessary step, though a default is already set upon start. "
    "PuRe and PuReST usually perform well, and are efficient, while they also provide a confidence output. "
    "PupilEXT supports several pupil detection algorithms, which can be parametrized in different ways. "
    "If you just would like to use them without much customization, we advise to select the Automatic Parametrization "
    "option in the Pupil Detection Settings dialog. This configuration setting can be set for every algorithm. "
    "<br><br>"
    "We also advise to use pupil detection ROI preprocessing, using the appropriate checkbox in the same dialog. "
    "If pupil detection ROI preprocessing is enabled, only a fraction of the recorded image is scanned for pupil(s), "
    "and we highly recommend you to use this option for better performance. You can set pupil detection ROI in the "
    "Camera View window, when a camera (or image recording) is opened in the Pupil Detection menu. Please note that "
    "Image Acquisition ROI is a completely different setting, having to do with which part of the camera sensor you "
    "are reading image content from (a smaller im. acq. ROI can improve FPS). "
    "<br><br>"
    "If you have previously calibrated your "
    "camera, and it is loaded, you can opt for undistorting the pupil size on each frame inside the selected pupil "
    "detection ROI. You can also opt for undistorting the whole image, though not recommended. The easiest is to use "
    "Automatic Parametrization (recommended, and can be enabled in Pupil Detection Settings dialog). With that, you "
    "can specify the expected pupil diameter for the algorithm, using Pupil Detection menu in the Camera View window. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide07() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Using pupil tracking";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "To activate pupil tracking, you need to click on the Pupil tracking icon in the toolbar on "
    "the left side of main window. This function has two main use cases: "
    "<ol>"
    "<li>If you want to perform live pupil detection to write output to e.g. a .csv file or stream it on the spot</li>"
    "<li>If you want to run analysis on previously made image recordings</li>"
    "</ol>"
    "Yet, as of PupilEXT version v0.1.2 the latter is recommended, to ensure precise trial and message triggering. "
    "Now we advise to first acquire images and run pupil detection on the recordings later, to produce data files.";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide08() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Making data recordings";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "To start .csv data file recording, you first need to set an output file path and name, using "
    "the icon toolbar on the left of the main window. It is also automatically set to a default output file name "
    "upon opening an image recording, to a .csv file located in the parent folder of the image recording folder. "
    "<br><br>"
    "In case you have been previously looking around in the recording already, now you should wind back to the "
    "first frame using the image playback control dialog. Then you may click on the red Record button "
    "to start recording, and click on the Play button of the image playback control dialog "
    "to start producing the active pupil detection output to a .csv file. "
    "<br><br>"
    "Note that yet we only recommend "
    "to record to .csv files offline, when the image set is recorded beforehand, and is analyzed for "
    "pupil detection at a later point in time. This is because the exact timing of trial increment trigger "
    "timestamps and messages received from the Experiment computer are ensured to be precise only in case "
    "of image recordings and later offline analysis. "
    "<br><br>"
    "To stop the data recording, wait until the playback "
    "reaches end (be sure to untick \"Loop playback\" to let it finish), and click the Stop recording icon "
    "in the toolbar on the left side of main window. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide09() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Streaming data";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "To stream pupil detection output, you need to first set up the streaming target, and the interface through "
    "which you would like to stream the data. The program currently supports streaming via a Serial (COM) connection "
    "and/or an Ethernet connection using UDP. "
    "<br><br>"
    "The data can be encapsulated into CSV-rows, as well as XML, JSON or "
    "YAML structures. You can stream to the same target where a Remote Control Connection is already set up from, "
    "and streaming can happen on Serial and over UDP at the same time, but only to one-one target(s). "
    "<br><br>"
    "It is highly "
    "recommended to only use streaming in case low-FPS image acquisition. You can stream pupil detection output "
    "from live camera input, but also from image recording playback, for e.g. testing purposes. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide10() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "Integrating PupilEXT into a pupillometry experiment";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
    "You can easily integrate PupilEXT with a <b>PsychoPy</b> experiment program (Builder or Coder as well) or with <b>Matlab</b> "
    "using Psychtoolbox. You can either use serial data cable or ethernet cable for connecting the Experiment computer "
    "(running the PsychoPy or Matlab code) and the Host computer (running PupilEXT, connected to the camera(s)). "
    "<br><br>"
    "<b>Ready-to-use examples are included in the GibHub repository, under: <br>\"Misc/Experiment_Integration_Examples/\"</b>"
    "<br><br>"
    "Important note: These examples only employ image data recording for offline analysis. You can still uncomment "
    "the necessary lines and use PupilEXT with real-time pupillometry, and saving the results in a .csv file "
    "on the spot. However, we highly recommend to yet only use PupilEXT for recording the image set first, and then "
    "offline analysing it and generating the .csv then and there. Using the image recording method we successfully "
    "tested PupilEXT to be very accurate regarding trial increment trigger timestamps, though this accuracy is not "
    "ensured in case of real-time pupil detection use. If you are using PupilEXT for research-grade data acquisition, "
    "please yet use it this way, until a new release is out. ";

    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createUserGuide11() {
    QWizardPage *page = new QWizardPage;
    QString userGuideSubHeadline = "A note on reproducibility";
    if(purpose == ABOUT_AND_USERGUIDE) {
        page->setTitle("First Steps");
        page->setSubTitle("<b>"+userGuideSubHeadline+"</b>");
    } else {
        page->setTitle(userGuideSubHeadline);
    }

    QString labelText =
            "As any scientific measurement system, PupilEXT software is built with special attention paid to research "
            "reproducibility needs. Also, as it is an open-source system, and can be used with varied hardware, it is "
            "the responsibility of the researcher mostly to minimise any between-subjects differences in data acquisition "
            "circumstances, while the software does its best to also minimise undesired within-subject variations, and "
            "offers functionalities to help the researcher with the former. "
            "<br><br>"
            "The software currently automatically saves many of its settings in a structured format alongside with image "
            "and \".csv.\" data recordings in an arbitrary, human-readable format: in an \".xml\" file, named with a \"_meta\" "
            "postfix. Also, it supports storing and re-loading <i>ALL</i> the application settings for different subjects, "
            "using the Subjects dialog, accessible with the Subjects icon in the icon bar: in an \".ini\" file, which upon loading,"
            "would reset all settings, including remote control connection, streaming, etc. "
            "<br>The purpose of the meta files is to provide a way to manually, visually "
            "inspect any application setting related to image acquisition and pupil detection, when needed by the user "
            "at a later point in time, while the purpose of subject configurations is to provide a way to set the application "
            "to a previous state when an image or data acquisition was carried out."
            "<br>At the moment, these two functionalities are partly overlapping, and will be merged into one in a next "
            "release of the PupilEXT software. ";


    QLabel *label = new QLabel(labelText);
    label->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GettingStartedWizard::createConclusion01()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Conclusion");

    QLabel *label = new QLabel("You are now ready to conduct pupillometry experiments! <br><br><br>Consider visiting <a href=\"https://openpupil.io\">openPupil.io</a> (under construction) to review openly available pupil datasets and contribute your insights.");
    label->setWordWrap(true);

    QLabel *label2= new QLabel("For more information and detailed instructions visit the project website: <a href=\"https://github.com/openPupil/Open-PupilEXT\">PupilEXT</a>");
    label2->setWordWrap(true);
    label2->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(label2);
    page->setLayout(layout);

    return page;
}

