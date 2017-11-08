#include "displaymatrix.h"
#include <QThread>
#include <QDebug>
#include <QObject>

displayMatrix::displayMatrix(QObject *parent) : QObject(parent)
{
    //qDebug() << "displayMatrix::displayMatrix() Construction display object";
}

void displayMatrix::clear_display()
{
    this->executeDisplay("display_off.py");
    //qDebug() << "displayMatrix::clear_display() Construction display object";
}

void displayMatrix::display_flash()
{
    this->display_X();
    //qDebug() << "displayMatrix::display_flash()";
}

void displayMatrix::display_X()
{
    this->executeDisplay("display_X.py");
    //qDebug() << "displayMatrix::display_X()";
}

void displayMatrix::display_x()
{
    this->executeDisplay("display_x.py");
    //qDebug() << "displayMatrix::display_x()";
}

void displayMatrix::display_finished(int i )
{
    Q_UNUSED(i);
    QThread::sleep(1);
    this->clear_display();
}

void displayMatrix::executeDisplay(QString pythonScriptName)
{
    pythonScriptName.prepend("/usr/bin/python /home/user/BeagleTag/");
    //qDebug() << "displayMatrix::executeDisplay() Attempting: " << pythonScriptName;

    P = new QProcess();
    // wait for the display to be painted the call display_finished to clear the display
    //connect(P, SIGNAL(finished(int)), this, SLOT(display_finished(int)));
    P->waitForFinished(10000);
    P->start(pythonScriptName);

    //qDebug() << "displayMatrix::executeDisplay() finishing: " << P->errorString();
}
