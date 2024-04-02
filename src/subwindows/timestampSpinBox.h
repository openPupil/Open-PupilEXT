#ifndef PUPILEXT_TIMESTAMPSPINBOX_H
#define PUPILEXT_TIMESTAMPSPINBOX_H

/**
    @author Attila Boncsér
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
#endif //PUPILEXT_TIMESTAMPSPINBOX_H