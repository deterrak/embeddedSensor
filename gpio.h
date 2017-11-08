#ifndef GPIO_H
#define GPIO_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QHostAddress>

class gpio : public QObject
{
    Q_OBJECT
public:
    explicit gpio(QObject *parent = 0, QString GPIO = "");
    ~gpio();
    QString getGPIO();
    bool isGPIOValid();
    QString getGPIODirection();
    bool setGPIODirection(QString direction);
    bool isGPIO_out();
    bool isGPIO_in();
    QString getGPIOValue();
    bool setGPIOValue(QString value);
    QString getGPIOEdgeDetection();
    bool setGPIOEdgeDetection(QString edge);

signals:
    void dataReceived(QString);
    void IrReceived();
    void lazerTagHitReceived();


public slots:
    void fileChanged(QString);
    void toggelLED();
    void flash();
    void flashLong();
    void blinkSlow();
    void blinkFast();
    void blinkStop();
    void LEDon();
    void LEDoff();
private slots:
    void decrementNumberOfHits();

private:
    QString mGPIO;
    QString mPath;
    QString mTmpPath; //location of the gpiop-IR gpio files mirrored from the /sys/class/ files
    bool writeGPIO(QString attribute, QString value);
    QString readGPIO(QString attribute);
    QFileSystemWatcher *gpioNotify;
    QTimer *LEDTimer;
    QTimer *numberOfHitsTimer;
    void filterGpio(QString buffer);
    int numberOfHits;
    void sendNumberOfHits(int hits);
    QHostAddress groupAddress;

};

#endif // GPIO_H
