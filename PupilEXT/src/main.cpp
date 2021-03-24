#include "mainwindow.h"
#include <QApplication>

#include <pylon/PylonIncludes.h>

// Stream operator for custom types needed to save those types to QTs application settings structure
#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const QList<QPair<QString,QString>> &l) {
    int s = l.size();
    out << s;
    if (s) for (int i = 0; i < s; ++i) out << l[i].first << l[i].second;
    return out;
}
QDataStream &operator>>(QDataStream &in, QList<QPair<QString,QString>> &l) {
    if (!l.empty()) l.clear();
    int s = 0;
    in >> s;
    if (s) {
        l.reserve(s);
        for (int i = 0; i < s; ++i) {
            QString f, sec;
            in >> f >> sec;
            l.append(QPair<QString, QString>(f, sec));
        }
    }
    return in;
}

QDataStream &operator<<(QDataStream &stream, const QMap<QString, QList<float>> &map)
{
    QMapIterator<QString, QList<float>> i(map);
    while (i.hasNext()) {
        i.next();
        stream << i.key() << i.value();
    }
    return stream;
}
QDataStream &operator>>(QDataStream &stream, QMap<QString, QList<float>> &map)
{
    while(!stream.atEnd()) {
        QString key;
        QList<float> value;
        stream >> key;
        stream >> value;
        if(!key.isEmpty())
            map[key] = value;
    }
    return stream;
}
#endif


int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    // Changing this may change the settings path thus not loading old application settings!
    QCoreApplication::setOrganizationName("FGLT");
    QCoreApplication::setApplicationName("PupilEXT");
    QCoreApplication::setApplicationVersion("0.1.0 Beta");

    qRegisterMetaTypeStreamOperators<QMap<QString, QList<float>>>("QMap<QString,QList<float>>");
    qRegisterMetaTypeStreamOperators<QList<QPair<QString, QString>>>("QList<QPair<QString, QString>>");

    Pylon::PylonAutoInitTerm autoInitTerm;  // PylonInitialize() will be called here

    MainWindow w;
    w.setWindowIcon(QIcon(":/icon.svg"));
    w.show();

    return a.exec();
}
