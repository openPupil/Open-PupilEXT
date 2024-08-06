
#ifndef PUPILEXT_SWIRSKI2DSETTINGS_H
#define PUPILEXT_SWIRSKI2DSETTINGS_H

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include "PupilMethodSetting.h"
#include "../../pupil-detection-methods/Swirski2D.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../SVGIconColorAdjuster.h"

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the Swirski2D algorithm, displayed in the pupil detection setting dialog
*/
class Swirski2DSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit Swirski2DSettings(PupilDetection * pupilDetection, Swirski2D *m_swirski, QWidget *parent=0) : 
        PupilMethodSetting("Swirski2DSettings.configParameters","Swirski2DSettings.configIndex", parent), 
        p_swirski(m_swirski), 
        pupilDetection(pupilDetection){

        PupilMethodSetting::setDefaultParameters(defaultParameters);
        createForm();
        configsBox->setCurrentText(settingsMap.key(configIndex));
        // GB added begin
        if(isAutoParamEnabled()) {
            minRadiusBox->setEnabled(false);
            maxRadiusBox->setEnabled(false);
        } else {
            minRadiusBox->setEnabled(true);
            maxRadiusBox->setEnabled(true);
        } 
        // GB added begin

        QGridLayout *infoLayout = new QGridLayout(infoBox);

        QPushButton *iLabelFakeButton = new QPushButton();
        iLabelFakeButton = new QPushButton();
        iLabelFakeButton->setFlat(true);
        iLabelFakeButton->setAttribute(Qt::WA_NoSystemBackground, true);
        iLabelFakeButton->setAttribute(Qt::WA_TranslucentBackground, true);
        iLabelFakeButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px }");
        iLabelFakeButton->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/status/22/dialog-information.svg"), applicationSettings));
        iLabelFakeButton->setFixedSize(QSize(32,32));
        iLabelFakeButton->setIconSize(QSize(32,32));
        infoLayout->addWidget(iLabelFakeButton, 0, 0);

        QLabel *pLabel = new QLabel();
        pLabel->setWordWrap(true);
        pLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        pLabel->setOpenExternalLinks(true);
        pLabel->setText("Lech Swirski, Andreas Bulling, Neil A. Dodgson, \"Robust real-time pupil tracking in highly off-axis images\", 2012 <a href=\"http://www.cl.cam.ac.uk/research/rainbow/projects/pupiltracking\">Website</a><br/>License: <a href=\"https://opensource.org/licenses/MIT\">MIT</a>");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB NOTE: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(p_swirski->hasConfidence())
            confLabel = new QLabel("Info: This method does provide its own confidence.");
        else
            confLabel = new QLabel("Info: This method does not provide its own confidence, use the outline confidence.");
        confLabel->setWordWrap(true);
        infoLayout->addWidget(confLabel, 2, 0);

        QLabel *infoLabel = new QLabel("CAUTION: Processing using this algorithm may be very slow, reduce the camera acquiring fps accordingly.");
        infoLabel->setWordWrap(true);
        infoLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(infoLabel, 3, 0);
#if _DEBUG
        QLabel *warnLabel = new QLabel("CAUTION: Debug build may perform very slow. Use release build or adjust processing speed to not risk memory overflow.");
        warnLabel->setWordWrap(true);
        warnLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(warnLabel, 4, 0);
#endif

        infoBox->setLayout(infoLayout);
        // GB modified end
    }

    ~Swirski2DSettings() override = default;

    void add2(Swirski2D *s_swirski) {
        swirski2 = s_swirski;
    }
    void add3(Swirski2D *s_swirski) {
        swirski3 = s_swirski;
    }
    void add4(Swirski2D *s_swirski) {
        swirski4 = s_swirski;
    }
