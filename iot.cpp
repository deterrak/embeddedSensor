#include "iot.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QDebug>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>

iot::iot(QObject *parent) : QObject(parent)
{
    this->readConfiguration();
}

QString iot::getValue(QString key)
{
    return mJsonObj.value(key).toString();
}

QString iot::getEth0IP()
{
    // find the ipv4 address of eth0
    QStringList items;
    QString eth0_interface_ip;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
            foreach (QNetworkAddressEntry entry, interface.addressEntries())
            {
                //if ( interface.hardwareAddress() != "00:00:00:00:00:00" && entry.ip().toString().contains(".") && !interface.humanReadableName().contains("usb"))
                if ( interface.humanReadableName().contains("eth0") && entry.ip().toString().contains("."))
                {
                    items << interface.name() + ";"+ entry.ip().toString();
                    eth0_interface_ip = entry.ip().toString();
                }
            }
    }
    return eth0_interface_ip;
}

int iot::readConfiguration()
{
    // open a file for reading
    QFile file("/home/user/QT/build-embeddedSensor-Desktop-Debug/iot.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug() << "iot::getConfiguration: iot file was opened!";
    }
    else
        qDebug() << "iot::getConfiguration: iot file was not opened";

    // read the contents
    QByteArray contents;
    contents = file.readAll();
    file.close();

    // convert to JSON
    QJsonParseError *error = new QJsonParseError();
    QJsonDocument jsonDoc;
    jsonDoc = QJsonDocument::fromJson(contents,  error);

    mJsonObj = jsonDoc.object();

    // want to add some device specific information that is not in the configuration file.
    mJsonObj.insert("hostname", QHostInfo::localHostName());
    mJsonObj.insert("ipaddress", this->getEth0IP());

    if (!error->errorString().contains("no error occurred"))
    {
        qDebug() << "iot::getConfiguration: json parse erros string is -> " << error->errorString();
    }
    free(error);
    return 1;
}
