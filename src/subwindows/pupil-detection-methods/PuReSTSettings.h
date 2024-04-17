
#ifndef PUPILEXT_PURESTSETTINGS_H
#define PUPILEXT_PURESTSETTINGS_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../pupil-detection-methods/PuReST.h"
#include "PupilMethodSetting.h"
#include "../../SVGIconColorAdjuster.h"

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the PuReST algorithm, displayed in the pupil detection setting dialog
*/
class PuReSTSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit PuReSTSettings(PupilDetection * pupilDetection, PuReST *purest, QWidget *parent=0) : 
        PupilMethodSetting("PuReSTSettings.configParameters","PuReSTSettings.configIndex", parent), 
        purest(purest), 
        pupilDetection(pupilDetection){
        PupilMethodSetting::setDefaultParameters(defaultParameters);
        createForm();
        parameterConfigs->setCurrentText(settingsMap.key(configIndex));
        // GB added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            canthiDistanceBox->setEnabled(false);
            minPupilBox->setEnabled(false);
            maxPupilBox->setEnabled(false);
        } else {
            canthiDistanceBox->setEnabled(true);
            minPupilBox->setEnabled(true);
            maxPupilBox->setEnabled(true);
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
        pLabel->setText("Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, \"PuReST: Robust Pupil Tracking for Real-Time Pervasive Eye.\", 2018<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(purest->hasConfidence())
            confLabel = new QLabel("Info: This method does provide its own confidence.");
        else
            confLabel = new QLabel("Info: This method does not provide its own confidence, use the outline confidence.");
        confLabel->setWordWrap(true);
        infoLayout->addWidget(confLabel, 2, 0);

#if _DEBUG
        QLabel *warnLabel = new QLabel("CAUTION: Debug build may perform very slow. Use release build or adjust processing speed to not risk memory overflow.");
        warnLabel->setWordWrap(true);
        warnLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(warnLabel, 3, 0);
#endif
        infoBox->setLayout(infoLayout);
        // GB modified end
    }

    ~PuReSTSettings() override {
    };

    void add2(PuReST *s_purest) {
        purest2 = s_purest;
    }
    void add3(PuReST *s_purest) {
        purest3 = s_purest;
    }
    void add4(PuReST *s_purest) {
        purest4 = s_purest;
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

            canthiDistanceBox->setEnabled(false);
            minPupilBox->setEnabled(false);
            maxPupilBox->setEnabled(false);
        } else {
            pupilDetection->setAutoParamEnabled(false);
            canthiDistanceBox->setEnabled(true);
            minPupilBox->setEnabled(true);
            maxPupilBox->setEnabled(true);
        } 
        // added end

        updateSettings();
    }

    void updateSettings() override {

        // GB modified begin
        
        // First come the parameters roughly independent from ROI size and relative pupil size 
        int baseWidth = purest->baseSize.width;
        int baseHeight = purest->baseSize.height;

        baseWidth = imageWidthBox->value();
        baseHeight = imageHeightBox->value();
        purest->baseSize = cv::Size(baseWidth, baseHeight);

        QList<float>& currentParameters = getCurrentParameters();
        currentParameters[0] = baseWidth;
        currentParameters[1] = baseHeight;

        if(purest2) {
            purest2->baseSize = cv::Size(baseWidth, baseHeight);
        }
        if(purest3) {
            purest3->baseSize = cv::Size(baseWidth, baseHeight);
        }
        if(purest4) {
            purest4->baseSize = cv::Size(baseWidth, baseHeight);
        }

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

        } else {
            float meanCanthiDistanceMM = purest->meanCanthiDistanceMM;
            float maxPupilDiameterMM = purest->maxPupilDiameterMM;
            float minPupilDiameterMM = purest->minPupilDiameterMM;

            meanCanthiDistanceMM = canthiDistanceBox->value();
            maxPupilDiameterMM = maxPupilBox->value();
            minPupilDiameterMM = minPupilBox->value();

            purest->meanCanthiDistanceMM = meanCanthiDistanceMM;
            purest->maxPupilDiameterMM = maxPupilDiameterMM;
            purest->minPupilDiameterMM = minPupilDiameterMM;

            currentParameters[2] = meanCanthiDistanceMM;
            currentParameters[3] = minPupilDiameterMM;
            currentParameters[4] = maxPupilDiameterMM;

            if(purest2) {
                purest2->meanCanthiDistanceMM = meanCanthiDistanceMM;
                purest2->maxPupilDiameterMM = maxPupilDiameterMM;
                purest2->minPupilDiameterMM = minPupilDiameterMM;
            }
            if(purest3) {
                purest3->meanCanthiDistanceMM = meanCanthiDistanceMM;
                purest3->maxPupilDiameterMM = maxPupilDiameterMM;
                purest3->minPupilDiameterMM = minPupilDiameterMM;
            }
            if(purest4) {
                purest4->meanCanthiDistanceMM = meanCanthiDistanceMM;
                purest4->maxPupilDiameterMM = maxPupilDiameterMM;
                purest4->minPupilDiameterMM = minPupilDiameterMM;
            }
            
        }
        // GB modified end

        emit onConfigChange(parameterConfigs->currentText());

        PupilMethodSetting::updateSettings();
    }

