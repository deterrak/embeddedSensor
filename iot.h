#ifndef IOT_H
#define IOT_H

#include <QObject>
#include <QJsonObject>
#include <QJsonObject>

class iot : public QObject
{
    Q_OBJECT
public:
    explicit iot(QObject *parent = 0);
    QString getValue(QString);
    QString getEth0IP();


signals:
    void iotIs(QJsonObject);

public slots:

private:
    int readConfiguration();
    QJsonObject mJsonObj;
};

#endif // CONFIGURATION_H
