
#ifndef PUPILEXT_PURESETTINGS_H
#define PUPILEXT_PURESETTINGS_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "PupilMethodSetting.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../pupil-detection-methods/PuRe.h"

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the PuRe algorithm, displayed in the pupil detection setting dialog
*/
class PuReSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit PuReSettings(PupilDetection * pupilDetection, PuRe *pure, QWidget *parent=0) : 
        PupilMethodSetting("PuReSettings.configParameters", "PuReSettings.configIndex", parent), 
        pure(pure), 
        pupilDetection(pupilDetection) {

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

        QIcon trackOnIcon = QIcon(":/icons/Breeze/status/22/dialog-information.svg");
        QLabel *iLabel = new QLabel();
        iLabel->setPixmap(trackOnIcon.pixmap(QSize(32, 32)));
        infoLayout->addWidget(iLabel, 0, 0);


        QLabel *pLabel = new QLabel();
        pLabel->setWordWrap(true);
        pLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        pLabel->setOpenExternalLinks(true);
        pLabel->setText("Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, \"PuRe: Robust pupil detection for real-time pervasive eye tracking.\", 2018<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(pure->hasConfidence())
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

    ~PuReSettings() override = default;

    void add2(PuRe *s_pure) {
        pure2 = s_pure;
    }
    void add3(PuRe *s_pure) {
        pure3 = s_pure;
    }
    void add4(PuRe *s_pure) {
        pure4 = s_pure;
    }

public slots:

    void loadSettings() override {

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
        // GB added end

        updateSettings();
    }

    void updateSettings() override {

        // GB modified begin
        // NOTE: To support autoParam, and also 4 threaded pupil detection for 4 pupils

        // First come the parameters roughly independent from ROI size and relative pupil size 
        int baseWidth = pure->baseSize.width;
        int baseHeight = pure->baseSize.height;

        baseWidth = imageWidthBox->value();
        baseHeight = imageHeightBox->value();
        pure->baseSize = cv::Size(baseWidth, baseHeight);

        QList<float>& currentParameters = getCurrentParameters();
        currentParameters[0] = baseWidth;
        currentParameters[1] = baseHeight;

        if(pure2) {
            pure2->baseSize = cv::Size(baseWidth, baseHeight);
        }
        if(pure3) {
            pure3->baseSize = cv::Size(baseWidth, baseHeight);
        }
        if(pure4) {
            pure4->baseSize = cv::Size(baseWidth, baseHeight);
        }

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

        } else {
            float meanCanthiDistanceMM = pure->meanCanthiDistanceMM;
            float maxPupilDiameterMM = pure->maxPupilDiameterMM;
            float minPupilDiameterMM = pure->minPupilDiameterMM;

            meanCanthiDistanceMM = canthiDistanceBox->value();
            maxPupilDiameterMM = maxPupilBox->value();
            minPupilDiameterMM = minPupilBox->value();

            pure->meanCanthiDistanceMM = meanCanthiDistanceMM;
            pure->maxPupilDiameterMM = maxPupilDiameterMM;
            pure->minPupilDiameterMM = minPupilDiameterMM;

            currentParameters[2] = meanCanthiDistanceMM;
            currentParameters[3] = minPupilDiameterMM;
            currentParameters[4] = maxPupilDiameterMM;

            if(pure2) {
                pure2->meanCanthiDistanceMM = meanCanthiDistanceMM;
                pure2->maxPupilDiameterMM = maxPupilDiameterMM;
                pure2->minPupilDiameterMM = minPupilDiameterMM;
            }
            if(pure3) {
                pure3->meanCanthiDistanceMM = meanCanthiDistanceMM;
                pure3->maxPupilDiameterMM = maxPupilDiameterMM;
                pure3->minPupilDiameterMM = minPupilDiameterMM;
            }
            if(pure4) {
                pure4->meanCanthiDistanceMM = meanCanthiDistanceMM;
                pure4->maxPupilDiameterMM = maxPupilDiameterMM;
                pure4->minPupilDiameterMM = minPupilDiameterMM;
            }
            
        }
        // GB modified end

        emit onConfigChange(parameterConfigs->currentText());

        PupilMethodSetting::updateSettings();
    }

private:

    PuRe *pure;
    //PuRe *secondaryPure = nullptr; //GB: refactored
    // GB added begin
    PuRe *pure2 = nullptr;
    PuRe *pure3 = nullptr;
    PuRe *pure4 = nullptr;

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
        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));

        QGroupBox *sizeGroup = new QGroupBox("Algorithm specific: Image Size (Downscaling)"); // GB: "Algorithm specific: "
        QGroupBox *physGroup = new QGroupBox("Algorithm specific: Physical Parameter");

        QFormLayout *sizeLayout = new QFormLayout();
        QFormLayout *physLayout = new QFormLayout();

        // GB modified begin
        // GB NOTE: to fit in smaller screen area
        QLabel *widthLabel = new QLabel(tr("Image width [px]:")); // GB: px notation added
        imageWidthBox = new QSpinBox();
        imageWidthBox->setMaximum(5000);
        imageWidthBox->setValue(baseWidth);
        imageWidthBox->setFixedWidth(50); // GB
        
        QLabel *heightLabel = new QLabel(tr("Image height [px]:")); // GB: px notation added
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
            { Settings::DEFAULT, {320, 240, 27.6f, 2.0f, 8.0f} },
            { Settings::ROI_0_3_OPTIMIZED, {320, 240, 49.4f, 1.9f, 20.0f} },
            { Settings::ROI_0_6_OPTIMIZED, {320, 240, 38.7f, 1.9f, 16.8f} },
            { Settings::FULL_IMAGE_OPTIMIZED, {320, 240, 94.1f, 0.1f, 16.0f} },
            { Settings::AUTOMATIC_PARAMETRIZATION, {320, 240, -1.0f, -1.0f, -1.0f} },
            { Settings::CUSTOM, {320, 240, -1.0f, -1.0f, -1.0f} } // GB added
    };

      // Parameters from second optimization run
//    QMap<QString, QList<float>> defaultParameters = {
//            { "Default", {320, 240, 27.6f, 2.0f, 8.0f} },
//            { "ROI 0.3 Optimized", {320, 240, 49.1f, 5.4f, 11.9f} },
//            { "ROI 0.6 Optimized", {320, 240, 58.5f, 3.4f, 8.7f} },
//            { "Full Image Optimized", {320, 240, 76.1f, 2.4f, 5.6f} }
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


#endif //PUPILEXT_PURESETTINGS_H
