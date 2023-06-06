
#include <QtWidgets/QWidget>
#include <QtWidgets/QDoubleSpinBox>
#include "timestampSpinBox.h"


TimestampSpinBox::TimestampSpinBox(FileCamera *fileCamera) : QDoubleSpinBox(),
        fileCamera(fileCamera){
}

TimestampSpinBox::~TimestampSpinBox() {
}

QString TimestampSpinBox::textFromValue(double value) const {
    uint64_t timestamp = fileCamera->getTimestampForFrameNumber(value-1);
    return QString::number(timestamp);
}

double TimestampSpinBox::valueFromText(const QString &text) const {
    bool success;
    uint64_t timestamp = text.toULongLong(&success);
    return fileCamera->getFrameNumberForTimestamp(timestamp);
}