#include <iomanip>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QtWidgets>
#include "../supportFunctions.h"
#include "sceneImageView.h"
#include "../SVGIconColorAdjuster.h"

SceneImageView::SceneImageView(bool sceneFrozen, QWidget *parent) :
        QWidget(parent),
        sceneFrozen(sceneFrozen),
        applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), this)) {

    setWindowTitle("Scene Image View");

    QVBoxLayout* layout = new QVBoxLayout(this);

    toolBar = new QToolBar();

    // E.g. for "Calibration grid" with: 5 /9 /12 points options
    QMenu *configAMenu = new QMenu("Config A menu");
    configAMenuAct = configAMenu->menuAction();
    connect(configAMenuAct, &QAction::triggered, this, &SceneImageView::onConfigAMenuClick);
    configA1Act = new QAction("Config1");
    configA2Act = new QAction("Config2");
    configA3Act = new QAction("Config3");
    configA1Act->setCheckable(true);
    configA2Act->setCheckable(true);
    configA3Act->setCheckable(true);
    configAMenu->addAction(configA1Act);
    configAMenu->addAction(configA2Act);
    configAMenu->addAction(configA3Act);
    connect(configA1Act, &QAction::triggered, this, &SceneImageView::onConfigA1Selected);
    connect(configA2Act, &QAction::triggered, this, &SceneImageView::onConfigA2Selected);
    connect(configA3Act, &QAction::triggered, this, &SceneImageView::onConfigA3Selected);
    toolBar->addAction(configAMenuAct);
    toolBar->addSeparator();

    // E.g. for "Scene view" with options for:
    // - displaying blank color filled view (for calib points overlay best visible on it) or
    // - display the screen of the experiment computer live (there is ready-made Qt widget for displaying a VNC cast screen)
    //      see: https://bitbucket.org/amahta/qvncclient/src/master/
    //      Also, in the future, actions could be supported through VNC by eye events, e.g. HCI-BCI use cases
    //          e.g. for clicking on a button of the remote machine (seen on scene image) by double blinking
    QMenu *configBMenu = new QMenu("Config B menu");
    configBMenuAct = configBMenu->menuAction();
    connect(configBMenuAct, &QAction::triggered, this, &SceneImageView::onConfigBMenuClick);
    configB1Act = new QAction("Config1");
    configB2Act = new QAction("Config2");
    configB1Act->setCheckable(true);
    configB2Act->setCheckable(true);
    configBMenu->addAction(configB1Act);
    configBMenu->addAction(configB2Act);
    toolBar->addAction(configBMenuAct);
    toolBar->addSeparator();

    freezeAct = toolBar->addAction("Freeze", this, &SceneImageView::onFreezeClicked);
    freezeAct->setCheckable(true);
    freezeAct->setChecked(sceneFrozen);
    // TODO: put this in some menu, at least because it is checkable now

    // These could also go to the mainwindow icon menu
//    calibrateAct = toolBar->addAction("Calibrate gaze", this, &SceneImageView::onCalibrateClicked);
//    validateAct = toolBar->addAction("Validate gaze", this, &SceneImageView::onValidateClicked);

    calibrateButton = new QPushButton();
    calibrateButton->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/crosshairs-gaze-calibration.svg"), applicationSettings));
    calibrateButton->setIconSize(QSize(32,32));
//    calibrateButton->setStyleSheet("text-align:left; padding-left : 10px; padding-top : 3px; padding-bottom : 3px;"); //
    calibrateButton->setStyleSheet("text-align:left;");
    calibrateButton->setLayout(new QGridLayout);
    QLabel* calibrateButtonLabel = new QLabel("Calibrate gaze");
    calibrateButtonLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    calibrateButtonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    calibrateButton->layout()->addWidget(calibrateButtonLabel);
    calibrateButton->layout()->setContentsMargins(5,0,10,0);
    calibrateButton->setFixedWidth(115);
//    calibrateButton->setMaximumHeight(36);
    toolBar->addWidget(calibrateButton);

    validateButton = new QPushButton();
    validateButton->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/crosshairs-gaze-validation.svg"), applicationSettings));
    validateButton->setIconSize(QSize(32,32));
//    validateButton->setStyleSheet("text-align:left; padding-left : 10px; padding-top : 3px; padding-bottom : 3px;"); //
    validateButton->setStyleSheet("text-align:left;");
    validateButton->setLayout(new QGridLayout);
    QLabel* validateButtonLabel = new QLabel("Validate gaze");
    validateButtonLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    validateButtonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    validateButton->layout()->addWidget(validateButtonLabel);
    validateButton->layout()->setContentsMargins(5,0,10,0);
    validateButton->setFixedWidth(115);
//    validateButton->setMaximumHeight(36);
    toolBar->addWidget(validateButton);

    toolBar->setStyleSheet("QToolBar{spacing:6px;}");
//    toolBar->setStyleSheet("QToolButton{height:34px;}");



    toolBar->setFixedHeight(44); // 36

    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    layout->addWidget(toolBar);

    // GB NOTE: first just create the videoView instance like it is for a single ROI, and then we can change
    sceneImageWidget = new SceneImageWidget();
    layout->addWidget(sceneImageWidget);

    statusBar = new QStatusBar();

    QWidget * status1Widget = new QWidget();
    QHBoxLayout *status1Layout = new QHBoxLayout();
    status1Layout->setContentsMargins(8,0,8,0);

    status1Label = new QLabel();
    status1Label->setText("Status label: ");
    status1Value = new QLabel();
    status1Value->setText("value");

    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    status1Layout->addWidget(sep1);

    status1Layout->addWidget(status1Label);
    status1Layout->addWidget(status1Value);

    status1Widget->setLayout(status1Layout);
    statusBar->addPermanentWidget(status1Widget);

    layout->addWidget(statusBar);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setLayout(layout);

//    // Connect the pupil detection process to inform this widget of changes
//    connect(pupilDetection, SIGNAL(processingStarted()), this, SLOT(onPupilDetectionStart()));
//    connect(pupilDetection, SIGNAL(processingFinished()), this, SLOT(onPupilDetectionStop()));

    loadSettings();
}

