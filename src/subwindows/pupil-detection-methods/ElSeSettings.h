#pragma once

/**
    @authors Moritz Lode, Gabor Benyei, Attila Boncser
*/

#include "PupilMethodSetting.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../pupil-detection-methods/ElSe.h"
#include "../../SVGIconColorAdjuster.h"

#include "json.h"
#include "../../supportFunctions.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the ElSe algorithm, displayed in the pupil detection setting dialog
*/
class ElSeSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    explicit ElSeSettings(PupilDetection * pupilDetection, ElSe *m_else, QWidget *parent=0) : 
        PupilMethodSetting("ElSeSettings.configParameters","ElSeSettings.configIndex", parent), 
        p_else(m_else), 
        pupilDetection(pupilDetection)  {
        
        PupilMethodSetting::setDefaultParameters(defaultParameters);
        createForm();
        configsBox->setCurrentText(settingsMap.key(configIndex));

        if(isAutoParamEnabled()) {
            minAreaBox->setEnabled(false);
            maxAreaBox->setEnabled(false);
        } else {
            minAreaBox->setEnabled(true);
            maxAreaBox->setEnabled(true);
        }

        QVBoxLayout *infoLayout = new QVBoxLayout(infoBox);
        QHBoxLayout *infoLayoutRow1 = new QHBoxLayout();
        QPushButton *iLabelFakeButton = new QPushButton();
        iLabelFakeButton->setFlat(true);
        iLabelFakeButton->setAttribute(Qt::WA_NoSystemBackground, true);
        iLabelFakeButton->setAttribute(Qt::WA_TranslucentBackground, true);
        iLabelFakeButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px }");
        iLabelFakeButton->setIcon(SVGIconColorAdjuster::loadAndAdjustColors(QString(":/icons/Breeze/status/22/dialog-information.svg"), applicationSettings));
        iLabelFakeButton->setFixedSize(QSize(32,32));
        iLabelFakeButton->setIconSize(QSize(32,32));
        infoLayoutRow1->addWidget(iLabelFakeButton);

        QLabel *pLabel = new QLabel();
        pLabel->setWordWrap(true);
        pLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        pLabel->setOpenExternalLinks(true);
        SupportFunctions::setSmallerLabelFontSize(pLabel);
        pLabel->setText("Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci, \"ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments.\", 2016<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini / University of Tübingen");
        infoLayoutRow1->addWidget(pLabel);

        infoLayout->addLayout(infoLayoutRow1);

        QLabel *confLabel;
        if(p_else->hasConfidence())
            confLabel = new QLabel("Info: This method does provide its own confidence.");
        else
            confLabel = new QLabel("Info: This method does not provide its own confidence, use the outline confidence.");
        SupportFunctions::setSmallerLabelFontSize(confLabel);
        confLabel->setWordWrap(true);
        infoLayout->addWidget(confLabel);

        QLabel *infoLabel = new QLabel("CAUTION: Processing using this algorithm may be very slow, reduce the camera acquiring fps accordingly.");
        SupportFunctions::setSmallerLabelFontSize(infoLabel);
        infoLabel->setWordWrap(true);
        infoLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(infoLabel);

#if _DEBUG
        QLabel *warnLabel = new QLabel("CAUTION: Debug build may perform very slow. Use release build or adjust processing speed to not risk memory overflow.");
        SupportFunctions::setSmallerLabelFontSize(warnLabel);
        warnLabel->setWordWrap(true);
        warnLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(warnLabel);
#endif
        infoBox->setLayout(infoLayout);
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

