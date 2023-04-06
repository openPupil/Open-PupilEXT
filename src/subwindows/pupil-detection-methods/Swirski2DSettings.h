
#ifndef PUPILEXT_SWIRSKI2DSETTINGS_H
#define PUPILEXT_SWIRSKI2DSETTINGS_H

/**
    @authors Moritz Lode, Gábor Bényei
*/

#include "PupilMethodSetting.h"
#include "../../pupil-detection-methods/Swirski2D.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QLabel>

#include "json.h"
#include <fstream>
// for convenience
using json = nlohmann::json;

/**
    Pupil Detection Algorithm setting for the Swirski2D algorithm, displayed in the pupil detection setting dialog
*/
class Swirski2DSettings : public PupilMethodSetting {
    Q_OBJECT

public:

    // GB: added pupilDetection instance to get the actual ROIs for Autometric Parametrization calculations
    explicit Swirski2DSettings(PupilDetection * pupilDetection, Swirski2D *m_swirski, QWidget *parent=0) : 
        PupilMethodSetting(parent), 
        p_swirski(m_swirski), 
        pupilDetection(pupilDetection), 
        configParameters(defaultParameters)  {
        
        configParameters = applicationSettings->value("Swirski2DSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();

        configIndex = applicationSettings->value("Swirski2DSettings.configIndex", "Default").toString();

        createForm();

        if(parameterConfigs->findText(configIndex) < 0) {
            std::cout<<"Did not found config: "<<configIndex.toStdString()<<std::endl;
            parameterConfigs->setCurrentText("Default");
        } else {
            parameterConfigs->setCurrentText(configIndex);
        }

        // GB added begin
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            minRadiusBox->setEnabled(false);
            maxRadiusBox->setEnabled(false);
        } else {
            minRadiusBox->setEnabled(true);
            maxRadiusBox->setEnabled(true);
        } 
        // GB added begin

        QGridLayout *infoLayout = new QGridLayout(infoBox);

        QIcon trackOnIcon = QIcon(":/icons/Breeze/status/22/dialog-information.svg");
        QLabel *iLabel = new QLabel();
        iLabel->setPixmap(trackOnIcon.pixmap(QSize(32, 32)));
        infoLayout->addWidget(iLabel, 0, 0);

        QLabel *pLabel = new QLabel();
        pLabel->setWordWrap(true);
        pLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        pLabel->setOpenExternalLinks(true);
        pLabel->setText("Lech Swirski, Andreas Bulling, Neil A. Dodgson, \"Robust real-time pupil tracking in highly off-axis images\", 2012 <a href=\"http://www.cl.cam.ac.uk/research/rainbow/projects/pupiltracking\">Website</a><br/>License: <a href=\"https://opensource.org/licenses/MIT\">MIT</a>");
        infoLayout->addWidget(pLabel, 1, 0);

        // GB modified begin
        // GB NOTE: removed \n to let it fit more efficiently
        QLabel *confLabel;
        if(p_swirski->hasConfidence())
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

    ~Swirski2DSettings() override = default;

    void add2(Swirski2D *s_swirski) {
        swirski2 = s_swirski;
    }
    void add3(Swirski2D *s_swirski) {
        swirski3 = s_swirski;
    }
    void add4(Swirski2D *s_swirski) {
        swirski4 = s_swirski;
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
        configParameters = applicationSettings->value("Swirski2DSettings.configParameters", QVariant::fromValue(configParameters)).value<QMap<QString, QList<float>>>();
        configIndex = applicationSettings->value("Swirski2DSettings.configIndex", "Default").toString();

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

            minRadiusBox->setEnabled(false);
            maxRadiusBox->setEnabled(false);
        } else {
            pupilDetection->setAutoParamEnabled(false);
            minRadiusBox->setEnabled(true);
            maxRadiusBox->setEnabled(true);
        } 
        // GB added end

        updateSettings();
    }

