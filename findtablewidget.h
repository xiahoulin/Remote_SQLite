#ifndef FINDTABLEWIDGET_H
#define FINDTABLEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QTableView>
#include <QTcpSocket>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QPushButton>
#include "tabledata.h"
#include "sqlprocesshandler.h"

// 修改查询类型枚举
enum class QueryType {
    TableList,
    TableData,
    UpdateData,
    DeleteData
};

class FindTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindTableWidget(QTcpSocket* socket, QWidget *parent = nullptr);
    ~FindTableWidget();

private slots:
    void onTableSelected(const QString& tableName);
    void onDataReceived(const QByteArray& data);
    void onTableDataChanged(QStandardItem* item);
    void onDeleteButtonClicked();

private:
    QTcpSocket* tcpSocket;
    QComboBox* tableComboBox;
    QTableView* resultView;
    SqlProcessHandler* sqlHandler;
    QStandardItemModel* tableModel;
    QueryType currentQueryType;
    QString currentTable;

    void setupUI();
    void initConnections();
    void loadTableList();
    void updateTableView(const TableData& data);
    QString generateUpdateSql(int row, int column, const QString& newValue);
    QString generateDeleteSql(int row);
    void addDeleteButton(int row);
};

#endif // FINDTABLEWIDGET_H 