#include "findtablewidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

FindTableWidget::FindTableWidget(QTcpSocket* socket, QWidget *parent)
    : QWidget(parent), tcpSocket(socket)
{
    if (!tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "警告", "请先连接到数据库服务器！");
        return;
    }

    sqlHandler = SqlProcessHandler::getInstance();
    sqlHandler->setSocket(socket);
    tableModel = new QStandardItemModel(this);
    setupUI();
    initConnections();
    loadTableList();
}

FindTableWidget::~FindTableWidget()
{
    delete tableModel;
}

void FindTableWidget::setupUI()
{
    // 创建主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 创建下拉框
    tableComboBox = new QComboBox(this);
    tableComboBox->setMinimumSize(150, 50);
    tableComboBox->setFont(QFont("Microsoft YaHei", 11));
    
    // 创建水平布局来居中显示下拉框
    QHBoxLayout* comboLayout = new QHBoxLayout();
    comboLayout->addStretch();
    comboLayout->addWidget(tableComboBox);
    comboLayout->addStretch();
    
    mainLayout->addLayout(comboLayout);

    // 创建表格视图
    resultView = new QTableView(this);
    resultView->setModel(tableModel);
    resultView->setMinimumWidth(800);
    resultView->setEditTriggers(QAbstractItemView::DoubleClicked | 
                               QAbstractItemView::EditKeyPressed);
    mainLayout->addWidget(resultView);

    setWindowTitle("查找表");
    setMinimumWidth(1000);
}

void FindTableWidget::initConnections()
{
    connect(tableComboBox, &QComboBox::currentTextChanged,
            this, &FindTableWidget::onTableSelected);
    connect(sqlHandler, &SqlProcessHandler::dataReceived,
            this, &FindTableWidget::onDataReceived);
    connect(tableModel, &QStandardItemModel::itemChanged,
            this, &FindTableWidget::onTableDataChanged);
}

void FindTableWidget::loadTableList()
{
    // 构造获取表列表的命令
    QJsonObject sqlObj;
    sqlObj["sqlstr"] = "SELECT name FROM sqlite_master WHERE type='table';";
    QString cmd = sqlHandler->convertCmd(EXEC_SQL, sqlObj);
    currentQueryType = QueryType::TableList;  // 设置查询类型
    tcpSocket->write(cmd.toUtf8());
}

void FindTableWidget::onDataReceived(const QByteArray& data)
{
    // 打印原始数据于调试
    qDebug() << "Received data:" << data;

    // 移除可能的转义字符并解析JSON
    QString jsonStr = QString::fromUtf8(data).trimmed();
    if (jsonStr.startsWith("\"") && jsonStr.endsWith("\"")) {
        jsonStr = jsonStr.mid(1, jsonStr.length() - 2);
    }
    jsonStr.replace("\\\"", "\"");
    jsonStr.replace("\\\\", "\\");

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isObject()) {
        QMessageBox::warning(this, "错误", "返回数据格式错误\n" + jsonStr);
        return;
    }

    QJsonObject jsonObj = doc.object();
    TableData tableData;
    
    tableData.setStatus(jsonObj["status"].toInt());
    tableData.setMsg(jsonObj["msg"].toString());

    if (tableData.getStatus() != 0) {
        QMessageBox::warning(this, "错误", tableData.getMsg());
        return;
    }

    // 根据查询类型处理不同的返回数据
    switch (currentQueryType) {
        case QueryType::TableList:
            // 处理表列表数据
            {
                QJsonArray rows = jsonObj["rows"].toArray();
                tableComboBox->clear();
                for (const auto& row : rows) {
                    QJsonObject rowObj = row.toObject();
                    QString tableName = rowObj["name"].toString();
                    if (!tableName.isEmpty()) {
                        tableComboBox->addItem(tableName);
                    }
                }
            }
            break;

        case QueryType::TableData:
            // 处理表数据
            {
                // 获取列信息
                QJsonObject columns = jsonObj["columns"].toObject();
                if (columns.isEmpty()) {
                    QMessageBox::warning(this, "错误", "未找到列信息");
                    return;
                }

                // 在设置数据前断开信号连接
                disconnect(tableModel, &QStandardItemModel::itemChanged,
                          this, &FindTableWidget::onTableDataChanged);

                // 设置表格列
                tableModel->clear();
                QStringList headerLabels = columns.keys();
                headerLabels << "操作";  // 添加操作列
                tableModel->setHorizontalHeaderLabels(headerLabels);

                // 填充数据
                QJsonArray rows = jsonObj["rows"].toArray();
                for (int row = 0; row < rows.size(); ++row) {
                    QJsonObject rowObj = rows[row].toObject();
                    for (int col = 0; col < headerLabels.size() - 1; ++col) {  // -1 是因为最后一列是操作列
                        QString value = rowObj[headerLabels[col]].toString();
                        QStandardItem* item = new QStandardItem(value);
                        tableModel->setItem(row, col, item);
                    }
                    addDeleteButton(row);  // 添加删除按钮
                }

                // 调整表格列宽
                resultView->resizeColumnsToContents();
                resultView->resizeRowsToContents();

                // 数据填充完成后重新连接信号
                connect(tableModel, &QStandardItemModel::itemChanged,
                        this, &FindTableWidget::onTableDataChanged);
            }
            break;

        case QueryType::UpdateData:
            // 处理更新结果
            if (tableData.getStatus() != 0) {
                QMessageBox::warning(this, "更新失败", tableData.getMsg());
                // 刷新表格数据
                onTableSelected(currentTable);
            }
            break;

        case QueryType::DeleteData:
            // 处理删除结果
            if (tableData.getStatus() != 0) {
                QMessageBox::warning(this, "删除失败", tableData.getMsg());
            }
            // 刷新表格数据
            onTableSelected(currentTable);
            break;
    }
}