public slots:

    void loadSettings() override {
        PupilMethodSetting::loadSettings();

        // GB added begin
        if(isAutoParamEnabled()) {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamEnabled(true);
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

            minRadiusBox->setEnabled(false);
            maxRadiusBox->setEnabled(false);
        } else {
            pupilDetection->setAutoParamEnabled(false);
            minRadiusBox->setEnabled(true);
            maxRadiusBox->setEnabled(true);
        } 
        // GB added end

        applySpecificSettings();
    }

    void applySpecificSettings() override {

        // GB modified begin
        
        // First come the parameters roughly independent from ROI size and relative pupil size 
        p_swirski->params.CannyBlur = cannyBlurBox->value();
        p_swirski->params.CannyThreshold1 = cannyThreshold1Box->value();
        p_swirski->params.CannyThreshold2 = cannyThreshold2Box->value();
        p_swirski->params.StarburstPoints = starburstPointsBox->value();

        p_swirski->params.PercentageInliers = percInlierBox->value();
        p_swirski->params.InlierIterations = iterInlierBox->value();
        p_swirski->params.ImageAwareSupport = imageAwareBox->isChecked();
        p_swirski->params.EarlyTerminationPercentage = termPercBox->value();
        p_swirski->params.EarlyRejection = earlyRejectionBox->isChecked();

        QList<float>& currentParameters = getCurrentParameters();
        currentParameters[2] = cannyBlurBox->value();
        currentParameters[3] = cannyThreshold1Box->value();
        currentParameters[4] = cannyThreshold2Box->value();
        currentParameters[5] = starburstPointsBox->value();
        currentParameters[6] = percInlierBox->value();
        currentParameters[7] = iterInlierBox->value();
        currentParameters[8] = termPercBox->value();
        currentParameters[9] = imageAwareBox->isChecked();
        currentParameters[10] = earlyRejectionBox->isChecked();

        if(swirski2) {
            swirski2->params.CannyBlur = cannyBlurBox->value();
            swirski2->params.CannyThreshold1 = cannyThreshold1Box->value();
            swirski2->params.CannyThreshold2 = cannyThreshold2Box->value();
            swirski2->params.StarburstPoints = starburstPointsBox->value();

            swirski2->params.PercentageInliers = percInlierBox->value();
            swirski2->params.InlierIterations = iterInlierBox->value();
            swirski2->params.ImageAwareSupport = imageAwareBox->isChecked();
            swirski2->params.EarlyTerminationPercentage = termPercBox->value();
            swirski2->params.EarlyRejection = earlyRejectionBox->isChecked();
        }
        if(swirski3) {
            swirski3->params.CannyBlur = cannyBlurBox->value();
            swirski3->params.CannyThreshold1 = cannyThreshold1Box->value();
            swirski3->params.CannyThreshold2 = cannyThreshold2Box->value();
            swirski3->params.StarburstPoints = starburstPointsBox->value();

            swirski3->params.PercentageInliers = percInlierBox->value();
            swirski3->params.InlierIterations = iterInlierBox->value();
            swirski3->params.ImageAwareSupport = imageAwareBox->isChecked();
            swirski3->params.EarlyTerminationPercentage = termPercBox->value();
            swirski3->params.EarlyRejection = earlyRejectionBox->isChecked();
        }
        if(swirski4) {
            swirski4->params.CannyBlur = cannyBlurBox->value();
            swirski4->params.CannyThreshold1 = cannyThreshold1Box->value();
            swirski4->params.CannyThreshold2 = cannyThreshold2Box->value();
            swirski4->params.StarburstPoints = starburstPointsBox->value();

            swirski4->params.PercentageInliers = percInlierBox->value();
            swirski4->params.InlierIterations = iterInlierBox->value();
            swirski4->params.ImageAwareSupport = imageAwareBox->isChecked();
            swirski4->params.EarlyTerminationPercentage = termPercBox->value();
            swirski4->params.EarlyRejection = earlyRejectionBox->isChecked();
        }

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(isAutoParamEnabled()) {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

        } else {

            p_swirski->params.Radius_Min = minRadiusBox->value();
            p_swirski->params.Radius_Max = maxRadiusBox->value();

            currentParameters[0] = minRadiusBox->value();
            currentParameters[1] = maxRadiusBox->value();

            if(swirski2) {
                swirski2->params.Radius_Min = minRadiusBox->value();
                swirski2->params.Radius_Max = maxRadiusBox->value();
            }
            if(swirski3) {
                swirski3->params.Radius_Min = minRadiusBox->value();
                swirski3->params.Radius_Max = maxRadiusBox->value();
            }
            if(swirski4) {
                swirski4->params.Radius_Min = minRadiusBox->value();
                swirski4->params.Radius_Max = maxRadiusBox->value();
            }
            
        }
        // GB modified end

        emit onConfigChange(configsBox->currentText());
    }

    void applyAndSaveSpecificSettings() override {
        applySpecificSettings();
        PupilMethodSetting::saveSpecificSettings();
    }

