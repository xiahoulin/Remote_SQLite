#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include "sqlprocesshandler.h"
#include "tabledata.h"

namespace Ui {
class ScriptWidget;
}

class ScriptWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptWidget(QTcpSocket* socket, QWidget *parent = nullptr);
    ~ScriptWidget();
    
    void setScriptContent(const QString& content);

private:
    QTcpSocket* tcpSocket;
    QTextEdit* scriptEdit;
    QPushButton* executeBtn;
    QPushButton* clearBtn;
    QTableView* resultView;
    SqlProcessHandler *sqlHandler;
    QStandardItemModel* tableModel;
    
    void setupUI();
    void initConnections();
    void updateTableView(const TableData& data);

private slots:
    void onExecuteClicked();
    void onClearClicked();
    void onDataReceived(const QByteArray& data);
};

#endif // SCRIPTWIDGET_H
