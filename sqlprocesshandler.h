#ifndef SQLPROCESSHANDLER_H
#define SQLPROCESSHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <string>
#include "tabledata.h"
#include "funcid.h"

class SqlProcessHandler : public QObject
{
    Q_OBJECT

public:
    static SqlProcessHandler* getInstance();
    void setSocket(QTcpSocket* socket);
    void execSql(const QString& sql);
    QString convertCmd(QString funcid, QJsonObject obj);
    int convertInsertSql(TableData *pData);
    int convertUpdateSql(TableData *pData);
    int convertDeleteSql(TableData *pData);
    int convertQueryListSql(std::string tableName);

signals:
    void dataReceived(const QByteArray& data);

private slots:
    void handleReadyRead();

private:
    explicit SqlProcessHandler(QObject *parent = nullptr);
    virtual ~SqlProcessHandler();
    static SqlProcessHandler* instance;
    
    QTcpSocket* tcpSocket;
    QByteArray buffer;
};

#endif // SQLPROCESSHANDLER_H
