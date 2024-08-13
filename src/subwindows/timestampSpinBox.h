#pragma once

/**
    @author Attila Boncser
*/

#include <QtWidgets>
#include <QtWidgets/QDoubleSpinBox>
#include "../devices/fileCamera.h"

class TimestampSpinBox : public QDoubleSpinBox {
    Q_OBJECT

public:
    explicit TimestampSpinBox(FileCamera *fileCamera);

    ~TimestampSpinBox() override;

    QString textFromValue(double value) const override;
    double valueFromText(const QString &text) const override;

private:
    FileCamera *fileCamera;

    uint64_t getNextTimestamp(int frameNumber);
    uint64_t getPreviousTimestamp(int frameNumber);
};
