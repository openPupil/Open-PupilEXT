
#ifndef PUPILEXT_ELSESETTINGS_H
#define PUPILEXT_ELSESETTINGS_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "PupilMethodSetting.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../pupil-detection-methods/ElSe.h"

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the ElSe algorithm, displayed in the pupil detection setting dialog
*/
class ElSeSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit ElSeSettings(PupilDetection * pupilDetection, ElSe *m_else, QWidget *parent=0) : 
        PupilMethodSetting(parent), 
        p_else(m_else), 
        pupilDetection(pupilDetection), 
        configParameters(defaultParameters)  {

        configParameters = applicationSettings->value("ElSeSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();

        configIndex = applicationSettings->value("ElSeSettings.configIndex", "Default").toString();

        createForm();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

        // GB added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            minAreaBox->setEnabled(false);
            maxAreaBox->setEnabled(false);
        } else {
            minAreaBox->setEnabled(true);
            maxAreaBox->setEnabled(true);
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
        pLabel->setText("Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci, \"ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments.\", 2016<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini / University of Tübingen");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB NOTE: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(p_else->hasConfidence())
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

    ~ElSeSettings() override = default;

    void add2(ElSe *s_else) {
        else2 = s_else;
    }
    void add3(ElSe *s_else) {
        else3 = s_else;
    }
    void add4(ElSe *s_else) {
        else4 = s_else;
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
        configParameters = applicationSettings->value("ElSeSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();
        configIndex = applicationSettings->value("ElSeSettings.configIndex", "Default").toString();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

        // GG added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamEnabled(true);
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

            minAreaBox->setEnabled(false);
            maxAreaBox->setEnabled(false);
        } else {
            pupilDetection->setAutoParamEnabled(false);
            minAreaBox->setEnabled(true);
            maxAreaBox->setEnabled(true);
        } 
        // GB added end

//        QList<float> selectedParameter = configParameters.value(configIndex);
//
//        minAreaBox->setValue(selectedParameter[0]);
//        maxAreaBox->setValue(selectedParameter[1]);

        updateSettings();
    }

    void updateSettings() override {

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        // (NONE HERE)

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

        } else {
            float minAreaRatio = p_else->minAreaRatio;
            float maxAreaRatio = p_else->maxAreaRatio;

            minAreaRatio = minAreaBox->value();
            maxAreaRatio = maxAreaBox->value();

            p_else->minAreaRatio = minAreaRatio;
            p_else->maxAreaRatio = maxAreaRatio;

            configParameters[parameterConfigs->currentText()][0] = minAreaRatio;
            configParameters[parameterConfigs->currentText()][1] = maxAreaRatio;

            if(else2) {
                else2->minAreaRatio = minAreaRatio;
                else2->maxAreaRatio = maxAreaRatio;
            }
            if(else3) {
                else3->minAreaRatio = minAreaRatio;
                else3->maxAreaRatio = maxAreaRatio;
            }
            if(else4) {
                else4->minAreaRatio = minAreaRatio;
                else4->maxAreaRatio = maxAreaRatio;
            }
            
        }
        // GB modified end

        emit onConfigChange(parameterConfigs->currentText());

        applicationSettings->setValue("ElSeSettings.configParameters", QVariant::fromValue(configParameters));
        applicationSettings->setValue("ElSeSettings.configIndex", parameterConfigs->currentText());
    }

private:

    ElSe *p_else;
    //ElSe *secondaryElse = nullptr; // GB refactored
    // GB added begin
    ElSe *else2 = nullptr;
    ElSe *else3 = nullptr;
    ElSe *else4 = nullptr;

    PupilDetection *pupilDetection; 
    // GB added end

    QDoubleSpinBox *minAreaBox;
    QDoubleSpinBox *maxAreaBox;

    QPushButton *resetButton;
    QComboBox *parameterConfigs;
    QPushButton *fileButton;

    void createForm() {

        QList<float> selectedParameter = configParameters.value(configIndex);

        float minAreaRatio = selectedParameter[0];
        float maxAreaRatio = selectedParameter[1];

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

        QGroupBox *sizeGroup = new QGroupBox("Algorithm specific: Pupil Area Proportion"); // GB: "Algorithm specific: "

        QFormLayout *sizeLayout = new QFormLayout();

        QLabel *minAreaLabel = new QLabel(tr("Min. Area [%]:"));
        minAreaBox = new QDoubleSpinBox();
        minAreaBox->setDecimals(3);
        minAreaBox->setSingleStep(0.001);
        minAreaBox->setMaximum(100);
        minAreaBox->setValue(minAreaRatio);
        minAreaBox->setFixedWidth(60); // GB
        sizeLayout->addRow(minAreaLabel, minAreaBox);

        QLabel *maxAreaLabel = new QLabel(tr("Max. Area [%]:"));
        maxAreaBox = new QDoubleSpinBox();
        maxAreaBox->setDecimals(3);
        maxAreaBox->setSingleStep(0.001);
        maxAreaBox->setMaximum(100);
        maxAreaBox->setValue(maxAreaRatio);
        maxAreaBox->setFixedWidth(60); // GB
        sizeLayout->addRow(maxAreaLabel, maxAreaBox);

        sizeGroup->setLayout(sizeLayout);
        mainLayout->addWidget(sizeGroup);

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

        customs[0] = j["Parameter Set"]["minAreaRatio"];
        customs[1] = j["Parameter Set"]["maxAreaRatio"];

        configParameters.insert("Custom", customs);

        if(parameterConfigs->findText("Custom") < 0) {
            parameterConfigs->addItem("Custom");
        }
        parameterConfigs->setCurrentText("Custom");

    }

    QMap<QString, QList<float>> defaultParameters = {
            { "Default", {0.005f, 0.2f} },
            { "ROI 0.3 Optimized", {0.001f, 0.823f} },
            { "ROI 0.6 Optimized", {0.001f, 0.131f} },
            { "Full Image Optimized", {0.001f, 0.038f} },
            { "Automatic Parametrization", {-1.0f, -1.0f} } // GB added
    };

    QMap<QString, QList<float>> configParameters;
    QString configIndex;


private slots:

    void onParameterConfigSelection(QString configKey) {
        QList<float> selectedParameter = configParameters.value(configKey);

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        // (NONE HERE)

        // Then the specific ones that are set by autoParam
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            minAreaBox->setEnabled(false);
            maxAreaBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            minAreaBox->setEnabled(true);
            maxAreaBox->setEnabled(true);
            
            minAreaBox->setValue(selectedParameter[0]);
            maxAreaBox->setValue(selectedParameter[1]);
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


#endif //PUPILEXT_ELSESETTINGS_H
