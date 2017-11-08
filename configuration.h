#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QJsonObject>

class configuration : public QObject
{
    Q_OBJECT
public:
    explicit configuration(QObject *parent = 0);
    QString getWfcUrl();
    QString getUsername();
    QString getPassword();
    QString getWfcAuthUrl();
    QString getDescription();

signals:
    void configurationIs(QJsonObject);

public slots:

private:
    int readConfiguration();
    QString mGpioName;
    QString mWfc_webhook_url;
    QString mWfc_auth_url;
    QString mUsername;
    QString mPassword;
    QString mDescription;
};

#endif // CONFIGURATION_H
