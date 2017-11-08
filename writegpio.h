#ifndef WRITEGPIO_H
#define WRITEGPIO_H

#include <QObject>

class writeGpio : public QObject
{
    Q_OBJECT
public:
    explicit writeGpio(QObject *parent = 0);

signals:

public slots:
};

#endif // WRITEGPIO_H