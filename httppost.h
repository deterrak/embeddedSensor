#ifndef HTTPPOST_H
#define HTTPPOST_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include "displaymatrix.h"

class httpPost : public QObject
{
    Q_OBJECT
public:
    httpPost();
    void setAuthenticationURL(QString URL);
    void setUsername(QString username);
    void setPassword(QString password);
    void setWebHookUrl(QString URL);
    void setDescriptiom(QString Description);

public slots:
    void eventReceived(QString input);
    void getAuthenticationToken();
    void httpPostToURL(QString URL, QJsonObject postPayload);
    void tokenReplyFinished(QNetworkReply*);
    void webhookReplyFinished(QNetworkReply*);
    void onSslError(QNetworkReply*, QList<QSslError>);
    void configuration(QJsonObject config);

signals:
    void workflowIsActive();
    void workflowNotActive();
    void authenticated();
    void authFailure();

private slots:
    void workFlowActiveTimer();

private:
    QNetworkAccessManager *authenticationNam;
    QNetworkAccessManager *webhookNam;
    QNetworkReply* reply;
    QString authenticationURL;
    QString mUsername;
    QString mPassword;
    QString webHookURL;
    QByteArray authenticationToken;
    QJsonObject mConfig;
    QString getHostname();
    QString description;
    QJsonObject mConfiguration;
    displayMatrix *display;
    bool isWorkflowActive;
    QTimer *timer;
};

#endif // HTTPPOST_H