void FindTableWidget::onTableSelected(const QString& tableName)
{
    if (tableName.isEmpty()) {
        return;
    }

    currentTable = tableName;  // 保存当前表名
    // 构造查询整表的SQL语句
    QJsonObject sqlObj;
    QString querySQL = QString("SELECT * FROM %1;").arg(tableName);
    sqlObj["sqlstr"] = querySQL;
    
    // 发送查询命令
    QString cmd = sqlHandler->convertCmd(EXEC_SQL, sqlObj);
    currentQueryType = QueryType::TableData;  // 设置查询类型
    tcpSocket->write(cmd.toUtf8());
}

void FindTableWidget::onTableDataChanged(QStandardItem* item)
{
    if (!item || currentTable.isEmpty()) {
        return;
    }

    // 获取修改的行、列和新值
    int row = item->row();
    int column = item->column();
    QString newValue = item->text();

    // 生成更新SQL语句
    QString updateSql = generateUpdateSql(row, column, newValue);
    if (updateSql.isEmpty()) {
        return;
    }

    // 发送更新命令
    QJsonObject sqlObj;
    sqlObj["sqlstr"] = updateSql;
    QString cmd = sqlHandler->convertCmd(EXEC_SQL, sqlObj);
    currentQueryType = QueryType::UpdateData;
    tcpSocket->write(cmd.toUtf8());
}

QString FindTableWidget::generateUpdateSql(int row, int column, const QString& newValue)
{
    // 获取表头（列名）
    QStringList headers;
    for (int i = 0; i < tableModel->columnCount(); ++i) {
        headers << tableModel->horizontalHeaderItem(i)->text();
    }

    // 找到ID列的索引
    int idColumnIndex = -1;
    for (int i = 0; i < headers.size(); ++i) {
        if (headers[i].toLower() == "id") {  // 不区分大小写查找ID列
            idColumnIndex = i;
            break;
        }
    }

    // 如果没有找到ID列，返回空
    if (idColumnIndex == -1) {
        QMessageBox::warning(this, "错误", "未找到ID列，无法更新数据");
        return QString();
    }

    // 获取ID列的值
    QString idValue = tableModel->item(row, idColumnIndex)->text();
    if (idValue.isEmpty()) {
        QMessageBox::warning(this, "错误", "ID值为空，无法更新数据");
        return QString();
    }

    // 构造UPDATE语句
    QString updateSql = QString("UPDATE %1 SET %2 = '%3' WHERE id = %4;")
                           .arg(currentTable)
                           .arg(headers[column])
                           .arg(newValue)
                           .arg(idValue);

    return updateSql;
}

void FindTableWidget::addDeleteButton(int row)
{
    QPushButton* deleteButton = new QPushButton("删除");
    deleteButton->setProperty("row", row);  // 保存行号
    connect(deleteButton, &QPushButton::clicked, this, &FindTableWidget::onDeleteButtonClicked);
    
    resultView->setIndexWidget(
        tableModel->index(row, tableModel->columnCount() - 1),
        deleteButton
    );
}

void FindTableWidget::onDeleteButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int row = button->property("row").toInt();
    
    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "确认删除", 
        "确定要删除这条记录吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 生成删除SQL语句
    QString deleteSql = generateDeleteSql(row);
    if (deleteSql.isEmpty()) {
        return;
    }

    // 发送删除命令
    QJsonObject sqlObj;
    sqlObj["sqlstr"] = deleteSql;
    QString cmd = sqlHandler->convertCmd(EXEC_SQL, sqlObj);
    currentQueryType = QueryType::DeleteData;
    tcpSocket->write(cmd.toUtf8());
}

QString FindTableWidget::generateDeleteSql(int row)
{
    // 获取表头（列名）
    QStringList headers;
    for (int i = 0; i < tableModel->columnCount() - 1; ++i) {  // -1 是因为最后一列是操作列
        headers << tableModel->horizontalHeaderItem(i)->text();
    }

    // 找到ID列的索引
    int idColumnIndex = -1;
    for (int i = 0; i < headers.size(); ++i) {
        if (headers[i].toLower() == "id") {
            idColumnIndex = i;
            break;
        }
    }

    // 如果没有找到ID列，返回空
    if (idColumnIndex == -1) {
        QMessageBox::warning(this, "错误", "未找到ID列，无法删除数据");
        return QString();
    }

    // 获取ID列的值
    QString idValue = tableModel->item(row, idColumnIndex)->text();
    if (idValue.isEmpty()) {
        QMessageBox::warning(this, "错误", "ID值为空，无法删除数据");
        return QString();
    }

    // 构造DELETE语句
    return QString("DELETE FROM %1 WHERE id = %2;")
        .arg(currentTable)
        .arg(idValue);
}
