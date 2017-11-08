#include "configuration.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QDebug>

configuration::configuration(QObject *parent) : QObject(parent)
{
    this->readConfiguration();
}

QString configuration::getWfcUrl()
{
    return mWfc_webhook_url;
}

QString configuration::getUsername()
{
    return mUsername;
}

QString configuration::getPassword()
{
    return mPassword;
}

QString configuration::getWfcAuthUrl()
{
    return mWfc_auth_url;
}

QString configuration::getDescription()
{
    return mDescription;
}

int configuration::readConfiguration()
{
    // open a file for reading
    QFile file("/home/user/QT/build-embeddedSensor-Desktop-Debug/config.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug() << "configuration::getConfiguration: configuration file was opened!";
    }
    else
        qDebug() << "configuration::getConfiguration: configuration file was not opened";

    // read the contents
    QByteArray contents;
    contents = file.readAll();
    file.close();

#if 0
    qDebug() << "sensorThread::getConfiguration: file conetents are:";
    qDebug() << "---";
    qDebug() << contents;
    qDebug() << "---";
#endif

    // convert to JSON
    QJsonParseError *error = new QJsonParseError();
    QJsonDocument jsonDoc;
    jsonDoc = QJsonDocument::fromJson(contents,  error);

    QJsonObject jsonObj = jsonDoc.object();

    if (!error->errorString().contains("no error occurred"))
    {
        qDebug() << "configuration::getConfiguration: json parse erros string is -> " << error->errorString();
    }
    free(error);

    // Parse the JSON values and store in private variables
    qDebug() << "configuration::getConfiguration: configuration is ---> " << jsonDoc;
    mWfc_webhook_url = jsonObj.value("wfcWebhookUrl").toString();
    mWfc_auth_url = jsonObj.value("wfcAuthenticationUrl").toString();
    mUsername = jsonObj.value("username").toString();
    mPassword = jsonObj.value("password").toString();
    mDescription = jsonObj.value("description").toString();

    // send the configuration to httppost so that we can include that informaiton in the post to WFC
    emit configurationIs(jsonObj);

    return 1;
}
