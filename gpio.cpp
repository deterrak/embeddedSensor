#include "gpio.h"
#include "iot.h"
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QThread>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QUdpSocket>

gpio::gpio(QObject *parent, QString GPIO) : QObject(parent)
{
    numberOfHits = 0;
    groupAddress = QHostAddress("239.1.1.2");


    mGPIO = GPIO;

    // setup thepath to the kernel interfac to the GPIO
    mPath = "/sys/class/gpio/" + mGPIO;

    // setup the path  to the regualr posix file that we can watch for changes
    mTmpPath = "/tmp/" + mGPIO + ".txt";

    //setup a QfileSystemWatch
    gpioNotify = new QFileSystemWatcher();
    gpioNotify->addPath(mTmpPath);

    if (this->isGPIO_in())
    {
        qDebug() << "gpio::gpio: is configured for input";
        qDebug() << "gpio::gpio: edge is configured for: " << this->getGPIOEdgeDetection();
        qDebug() << "gpio::gpio: current value is: " << this->getGPIOValue();
    }

    // setup a signal that can be emitted by GPIO
    connect(gpioNotify, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
    qDebug() << "gpio::gpio: Constructed [" << mGPIO << "]  [" << mPath << "]" << "[" << mTmpPath << "]";

    //define a timer to blink an LED and connect it to our toggel slot.
    LEDTimer = new QTimer();
    connect(LEDTimer, SIGNAL(timeout()), this, SLOT(toggelLED()));

    // every 30 seconds decrement the number of hits
    numberOfHitsTimer = new QTimer();
    connect(numberOfHitsTimer, SIGNAL(timeout()), this, SLOT(decrementNumberOfHits()));
    numberOfHitsTimer->start(30*1000);
}

gpio::~gpio()
{
    this->deleteLater();
    qDebug() << "gpio::~gpio: Destructor caled";
}

QString gpio::getGPIO()
{
    return mGPIO;
}

bool gpio::isGPIOValid()
{
    // check to see if the GPIO file exists
    QFileInfo check_file(mPath + "/value");
    // check if file exists and if yes: Is it really a file and not a directory?
    if (check_file.exists() && check_file.isFile())
    {
        return true;
    }
    else
    {
        qDebug() << "Error::gpio::isGPIOValid: GPIO doesn't exist [" << mPath << "]";
        return false;
    }
}

QString gpio::getGPIODirection()
{
    return this->readGPIO("direction");
}

bool gpio::setGPIODirection(QString direction)
{
    // values for edge can be "in, out"
    if ((direction.compare("in") == 0) || (direction.compare("out") == 0))
    {
        if (this->writeGPIO("direction",direction))
            return true;
    }
    else
    {
        qDebug() <<"gpio::setGPIODirection: unable to set unknown direction [" << direction << "] on GPIO [" << this->mGPIO << "]";
    }
    return false;
}

bool gpio::isGPIO_out()
{
    // check to see if the GPIO is set to out
    if (this->readGPIO("direction").contains("out"))
    {
        return true;
    }
    else
    {
        qDebug() << "ERROR::gpio::isGPIO_out: GPIO direction is not out [" << this->readGPIO("direction") << "]";
    }
    // else return false
    return false;
}

bool gpio::isGPIO_in()
{
    // check to see if the GPIO is set to in
    if (this->readGPIO("direction").contains("in"))
    {
        return true;
    }
    else
    {
        qDebug() << "ERROR::gpio::isGPIO_in: GPIO direction is not in";
    }
    return false;
}

QString gpio::getGPIOValue()
{
    return this->readGPIO("value");
}

bool gpio::setGPIOValue(QString value)
{
    // values for edge can be "0, 1"
    if ((value.compare("0") == 0) || (value.compare("1") == 0))
    {
        if (this->writeGPIO("value",value))
            return true;
    }
    else
    {
        qDebug() << "Error::gpio::setGPIOValue: Attempt to set and unknown value [" << value << "]";
    }
    return false;
}

QString gpio::getGPIOEdgeDetection()
{
    return this->readGPIO("edge");
}

bool gpio::setGPIOEdgeDetection(QString edge)
{
    // values for edge can be "rising, falling, both"
    if ((edge.compare("rising") == 0) || (edge.compare("falling") == 0) || (edge.compare("both") == 0))
    {
        if (this->writeGPIO("edge", edge))
            return true;
    }
    else
    {
        qDebug() << "Error::gpio::setGPIOEdgeDetection: Attempt to set and unknown edge [" << edge << "]";
    }
    return false;
}

void gpio::fileChanged(QString filename)
{
    emit  IrReceived();
    //qDebug() << "gpio::fileChanged [" << filename << "]";

    // set a delay (a buffer depth) so that all the bits are in place
    // as the bits are serialized, we can now emit them all as a serial stream
    QThread::msleep(100);

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
        //qDebug() << "gpio::filechanged: Value is [" << buffer << "] clearing";
    }

    this->filterGpio(buffer);
#if 0
    // 111111111 is emitted by the lazer tag gun
    if (buffer.compare("111111111", Qt::CaseInsensitive) == 0)
    {
        // want to send some JSON formatted data.
        // gpioName: GPIO66
        // gpioValue: 111111111
        // IotDescription: sensor#1

        QJsonObject jsonObject;

        // look up the GPIO -> Description in the iot.json file
        // getGPIO() returns the GPIO# of the instance.
        iot iotCfg;
        QString name(this->getGPIO());

        jsonObject["description"] = iotCfg.getValue("description"); // read the description key from the iot.json file
        jsonObject["trigger"] = iotCfg.getValue(name); // look up the gpio# and find the sensor name in iot.josn file
        jsonObject["hostname"] = iotCfg.getValue("hostname"); // look up the gpio# and find the sensor name in iot.josn file
        jsonObject["ipaddress"] = iotCfg.getValue("ipaddress"); // look up the gpio# and find the sensor name in iot.josn file

        jsonObject["gpio"] = name; // the gpio name
        jsonObject["value"] = buffer; // the current gpio value

        QJsonDocument jsonDocument;
        jsonDocument.setObject(jsonObject);

        qDebug() << "gpio::fileChanged: jsondDocument [" << jsonDocument.toJson() << "]";

        // serialize the json to a qstring

        // emit the serialized data
        QString out(jsonDocument.toJson(QJsonDocument::Compact));
        emit dataReceived(out);
        qDebug() << "Lazer Tag Hit !!! :: " << jsonDocument.toJson(QJsonDocument::Compact);
    }
#endif
}

