
#ifndef PUPILEXT_STARBURSTSETTINGS_H
#define PUPILEXT_STARBURSTSETTINGS_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "PupilMethodSetting.h"
#include "../../pupil-detection-methods/Starburst.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../SVGIconColorAdjuster.h"

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the Starburst algorithm, displayed in the pupil detection setting dialog
*/
class StarburstSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit StarburstSettings(PupilDetection * pupilDetection, Starburst *m_starburst, QWidget *parent=0) : 
        PupilMethodSetting("StarburstSettings.configParameters","StarburstSettings.configIndex", parent), 
        p_starburst(m_starburst), 
        pupilDetection(pupilDetection){

        PupilMethodSetting::setDefaultParameters(defaultParameters);
        createForm();
        parameterConfigs->setCurrentText(settingsMap.key(configIndex));
        // GB added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            edgeThresholdBox->setEnabled(false);
            //
            crRatioBox->setEnabled(false);
            crWindowSizeBox->setEnabled(false);
        } else {
            edgeThresholdBox->setEnabled(true);
            //
            crRatioBox->setEnabled(true);
            crWindowSizeBox->setEnabled(true);
        } 
        // GB added end

        QGridLayout *infoLayout = new QGridLayout(infoBox);

        QIcon trackOnIcon = SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/status/22/dialog-information.svg"), applicationSettings);
        QLabel *iLabel = new QLabel();
        iLabel->setPixmap(trackOnIcon.pixmap(QSize(32, 32)));
        infoLayout->addWidget(iLabel, 0, 0);

        QLabel *pLabel = new QLabel();
        pLabel->setWordWrap(true);
        pLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        pLabel->setOpenExternalLinks(true);
        pLabel->setText("Li, Dongheng & Winfield, D. & Parkhurst, D.J., \"Starburst: A hybrid algorithm for video-based eye tracking combining feature-based and model-based approaches.\", 2005<br/>Part of the <a href=\"http://thirtysixthspan.com/openEyes/software.html\">cvEyeTracker</a> software License: <a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">GPL</a>");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB NOTE: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(p_starburst->hasConfidence())
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

    ~StarburstSettings() override = default;

    void add2(Starburst *s_starburst) {
        starburst2 = s_starburst;
    }
    void add3(Starburst *s_starburst) {
        starburst3 = s_starburst;
    }
    void add4(Starburst *s_starburst) {
        starburst4 = s_starburst;
    }

public slots:

    void loadSettings() override {
        PupilMethodSetting::loadSettings();

        // GB added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamEnabled(true);
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

            edgeThresholdBox->setEnabled(false);
            //
            crRatioBox->setEnabled(false);
            crWindowSizeBox->setEnabled(false);
        } else {
            pupilDetection->setAutoParamEnabled(false);
            edgeThresholdBox->setEnabled(true);
            //
            crRatioBox->setEnabled(true);
            crWindowSizeBox->setEnabled(true);
        } 
        // GB added end

        updateSettings();
    }

    void updateSettings() override {

        // GB modified begin
        
        // First come the parameters roughly independent from ROI size and relative pupil size 
        int rays = p_starburst->rays;				            //number of rays to use to detect feature points
        int min_feature_candidates = p_starburst->min_feature_candidates;	//minimum number of pupil feature candidates

        p_starburst->rays = numRaysBox->value();
        p_starburst->min_feature_candidates = minFeatureCandidatesBox->value();

        QList<float>& currentParameters = getCurrentParameters();
        currentParameters[1] = numRaysBox->value();
        currentParameters[2] = minFeatureCandidatesBox->value();

        if(starburst2) {
            starburst2->rays = numRaysBox->value();
            starburst2->min_feature_candidates = minFeatureCandidatesBox->value();
        }
        if(starburst3) {
            starburst3->rays = numRaysBox->value();
            starburst3->min_feature_candidates = minFeatureCandidatesBox->value();
        }
        if(starburst4) {
            starburst4->rays = numRaysBox->value();
            starburst4->min_feature_candidates = minFeatureCandidatesBox->value();
        }

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            // TODO: GET VALUE FROM pupildetection failsafe
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

        } else {

            int edge_threshold = p_starburst->edge_threshold;		//threshold of pupil edge points detection
            //
            int corneal_reflection_ratio_to_image_size = p_starburst->corneal_reflection_ratio_to_image_size; // approx max size of the reflection relative to image height -> height/this
            int crWindowSize = p_starburst->crWindowSize;		    //corneal reflection search window size

            p_starburst->edge_threshold = edgeThresholdBox->value();
            //
            p_starburst->corneal_reflection_ratio_to_image_size = crRatioBox->value();
            p_starburst->crWindowSize = crWindowSizeBox->value();

            currentParameters[0] = edgeThresholdBox->value();
            //
            currentParameters[3] = crRatioBox->value(); 
            currentParameters[4] = crWindowSizeBox->value(); 

            if(starburst2) {
                starburst2->edge_threshold = edgeThresholdBox->value();
                //
                starburst2->corneal_reflection_ratio_to_image_size = crRatioBox->value();
                starburst2->crWindowSize = crWindowSizeBox->value();
            }
            if(starburst3) {
                starburst3->edge_threshold = edgeThresholdBox->value();
                //
                starburst3->corneal_reflection_ratio_to_image_size = crRatioBox->value();
                starburst3->crWindowSize = crWindowSizeBox->value();
            }
            if(starburst4) {
                starburst4->edge_threshold = edgeThresholdBox->value();
                //
                starburst4->corneal_reflection_ratio_to_image_size = crRatioBox->value();
                starburst4->crWindowSize = crWindowSizeBox->value();
            }
            
        }
        // GB modified end

        emit onConfigChange(parameterConfigs->currentText());

        PupilMethodSetting::updateSettings();
    }