private:

    Swirski2D *p_swirski;
    //Swirski2D *secondarySwirski = nullptr; // GB: refactored
    // GB added begin
    Swirski2D *swirski2 = nullptr;
    Swirski2D *swirski3 = nullptr;
    Swirski2D *swirski4 = nullptr;

    PupilDetection *pupilDetection;
    // GB added end

    QSpinBox *minRadiusBox;
    QSpinBox *maxRadiusBox;

    QDoubleSpinBox *cannyBlurBox;
    QDoubleSpinBox *cannyThreshold1Box;
    QDoubleSpinBox *cannyThreshold2Box;
    QSpinBox *starburstPointsBox;

    QSpinBox *percInlierBox;
    QSpinBox *iterInlierBox;
    QSpinBox *termPercBox;

    QCheckBox *imageAwareBox;
    QCheckBox *earlyRejectionBox;

    void createForm() {
        PupilMethodSetting::loadSettings();
        QList<float>& selectedParameter = getCurrentParameters();

        int Radius_Min = selectedParameter[0];
        int Radius_Max = selectedParameter[1];

        double CannyBlur = selectedParameter[2];
        double CannyThreshold1 = selectedParameter[3];
        double CannyThreshold2 = selectedParameter[4];
        int StarburstPoints = selectedParameter[5];

        int PercentageInliers = selectedParameter[6];
        int InlierIterations = selectedParameter[7];
        bool ImageAwareSupport = selectedParameter[8];
        int EarlyTerminationPercentage = selectedParameter[9];
        bool EarlyRejection = selectedParameter[10];

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QHBoxLayout *configsLayout = new QHBoxLayout();

        configsBox = new QComboBox();
        // GB modified begin
        QLabel *parameterConfigsLabel = new QLabel(tr("Parameter configuration:"));
        configsBox->setFixedWidth(250);
        configsLayout->addWidget(parameterConfigsLabel);
        // GB modified end
        configsLayout->addWidget(configsBox);

        for (QMap<QString, Settings>::const_iterator cit = settingsMap.cbegin(); cit != settingsMap.cend(); cit++)
        {
            configsBox->addItem(cit.key());
        }

        connect(configsBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onParameterConfigSelection(QString)));

        mainLayout->addLayout(configsLayout);

        QHBoxLayout *configsNoteLayout = new QHBoxLayout();
        QLabel* configsNoteLabel = new QLabel(tr("Note: Configurations marked with an asterisk (*) are recommended for Basler\nacA2040-120um (1/1.8\" sensor format) camera(s) equipped with f=50 mm 2/3\"\nnominal sensor format lens, using 4:3 aspect ratio pupil detection ROI(s)."));
        //configsNoteLabel->setFixedWidth((int)this->size().width()/2);
        configsNoteLabel->setFixedWidth(420);
        configsNoteLabel->setFixedHeight(45);
        configsNoteLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        configsNoteLayout->addWidget(configsNoteLabel);
        mainLayout->addLayout(configsNoteLayout);

        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));


        QGroupBox *ellipseGroup = new QGroupBox("Algorithm specific: Ellipse Detection"); // GB: added "Algorithm specific: "
        QGroupBox *optionsGroup = new QGroupBox("Algorithm specific: Options"); // GB: "Algorithm specific: "

        QFormLayout *ellipseLayout = new QFormLayout();
        QFormLayout *optionsLayout = new QFormLayout();

        // GB modified begin
        // GB NOTE: to fit in smaller screen area
        QLabel *minRadiusLabel = new QLabel(tr("Min. Radius [px]:")); // GB: px added
        minRadiusBox = new QSpinBox();
        minRadiusBox->setMaximum(5000);
        minRadiusBox->setValue(Radius_Min);
        minRadiusBox->setFixedWidth(50); // GB

        QLabel *maxRadiusLabel = new QLabel(tr("Max. Radius [px]:")); // GB: px added
        maxRadiusBox = new QSpinBox();
        maxRadiusBox->setMaximum(5000);
        maxRadiusBox->setValue(Radius_Max);
        maxRadiusBox->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow1 = new QHBoxLayout;
        layoutRow1->addWidget(minRadiusBox);
        QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow1->addSpacerItem(sp1);
        layoutRow1->addWidget(maxRadiusLabel);
        layoutRow1->addWidget(maxRadiusBox);
        //layoutRow1->addSpacerItem(sp);
        ellipseLayout->addRow(minRadiusLabel, layoutRow1);


        QLabel *cannyBlurLabel = new QLabel(tr("Canny Blur:"));
        cannyBlurBox = new QDoubleSpinBox();
        cannyBlurBox->setValue(CannyBlur);
        cannyBlurBox->setFixedWidth(50); // GB
        ellipseLayout->addRow(cannyBlurLabel, cannyBlurBox);

        QLabel *cannyThreshold1Label = new QLabel(tr("Canny Threshold 1:"));
        cannyThreshold1Box = new QDoubleSpinBox();
        cannyThreshold1Box->setValue(CannyThreshold1);
        cannyThreshold1Box->setFixedWidth(50); // GB

        QLabel *cannyThreshold2Label = new QLabel(tr("Canny Threshold 2:"));
        cannyThreshold2Box = new QDoubleSpinBox();
        cannyThreshold2Box->setValue(CannyThreshold2);
        cannyThreshold2Box->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow2 = new QHBoxLayout;
        layoutRow2->addWidget(cannyThreshold1Box);
        QSpacerItem *sp2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow2->addSpacerItem(sp2);
        layoutRow2->addWidget(cannyThreshold2Label);
        layoutRow2->addWidget(cannyThreshold2Box);
        //layoutRow2->addSpacerItem(sp);
        ellipseLayout->addRow(cannyThreshold1Label, layoutRow2);


        QLabel *starburstPointsLabel = new QLabel(tr("Starburst Points:"));
        starburstPointsBox = new QSpinBox();
        starburstPointsBox->setValue(StarburstPoints);
        starburstPointsBox->setFixedWidth(50); // GB
        ellipseLayout->addRow(starburstPointsLabel, starburstPointsBox);

        QLabel *percInlierLabel = new QLabel(tr("Perc. Inliers:"));
        percInlierBox = new QSpinBox();
        percInlierBox->setValue(PercentageInliers);
        percInlierBox->setFixedWidth(50); // GB

        QLabel *iterInlierLabel = new QLabel(tr("Inlier Iterations:"));
        iterInlierBox = new QSpinBox();
        iterInlierBox->setValue(InlierIterations);
        iterInlierBox->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow3 = new QHBoxLayout;
        layoutRow3->addWidget(percInlierBox);
        QSpacerItem *sp3 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow3->addSpacerItem(sp3);
        layoutRow3->addWidget(iterInlierLabel);
        layoutRow3->addWidget(iterInlierBox);
        //layoutRow2->addSpacerItem(sp);
        ellipseLayout->addRow(percInlierLabel, layoutRow3);


        ellipseGroup->setLayout(ellipseLayout);
        mainLayout->addWidget(ellipseGroup);


        QLabel *termPercLabel = new QLabel(tr("Early Termination Perc.:"));
        termPercBox = new QSpinBox();
        termPercBox->setValue(EarlyTerminationPercentage);
        termPercBox->setFixedWidth(50); // GB
        optionsLayout->addRow(termPercLabel, termPercBox);

        QLabel *imageAwareLabel = new QLabel(tr("Image Aware RANSAC:"));
        imageAwareBox = new QCheckBox();
        imageAwareBox->setChecked(ImageAwareSupport);
        optionsLayout->addRow(imageAwareLabel, imageAwareBox);

        QLabel *earlyRejectionLabel = new QLabel(tr("Early Rejection:"));
        earlyRejectionBox = new QCheckBox();
        earlyRejectionBox->setChecked(EarlyRejection);
        optionsLayout->addRow(earlyRejectionLabel, earlyRejectionBox);
        // GB modified end

        optionsGroup->setLayout(optionsLayout);
        mainLayout->addWidget(optionsGroup);


        QHBoxLayout *buttonsLayout = new QHBoxLayout();

        resetButton = new QPushButton("Reset algorithm parameters"); // GB: clarified text
        fileButton = new QPushButton("Load config file"); // GB: clarified text

        buttonsLayout->addWidget(resetButton);
        connect(resetButton, SIGNAL(clicked()), this, SLOT(onResetClick()));
        buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

        buttonsLayout->addWidget(fileButton);
        connect(fileButton, SIGNAL(clicked()), this, SLOT(onLoadFileClick()));

        mainLayout->addLayout(buttonsLayout);

        setLayout(mainLayout);
    }

    void loadSettingsFromFile(QString filename) {

        std::ifstream file(filename.toStdString());
        json j;
        file>> j;

        //std::cout << std::setw(4) << j << std::endl;

        QList<float> customs = defaultParameters[Settings::DEFAULT];

        customs[0] = j["Parameter Set"]["Radius_Min"];
        customs[1] = j["Parameter Set"]["Radius_Max"];
        customs[2] = j["Parameter Set"]["CannyBlur"];
        customs[3] = j["Parameter Set"]["CannyThreshold1"];
        customs[4] = j["Parameter Set"]["CannyThreshold2"];
        customs[5] = j["Parameter Set"]["StarburstPoints"];
        customs[6] = j["Parameter Set"]["PercentageInliers"];
        customs[7] = j["Parameter Set"]["InlierIterations"];
        customs[8] = j["Parameter Set"]["EarlyTerminationPercentage"];
        customs[9] = j["Parameter Set"]["ImageAwareSupport"];
        customs[10] = j["Parameter Set"]["EarlyRejection"];

        insertCustomEntry(customs);

//        minRadiusBox->setValue(j["Parameter Set"]["Radius_Min"]);
//        maxRadiusBox->setValue(j["Parameter Set"]["Radius_Max"]);
//
//        cannyBlurBox->setValue(j["Parameter Set"]["CannyBlur"]);
//        cannyThreshold1Box->setValue(j["Parameter Set"]["CannyThreshold1"]);
//        cannyThreshold2Box->setValue(j["Parameter Set"]["CannyThreshold2"]);
//        starburstPointsBox->setValue(j["Parameter Set"]["StarburstPoints"]);
//
//        percInlierBox->setValue(j["Parameter Set"]["PercentageInliers"]);
//        iterInlierBox->setValue(j["Parameter Set"]["InlierIterations"]);
//        termPercBox->setValue(j["Parameter Set"]["EarlyTerminationPercentage"]);
//        imageAwareBox->setChecked(j["Parameter Set"]["ImageAwareSupport"]);
//        earlyRejectionBox->setChecked(j["Parameter Set"]["EarlyRejection"]);
    }

