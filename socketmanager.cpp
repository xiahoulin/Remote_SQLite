#include "socketmanager.h"

SocketManager* SocketManager::instance = nullptr;

SocketManager* SocketManager::getInstance()
{
    if (!instance) {
        instance = new SocketManager();
    }
    return instance;
}

SocketManager::SocketManager(QObject *parent)
    : QObject(parent), socket(nullptr)
{
}

SocketManager::~SocketManager()
{
    closeSocket();
}

void SocketManager::setSocket(QTcpSocket* newSocket)
{
    if (socket) {
        closeSocket();
    }
    socket = newSocket;
}

void SocketManager::closeSocket()
{
    if (socket) {
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->disconnectFromHost();
            socket->waitForDisconnected();
        }
        delete socket;
        socket = nullptr;
    }
} 