private:

    PuReST *purest;
    //PuReST *secondaryPurest = nullptr; // GB: refactored
    // GB added begin
    PuReST *purest2 = nullptr;
    PuReST *purest3 = nullptr;
    PuReST *purest4 = nullptr;

    PupilDetection *pupilDetection;
    // GB added end

    QSpinBox *imageWidthBox;
    QSpinBox *imageHeightBox;

    QDoubleSpinBox *canthiDistanceBox;
    QDoubleSpinBox *maxPupilBox;
    QDoubleSpinBox *minPupilBox;

    void createForm() {
        PupilMethodSetting::loadSettings();
        QList<float>& selectedParameter = getCurrentParameters();

        float meanCanthiDistanceMM = selectedParameter[2];
        float maxPupilDiameterMM = selectedParameter[4];
        float minPupilDiameterMM = selectedParameter[3];

        int baseWidth = selectedParameter[0];
        int baseHeight = selectedParameter[1];

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

        QHBoxLayout *configsNoteLayout = new QHBoxLayout();
        QLabel* configsNoteLabel = new QLabel(tr("Note: Configurations marked with an asterisk (*) are recommended for Basler\nacA2040-120um (1/1.8\" sensor format) camera(s) equipped with f=50 mm 2/3\"\nnominal sensor format lens, using 4:3 aspect ratio pupil detection ROI(s)."));
        //configsNoteLabel->setFixedWidth((int)this->size().width()/2);
        configsNoteLabel->setFixedWidth(420);
        configsNoteLabel->setFixedHeight(45);
        configsNoteLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        configsNoteLayout->addWidget(configsNoteLabel);
        mainLayout->addLayout(configsNoteLayout);

        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));

        QGroupBox *sizeGroup = new QGroupBox("Algorithm specific: Image Size (Downscaling)"); // GB: "Algorithm specific: "
        QGroupBox *physGroup = new QGroupBox("Algorithm specific: Physical Parameter");

        QFormLayout *sizeLayout = new QFormLayout();
        QFormLayout *physLayout = new QFormLayout();

        // GB modified begin
        // GB NOTE: to fit in smaller screen area
        QLabel *widthLabel = new QLabel(tr("Image width [px]:")); // GB: px added
        imageWidthBox = new QSpinBox();
        imageWidthBox->setMaximum(5000);
        imageWidthBox->setValue(baseWidth);
        imageWidthBox->setFixedWidth(50); // GB

        QLabel *heightLabel = new QLabel(tr("Image height [px]:")); // GB: px added
        imageHeightBox = new QSpinBox();
        imageHeightBox->setMaximum(5000);
        imageHeightBox->setValue(baseHeight);
        imageHeightBox->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow1 = new QHBoxLayout;
        layoutRow1->addWidget(imageWidthBox);
        QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow1->addSpacerItem(sp1);
        layoutRow1->addWidget(heightLabel);
        layoutRow1->addWidget(imageHeightBox);
        //layoutRow1->addSpacerItem(sp);
        sizeLayout->addRow(widthLabel, layoutRow1);

        sizeGroup->setLayout(sizeLayout);
        mainLayout->addWidget(sizeGroup);


        QLabel *canthiDistanceLabel = new QLabel(tr("Mean Canthi Distance [mm]:"));
        canthiDistanceBox = new QDoubleSpinBox();
        canthiDistanceBox->setValue(meanCanthiDistanceMM);
        canthiDistanceBox->setFixedWidth(50); // GB
        physLayout->addRow(canthiDistanceLabel, canthiDistanceBox);

        QLabel *maxPupilLabel = new QLabel(tr("Max. Pupil Size [mm]:"));
        maxPupilBox = new QDoubleSpinBox();
        maxPupilBox->setValue(maxPupilDiameterMM);
        maxPupilBox->setFixedWidth(50); // GB

        QLabel *minPupilLabel = new QLabel(tr("Min. Pupil Size [mm]:"));
        minPupilBox = new QDoubleSpinBox();
        minPupilBox->setValue(minPupilDiameterMM);
        minPupilBox->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow2 = new QHBoxLayout;
        layoutRow2->addWidget(maxPupilBox);
        QSpacerItem *sp2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow2->addSpacerItem(sp2);
        layoutRow2->addWidget(minPupilLabel);
        layoutRow2->addWidget(minPupilBox);
        //layoutRow2->addSpacerItem(sp);
        physLayout->addRow(maxPupilLabel, layoutRow2);
        // GB modified end

        physGroup->setLayout(physLayout);
        mainLayout->addWidget(physGroup);


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

        customs[2] = j["Parameter Set"]["meanCanthiDistanceMM"];
        customs[3] = j["Parameter Set"]["minPupilDiameterMM"];
        customs[4] = j["Parameter Set"]["maxPupilDiameterMM"];

        insertCustomEntry(customs);
    }

    QMap<Settings, QList<float>> defaultParameters = {
            { Settings::DEFAULT, {320.0f, 240.0f, 27.6f, 2.0f, 8.0f} },
            { Settings::ROI_0_3_OPTIMIZED, {320.0f, 240.0f, 65.9f, 4.7f, 17.5f} },
            { Settings::ROI_0_6_OPTIMIZED, {320.0f, 240.0f, 58.4f, 1.5f, 12.8f} },
            { Settings::FULL_IMAGE_OPTIMIZED, {320.0f, 240.0f, 99.6f, 1.8f, 7.2f} },
            { Settings::AUTOMATIC_PARAMETRIZATION, {320.0f, 240.0f, -1.0f, -1.0f, -1.0f} },
            { Settings::CUSTOM, {320.0f, 240.0f, -1.0f, -1.0f, -1.0f} } // GB added
    };