//    QMap<QString, QList<float>> defaultParameters = {
//            { "Default", {40, 80, 1.6f, 20., 40, 0, 20, 2, 95, 1, 1} },
//            { "ROI 0.3 Optimized", {40, 64, 0.5f, 10, 11, 26, 29, 9, 74, 1, 0} },
//            { "ROI 0.6 Optimized", {40, 63, 0.1f, 14, 18, 28, 25, 2, 93, 1, 1} },
//            { "Full Image Optimized", {40, 43, 6.4f, 21, 22, 26, 16, 10, 34, 0, 1} }
//    };

    QMap<Settings, QList<float>> defaultParameters = {
            { Settings::DEFAULT, {40.0f, 80.0f, 1.6f, 20.0f, 40.0f, 0.0f, 20.0f, 2.0f, 95.0f, 1.0f, 1.0f} },
            { Settings::ROI_0_3_OPTIMIZED, {10.0f, 112.0f, 0.1f, 33.0f, 64.0f, 26.0f, 21.0f, 6.0f, 22.0f, 0.0f, 1.0f} },
            { Settings::ROI_0_6_OPTIMIZED, {21.0f, 108.0f, 0.3f, 21.0f, 79.0f, 31.0f, 25.0f, 2.0f, 14.0f, 1.0f, 0.0f} },
            { Settings::FULL_IMAGE_OPTIMIZED, {19.0f, 115.0f, 0.4f, 46.0f, 63.0f, 25.0f, 38.0f, 10.0f, 27.0f, 0.0f, 1.0f} },
            { Settings::AUTOMATIC_PARAMETRIZATION, {-1.0f, -1.0f, 0.4f, 46.0f, 63.0f, 25.0f, 38.0f, 10.0f, 27.0f, 0.0f, 1.0f} },
            { Settings::CUSTOM, {-1.0f, -1.0f, 0.4f, 46.0f, 63.0f, 25.0f, 38.0f, 10.0f, 27.0f, 0.0f, 1.0f} } // GB added
    };