    void updateSettings() override {

        // GB modified begin
        
        // First come the parameters roughly independent from ROI size and relative pupil size 
        p_swirski->params.CannyBlur = cannyBlurBox->value();
        p_swirski->params.CannyThreshold1 = cannyThreshold1Box->value();
        p_swirski->params.CannyThreshold2 = cannyThreshold2Box->value();
        p_swirski->params.StarburstPoints = starburstPointsBox->value();

        p_swirski->params.PercentageInliers = percInlierBox->value();
        p_swirski->params.InlierIterations = iterInlierBox->value();
        p_swirski->params.ImageAwareSupport = imageAwareBox->isChecked();
        p_swirski->params.EarlyTerminationPercentage = termPercBox->value();
        p_swirski->params.EarlyRejection = earlyRejectionBox->isChecked();

        configParameters[parameterConfigs->currentText()][2] = cannyBlurBox->value();
        configParameters[parameterConfigs->currentText()][3] = cannyThreshold1Box->value();
        configParameters[parameterConfigs->currentText()][4] = cannyThreshold2Box->value();
        configParameters[parameterConfigs->currentText()][5] = starburstPointsBox->value();
        configParameters[parameterConfigs->currentText()][6] = percInlierBox->value();
        configParameters[parameterConfigs->currentText()][7] = iterInlierBox->value();
        configParameters[parameterConfigs->currentText()][8] = termPercBox->value();
        configParameters[parameterConfigs->currentText()][9] = imageAwareBox->isChecked();
        configParameters[parameterConfigs->currentText()][10] = earlyRejectionBox->isChecked();

        if(swirski2) {
            swirski2->params.CannyBlur = cannyBlurBox->value();
            swirski2->params.CannyThreshold1 = cannyThreshold1Box->value();
            swirski2->params.CannyThreshold2 = cannyThreshold2Box->value();
            swirski2->params.StarburstPoints = starburstPointsBox->value();

            swirski2->params.PercentageInliers = percInlierBox->value();
            swirski2->params.InlierIterations = iterInlierBox->value();
            swirski2->params.ImageAwareSupport = imageAwareBox->isChecked();
            swirski2->params.EarlyTerminationPercentage = termPercBox->value();
            swirski2->params.EarlyRejection = earlyRejectionBox->isChecked();
        }
        if(swirski3) {
            swirski3->params.CannyBlur = cannyBlurBox->value();
            swirski3->params.CannyThreshold1 = cannyThreshold1Box->value();
            swirski3->params.CannyThreshold2 = cannyThreshold2Box->value();
            swirski3->params.StarburstPoints = starburstPointsBox->value();

            swirski3->params.PercentageInliers = percInlierBox->value();
            swirski3->params.InlierIterations = iterInlierBox->value();
            swirski3->params.ImageAwareSupport = imageAwareBox->isChecked();
            swirski3->params.EarlyTerminationPercentage = termPercBox->value();
            swirski3->params.EarlyRejection = earlyRejectionBox->isChecked();
        }
        if(swirski4) {
            swirski4->params.CannyBlur = cannyBlurBox->value();
            swirski4->params.CannyThreshold1 = cannyThreshold1Box->value();
            swirski4->params.CannyThreshold2 = cannyThreshold2Box->value();
            swirski4->params.StarburstPoints = starburstPointsBox->value();

            swirski4->params.PercentageInliers = percInlierBox->value();
            swirski4->params.InlierIterations = iterInlierBox->value();
            swirski4->params.ImageAwareSupport = imageAwareBox->isChecked();
            swirski4->params.EarlyTerminationPercentage = termPercBox->value();
            swirski4->params.EarlyRejection = earlyRejectionBox->isChecked();
        }

        // Then the specific ones that are set by autoParam
        int procMode = pupilDetection->getCurrentProcMode();
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            float autoParamPupSizePercent = applicationSettings->value("autoParamPupSizePercent", pupilDetection->getAutoParamPupSizePercent()).toFloat();
            pupilDetection->setAutoParamPupSizePercent(autoParamPupSizePercent);
            pupilDetection->setAutoParamScheduled(true);

        } else {

            p_swirski->params.Radius_Min = minRadiusBox->value();
            p_swirski->params.Radius_Max = maxRadiusBox->value();

            configParameters[parameterConfigs->currentText()][0] = minRadiusBox->value();
            configParameters[parameterConfigs->currentText()][1] = maxRadiusBox->value();

            if(swirski2) {
                swirski2->params.Radius_Min = minRadiusBox->value();
                swirski2->params.Radius_Max = maxRadiusBox->value();
            }
            if(swirski3) {
                swirski3->params.Radius_Min = minRadiusBox->value();
                swirski3->params.Radius_Max = maxRadiusBox->value();
            }
            if(swirski4) {
                swirski4->params.Radius_Min = minRadiusBox->value();
                swirski4->params.Radius_Max = maxRadiusBox->value();
            }
            
        }
        // GB modified end