//    QMap<QString, QList<float>> defaultParameters = {
//            { "Default", {320.0f, 240.0f, 27.6f, 2.0f, 8.0f} },
//            { "ROI 0.3 Optimized", {320.0f, 240.0f, 40.9f, 4.8f, 11.7f} },
//            { "ROI 0.6 Optimized", {320.0f, 240.0f, 97.0f, 6.1f, 14.4f} },
//            { "Full Image Optimized", {320.0f, 240.0f, 71.3f, 2.2f, 6.3f} }
//    };


private slots:

    void onParameterConfigSelection(QString configKey) {
        setConfigIndex(configKey);
        QList<float>& selectedParameter = getCurrentParameters();

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        imageWidthBox->setValue(selectedParameter[0]);
        imageHeightBox->setValue(selectedParameter[1]);

        // Then the specific ones that are set by autoParam
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            canthiDistanceBox->setEnabled(false);
            minPupilBox->setEnabled(false);
            maxPupilBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            canthiDistanceBox->setEnabled(true);
            minPupilBox->setEnabled(true);
            maxPupilBox->setEnabled(true);
            
            canthiDistanceBox->setValue(selectedParameter[2]);
            minPupilBox->setValue(selectedParameter[3]);
            maxPupilBox->setValue(selectedParameter[4]);
        }
        // GB modified end

        //updateSettings(); // settings are only updated when apply click in pupildetectionsettingsdialog
    }

};


#endif //PUPILEXT_PURESTSETTINGS_H
