#ifndef CONFIGURATIONFILE
#define CONFIGURATIONFILE

#include <QFile>
#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QIODevice>
#include <QDateTime>


class configurationFile : public QFile{
    Q_OBJECT

public:
    configurationFile(const QString &, QObject *parent= 0);
    double readLifePrice();
    double readInityalVelocity();
    QVariant readKey(const QString &key);
    static void writeOnFile(QString name, QVariant value);
    static QVariant readKey(const QString &name, const QString &keyA);
    //static int *vectorTubosNivel(QString v);

    QByteArray data;
};

#endif