void gpio::toggelLED()
{
    // check to see if this gpio i s configured for output
    if(this->isGPIO_out())
    {
        // get the current state of the LED
        if (this->readGPIO("value").compare("0\n") == 0)
        {
            //LED is off turn it on
            this->LEDon();
        }
        else
        {
            //LED is on turn it off
            this->LEDoff();
        }
    }
}

void gpio::flash()
{
    if(this->isGPIO_out())
    {
        // then we will set LED on
        this->writeGPIO("value","1");
        //QThread::sleep(50);
        this->writeGPIO("value","0");
    }
}

void gpio::flashLong()
{
    if(this->isGPIO_out())
    {
        // then we will set LED on
        this->writeGPIO("value","1");
        //QThread::sleep(500);
        this->writeGPIO("value","0");
    }
}

void gpio::blinkSlow()
{
    LEDTimer->start(500);
}

void gpio::blinkFast()
{
    LEDTimer->start(100);
}

void gpio::blinkStop()
{
    LEDTimer->stop();
    LEDoff();
}

void gpio::LEDon()
{
    if(this->isGPIO_out())
    {
        // then we will set LED on
        this->writeGPIO("value","1");
    }
}

void gpio::LEDoff()
{
    if(this->isGPIO_out())
    {
        // then we will set LED to off
        this->writeGPIO("value","0");
    }
}