private:

    Starburst *p_starburst;
    //Starburst *secondaryStarburst = nullptr; // GB: refactored
    // GB added begin
    Starburst *starburst2 = nullptr;
    Starburst *starburst3 = nullptr;
    Starburst *starburst4 = nullptr;

    PupilDetection *pupilDetection; 
    // GB added end

    QSpinBox *edgeThresholdBox;
    QSpinBox *numRaysBox;
    QSpinBox *minFeatureCandidatesBox;
    QSpinBox *crRatioBox;
    QSpinBox *crWindowSizeBox;

    void createForm() {
        PupilMethodSetting::loadSettings();
        QList<float> selectedParameter = configParameters.value(configIndex);

        int edge_threshold = selectedParameter[0];
        int rays = selectedParameter[1];
        int min_feature_candidates = selectedParameter[2];
        int corneal_reflection_ratio_to_image_size = selectedParameter[3];
        int crWindowSize = selectedParameter[4];


        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QHBoxLayout *configsLayout = new QHBoxLayout();

        parameterConfigs = new QComboBox();
        // GB modified begin
        QLabel *parameterConfigsLabel = new QLabel(tr("Parameter configuration:"));
        parameterConfigs->setFixedWidth(250);
        configsLayout->addWidget(parameterConfigsLabel);
        // GB modified end
        configsLayout->addWidget(parameterConfigs);

        for (QMap<QString, Settings>::const_iterator cit = settingsMap.cbegin(); cit != settingsMap.cend(); cit++)
        {
            parameterConfigs->addItem(cit.key());
        }
        connect(parameterConfigs, SIGNAL(currentTextChanged(QString)), this, SLOT(onParameterConfigSelection(QString)));

        mainLayout->addLayout(configsLayout);
        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));

        QGroupBox *edgeGroup = new QGroupBox("Algorithm specific: Edge Detection"); // GB: "Algorithm specific: "
        QGroupBox *crGroup = new QGroupBox("Algorithm specific: Corneal Reflection (CR)");

        QFormLayout *edgeLayout = new QFormLayout();
        QFormLayout *crLayout = new QFormLayout();

        QLabel *edgeThresholdLabel = new QLabel(tr("Edge Threshold:"));
        edgeThresholdBox = new QSpinBox();
        edgeThresholdBox->setMaximum(1000);
        edgeThresholdBox->setValue(edge_threshold);
        edgeThresholdBox->setFixedWidth(50); // GB
        edgeLayout->addRow(edgeThresholdLabel, edgeThresholdBox);

        QLabel *numRaysLabel = new QLabel(tr("Number of Rays:"));
        numRaysBox = new QSpinBox();
        numRaysBox->setValue(rays);
        numRaysBox->setFixedWidth(50); // GB
        edgeLayout->addRow(numRaysLabel, numRaysBox);

        QLabel *minFeatureCandidatesLabel = new QLabel(tr("Min. Feature Candidates:"));
        minFeatureCandidatesBox = new QSpinBox();
        minFeatureCandidatesBox->setValue(min_feature_candidates);
        minFeatureCandidatesBox->setFixedWidth(50); // GB
        edgeLayout->addRow(minFeatureCandidatesLabel, minFeatureCandidatesBox);

        edgeGroup->setLayout(edgeLayout);
        mainLayout->addWidget(edgeGroup);

        QLabel *crRatioLabel = new QLabel(tr("CR Ratio (to Image Height):"));
        crRatioBox = new QSpinBox();
        crRatioBox->setValue(corneal_reflection_ratio_to_image_size);
        crRatioBox->setFixedWidth(50); // GB
        crLayout->addRow(crRatioLabel, crRatioBox);

        QLabel *crWindowSizeLabel = new QLabel(tr("CR Window Size [px]:"));
        crWindowSizeBox = new QSpinBox();
        crWindowSizeBox->setMaximum(5000);
        crWindowSizeBox->setValue(crWindowSize);
        crWindowSizeBox->setFixedWidth(50); // GB
        crLayout->addRow(crWindowSizeLabel, crWindowSizeBox);

        crGroup->setLayout(crLayout);
        mainLayout->addWidget(crGroup);

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

        customs[0] = j["Parameter Set"]["edge_threshold"];
        customs[1] =j["Parameter Set"]["rays"];
        customs[2] =j["Parameter Set"]["min_feature_candidates"];
        customs[3] =j["Parameter Set"]["corneal_reflection_ratio_to_image_size"];
        customs[4] =j["Parameter Set"]["crWindowSize"];


      insertCustomEntry(customs);

    }

    QMap<Settings, QList<float>> defaultParameters = {
            { Settings::DEFAULT, {20.0f, 18.0f, 10.0f, 10.0f, 301.0f} },
            { Settings::ROI_0_3_OPTIMIZED, {77.0f, 8.0f, 2.0f, 4.0f, 417.0f} },
            { Settings::ROI_0_6_OPTIMIZED, {27.0f, 8.0f, 1.0f, 10.0f, 197.0f} },
            { Settings::FULL_IMAGE_OPTIMIZED, {21.0f, 32.0f, 7.0f, 10.0f, 433.0f} },
            { Settings::AUTOMATIC_PARAMETRIZATION, {-1.0f, 8.0f, 7.0f, -1.0f, -1.0f} },
            { Settings::CUSTOM, {-1.0f, 8.0f, 7.0f, -1.0f, -1.0f} } // GB added
    };


private slots:

    void onParameterConfigSelection(QString configKey) {
        setConfigIndex(configKey);
        QList<float>& selectedParameter = getCurrentParameters();

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        numRaysBox->setValue(selectedParameter[1]);
        minFeatureCandidatesBox->setValue(selectedParameter[2]);

        // Then the specific ones that are set by autoParam
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            edgeThresholdBox->setEnabled(false);
            //
            crRatioBox->setEnabled(false);
            crWindowSizeBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            edgeThresholdBox->setEnabled(true);
            //
            crRatioBox->setEnabled(true);
            crWindowSizeBox->setEnabled(true);

            edgeThresholdBox->setValue(selectedParameter[0]);
            //
            crRatioBox->setValue(selectedParameter[3]);
            crWindowSizeBox->setValue(selectedParameter[4]);
        }
        // GB modified end

        //updateSettings(); // settings are only updated when apply click in pupildetectionsettingsdialog
    }

};


#endif //PUPILEXT_STARBURSTSETTINGS_H
