#ifndef IRDECODER_H
#define IRDECODER_H

#include <QObject>
#include <QFileSystemWatcher>

class irdecoder : public QObject
{
    Q_OBJECT
public:
    explicit irdecoder(QObject *parent = 0);

signals:
    void dataReceived(QString);

public slots:
    void fileChanged(QString filename);

private:
    QFileSystemWatcher *irDecodeNotify;
    void filterGpio(QString buffer);
    bool kompareStrs(QStringList list, QString str);
};

#endif // IRDECODER_H
