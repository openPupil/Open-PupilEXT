
#ifndef PUPILEXT_EXCUSESETTINGS_H
#define PUPILEXT_EXCUSESETTINGS_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "PupilMethodSetting.h"
#include "../../pupil-detection-methods/ExCuSe.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>
#include "../../SVGIconColorAdjuster.h"

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the ExCuSe algorithm, displayed in the pupil detection setting dialog
*/
class ExCuSeSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit ExCuSeSettings(PupilDetection * pupilDetection, ExCuSe *m_excuse, QWidget *parent=0) : 
        PupilMethodSetting("ExCuSeSettings.configParameters", "ExCuSeSettings.configIndex", parent), 
        p_excuse(m_excuse), 
        pupilDetection(pupilDetection) {

        PupilMethodSetting::setDefaultParameters(defaultParameters);
        createForm();
        parameterConfigs->setCurrentText(settingsMap.key(configIndex));

        // GB added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            maxRadiBox->setEnabled(false);
        } else {
            maxRadiBox->setEnabled(true);
        }
        // GB added end

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
        pLabel->setText("Wolfgang Fuhl, Thomas Kübler, Katrin Sippel, Wolfgang Rosenstiel, Enkelejda Kasneci, \"ExCuSe: Robust Pupil Detection in Real-World Scenarios.\", 2015<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini / University of Tübingen");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB NOTE: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(p_excuse->hasConfidence())
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

    ~ExCuSeSettings() override = default;

    void add2(ExCuSe *s_excuse) {
        excuse2 = s_excuse;
    }
    void add3(ExCuSe *s_excuse) {
        excuse3 = s_excuse;
    }
    void add4(ExCuSe *s_excuse) {
        excuse4 = s_excuse;
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

            maxRadiBox->setEnabled(false);
        } else {
            pupilDetection->setAutoParamEnabled(false);
            maxRadiBox->setEnabled(true);
        } 
        // GB added end

//        QList<float> selectedParameter = configParameters.value(configIndex);
//
//        maxRadiBox->setValue(selectedParameter[0]);
//        ellipseThresholdBox->setValue(selectedParameter[1]);

        updateSettings();
    }

    void updateSettings() override {

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        int good_ellipse_threshold = p_excuse->good_ellipse_threshold;

        good_ellipse_threshold = ellipseThresholdBox->value();

        p_excuse->good_ellipse_threshold = good_ellipse_threshold;

        QList<float>& currentParameters = getCurrentParameters();
        currentParameters[1] = good_ellipse_threshold;

        if(excuse2) {
            excuse2->good_ellipse_threshold = good_ellipse_threshold;
        }
        if(excuse3) {
            excuse3->good_ellipse_threshold = good_ellipse_threshold;
        }
        if(excuse4) {
            excuse4->good_ellipse_threshold = good_ellipse_threshold;
        }


        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);
            
        } else {
            int max_ellipse_radi = p_excuse->max_ellipse_radi;
            
            max_ellipse_radi = maxRadiBox->value();
            
            p_excuse->max_ellipse_radi = max_ellipse_radi;

            currentParameters[0] = max_ellipse_radi;

            if(excuse2) {
                excuse2->max_ellipse_radi = max_ellipse_radi;
            }
            if(excuse3) {
                excuse3->max_ellipse_radi = max_ellipse_radi;
            }
            if(excuse4) {
                excuse4->max_ellipse_radi = max_ellipse_radi;
            }
            
        }
        // GB modified end
        emit onConfigChange(parameterConfigs->currentText());

        PupilMethodSetting::updateSettings();
    }

private:

    ExCuSe *p_excuse;
    //ExCuSe *secondaryExcuse = nullptr; // GB refactored
    // GB added begin
    ExCuSe *excuse2 = nullptr;
    ExCuSe *excuse3 = nullptr;
    ExCuSe *excuse4 = nullptr;

    PupilDetection *pupilDetection;
    // GB added end

    QSpinBox *maxRadiBox;
    QSpinBox *ellipseThresholdBox;

    void createForm() {
        PupilMethodSetting::loadSettings();
        QList<float>& selectedParameter = getCurrentParameters();


        int max_ellipse_radi = selectedParameter[0];
        int good_ellipse_threshold = selectedParameter[1];

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


        QGroupBox *sizeGroup = new QGroupBox("Algorithm specific: Ellipse Fit"); // GB: "Algorithm specific: "

        QFormLayout *sizeLayout = new QFormLayout();

        QLabel *maxRadiLabel = new QLabel(tr("Max. Ellipse Radius [px]:"));
        maxRadiBox = new QSpinBox();
        maxRadiBox->setMaximum(5000);
        maxRadiBox->setValue(max_ellipse_radi);
        maxRadiBox->setFixedWidth(50); // GB
        sizeLayout->addRow(maxRadiLabel, maxRadiBox);

        QLabel *ellipseThresholdLabel = new QLabel(tr("Ellipse Goodness Threshold:"));
        ellipseThresholdBox = new QSpinBox();
        ellipseThresholdBox->setMaximum(100);
        ellipseThresholdBox->setValue(good_ellipse_threshold);
        ellipseThresholdBox->setFixedWidth(50); // GB
        sizeLayout->addRow(ellipseThresholdLabel, ellipseThresholdBox);

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

        QList<float> customs = PupilMethodSetting::defaultParameters[Settings::DEFAULT];

        customs[0] = j["Parameter Set"]["max_ellipse_radi"];
        customs[1] = j["Parameter Set"]["good_ellipse_threshold"];

        insertCustomEntry(customs);
    }

    QMap<Settings, QList<float>> defaultParameters = {
            { Settings::DEFAULT, {50.0f, 15.0f} },
            { Settings::ROI_0_3_OPTIMIZED, {146.0f, 7.0f} },
            { Settings::ROI_0_6_OPTIMIZED, {216.0f, 34.0f} },
            { Settings::FULL_IMAGE_OPTIMIZED, {39.0f, 0.0f} },
            { Settings::AUTOMATIC_PARAMETRIZATION, {-1.0f, 15.0f} },
            { Settings::CUSTOM, {-1.0f, 15.0f} } // GB added
    };

private slots:

    void onParameterConfigSelection(QString configKey) {
        setConfigIndex(configKey);
        QList<float>& selectedParameter = getCurrentParameters();

        // GB modified begin
        ellipseThresholdBox->setValue(selectedParameter[1]);

        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            maxRadiBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            maxRadiBox->setEnabled(true);
            maxRadiBox->setValue(selectedParameter[0]);
        }
        // GB modified end

        //updateSettings(); // settings are only updated when apply click in pupildetectionsettingsdialog
    }

};


#endif //PUPILEXT_EXCUSESETTINGS_H