        emit onConfigChange(parameterConfigs->currentText());

        applicationSettings->setValue("Swirski2DSettings.configParameters", QVariant::fromValue(configParameters));
        applicationSettings->setValue("Swirski2DSettings.configIndex", parameterConfigs->currentText());
    }

private:

    Swirski2D *p_swirski;
    //Swirski2D *secondarySwirski = nullptr; // GB: refactored
    // GB added begin
    Swirski2D *swirski2 = nullptr;
    Swirski2D *swirski3 = nullptr;
    Swirski2D *swirski4 = nullptr;

    PupilDetection *pupilDetection;
    // GB added end

    QSpinBox *minRadiusBox;
    QSpinBox *maxRadiusBox;

    QDoubleSpinBox *cannyBlurBox;
    QDoubleSpinBox *cannyThreshold1Box;
    QDoubleSpinBox *cannyThreshold2Box;
    QSpinBox *starburstPointsBox;

    QSpinBox *percInlierBox;
    QSpinBox *iterInlierBox;
    QSpinBox *termPercBox;

    QCheckBox *imageAwareBox;
    QCheckBox *earlyRejectionBox;

    QPushButton *resetButton;
    QComboBox *parameterConfigs;
    QPushButton *fileButton;

    void createForm() {

        QList<float> selectedParameter = configParameters.value(configIndex);

        int Radius_Min = selectedParameter[0];
        int Radius_Max = selectedParameter[1];

        double CannyBlur = selectedParameter[2];
        double CannyThreshold1 = selectedParameter[3];
        double CannyThreshold2 = selectedParameter[4];
        int StarburstPoints = selectedParameter[5];

        int PercentageInliers = selectedParameter[6];
        int InlierIterations = selectedParameter[7];
        bool ImageAwareSupport = selectedParameter[8];
        int EarlyTerminationPercentage = selectedParameter[9];
        bool EarlyRejection = selectedParameter[10];

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


        QGroupBox *ellipseGroup = new QGroupBox("Algorithm specific: Ellipse Detection"); // GB: added "Algorithm specific: "
        QGroupBox *optionsGroup = new QGroupBox("Algorithm specific: Options"); // GB: "Algorithm specific: "

        QFormLayout *ellipseLayout = new QFormLayout();
        QFormLayout *optionsLayout = new QFormLayout();

        // GB modified begin
        // GB NOTE: to fit in smaller screen area
        QLabel *minRadiusLabel = new QLabel(tr("Min. Radius [px]:")); // GB: px added
        minRadiusBox = new QSpinBox();
        minRadiusBox->setMaximum(5000);
        minRadiusBox->setValue(Radius_Min);
        minRadiusBox->setFixedWidth(50); // GB

        QLabel *maxRadiusLabel = new QLabel(tr("Max. Radius [px]:")); // GB: px added
        maxRadiusBox = new QSpinBox();
        maxRadiusBox->setMaximum(5000);
        maxRadiusBox->setValue(Radius_Max);
        maxRadiusBox->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow1 = new QHBoxLayout;
        layoutRow1->addWidget(minRadiusBox);
        QSpacerItem *sp1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow1->addSpacerItem(sp1);
        layoutRow1->addWidget(maxRadiusLabel);
        layoutRow1->addWidget(maxRadiusBox);
        //layoutRow1->addSpacerItem(sp);
        ellipseLayout->addRow(minRadiusLabel, layoutRow1);


        QLabel *cannyBlurLabel = new QLabel(tr("Canny Blur:"));
        cannyBlurBox = new QDoubleSpinBox();
        cannyBlurBox->setValue(CannyBlur);
        cannyBlurBox->setFixedWidth(50); // GB
        ellipseLayout->addRow(cannyBlurLabel, cannyBlurBox);

        QLabel *cannyThreshold1Label = new QLabel(tr("Canny Threshold 1:"));
        cannyThreshold1Box = new QDoubleSpinBox();
        cannyThreshold1Box->setValue(CannyThreshold1);
        cannyThreshold1Box->setFixedWidth(50); // GB

        QLabel *cannyThreshold2Label = new QLabel(tr("Canny Threshold 2:"));
        cannyThreshold2Box = new QDoubleSpinBox();
        cannyThreshold2Box->setValue(CannyThreshold2);
        cannyThreshold2Box->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow2 = new QHBoxLayout;
        layoutRow2->addWidget(cannyThreshold1Box);
        QSpacerItem *sp2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow2->addSpacerItem(sp2);
        layoutRow2->addWidget(cannyThreshold2Label);
        layoutRow2->addWidget(cannyThreshold2Box);
        //layoutRow2->addSpacerItem(sp);
        ellipseLayout->addRow(cannyThreshold1Label, layoutRow2);


        QLabel *starburstPointsLabel = new QLabel(tr("Starburst Points:"));
        starburstPointsBox = new QSpinBox();
        starburstPointsBox->setValue(StarburstPoints);
        starburstPointsBox->setFixedWidth(50); // GB
        ellipseLayout->addRow(starburstPointsLabel, starburstPointsBox);

        QLabel *percInlierLabel = new QLabel(tr("Perc. Inliers:"));
        percInlierBox = new QSpinBox();
        percInlierBox->setValue(PercentageInliers);
        percInlierBox->setFixedWidth(50); // GB

        QLabel *iterInlierLabel = new QLabel(tr("Inlier Iterations:"));
        iterInlierBox = new QSpinBox();
        iterInlierBox->setValue(InlierIterations);
        iterInlierBox->setFixedWidth(50); // GB

        QHBoxLayout *layoutRow3 = new QHBoxLayout;
        layoutRow3->addWidget(percInlierBox);
        QSpacerItem *sp3 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum); 
        layoutRow3->addSpacerItem(sp3);
        layoutRow3->addWidget(iterInlierLabel);
        layoutRow3->addWidget(iterInlierBox);
        //layoutRow2->addSpacerItem(sp);
        ellipseLayout->addRow(percInlierLabel, layoutRow3);


        ellipseGroup->setLayout(ellipseLayout);
        mainLayout->addWidget(ellipseGroup);


        QLabel *termPercLabel = new QLabel(tr("Early Termination Perc.:"));
        termPercBox = new QSpinBox();
        termPercBox->setValue(EarlyTerminationPercentage);
        termPercBox->setFixedWidth(50); // GB
        optionsLayout->addRow(termPercLabel, termPercBox);

        QLabel *imageAwareLabel = new QLabel(tr("Image Aware RANSAC:"));
        imageAwareBox = new QCheckBox();
        imageAwareBox->setChecked(ImageAwareSupport);
        optionsLayout->addRow(imageAwareLabel, imageAwareBox);

        QLabel *earlyRejectionLabel = new QLabel(tr("Early Rejection:"));
        earlyRejectionBox = new QCheckBox();
        earlyRejectionBox->setChecked(EarlyRejection);
        optionsLayout->addRow(earlyRejectionLabel, earlyRejectionBox);
        // GB modified end

        optionsGroup->setLayout(optionsLayout);
        mainLayout->addWidget(optionsGroup);


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

        customs[0] = j["Parameter Set"]["Radius_Min"];
        customs[1] = j["Parameter Set"]["Radius_Max"];
        customs[2] = j["Parameter Set"]["CannyBlur"];
        customs[3] = j["Parameter Set"]["CannyThreshold1"];
        customs[4] = j["Parameter Set"]["CannyThreshold2"];
        customs[5] = j["Parameter Set"]["StarburstPoints"];
        customs[6] = j["Parameter Set"]["PercentageInliers"];
        customs[7] = j["Parameter Set"]["InlierIterations"];
        customs[8] = j["Parameter Set"]["EarlyTerminationPercentage"];
        customs[9] = j["Parameter Set"]["ImageAwareSupport"];
        customs[10] = j["Parameter Set"]["EarlyRejection"];

        configParameters.insert("Custom", customs);

        if(parameterConfigs->findText("Custom") < 0) {
            parameterConfigs->addItem("Custom");
        }
        parameterConfigs->setCurrentText("Custom");