public slots:

    void loadSettings() override {
        PupilMethodSetting::loadSettings();

        if(isAutoParamEnabled()) {
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

//        QList<float> selectedParameter = configParameters.value(configIndex);
//
//        minAreaBox->setValue(selectedParameter[0]);
//        maxAreaBox->setValue(selectedParameter[1]);

        applySpecificSettings();
    }

    void applySpecificSettings() override {

        // First come the parameters roughly independent from ROI size and relative pupil size 
        // (NONE HERE)

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(isAutoParamEnabled()) {
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

            QList<float>& currentParameters = getCurrentParameters();
            currentParameters[0] = minAreaRatio;
            currentParameters[1] = maxAreaRatio;

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

        emit onConfigChange(configsBox->currentText());
    }

    void applyAndSaveSpecificSettings() override {
        applySpecificSettings();
        PupilMethodSetting::saveSpecificSettings();
    }

private:

    ElSe *p_else;
    ElSe *else2 = nullptr;
    ElSe *else3 = nullptr;
    ElSe *else4 = nullptr;

    PupilDetection *pupilDetection;

    QDoubleSpinBox *minAreaBox;
    QDoubleSpinBox *maxAreaBox;

    void createForm() {
        PupilMethodSetting::loadSettings();
        QList<float>& selectedParameter = getCurrentParameters();

        float minAreaRatio = selectedParameter[0];
        float maxAreaRatio = selectedParameter[1];

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QHBoxLayout *configsLayout = new QHBoxLayout();

        configsBox = new QComboBox();
        QLabel *parameterConfigsLabel = new QLabel(tr("Parameter configuration:"));
        configsBox->setFixedWidth(250);
        configsLayout->addWidget(parameterConfigsLabel);
        configsLayout->addWidget(configsBox);

        for (QMap<QString, Settings>::const_iterator cit = settingsMap.cbegin(); cit != settingsMap.cend(); cit++)
        {
            configsBox->addItem(cit.key());
        }

        connect(configsBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onParameterConfigSelection(QString)));

        mainLayout->addLayout(configsLayout);

        QHBoxLayout *configsNoteLayout = new QHBoxLayout();
        QLabel* configsNoteLabel = new QLabel(tr("Note: Configurations marked with an asterisk (*) are recommended for Basler\nacA2040-120um (1/1.8\" sensor format) camera(s) equipped with f=50 mm 2/3\"\nnominal sensor format lens, using 4:3 aspect ratio pupil detection ROI(s)."));
        SupportFunctions::setSmallerLabelFontSize(configsNoteLabel);
        configsNoteLabel->setFixedHeight(60);
        configsNoteLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        configsNoteLayout->addWidget(configsNoteLabel);
        mainLayout->addLayout(configsNoteLayout);

        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));

        QGroupBox *sizeGroup = new QGroupBox("Algorithm specific: Pupil Area Proportion");

        QFormLayout *sizeLayout = new QFormLayout();

        QLabel *minAreaLabel = new QLabel(tr("Min. Area [%]:"));
        minAreaBox = new QDoubleSpinBox();
        minAreaBox->setDecimals(3);
        minAreaBox->setSingleStep(0.001);
        minAreaBox->setMaximum(100);
        minAreaBox->setValue(minAreaRatio);
        minAreaBox->setFixedWidth(80);
        sizeLayout->addRow(minAreaLabel, minAreaBox);

        QLabel *maxAreaLabel = new QLabel(tr("Max. Area [%]:"));
        maxAreaBox = new QDoubleSpinBox();
        maxAreaBox->setDecimals(3);
        maxAreaBox->setSingleStep(0.001);
        maxAreaBox->setMaximum(100);
        maxAreaBox->setValue(maxAreaRatio);
        maxAreaBox->setFixedWidth(80);
        sizeLayout->addRow(maxAreaLabel, maxAreaBox);

        sizeGroup->setLayout(sizeLayout);
        mainLayout->addWidget(sizeGroup);

        QHBoxLayout *buttonsLayout = new QHBoxLayout();

        resetButton = new QPushButton("Reset algorithm parameters");
        fileButton = new QPushButton("Load config file");

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

        customs[0] = j["Parameter Set"]["minAreaRatio"];
        customs[1] = j["Parameter Set"]["maxAreaRatio"];

        insertCustomEntry(customs);

    }

    QMap<Settings, QList<float>> defaultParameters = {
            { Settings::DEFAULT, {0.005f, 0.2f} },
            { Settings::ROI_0_3_OPTIMIZED, {0.001f, 0.823f} },
            { Settings::ROI_0_6_OPTIMIZED, {0.001f, 0.131f} },
            { Settings::FULL_IMAGE_OPTIMIZED, {0.001f, 0.038f} },
            { Settings::AUTOMATIC_PARAMETRIZATION, {-1.0f, -1.0f} },
            { Settings::CUSTOM, {-1.0f, -1.0f} }
    };


private slots:

    void onParameterConfigSelection(QString configKey) {
        setConfigIndex(configKey);
        QList<float>& selectedParameter = getCurrentParameters();

        // First come the parameters roughly independent from ROI size and relative pupil size 
        // (NONE HERE)

        // Then the specific ones that are set by autoParam
        if(isAutoParamEnabled()) {
            minAreaBox->setEnabled(false);
            maxAreaBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            minAreaBox->setEnabled(true);
            maxAreaBox->setEnabled(true);
            
            minAreaBox->setValue(selectedParameter[0]);
            maxAreaBox->setValue(selectedParameter[1]);
        }

        //applySpecificSettings(); // settings are only updated when apply click in pupildetectionsettingsdialog
    }

};
