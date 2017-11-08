#include "httppost.h"
#include <QProcess>
#include <QRegExp>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

httpPost::httpPost()
{
    isWorkflowActive = false;
    timer = new QTimer(); //setup a timer so that we don't trigger workflows too quickly.
    authenticationNam = new QNetworkAccessManager();
    webhookNam = new QNetworkAccessManager();
    display = new displayMatrix();
}

void httpPost::setAuthenticationURL(QString URL)
{
    authenticationURL.clear();
    authenticationURL.append(URL);
}

void httpPost::setUsername(QString username)
{
    mUsername.clear();
    mUsername.append(username);
}

void httpPost::setPassword(QString password)
{
    mPassword.clear();
    mPassword.append(password);
}

void httpPost::setWebHookUrl(QString URL)
{
    webHookURL.clear();
    webHookURL = URL;
}

void httpPost::setDescriptiom(QString Description)
{
    description.clear();
    description.append(Description);
}

void httpPost::eventReceived(QString input)
{
    // prevent us from triggering a workflow while a workflow is active.
    if (!isWorkflowActive)
    {
        // this is the SLOT that we will receive the packet cont ents from the tcpdumThread
        //qDebug() << "httpPost:eventReceived: received the signal: " << input;
        QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());

        this->httpPostToURL(webHookURL, doc.object());
        isWorkflowActive = true;
        emit workflowIsActive();
        QTimer::singleShot(73 * 1000, this, SLOT(workFlowActiveTimer()));
    }
}

void httpPost::getAuthenticationToken()
{
    // This SLOT is periodically triggered. It retrives authentication token and saves it in authenticationToken
    QByteArray postData("{\"username\": \"st2admin\", \"password\": \"password\", \"auth_url\": \"https://localhost/auth/v1/tokens\"}");
    postData.clear(); //postData has to be empty it seems... not sure why.

    QNetworkAccessManager *authenticationNam = new QNetworkAccessManager();
    connect(authenticationNam, SIGNAL(finished(QNetworkReply*)), this, SLOT(tokenReplyFinished(QNetworkReply*)));
    connect(authenticationNam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onSslError(QNetworkReply*, QList<QSslError>)));

    QUrl url;
    url.setUrl(authenticationURL);
    //QByteArray userPass("st2admin:password");
    QByteArray userPass("");
    userPass.append(mUsername);
    userPass.append(":");
    userPass.append(mPassword);
    qDebug() << userPass << "base64 is -> " << userPass.toBase64();

    QByteArray auth_array;
    auth_array.append("Basic ");
    auth_array.append(userPass.toBase64());

    QNetworkRequest netRequest(url);
    netRequest.attribute(QNetworkRequest::FollowRedirectsAttribute);
    netRequest.setRawHeader("Authorization", auth_array);
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, "python-openstackclient");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "*/*");

    reply = authenticationNam->post(netRequest,postData); //get the authentication token
}

void httpPost::httpPostToURL(QString WFCURL, QJsonObject postPayload)
{
    display->display_flash();

    QNetworkAccessManager *webhookNam = new QNetworkAccessManager();
    // goto tokenReplyFinished
    connect(webhookNam, SIGNAL(finished(QNetworkReply*)), this, SLOT(webhookReplyFinished(QNetworkReply*)));
    connect(webhookNam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onSslError(QNetworkReply*, QList<QSslError>)));

    // we will post the JSON structure of the packet captured to the workflow composer
    QJsonDocument doc(postPayload);
    qDebug() << "httpPost::httpPostToURL: size is  = " << postPayload.size();
    QByteArray postData = doc.toJson();
    qDebug() << "httpPost::httpPostToURL: converted to QBytArray->" << postData;

    QUrl url;
    url.setUrl(WFCURL);
    qDebug() << "httpPost::httpPostToURL : URL is => " << url.toString();

    QNetworkRequest URL(url);
    QByteArray x_auth_token("X-Auth-Token");

    URL.setRawHeader(x_auth_token, authenticationToken);
    URL.setHeader(QNetworkRequest::UserAgentHeader, "User-Agent: python-openstackclient");
    URL.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    URL.setRawHeader("Accept", "application/json");

    qDebug() << "httpPost::httpPostToURL : " << postData << " to URL-> " << url.toString();
    reply = webhookNam->post(URL,postData);
}

void httpPost::tokenReplyFinished(QNetworkReply *reply)
{
    //this code is triggered if the token was successfully received.
    QString output((QString)reply->readAll());
    qDebug() << "httpPost::tokenReplyFinished() : TOKEN RECEIVED-> " << output;

#if 0
    if (reply->NoError)
    {
        qDebug() << "no reply error detected";
    }
    else
    {
        qDebug() << "reply error detected " << reply->error();
    }
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    qDebug() << "connection is encrypted? " << reply->attribute((QNetworkRequest::ConnectionEncryptedAttribute)).toBool();
    qDebug() << "redirect to " << possibleRedirectUrl.toString();
#endif

    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode == 301) {
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            qDebug() << "redirect URL is " << redirectUrl.toString();
        }
        else
        {
            //qDebug() << "url error code is =" << statusCode;
        }
    }

    qDebug() << "httpPost::tokenReplyFinished() : URL was -> " << authenticationURL;
    qDebug() << "httpPost::tokenReplyFinished() : ca certs->" << reply->sslConfiguration().caCertificates();
#if 0
    qDebug() << "httpPost::tokenReplyFinished() : peer error->" << reply->errorString();
    qDebug() << "httpPost::tokenReplyFinished() : peer cert->" << reply->sslConfiguration().peerCertificateChain();
#endif

    // Example Token:
    // "{\"user\": \"st2admin\", \"token\": \"2a188d4ebe4a43bc87eb46e53a6e87fd\", \"expiry\": \"2016-12-14T19:11:35.620520Z\", \"id\": \"585047e78121922fb61fd530\", \"metadata\": {}}"
    // Parse the JSON output to get the token
    QJsonDocument tokenDocument = QJsonDocument::fromJson(output.toUtf8());
    QJsonObject jsonObject = tokenDocument.object();

    // Store the token in a private variable
    authenticationToken.clear();
    authenticationToken.append(jsonObject.value("token").toString());
    if (authenticationToken.size() > 3)
    {
        qDebug() << "httpPost::tokenReplyFinished() -> authentication token was received from workflow composer!";
        emit authenticated();
    }
    else
    {
        qDebug() << "httpPost::tokenReplyFinished() -> authentication Failure";
        emit authFailure();
    }
}

void httpPost::webhookReplyFinished(QNetworkReply *reply)
{
    qDebug() << "httpPost::webhookReplyFinished : readall";
    QString output((QString)reply->readAll());
    qDebug() << "HTTP POST-> " << output;
}

void httpPost::onSslError(QNetworkReply *reply, QList<QSslError>)
{
    // If SSL is using a self signed certificate, ignore the error that is generated
    reply->ignoreSslErrors();
    qDebug() << "ignoring SSL error";
}

void httpPost::configuration(QJsonObject config)
{
    qDebug() << "httpPost::configuration() : " << config;
    mConfiguration = config;
}

void httpPost::workFlowActiveTimer()
{
    isWorkflowActive = false;
    emit workflowNotActive();
}


QString httpPost::getHostname()
{
    // Set the hostname
    QProcess P;
    P.start("/bin/hostname");
    P.waitForFinished(1000);
    QString hostname(P.readAll());
    return(hostname);
}
