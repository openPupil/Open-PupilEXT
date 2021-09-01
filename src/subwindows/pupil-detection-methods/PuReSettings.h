
#ifndef PUPILEXT_PURESETTINGS_H
#define PUPILEXT_PURESETTINGS_H

/**
    @author Moritz Lode
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

    explicit PuReSettings(PuRe *pure, QWidget *parent=0) : PupilMethodSetting(parent), pure(pure), configParameters(defaultParameters)  {
        configParameters = applicationSettings->value("PuReSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();

        configIndex = applicationSettings->value("PuReSettings.configIndex", "Default").toString();

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
        pLabel->setText("Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, \"PuRe: Robust pupil detection for real-time pervasive eye tracking.\", 2018<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini");
        infoLayout->addWidget(pLabel, 1, 0);

        QLabel *confLabel;
        if(pure->hasConfidence())
            confLabel = new QLabel("Info:\nThis method does provide its own confidence.");
        else
            confLabel = new QLabel("Info:\nThis method does not provide its own confidence, use the outline confidence.");
        confLabel->setWordWrap(true);
        infoLayout->addWidget(confLabel, 2, 0);

#if _DEBUG
        QLabel *warnLabel = new QLabel("CAUTION:\nDebug build may perform very slow.\nUse release build or adjust processing speed to not risk memory overflow.");
        warnLabel->setWordWrap(true);
        warnLabel->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        infoLayout->addWidget(warnLabel, 3, 0);
#endif
        infoBox->setLayout(infoLayout);
    }

    ~PuReSettings() override = default;

    void addSecondary(PuRe *s_pure) {
        secondaryPure = s_pure;
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

        configParameters = applicationSettings->value("PuReSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();
        configIndex = applicationSettings->value("PuReSettings.configIndex", "Default").toString();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

        updateSettings();
    }

    void updateSettings() override {
        float meanCanthiDistanceMM = pure->meanCanthiDistanceMM;
        float maxPupilDiameterMM = pure->maxPupilDiameterMM;
        float minPupilDiameterMM = pure->minPupilDiameterMM;

        int baseWidth = pure->baseSize.width;
        int baseHeight = pure->baseSize.height;

        meanCanthiDistanceMM = canthiDistanceBox->value();
        maxPupilDiameterMM = maxPupilBox->value();
        minPupilDiameterMM = minPupilBox->value();

        baseWidth = imageWidthBox->value();
        baseHeight = imageHeightBox->value();

        pure->baseSize = cv::Size(baseWidth, baseHeight);

        pure->meanCanthiDistanceMM = meanCanthiDistanceMM;
        pure->maxPupilDiameterMM = maxPupilDiameterMM;
        pure->minPupilDiameterMM = minPupilDiameterMM;

        configParameters[parameterConfigs->currentText()][0] = baseWidth;
        configParameters[parameterConfigs->currentText()][1] = baseHeight;
        configParameters[parameterConfigs->currentText()][2] = meanCanthiDistanceMM;
        configParameters[parameterConfigs->currentText()][3] = minPupilDiameterMM;
        configParameters[parameterConfigs->currentText()][4] = maxPupilDiameterMM;

        if(secondaryPure) {
            secondaryPure->baseSize = cv::Size(baseWidth, baseHeight);

            secondaryPure->meanCanthiDistanceMM = meanCanthiDistanceMM;
            secondaryPure->maxPupilDiameterMM = maxPupilDiameterMM;
            secondaryPure->minPupilDiameterMM = minPupilDiameterMM;
        }

        emit onConfigChange(parameterConfigs->currentText());

        applicationSettings->setValue("PuReSettings.configParameters", QVariant::fromValue(configParameters));
        applicationSettings->setValue("PuReSettings.configIndex", parameterConfigs->currentText());
    }

private:

    PuRe *pure;
    PuRe *secondaryPure = nullptr;

    QSpinBox *imageWidthBox;
    QSpinBox *imageHeightBox;

    QDoubleSpinBox *canthiDistanceBox;
    QDoubleSpinBox *maxPupilBox;
    QDoubleSpinBox *minPupilBox;

    QPushButton *resetButton;
    QPushButton *fileButton;

    QComboBox *parameterConfigs;

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
        configsLayout->addWidget(parameterConfigs);

        QMapIterator<QString, QList<float>> i(configParameters);
        while (i.hasNext()) {
            i.next();
            parameterConfigs->addItem(i.key());
        }

        connect(parameterConfigs, SIGNAL(currentTextChanged(QString)), this, SLOT(onParameterConfigSelection(QString)));

        mainLayout->addLayout(configsLayout);
        mainLayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Fixed));

        QGroupBox *sizeGroup = new QGroupBox("Image Size (Downscaling)");
        QGroupBox *physGroup = new QGroupBox("Physical Parameter");

        QFormLayout *sizeLayout = new QFormLayout();
        QFormLayout *physLayout = new QFormLayout();

        QLabel *widthLabel = new QLabel(tr("Image width:"));
        imageWidthBox = new QSpinBox();
        imageWidthBox->setMaximum(5000);
        imageWidthBox->setValue(baseWidth);
        sizeLayout->addRow(widthLabel, imageWidthBox);

        QLabel *heightLabel = new QLabel(tr("Image height:"));
        imageHeightBox = new QSpinBox();
        imageHeightBox->setMaximum(5000);
        imageHeightBox->setValue(baseHeight);
        sizeLayout->addRow(heightLabel, imageHeightBox);

        sizeGroup->setLayout(sizeLayout);
        mainLayout->addWidget(sizeGroup);


        QLabel *canthiDistanceLabel = new QLabel(tr("Mean Canthi Distance [mm]:"));
        canthiDistanceBox = new QDoubleSpinBox();
        canthiDistanceBox->setValue(meanCanthiDistanceMM);
        physLayout->addRow(canthiDistanceLabel, canthiDistanceBox);

        QLabel *maxPupilLabel = new QLabel(tr("Max. Pupil Size [mm]:"));
        maxPupilBox = new QDoubleSpinBox();
        maxPupilBox->setValue(maxPupilDiameterMM);
        physLayout->addRow(maxPupilLabel, maxPupilBox);

        QLabel *minPupilLabel = new QLabel(tr("Min. Pupil Size [mm]:"));
        minPupilBox = new QDoubleSpinBox();
        minPupilBox->setValue(minPupilDiameterMM);
        physLayout->addRow(minPupilLabel, minPupilBox);

        physGroup->setLayout(physLayout);
        mainLayout->addWidget(physGroup);

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
            { "Default", {320, 240, 27.6f, 2.0f, 8.0f} },
            { "ROI 0.3 Optimized", {320, 240, 49.4f, 1.9f, 20.0f} },
            { "ROI 0.6 Optimized", {320, 240, 38.7f, 1.9f, 16.8f} },
            { "Full Image Optimized", {320, 240, 94.1f, 0.1f, 16.0f} }
    };

      // Parameters from second optimization run
//    QMap<QString, QList<float>> defaultParameters = {
//            { "Default", {320, 240, 27.6f, 2.0f, 8.0f} },
//            { "ROI 0.3 Optimized", {320, 240, 49.1f, 5.4f, 11.9f} },
//            { "ROI 0.6 Optimized", {320, 240, 58.5f, 3.4f, 8.7f} },
//            { "Full Image Optimized", {320, 240, 76.1f, 2.4f, 5.6f} }
//    };

    QMap<QString, QList<float>> configParameters;
    QString configIndex;


private slots:

    void onParameterConfigSelection(QString configKey) {
        QList<float> selectedParameter = configParameters.value(configKey);

        imageWidthBox->setValue(selectedParameter[0]);
        imageHeightBox->setValue(selectedParameter[1]);

        canthiDistanceBox->setValue(selectedParameter[2]);
        minPupilBox->setValue(selectedParameter[3]);
        maxPupilBox->setValue(selectedParameter[4]);

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


#endif //PUPILEXT_PURESETTINGS_H
