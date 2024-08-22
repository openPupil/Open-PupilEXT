#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QIconEngine>
#include <QSvgRenderer>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QLabel>

#include <QDebug>

/**

    This header contains the code to help adapt the program to light or dark GUI mode of the OS

*/

// Most of SVGIconEngine code was used from this SO post: https://stackoverflow.com/a/44757951
class SVGIconEngine : public QIconEngine {

    QString data;
    QSettings *applicationSettings;

    static QString changeColors(const QString _content, bool doLighten, QIcon::Mode mode) {

        QString content = _content;
        content.detach(); 

        QRegularExpression re("#[A-Fa-f0-9]{6}");
        QRegularExpressionMatchIterator i = re.globalMatch(content);

        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QColor actualColor = QColor(match.captured(0));

            // qDebug() << "---------------------------------------";
            // qDebug() << match.captured(0);
            // qDebug() << QColor(match.captured(0)).lightnessF();
            // qDebug() << content.mid(match.capturedStart(0), match.capturedEnd(0)-match.capturedStart(0));
            // qDebug() << actualColor.name();
            // qDebug() << "---------------------------------------";

            // invert only the HSV "value"/intensity value (mirror it to 0.5 on a 0.0-1.0 range)
            if(doLighten && actualColor.valueF() <= 0.52f) {
                actualColor = QColor::fromHsvF(actualColor.hsvHueF(), actualColor.hsvSaturationF(), 1.0f-(actualColor.valueF()/2.0));
            }

            if(doLighten) {
                if(mode == QIcon::Mode::Disabled) {
                    actualColor = QColor::fromHsvF(actualColor.hsvHueF(), 0.1, 0.5);
                }
            } else {
                if(mode == QIcon::Mode::Disabled) {
                    actualColor = QColor::fromHsvF(actualColor.hsvHueF(), 0.1, 0.7);
                }
            }

            // TODO: only replace if different
            content.replace(match.capturedStart(0), match.capturedEnd(0)-match.capturedStart(0), actualColor.name());
        }

        return content;
    };

public:
    explicit SVGIconEngine(const QString iconBuffer, QSettings *_applicationSettings) {
        data = iconBuffer;
        data.detach();

        applicationSettings = _applicationSettings;
    };

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {

        QByteArray dataBA;

        // TODO: move these to a separate function as the same lines are used in CamImageRegionsWidget class too !!
        // -- begin 
        // NOTE: These 4 lines below are from a SO post: https://stackoverflow.com/a/21024983
        QLabel label("something");
        int text_hsv_value = label.palette().color(QPalette::WindowText).value();
        int bg_hsv_value = label.palette().color(QPalette::Window).value();
        // bool dark_theme_found = text_hsv_value > bg_hsv_value;
        //
        // basically the icon set we use is for light theme, so we only need to decide whether to lighten colors or not
        // GUIDarkAdaptMode: 0 = no, 1 = yes, 2 = let PupilEXT guess
        bool doLighten = applicationSettings->value("GUIDarkAdaptMode", "0") == "1" || (applicationSettings->value("GUIDarkMode", "0") == "2" && text_hsv_value > bg_hsv_value);
        // end --

        // TODO: run this only if there was any change that we can really react to (doLighten or disabled mode, but nothing else)
        dataBA = changeColors(data, doLighten, mode).toUtf8();

        QSvgRenderer renderer(dataBA);
        renderer.render(painter, rect);
    };

    QIconEngine *clone() const override { 
        return new SVGIconEngine(*this); 
    };

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        // This function is necessary to create an EMPTY pixmap. It's called always before paint()
        QImage img(size, QImage::Format_ARGB32);
        img.fill(qRgba(0, 0, 0, 0));
        QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
        {
            QPainter painter(&pix);
            QRect r(QPoint(0.0, 0.0), size);
            this->paint(&painter, r, mode, state);
        }
        return pix;
    };
};


class SVGIconColorAdjuster : public QObject {
    Q_OBJECT

public:

    // Accepts an .svg file resource file name, and a QSettings instance for accessing application settings
    // (must be changeable by the user because dark mode can not be safely always detected on all platforms)
    static QIcon loadAndAdjustColors( const QString &fileName, QSettings *applicationSettings ) {

        QFile svgFile(fileName);
        if(!svgFile.open(QFile::ReadOnly | QFile::Text)){
            return QIcon();
        }
        QTextStream inp(&svgFile);
        QString content = inp.readAll();
        svgFile.close();

        return QIcon(new SVGIconEngine(content, applicationSettings));
    };
};

