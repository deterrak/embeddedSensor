#include <QCoreApplication>
#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QJsonObject>

#include "configuration.h"
#include "gpio.h"
#include "httppost.h"
#include "irdecoder.h"

gpio *sensorGpio[64];

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "main: start the embedded sensor";

    httpPost postToWorkFlowComposer;
    configuration config;

    irdecoder m_irdecoder;
    QObject::connect(&m_irdecoder, SIGNAL(dataReceived(QString)), &postToWorkFlowComposer, SLOT(eventReceived(QString)));

    QStringList sensors;
    //sensors << "gpio66" << "gpio67" << "gpio69" << "gpio68" << "gpio44" << "gpio45";
    sensors << "gpio66" << "gpio44" << "gpio45";

    {
        int i=0;
        foreach (const QString &sensor, sensors)
        {
            sensorGpio[i] = new gpio(0,sensor);
            sensorGpio[i]->setObjectName(sensor);
            QObject::connect(sensorGpio[i], SIGNAL(dataReceived(QString)), &postToWorkFlowComposer, SLOT(eventReceived(QString)));
            i++;
            qDebug() << "setting up sensor = " << sensor;
        }
    }


    // LED #1 - flash when IR is detected fromIR_sensor#1
    gpio LED1(0,"gpio49");
    LED1.setGPIODirection("out");
    LED1.setGPIOValue("0");
    //QObject::connect(sensorGpio[0], SIGNAL(IrReceived()), &LED1, SLOT(flash()));
    //LED1.blinkFast();

#if 0
    // LED #2
    gpio LED2(0,"gpio40");
    LED2.setGPIODirection("out");
    LED2.setGPIOValue("0");
    //QObject::connect(&postToWorkFlowComposer, SIGNAL(blinkLED()), &LED2, SLOT(blinkFast()));
    //QObject::connect(&postToWorkFlowComposer, SIGNAL(blinkLEDStop()), &LED2, SLOT(blinkStop()));    //LED2.blinkFast();
#endif


    // LED #3 --bottom right LED blink while workflow is active
    gpio LED3(0,"gpio115");
    LED3.setGPIODirection("out");
    LED3.setGPIOValue("0");
    QObject::connect(&postToWorkFlowComposer, SIGNAL(workflowIsActive()), &LED3, SLOT(blinkFast()));
    QObject::connect(&postToWorkFlowComposer, SIGNAL(workflowNotActive()), &LED3, SLOT(blinkStop()));
    //LED3.blinkSlow();


    // LED #5 --flash LED5 when Sensor Authenticastes with Workflow Composer
    gpio LED5(0,"gpio20");
    LED5.setGPIODirection("out");
    LED5.setGPIOValue("0");
    QObject::connect(sensorGpio[0], SIGNAL(lazerTagHitReceived()), &LED5, SLOT(flashLong()));


    // read the configuration information and populate postToWorkFlowComposer object
    postToWorkFlowComposer.setDescriptiom(config.getDescription());
    postToWorkFlowComposer.setWebHookUrl(config.getWfcUrl());
    postToWorkFlowComposer.setAuthenticationURL(config.getWfcAuthUrl());
    postToWorkFlowComposer.setUsername(config.getUsername());
    postToWorkFlowComposer.setPassword(config.getPassword());

#if 0
    // GPIO31
    gpio LED6(0,"gpio31");
    LED6.setGPIODirection("out");
    LED6.setGPIOValue("0");
    QObject::connect(&postToWorkFlowComposer, SIGNAL(authenticated()), &LED6, SLOT(blinkFast()));
    QObject::connect(&postToWorkFlowComposer, SIGNAL(authFailure()), &LED6, SLOT(blinkStop()));
#endif

#if 0
    sensorThread captureEvents;

    postToWorkFlowComposer.setDescriptiom(captureEvents.getDescription());
    postToWorkFlowComposer.setWebHookUrl(captureEvents.getWfcUrl());
    postToWorkFlowComposer.setAuthenticationURL(captureEvents.getWfcAuthUrl());
    postToWorkFlowComposer.setUsername(captureEvents.getUsername());
    postToWorkFlowComposer.setPassword(captureEvents.getPassword());


    qDebug() << "main: setup the signal/slots connections";
    QObject::connect(&captureEvents, SIGNAL(eventReceived(QString)), &postToWorkFlowComposer, SLOT(eventReceived(QString)));
    QObject::connect(&captureEvents, SIGNAL(configuration(QJsonObject)), &postToWorkFlowComposer, SLOT(configuration(QJsonObject)));


    qDebug() << "main: start the captureEvents timer";
    // if the sensor stops for any reason, start it back up automatically.
    QTimer eventCaptureTtimer;
    QObject::connect(&eventCaptureTtimer, SIGNAL(timeout()), &captureEvents, SLOT(start()));
    eventCaptureTtimer.start(5000);
#endif

    //get the initial authentication
    QTimer::singleShot(50, &postToWorkFlowComposer, SLOT(getAuthenticationToken()));

    //refresh the authentication token
    int hourInMilliSeconds = 60*60*1000; // 60 minutes in hour * 60 seconds in minute * 1000 milliseconds in sec
    QTimer authenticationTokenTimer;
    QObject::connect(&authenticationTokenTimer, SIGNAL(timeout()), &postToWorkFlowComposer, SLOT(getAuthenticationToken()));
    authenticationTokenTimer.start(3 * hourInMilliSeconds);

#if 0
    qDebug() << "main: manually start the capturePactes the first time";
    captureEvents.setGPIO("gpio66");
    captureEvents.start();
#endif

    return a.exec();
}
