
#ifndef PUPILEXT_EXCUSESETTINGS_H
#define PUPILEXT_EXCUSESETTINGS_H

/**
    @author Moritz Lode
*/

#include "PupilMethodSetting.h"
#include "../../pupil-detection-methods/ExCuSe.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>

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

    explicit ExCuSeSettings(ExCuSe *m_excuse, QWidget *parent=0) : PupilMethodSetting(parent), p_excuse(m_excuse), configParameters(defaultParameters)  {
        configParameters = applicationSettings->value("ExCuSeSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();

        configIndex = applicationSettings->value("ExCuSeSettings.configIndex", "Default").toString();

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
        pLabel->setText("Wolfgang Fuhl, Thomas Kübler, Katrin Sippel, Wolfgang Rosenstiel, Enkelejda Kasneci, \"ExCuSe: Robust Pupil Detection in Real-World Scenarios.\", 2015<br/>Part of the <a href=\"https://www-ti.informatik.uni-tuebingen.de/santini/EyeRecToo\">EyeRecToo</a> software. Copyright (c) 2018, Thiago Santini / University of Tübingen");
        infoLayout->addWidget(pLabel, 1, 0);

        QLabel *confLabel;
        if(p_excuse->hasConfidence())
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

    ~ExCuSeSettings() override = default;

    void addSecondary(ExCuSe *s_excuse) {
        secondaryExcuse = s_excuse;
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
        configParameters = applicationSettings->value("ExCuSeSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();
        configIndex = applicationSettings->value("ExCuSeSettings.configIndex", "Default").toString();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

//        QList<float> selectedParameter = configParameters.value(configIndex);
//
//        maxRadiBox->setValue(selectedParameter[0]);
//        ellipseThresholdBox->setValue(selectedParameter[1]);

        updateSettings();
    }

    void updateSettings() override {
        int max_ellipse_radi = p_excuse->max_ellipse_radi;
        int good_ellipse_threshold = p_excuse->good_ellipse_threshold;

        max_ellipse_radi = maxRadiBox->value();
        good_ellipse_threshold = ellipseThresholdBox->value();

        p_excuse->max_ellipse_radi = max_ellipse_radi;
        p_excuse->good_ellipse_threshold = good_ellipse_threshold;

        configParameters[parameterConfigs->currentText()][0] = max_ellipse_radi;
        configParameters[parameterConfigs->currentText()][1] = good_ellipse_threshold;

        if(secondaryExcuse) {
            secondaryExcuse->max_ellipse_radi = max_ellipse_radi;
            secondaryExcuse->good_ellipse_threshold = good_ellipse_threshold;
        }

        emit onConfigChange(parameterConfigs->currentText());

        applicationSettings->setValue("ExCuSeSettings.configParameters", QVariant::fromValue(configParameters));
        applicationSettings->setValue("ExCuSeSettings.configIndex", parameterConfigs->currentText());
    }

private:

    ExCuSe *p_excuse;
    ExCuSe *secondaryExcuse = nullptr;

    QSpinBox *maxRadiBox;
    QSpinBox *ellipseThresholdBox;

    QPushButton *resetButton;
    QComboBox *parameterConfigs;
    QPushButton *fileButton;

    void createForm() {

        QList<float> selectedParameter = configParameters.value(configIndex);


        int max_ellipse_radi = selectedParameter[0];
        int good_ellipse_threshold = selectedParameter[1];

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


        QGroupBox *sizeGroup = new QGroupBox("Ellipse Fit");

        QFormLayout *sizeLayout = new QFormLayout();

        QLabel *maxRadiLabel = new QLabel(tr("Max. Ellipse Radius [px]:"));
        maxRadiBox = new QSpinBox();
        maxRadiBox->setMaximum(5000);
        maxRadiBox->setValue(max_ellipse_radi);
        sizeLayout->addRow(maxRadiLabel, maxRadiBox);

        QLabel *ellipseThresholdLabel = new QLabel(tr("Ellipse Goodness Threshold:"));
        ellipseThresholdBox = new QSpinBox();
        ellipseThresholdBox->setMaximum(100);
        ellipseThresholdBox->setValue(good_ellipse_threshold);
        sizeLayout->addRow(ellipseThresholdLabel, ellipseThresholdBox);

        sizeGroup->setLayout(sizeLayout);
        mainLayout->addWidget(sizeGroup);


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

        customs[0] = j["Parameter Set"]["max_ellipse_radi"];
        customs[1] = j["Parameter Set"]["good_ellipse_threshold"];

        configParameters.insert("Custom", customs);

        if(parameterConfigs->findText("Custom") < 0) {
            parameterConfigs->addItem("Custom");
        }
        parameterConfigs->setCurrentText("Custom");

    }

    QMap<QString, QList<float>> defaultParameters = {
            { "Default", {50, 15} },
            { "ROI 0.3 Optimized", {146, 7} },
            { "ROI 0.6 Optimized", {216, 34} },
            { "Full Image Optimized", {39, 0} }
    };


    QMap<QString, QList<float>> configParameters;
    QString configIndex;

private slots:

    void onParameterConfigSelection(QString configKey) {
        QList<float> selectedParameter = configParameters.value(configKey);

        maxRadiBox->setValue(selectedParameter[0]);
        ellipseThresholdBox->setValue(selectedParameter[1]);

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


#endif //PUPILEXT_EXCUSESETTINGS_H