//        minRadiusBox->setValue(j["Parameter Set"]["Radius_Min"]);
//        maxRadiusBox->setValue(j["Parameter Set"]["Radius_Max"]);
//
//        cannyBlurBox->setValue(j["Parameter Set"]["CannyBlur"]);
//        cannyThreshold1Box->setValue(j["Parameter Set"]["CannyThreshold1"]);
//        cannyThreshold2Box->setValue(j["Parameter Set"]["CannyThreshold2"]);
//        starburstPointsBox->setValue(j["Parameter Set"]["StarburstPoints"]);
//
//        percInlierBox->setValue(j["Parameter Set"]["PercentageInliers"]);
//        iterInlierBox->setValue(j["Parameter Set"]["InlierIterations"]);
//        termPercBox->setValue(j["Parameter Set"]["EarlyTerminationPercentage"]);
//        imageAwareBox->setChecked(j["Parameter Set"]["ImageAwareSupport"]);
//        earlyRejectionBox->setChecked(j["Parameter Set"]["EarlyRejection"]);
    }

//    QMap<QString, QList<float>> defaultParameters = {
//            { "Default", {40, 80, 1.6f, 20., 40, 0, 20, 2, 95, 1, 1} },
//            { "ROI 0.3 Optimized", {40, 64, 0.5f, 10, 11, 26, 29, 9, 74, 1, 0} },
//            { "ROI 0.6 Optimized", {40, 63, 0.1f, 14, 18, 28, 25, 2, 93, 1, 1} },
//            { "Full Image Optimized", {40, 43, 6.4f, 21, 22, 26, 16, 10, 34, 0, 1} }
//    };

    QMap<QString, QList<float>> defaultParameters = {
            { "Default", {40, 80, 1.6f, 20., 40, 0, 20, 2, 95, 1, 1} },
            { "ROI 0.3 Optimized", {10, 112, 0.1f, 33, 64, 26, 21, 6, 22, 0, 1} },
            { "ROI 0.6 Optimized", {21, 108, 0.3f, 21, 79, 31, 25, 2, 14, 1, 0} },
            { "Full Image Optimized", {19, 115, 0.4f, 46, 63, 25, 38, 10, 27, 0, 1} },
            { "Automatic Parametrization", {-1, -1, 0.4f, 46, 63, 25, 38, 10, 27, 0, 1} } // GB added
    };


    QMap<QString, QList<float>> configParameters;
    QString configIndex;


