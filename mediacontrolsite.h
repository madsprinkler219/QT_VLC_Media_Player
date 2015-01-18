#ifndef MEDIACONTROLSITE_H
#define MEDIACONTROLSITE_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>

class MediaControlSite : public QObject
{
    Q_OBJECT
public:
    explicit MediaControlSite(QObject *parent = 0);
    QTcpServer *server;
    QTcpSocket *socket;

signals:

public slots:
    void newConnection();

};

#endif // MEDIACONTROLSITE_H
