#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include "socketmanager.h"
#include "funcid.h"

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();
    QTcpSocket* getSocket() { return tcpSocket; }

signals:
    void connectionEstablished(QTcpSocket* socket);

private slots:
    void connBtnClicked();
    void cancelBtnClicked();
    void handleSocketError(QAbstractSocket::SocketError error);

private:
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QLineEdit *dbLineEdit;
    QPushButton *connectButton;
    QPushButton *cancelButton;
    QTcpSocket *tcpSocket;
    
    void setupUI();
    void setupValidators();
};

#endif // CONNECTDIALOG_H
