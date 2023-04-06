
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
        PupilMethodSetting(parent), 
        purest(purest), 
        pupilDetection(pupilDetection), 
        configParameters(defaultParameters) {
        
        configParameters = applicationSettings->value("PuReSTSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();

        configIndex = applicationSettings->value("PuReSTSettings.configIndex", "Default").toString();

        createForm();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

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

    QMap<QString, QList<float>> getParameter() {
        return configParameters;
    }

    void setParameter(QMap<QString, QList<float>> params) {
        if(defaultParameters.size() == params.size())
            configParameters = params;
    }

    void reset() {
        configParameters = defaultParameters;
    }

    // GB modified begin
    bool isAutoParamEnabled() override {
        return (parameterConfigs->currentText()=="Automatic Parametrization");
    }
    // GB modified end

public slots:

    void loadSettings() override {
        configParameters = applicationSettings->value("PuReSTSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();
        configIndex = applicationSettings->value("PuReSTSettings.configIndex", "Default").toString();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

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

        configParameters[parameterConfigs->currentText()][0] = baseWidth;
        configParameters[parameterConfigs->currentText()][1] = baseHeight;

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

            configParameters[parameterConfigs->currentText()][2] = meanCanthiDistanceMM;
            configParameters[parameterConfigs->currentText()][3] = minPupilDiameterMM;
            configParameters[parameterConfigs->currentText()][4] = maxPupilDiameterMM;

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

        applicationSettings->setValue("PuReSTSettings.configParameters", QVariant::fromValue(configParameters));
        applicationSettings->setValue("PuReSTSettings.configIndex", parameterConfigs->currentText());
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

    QPushButton *resetButton;
    QComboBox *parameterConfigs;
    QPushButton *fileButton;

    void createForm() {

        QList<float> selectedParameter = configParameters.value(configIndex);

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

        QMapIterator<QString, QList<float>> i(configParameters);
        while (i.hasNext()) {
            i.next();
            parameterConfigs->addItem(i.key());
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

        QList<float> customs = defaultParameters["Default"];

        customs[2] = j["Parameter Set"]["meanCanthiDistanceMM"];
        customs[3] = j["Parameter Set"]["minPupilDiameterMM"];
        customs[4] = j["Parameter Set"]["maxPupilDiameterMM"];

        configParameters.insert("Custom", customs);

        if(parameterConfigs->findText("Custom") < 0) {
            parameterConfigs->addItem("Custom");
        }
        parameterConfigs->setCurrentText("Custom");
    }

    QMap<QString, QList<float>> defaultParameters = {
            { "Default", {320.0f, 240.0f, 27.6f, 2.0f, 8.0f} },
            { "ROI 0.3 Optimized", {320.0f, 240.0f, 65.9f, 4.7f, 17.5f} },
            { "ROI 0.6 Optimized", {320.0f, 240.0f, 58.4f, 1.5f, 12.8f} },
            { "Full Image Optimized", {320.0f, 240.0f, 99.6f, 1.8f, 7.2f} },
            { "Automatic Parametrization", {320.0f, 240.0f, -1.0f, -1.0f, -1.0f} } // GB added
    };


//    QMap<QString, QList<float>> defaultParameters = {
//            { "Default", {320.0f, 240.0f, 27.6f, 2.0f, 8.0f} },
//            { "ROI 0.3 Optimized", {320.0f, 240.0f, 40.9f, 4.8f, 11.7f} },
//            { "ROI 0.6 Optimized", {320.0f, 240.0f, 97.0f, 6.1f, 14.4f} },
//            { "Full Image Optimized", {320.0f, 240.0f, 71.3f, 2.2f, 6.3f} }
//    };

    QMap<QString, QList<float>> configParameters;
    QString configIndex;


private slots:

    void onParameterConfigSelection(QString configKey) {
        //QString configKey = parameterConfigs->itemText(parameterConfigs->currentIndex());
        QList<float> selectedParameter = configParameters.value(configKey);

        //applicationSettings->setValue("PuReSTSettings.configIndex", configKey); // GB: commented this out (possibly it was here by mistake?)


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

    void onResetClick() {
        QString configKey = parameterConfigs->itemText(parameterConfigs->currentIndex());
        configParameters[configKey] = defaultParameters.value(configKey);
        onParameterConfigSelection(configKey);
    }

    void onLoadFileClick() {
        QString filename = QFileDialog::getOpenFileName(this, tr("Open Algorithm Parameter File"), "", tr("JSON files (*.json)"));

        if(!filename.isEmpty()) {

            try {
                loadSettingsFromFile(filename);
            } catch(...) {
                QMessageBox msgBox;
                msgBox.setText("Error while loading parameter file. \nCorrect format and algorithm?");
                msgBox.exec();
            }
        }
    }

};


#endif //PUPILEXT_PURESTSETTINGS_H
