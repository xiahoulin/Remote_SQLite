#include "scriptwidget.h"
#include <QMessageBox>

ScriptWidget::ScriptWidget(QTcpSocket* socket, QWidget *parent)
    : QWidget(parent), tcpSocket(socket)
{
    setupUI();
    initConnections();
    sqlHandler = SqlProcessHandler::getInstance();
    sqlHandler->setSocket(socket);
    tableModel = new QStandardItemModel(this);
    resultView->setModel(tableModel);
}

ScriptWidget::~ScriptWidget()
{
    delete tableModel;
}

void ScriptWidget::setupUI()
{
    // 创建主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);  // 设置控件间距
    mainLayout->setContentsMargins(20, 20, 20, 20);  // 设置边距
    
    // 1. 创建文本编辑器
    scriptEdit = new QTextEdit(this);
    scriptEdit->setMinimumSize(1500, 400);  // 设置最小尺寸
    scriptEdit->setFont(QFont("Consolas", 11));
    scriptEdit->setPlaceholderText("请输入SQL语句...");
    mainLayout->addWidget(scriptEdit);
    
    // 2. 创建按钮水平布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    
    executeBtn = new QPushButton("执行", this);
    clearBtn = new QPushButton("清除", this);
    
    // 设置按钮最小高度
    executeBtn->setMinimumHeight(80);
    clearBtn->setMinimumHeight(80);
    executeBtn->setMinimumWidth(120);
    clearBtn->setMinimumWidth(120);
    
    // 设置按钮样式
    QString buttonStyle = "QPushButton { font-size: 14px; }";
    executeBtn->setStyleSheet(buttonStyle);
    clearBtn->setStyleSheet(buttonStyle);
    
    // 添加弹性空间使按钮靠右
    buttonLayout->addStretch();
    buttonLayout->addWidget(executeBtn);
    buttonLayout->addWidget(clearBtn);
    
    // 创建一个容器widget来容纳按钮布局，并设置最小高度
    QWidget* buttonContainer = new QWidget(this);
    buttonContainer->setMinimumHeight(100);
    buttonContainer->setLayout(buttonLayout);
    mainLayout->addWidget(buttonContainer);
    
    // 3. 创建表格视图
    resultView = new QTableView(this);
    resultView->setMinimumSize(1500, 500);
    resultView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);  // 水平滚动
    resultView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);    // 垂直滚动
    resultView->horizontalHeader()->setStretchLastSection(false);            // 不拉伸最后一列
    resultView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // 允许调整列宽
    resultView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);   // 允许调整行高
    resultView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);          // 需要时显示垂直滚动条
    resultView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);        // 需要时显示水平滚动条
    mainLayout->addWidget(resultView);
    
    // 设置窗口属性
    setWindowTitle("SQL脚本执行");
    setMinimumWidth(1600);  // 设置窗口最小宽度，略大于内部控件
}

void ScriptWidget::initConnections()
{
    connect(executeBtn, &QPushButton::clicked, this, &ScriptWidget::onExecuteClicked);
    connect(clearBtn, &QPushButton::clicked, this, &ScriptWidget::onClearClicked);
}

void ScriptWidget::onExecuteClicked()
{
    QString script = scriptEdit->toPlainText().trimmed();
    if(script.isEmpty()) {
        return;
    }
    
    // 清空现有表格数据
    tableModel->clear();
    
    // 执行SQL并连接返回数据的处理
    connect(sqlHandler, &SqlProcessHandler::dataReceived, 
            this, &ScriptWidget::onDataReceived);
    sqlHandler->execSql(script);
}

void ScriptWidget::onDataReceived(const QByteArray& data)
{
    // 打印原始数据用于调试
    qDebug() << "Received data:" << data;

    // 移除可能的转义字符并解析JSON
    QString jsonStr = QString::fromUtf8(data).trimmed();
    // 如果数据两端有引号，移除它们
    if (jsonStr.startsWith("\"") && jsonStr.endsWith("\"")) {
        jsonStr = jsonStr.mid(1, jsonStr.length() - 2);
    }
    // 处理转义字符
    jsonStr.replace("\\\"", "\"");
    jsonStr.replace("\\\\", "\\");

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isObject()) {
        QMessageBox::warning(this, "错误", "返回数据格式错误\n" + jsonStr);
        return;
    }

    QJsonObject jsonObj = doc.object();
    
    // 创建 TableData 对象并填充数据
    TableData tableData;
    
    // 设置状态和息
    tableData.setStatus(jsonObj["status"].toInt());
    tableData.setMsg(jsonObj["msg"].toString());

    QJsonObject columns = jsonObj["columns"].toObject();
    if(tableData.getStatus() != 0) {
        QMessageBox::warning(this, "错误", tableData.getMsg());
        return;
    }else{
        // 成功
        
        if(columns.isEmpty()){
            QMessageBox::information(this, "提示", tableData.getMsg());
            return;
        }
    }

    for (auto it = columns.begin(); it != columns.end(); ++it) {
        tableData.addColumn(it.key(), it.value().toString());
    }
    
    // 添加行数据
    QJsonArray rows = jsonObj["rows"].toArray();
    for (const auto& row : rows) {
        QJsonObject rowObj = row.toObject();
        QMap<QString, QString> rowData;
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            rowData[it.key()] = rowObj[it.key()].toString();
        }
        tableData.addRow(rowData);
    }
    
    // 更新表格视图
    updateTableView(tableData);
    
    // 断开信号连接，避免重复接收
    disconnect(sqlHandler, &SqlProcessHandler::dataReceived, 
              this, &ScriptWidget::onDataReceived);
}

void ScriptWidget::updateTableView(const TableData& data)
{
    // 检查状态
    if (data.getStatus() != 0) {
        QMessageBox::warning(this, "查询错误", data.getMsg());
        return;
    }

    // 获取列信息
    QMap<QString, QString> columns = data.getColumns();
    QVector<QMap<QString, QString>> rows = data.getRows();

    if (rows.isEmpty()) {
        QMessageBox::information(this, "提示", "查询结果为空");
        return;
    }

    // 设置表格列
    tableModel->clear();
    QStringList headerLabels = columns.keys();
    tableModel->setHorizontalHeaderLabels(headerLabels);

    // 填充数据
    for (int row = 0; row < rows.size(); ++row) {
        const QMap<QString, QString>& rowData = rows[row];
        for (int col = 0; col < headerLabels.size(); ++col) {
            QString value = rowData[headerLabels[col]];
            QStandardItem* item = new QStandardItem(value);
            tableModel->setItem(row, col, item);
        }
    }

    // 调整表格列宽以适应内容
    resultView->resizeColumnsToContents();
    resultView->resizeRowsToContents();
}

void ScriptWidget::onClearClicked()
{
    scriptEdit->clear();
    tableModel->clear();
}

void ScriptWidget::setScriptContent(const QString& content)
{
    scriptEdit->setText(content);
}