private slots:

    void onParameterConfigSelection(QString configKey) {
        QList<float> selectedParameter = configParameters.value(configKey);

        // GB modified begin

        // First come the parameters roughly independent from ROI size and relative pupil size 
        cannyBlurBox->setValue(selectedParameter[2]);
        cannyThreshold1Box->setValue(selectedParameter[3]);
        cannyThreshold2Box->setValue(selectedParameter[4]);
        starburstPointsBox->setValue(selectedParameter[5]);

        percInlierBox->setValue(selectedParameter[6]);
        iterInlierBox->setValue(selectedParameter[7]);
        termPercBox->setValue(selectedParameter[8]);
        imageAwareBox->setChecked(selectedParameter[9]);
        earlyRejectionBox->setChecked(selectedParameter[10]);

        // Then the specific ones that are set by autoParam
        if(parameterConfigs->currentText()=="Automatic Parametrization") {
            minRadiusBox->setEnabled(false);
            maxRadiusBox->setEnabled(false);
            // TODO: hide value text too
        } else {
            minRadiusBox->setEnabled(true);
            maxRadiusBox->setEnabled(true);

            minRadiusBox->setValue(selectedParameter[0]);
            maxRadiusBox->setValue(selectedParameter[1]);
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


#endif //PUPILEXT_SWIRSKI2DSETTINGS_H
