#ifndef DISPLAYMATRIX_H
#define DISPLAYMATRIX_H

#include <QObject>
#include <QProcess>

class displayMatrix : public QObject
{
    Q_OBJECT
public:
    explicit displayMatrix(QObject *parent = 0);

signals:

public slots:
    void clear_display();
    void display_flash();
    void display_X();
    void display_x();
    void display_finished(int);
private:
    void executeDisplay(QString pythonScriptName);
    QProcess *P;
};

#endif // DISPLAYMATRIX_H
