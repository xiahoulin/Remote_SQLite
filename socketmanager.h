#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QObject>
#include <QTcpSocket>

class SocketManager : public QObject
{
    Q_OBJECT

public:
    static SocketManager* getInstance();
    QTcpSocket* getSocket() { return socket; }
    void setSocket(QTcpSocket* newSocket);
    void closeSocket();

private:
    explicit SocketManager(QObject *parent = nullptr);
    ~SocketManager();
    static SocketManager* instance;
    QTcpSocket* socket;
};

#endif // SOCKETMANAGER_H 