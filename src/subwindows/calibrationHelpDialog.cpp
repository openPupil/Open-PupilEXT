#include <QtWidgets/qboxlayout.h>
#include "calibrationHelpDialog.h"

CalibrationHelpDialog::CalibrationHelpDialog(QWidget *parent) :
        QDialog(parent) {

    this->setMinimumSize(600, 700);
    this->setWindowTitle("Calibration Information");

    createForm();
    connect(closeButton, &QPushButton::clicked, this, &CalibrationHelpDialog::reject);
}


void CalibrationHelpDialog::reject() {
    QDialog::reject();
}

void CalibrationHelpDialog::createForm() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    QVBoxLayout *patternLayout = new QVBoxLayout();

    QHBoxLayout *chessboardLayout = new QHBoxLayout();
    QHBoxLayout *circleLayout = new QHBoxLayout();
    QHBoxLayout *asymcircleLayout = new QHBoxLayout();


    QImage chessboard = QImage(":/icons/chessboard.jpg");
    QLabel *chessboardLabel = new QLabel();
    chessboardLabel->setBackgroundRole(QPalette::Base);
    chessboardLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    chessboardLabel->setScaledContents(true);
    QPixmap chessboardPix = QPixmap::fromImage(chessboard);
    chessboardLabel->setPixmap(chessboardPix);

    QLabel *chessboardTextLabel = new QLabel();
    chessboardTextLabel->setText("<b>Chessboard</b><br><br>The calibration size of a chessboard pattern is defined by the number of corner feature points in each dimension (highlighted in the left sample image).<br> For a chessboard pattern with 11 columns and 8 rows this results in a feature point size of 10 columns and 7 rows for calibration.");
    chessboardTextLabel->setWordWrap(true);

    chessboardLayout->addWidget(chessboardLabel);
    chessboardLayout->addWidget(chessboardTextLabel);
    patternLayout->addLayout(chessboardLayout);

    QImage circle = QImage(":/icons/sym_circle.jpg");
    QLabel *circleLabel = new QLabel();
    circleLabel->setBackgroundRole(QPalette::Base);
    circleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    circleLabel->setScaledContents(true);
    QPixmap circlePix = QPixmap::fromImage(circle);
    circleLabel->setPixmap(circlePix);

    QLabel *circleTextLabel = new QLabel();
    circleTextLabel->setText("<b>Symmetric Circles</b><br><br>The calibration size of a symmetric circle pattern is defined by the number of circle feature points in each dimension (highlighted in the left sample image).<br> For a symmetric circle pattern with 11 columns and 8 rows this results in a feature point size of 11 columns and 8 rows for calibration.");
    circleTextLabel->setWordWrap(true);

    circleLayout->addWidget(circleLabel);
    circleLayout->addWidget(circleTextLabel);
    patternLayout->addLayout(circleLayout);

    QImage asymcircle = QImage(":/icons/asym_circle.jpg");
    QLabel *asymcircleLabel = new QLabel();
    asymcircleLabel->setBackgroundRole(QPalette::Base);
    asymcircleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    asymcircleLabel->setScaledContents(true);
    QPixmap asymcirclePix = QPixmap::fromImage(asymcircle);
    asymcircleLabel->setPixmap(asymcirclePix);

    QLabel *asymcircleTextLabel = new QLabel();
    asymcircleTextLabel->setText("<b>Asymmetric Circles</b><br><br>The calibration size of a asymmetric circle pattern is defined by the number of circle feature points in each dimension (highlighted in the left sample image).<br> For calibration, only the outer row feature points are counted, while column feature points are counted in the first and second outer row. For a asymmetric circle pattern with 11 columns and 8 rows, this results in a feature point size of 11 columns and 4 rows for calibration.");
    asymcircleTextLabel->setWordWrap(true);

    asymcircleLayout->addWidget(asymcircleLabel);
    asymcircleLayout->addWidget(asymcircleTextLabel);
    patternLayout->addLayout(asymcircleLayout);

    mainLayout->addLayout(patternLayout);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    closeButton = new QPushButton(tr("Close"));
    buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    buttonsLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
}


CalibrationHelpDialog::~CalibrationHelpDialog() = default;
