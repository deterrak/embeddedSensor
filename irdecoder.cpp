#include "irdecoder.h"
#include "iot.h"
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

irdecoder::irdecoder(QObject *parent) : QObject(parent)
{
    // setup the path  to the regualr posix file that we can watch for changes
    QString mTmpPath = "/tmp/irdecoder";

    //setup a QfileSystemWatch
    irDecodeNotify = new QFileSystemWatcher();
    irDecodeNotify->addPath(mTmpPath);

    // setup a signal that can be emitted by GPIO
    connect(irDecodeNotify, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

}

void irdecoder::fileChanged(QString filename)
{
    // set a delay (a buffer depth) so that all the bits are in place
    // as the bits are serialized, we can now emit them all as a serial stream
    QThread::msleep(50);

    // read the file  contents
    QFile file(filename);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Error::gpio:: fileChanged: could not open the file: " << filename;
        return;
    }

    // read the contents of the file / buffer
    QTextStream in(&file);
    QString buffer(in.readAll());

    //clear the contents of the file. i.e. empty the buffer.
    if (file.size() > 0)
    {
        file.resize(0);
        file.close();
        qDebug() << "irdecoder::filechanged: Value is [" << buffer << "] clearing";
    }

    this->filterGpio(buffer);
}

void irdecoder::filterGpio(QString buffer)
{
    // define a list of ir codes
    QStringList codes;
    codes << "e780" << "6981" << "69e0" << "6781";

    // compare the irdecode in buffer to a list of allowed codes
    if (!kompareStrs(codes, buffer))
    {
        qDebug() << "irdecoder::filterGpio: not a match: " << buffer;
    }
    else
    {
        QJsonObject jsonObject;

        // look up the GPIO -> Description in the iot.json file
        // getGPIO() returns the GPIO# of the instance.
        iot iotCfg;

        jsonObject["description"] = iotCfg.getValue("description"); // read the description key from the iot.json file
        jsonObject["trigger"] = "irdecoder"; // look up the gpio# and find the sensor name in iot.josn file
        jsonObject["hostname"] = iotCfg.getValue("hostname"); // look up the gpio# and find the sensor name in iot.josn file
        jsonObject["ipaddress"] = iotCfg.getValue("ipaddress"); // look up the gpio# and find the sensor name in iot.josn file

        jsonObject["gpio"] = "irdecoder"; // the gpio name
        jsonObject["value"] = buffer; // the current gpio value
        jsonObject["code"] = iotCfg.getValue(buffer);

        QJsonDocument jsonDocument;
        jsonDocument.setObject(jsonObject);

        // serialize the json to a qstring
        QString out(jsonDocument.toJson(QJsonDocument::Compact));

        // emit the serialized data
        emit dataReceived(out);
        qDebug() << "IR DECODE :: " << out;
    }
}

bool irdecoder::kompareStrs(QStringList list, QString str)
{
    QString item;
    foreach (item, list)
    {
        if(str.contains(item, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