private slots:

    void onParameterConfigSelection(QString configKey) {
        setConfigIndex(configKey);
        QList<float>& selectedParameter = getCurrentParameters();

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        cannyBlurBox->setValue(selectedParameter[2]);
        cannyThreshold1Box->setValue(selectedParameter[3]);
        cannyThreshold2Box->setValue(selectedParameter[4]);
        starburstPointsBox->setValue(selectedParameter[5]);

        percInlierBox->setValue(selectedParameter[6]);
        iterInlierBox->setValue(selectedParameter[7]);
        termPercBox->setValue(selectedParameter[8]);
        imageAwareBox->setChecked(selectedParameter[9]);
        earlyRejectionBox->setChecked(selectedParameter[10]);

        // Then the specific ones that are set by autoParam
        if(isAutoParamEnabled()) {
            minRadiusBox->setEnabled(false);
            maxRadiusBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            minRadiusBox->setEnabled(true);
            maxRadiusBox->setEnabled(true);

            minRadiusBox->setValue(selectedParameter[0]);
            maxRadiusBox->setValue(selectedParameter[1]);
        }
        // GB modified end

        //applySpecificSettings(); // settings are only updated when apply click in pupildetectionsettingsdialog
    }

};


#endif //PUPILEXT_SWIRSKI2DSETTINGS_H
