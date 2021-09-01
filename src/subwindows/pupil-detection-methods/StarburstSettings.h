
#ifndef PUPILEXT_STARBURSTSETTINGS_H
#define PUPILEXT_STARBURSTSETTINGS_H

/**
    @author Moritz Lode
*/

#include "PupilMethodSetting.h"
#include "../../pupil-detection-methods/Starburst.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>

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

    explicit StarburstSettings(Starburst *m_starburst, QWidget *parent=0) : PupilMethodSetting(parent), p_starburst(m_starburst), configParameters(defaultParameters) {
        configParameters = applicationSettings->value("StarburstSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();

        configIndex = applicationSettings->value("StarburstSettings.configIndex", "Default").toString();

        createForm();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

        QGridLayout *infoLayout = new QGridLayout(infoBox);

        QIcon trackOnIcon = QIcon(":/icons/Breeze/status/22/dialog-information.svg");
        QLabel *iLabel = new QLabel();
        iLabel->setPixmap(trackOnIcon.pixmap(QSize(32, 32)));
        infoLayout->addWidget(iLabel, 0, 0);

        QLabel *pLabel = new QLabel();
        pLabel->setWordWrap(true);
        pLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        pLabel->setOpenExternalLinks(true);
        pLabel->setText("Li, Dongheng & Winfield, D. & Parkhurst, D.J., \"Starburst: A hybrid algorithm for video-based eye tracking combining feature-based and model-based approaches.\", 2005<br/>Part of the <a href=\"http://thirtysixthspan.com/openEyes/software.html\">cvEyeTracker</a> software License: <a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">GPL</a>");
        infoLayout->addWidget(pLabel, 1, 0);

        QLabel *confLabel;
        if(p_starburst->hasConfidence())
            confLabel = new QLabel("Info:\nThis method does provide its own confidence.");
        else
            confLabel = new QLabel("Info:\nThis method does not provide its own confidence, use the outline confidence.");
        confLabel->setWordWrap(true);
        infoLayout->addWidget(confLabel, 2, 0);

        QLabel *infoLabel = new QLabel("CAUTION:\nProcessing using this algorithm may be very slow, reduce the camera acquiring fps accordingly.");
        infoLabel->setWordWrap(true);
        infoLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(infoLabel, 3, 0);
#if _DEBUG
        QLabel *warnLabel = new QLabel("CAUTION:\nDebug build may perform very slow.\nUse release build or adjust processing speed to not risk memory overflow.");
        warnLabel->setWordWrap(true);
        warnLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(warnLabel, 4, 0);
#endif

        infoBox->setLayout(infoLayout);
    }

    ~StarburstSettings() override = default;

    void addSecondary(Starburst *s_starburst) {
        secondaryStarburst = s_starburst;
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

public slots:

    void loadSettings() override {
        configParameters = applicationSettings->value("StarburstSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();
        configIndex = applicationSettings->value("StarburstSettings.configIndex", "Default").toString();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

//        QList<float> selectedParameter = configParameters.value(configIndex);
//
//        edgeThresholdBox->setValue(selectedParameter[0]);
//        numRaysBox->setValue(selectedParameter[1]);
//        minFeatureCandidatesBox->setValue(selectedParameter[2]);
//        crRatioBox->setValue(selectedParameter[3]);
//        crWindowSizeBox->setValue(selectedParameter[4]);

        updateSettings();
    }

    void updateSettings() override {
        int edge_threshold = p_starburst->edge_threshold;		//threshold of pupil edge points detection
        int rays = p_starburst->rays;				            //number of rays to use to detect feature points
        int min_feature_candidates = p_starburst->min_feature_candidates;	//minimum number of pupil feature candidates
        int corneal_reflection_ratio_to_image_size = p_starburst->corneal_reflection_ratio_to_image_size; // approx max size of the reflection relative to image height -> height/this
        int crWindowSize = p_starburst->crWindowSize;		    //corneal reflection search window size


        p_starburst->edge_threshold = edgeThresholdBox->value();
        p_starburst->rays = numRaysBox->value();
        p_starburst->min_feature_candidates = minFeatureCandidatesBox->value();
        p_starburst->corneal_reflection_ratio_to_image_size = crRatioBox->value();
        p_starburst->crWindowSize = crWindowSizeBox->value();

        configParameters[parameterConfigs->currentText()][0] = edgeThresholdBox->value();
        configParameters[parameterConfigs->currentText()][1] = numRaysBox->value();
        configParameters[parameterConfigs->currentText()][2] = minFeatureCandidatesBox->value();
        configParameters[parameterConfigs->currentText()][3] = crRatioBox->value();
        configParameters[parameterConfigs->currentText()][4] = crWindowSizeBox->value();

        if(secondaryStarburst) {
            secondaryStarburst->edge_threshold = edgeThresholdBox->value();
            secondaryStarburst->rays = numRaysBox->value();
            secondaryStarburst->min_feature_candidates = minFeatureCandidatesBox->value();
            secondaryStarburst->corneal_reflection_ratio_to_image_size = crRatioBox->value();
            secondaryStarburst->crWindowSize = crWindowSizeBox->value();
        }

        emit onConfigChange(parameterConfigs->currentText());

        applicationSettings->setValue("StarburstSettings.configParameters", QVariant::fromValue(configParameters));
        applicationSettings->setValue("StarburstSettings.configIndex", parameterConfigs->currentText());
    }

private:

    Starburst *p_starburst;
    Starburst *secondaryStarburst = nullptr;

    QSpinBox *edgeThresholdBox;
    QSpinBox *numRaysBox;
    QSpinBox *minFeatureCandidatesBox;
    QSpinBox *crRatioBox;
    QSpinBox *crWindowSizeBox;

    QPushButton *resetButton;
    QComboBox *parameterConfigs;
    QPushButton *fileButton;

    void createForm() {

        QList<float> selectedParameter = configParameters.value(configIndex);

        int edge_threshold = selectedParameter[0];
        int rays = selectedParameter[1];
        int min_feature_candidates = selectedParameter[2];
        int corneal_reflection_ratio_to_image_size = selectedParameter[3];
        int crWindowSize = selectedParameter[4];


        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QHBoxLayout *configsLayout = new QHBoxLayout();

        parameterConfigs = new QComboBox();
        configsLayout->addWidget(parameterConfigs);

        QMapIterator<QString, QList<float>> i(configParameters);
        while (i.hasNext()) {
            i.next();
            parameterConfigs->addItem(i.key());
        }
        connect(parameterConfigs, SIGNAL(currentTextChanged(QString)), this, SLOT(onParameterConfigSelection(QString)));

        mainLayout->addLayout(configsLayout);
        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));


        QGroupBox *edgeGroup = new QGroupBox("Edge Detection");
        QGroupBox *crGroup = new QGroupBox("Corneal Reflection (CR)");

        QFormLayout *edgeLayout = new QFormLayout();
        QFormLayout *crLayout = new QFormLayout();

        QLabel *edgeThresholdLabel = new QLabel(tr("Edge Threshold:"));
        edgeThresholdBox = new QSpinBox();
        edgeThresholdBox->setMaximum(1000);
        edgeThresholdBox->setValue(edge_threshold);
        edgeLayout->addRow(edgeThresholdLabel, edgeThresholdBox);

        QLabel *numRaysLabel = new QLabel(tr("Number of Rays:"));
        numRaysBox = new QSpinBox();
        numRaysBox->setValue(rays);
        edgeLayout->addRow(numRaysLabel, numRaysBox);

        QLabel *minFeatureCandidatesLabel = new QLabel(tr("Min. Feature Candidates:"));
        minFeatureCandidatesBox = new QSpinBox();
        minFeatureCandidatesBox->setValue(min_feature_candidates);
        edgeLayout->addRow(minFeatureCandidatesLabel, minFeatureCandidatesBox);

        edgeGroup->setLayout(edgeLayout);
        mainLayout->addWidget(edgeGroup);

        QLabel *crRatioLabel = new QLabel(tr("CR Ratio (to Image Height):"));
        crRatioBox = new QSpinBox();
        crRatioBox->setValue(corneal_reflection_ratio_to_image_size);
        crLayout->addRow(crRatioLabel, crRatioBox);

        QLabel *crWindowSizeLabel = new QLabel(tr("CR Window Size [px]:"));
        crWindowSizeBox = new QSpinBox();
        crWindowSizeBox->setMaximum(5000);
        crWindowSizeBox->setValue(crWindowSize);
        crLayout->addRow(crWindowSizeLabel, crWindowSizeBox);

        crGroup->setLayout(crLayout);
        mainLayout->addWidget(crGroup);

        QHBoxLayout *buttonsLayout = new QHBoxLayout();

        resetButton = new QPushButton("Reset");
        fileButton = new QPushButton("Load File");

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

        customs[0] = j["Parameter Set"]["edge_threshold"];
        customs[1] =j["Parameter Set"]["rays"];
        customs[2] =j["Parameter Set"]["min_feature_candidates"];
        customs[3] =j["Parameter Set"]["corneal_reflection_ratio_to_image_size"];
        customs[4] =j["Parameter Set"]["crWindowSize"];


        configParameters.insert("Custom", customs);

        if(parameterConfigs->findText("Custom") < 0) {
            parameterConfigs->addItem("Custom");
        }
        parameterConfigs->setCurrentText("Custom");

    }

    QMap<QString, QList<float>> defaultParameters = {
            { "Default", {20, 18, 10, 10, 301} },
            { "ROI 0.3 Optimized", {77, 8, 2, 4, 417} },
            { "ROI 0.6 Optimized", {27, 8, 1, 10, 197} },
            { "Full Image Optimized", {21, 32, 7, 10, 433} }
    };

    QMap<QString, QList<float>> configParameters;
    QString configIndex;


private slots:

    void onParameterConfigSelection(QString configKey) {
        QList<float> selectedParameter = configParameters.value(configKey);

        edgeThresholdBox->setValue(selectedParameter[0]);
        numRaysBox->setValue(selectedParameter[1]);
        minFeatureCandidatesBox->setValue(selectedParameter[2]);
        crRatioBox->setValue(selectedParameter[3]);
        crWindowSizeBox->setValue(selectedParameter[4]);

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


#endif //PUPILEXT_STARBURSTSETTINGS_H
