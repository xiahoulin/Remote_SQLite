#include "sqlprocesshandler.h"

SqlProcessHandler* SqlProcessHandler::instance = nullptr;

SqlProcessHandler* SqlProcessHandler::getInstance()
{
    if (!instance) {
        instance = new SqlProcessHandler();
    }
    return instance;
}

SqlProcessHandler::SqlProcessHandler(QObject *parent)
    : QObject(parent), tcpSocket(nullptr)
{
}

SqlProcessHandler::~SqlProcessHandler()
{
}

void SqlProcessHandler::setSocket(QTcpSocket* socket)
{
    // 安全断开之前的连接
    if (tcpSocket) {
        // 使用具体的信号槽断开连接
        disconnect(tcpSocket, &QTcpSocket::readyRead, 
                  this, &SqlProcessHandler::handleReadyRead);
    }
    
    tcpSocket = socket;
    if (tcpSocket) {
        // 连接新的信号槽
        connect(tcpSocket, &QTcpSocket::readyRead, 
                this, &SqlProcessHandler::handleReadyRead);
    }
    
    // 清空缓冲区
    buffer.clear();
}

void SqlProcessHandler::execSql(const QString& sql)
{
    // 将sql语句转换为json格式
    QJsonObject sqlObj;
    sqlObj["sqlstr"] = sql;
    QString cmd = convertCmd(EXEC_SQL, sqlObj);
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write(cmd.toUtf8());
    }
}

void SqlProcessHandler::handleReadyRead()
{
    QByteArray data = tcpSocket->readAll();
    emit dataReceived(data);
}

QString SqlProcessHandler::convertCmd(QString funcid, QJsonObject obj)
{
    QJsonObject root;
    root["funcid"] = funcid;
    root["appid"] = 10086;
    root["appkey"] = APPKEY;
    root["msg"] = obj;
    return QJsonDocument(root).toJson();
}

int SqlProcessHandler::convertInsertSql(TableData *pData)
{
    return 0;
}

int SqlProcessHandler::convertUpdateSql(TableData *pData)
{
    return 0;
}

int SqlProcessHandler::convertDeleteSql(TableData *pData)
{
    return 0;
}

int SqlProcessHandler::convertQueryListSql(std::string tableName)
{
    return 0;
}