void gpio::decrementNumberOfHits()
{
    if (numberOfHits > 0)
    {
        numberOfHits--;
        sendNumberOfHits(numberOfHits);
        qDebug() << this->getGPIO() << " Decrementing numberOfHits to: " << numberOfHits;
    }
}

bool gpio::writeGPIO(QString attribute, QString value)
{
    // if the GPIO is valid open the GPIO file for writing
    if (this->isGPIOValid())
    {
        //read the direction
        QFile file(mPath + "/" + attribute);
        if (!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Error::gpio::writeGPIO:was not able to open file" << file.fileName();
            return false;
        }
        else
        {
            //set the value
            file.write(value.toLocal8Bit());
            //close the file
            file.close();
            return true;
        }
    }
    else
    {
    }
    return false;
}

QString gpio::readGPIO(QString attribute)
{
    // if the GPIO is valid open the GPIO file for reading
    if (this->isGPIOValid())
    {
        //read the direction
        QFile file(mPath + "/" + attribute);
        if (!file.open(QIODevice::ReadOnly))
        {
            return "Error::gpio::readGPIO: was not able to open file"  + file.fileName();
        }
        else
        {
            //get the value
            QByteArray value_array(file.readAll());
            QString value(value_array);

            //qDebug() << "gpio::readGPIO: " << this->getGPIO() << " value is -> " << value;

            //close the file
            file.close();
            return value;
        }
    }
    else
    {
    }
    return "Errror: can't read [" + mPath + "\edge]";
}

void gpio::filterGpio(QString buffer)
{
    if (buffer.compare("111111111", Qt::CaseInsensitive) == 0)
    {
        emit lazerTagHitReceived();
        numberOfHits++;
        sendNumberOfHits(numberOfHits);
        qDebug() << this->getGPIO() << " number of hits are: " << numberOfHits;

        // send multicast update
    }

    // 111111111 is emitted by the lazer tag gun
    if ((buffer.compare("111111111", Qt::CaseInsensitive) == 0) && (numberOfHits > 9))
    {
        numberOfHits = 0;
        sendNumberOfHits(numberOfHits);

        // want to send some JSON formatted data.
        // gpioName: GPIO66
        // gpioValue: 111111111
        // IotDescription: sensor#1

        QJsonObject jsonObject;

        // look up the GPIO -> Description in the iot.json file
        // getGPIO() returns the GPIO# of the instance.
        iot iotCfg;
        QString name(this->getGPIO());

        jsonObject["description"] = iotCfg.getValue("description"); // read the description key from the iot.json file
        jsonObject["trigger"] = iotCfg.getValue(name); // look up the gpio# and find the sensor name in iot.josn file
        jsonObject["hostname"] = iotCfg.getValue("hostname"); // look up the gpio# and find the sensor name in iot.josn file
        jsonObject["ipaddress"] = iotCfg.getValue("ipaddress"); // look up the gpio# and find the sensor name in iot.josn file

        jsonObject["gpio"] = name; // the gpio name
        jsonObject["value"] = buffer; // the current gpio value

        QJsonDocument jsonDocument;
        jsonDocument.setObject(jsonObject);

        qDebug() << "gpio::fileChanged: jsondDocument [" << jsonDocument.toJson() << "]";

        // serialize the json to a qstring

        // emit the serialized data
        QString out(jsonDocument.toJson(QJsonDocument::Compact));
        emit dataReceived(out);
        qDebug() << "Lazer Tag Hit !!! :: " << jsonDocument.toJson(QJsonDocument::Compact);
    }
}

void gpio::sendNumberOfHits(int hits)
{
    QUdpSocket udpSocket;
    udpSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 2);
    QByteArray datagram = "NumberOfHits: " + QByteArray::number(hits);
    udpSocket.writeDatagram(datagram.data(), datagram.size(),
                                 groupAddress, 45454);

}