SceneImageView::~SceneImageView() {

}

void SceneImageView::loadSettings() {
    //...
}

void SceneImageView::onConfigAMenuClick() {
    configAMenuAct->menu()->exec(QCursor::pos());
}

void SceneImageView::onConfigBMenuClick() {
    configBMenuAct->menu()->exec(QCursor::pos());
}

void SceneImageView::onConfigA1Selected() {
    configA1Act->setChecked(true);
    configA2Act->setChecked(false);
    configA3Act->setChecked(false);
}

void SceneImageView::onConfigA2Selected() {
    configA1Act->setChecked(false);
    configA2Act->setChecked(true);
    configA3Act->setChecked(false);
}

void SceneImageView::onConfigA3Selected() {
    configA1Act->setChecked(false);
    configA2Act->setChecked(false);
    configA3Act->setChecked(true);
}

void SceneImageView::onConfigB1Selected() {
    configB2Act->setChecked(true);
    configB2Act->setChecked(false);
}

void SceneImageView::onConfigB2Selected() {
    configB1Act->setChecked(false);
    configB2Act->setChecked(true);
}

//void SceneImageView::onPupilDetectionStart() {
//
//}
//
//void SceneImageView::onPupilDetectionStop() {
//
//}

void SceneImageView::updateView(const int &procMode, const std::vector<Pupil> &Pupils) {

    sceneImageWidget->updateView(procMode, Pupils);
}

void SceneImageView::onSettingsChange() {
    loadSettings();
}

void SceneImageView::onFreezeClicked() {
    emit cameraPlaybackChanged();
}

void SceneImageView::onCameraPlaybackChanged() {
    sceneFrozen = !sceneFrozen;
    freezeAct->setChecked(sceneFrozen);
}

void SceneImageView::onCalibrateClicked() {

    // if ...
    // emit startGazeCalibration();
    // else
    // emit cancelGazeCalibration();
}

void SceneImageView::onValidateClicked() {

    // if ...
    // emit startGazeValidation();
    // else
    // emit cancelGazeValidation();
}

