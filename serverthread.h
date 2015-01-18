#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include "mediacontrolsite.h"

class ServerThread : public QThread
{
    Q_OBJECT
public:
    explicit ServerThread(QObject *parent = 0);
    void run();
    MediaControlSite *site;

signals:

public slots:

};

#endif // SERVERTHREAD_H
