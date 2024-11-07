#include "connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent, Qt::Window | Qt::WindowCloseButtonHint)
{
    tcpSocket = new QTcpSocket(this);
    setupUI();
    
    // 使用 lambda 表达式处理错误
    connect(tcpSocket, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &ConnectDialog::handleSocketError);
}

void ConnectDialog::setupUI()
{
    // 创建主垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);  
    mainLayout->setContentsMargins(30, 40, 30, 40);  
    
    // IP地址输入组
    QHBoxLayout *ipLayout = new QHBoxLayout();
    QLabel *ipLabel = new QLabel("IP地址:", this);
    ipLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);  // 标签左对齐且垂直居中
    ipLineEdit = new QLineEdit(this);
    ipLabel->setMinimumWidth(80);
    ipLineEdit->setPlaceholderText("请输入IP地址");
    ipLineEdit->setMinimumHeight(60);
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(ipLineEdit);
    mainLayout->addLayout(ipLayout);
    
    // 端口输入组
    QHBoxLayout *portLayout = new QHBoxLayout();
    QLabel *portLabel = new QLabel("端口:", this);
    portLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);  // 标签左对齐且垂直居中
    portLineEdit = new QLineEdit(this);
    portLabel->setMinimumWidth(80);
    portLineEdit->setPlaceholderText("请输入端口号");
    portLineEdit->setMinimumHeight(60);
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portLineEdit);
    mainLayout->addLayout(portLayout);
    
    // 数据库输入组
    QHBoxLayout *dbLayout = new QHBoxLayout();
    QLabel *dbLabel = new QLabel("数据库:", this);
    dbLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);  // 标签左对齐且垂直居中
    dbLineEdit = new QLineEdit(this);
    dbLabel->setMinimumWidth(80);
    dbLineEdit->setPlaceholderText("请输入数据库");
    dbLineEdit->setMinimumHeight(60);
    dbLayout->addWidget(dbLabel);
    dbLayout->addWidget(dbLineEdit);
    mainLayout->addLayout(dbLayout);
    
    // 添加弹性空间
    mainLayout->addStretch(1);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    
    connectButton = new QPushButton("连接", this);
    cancelButton = new QPushButton("取消", this);
    
    // 设置按钮大小
    connectButton->setFixedSize(140, 45);
    cancelButton->setFixedSize(140, 45);
    
    // 设置按钮样式
    QString buttonStyle = "QPushButton { font-size: 16px; }";
    connectButton->setStyleSheet(buttonStyle);
    cancelButton->setStyleSheet(buttonStyle);
    
    // 修改按钮布局，移除左侧弹性空间，使按钮靠右
    buttonLayout->addStretch(1);  // 添加弹性空间将按钮推到右边
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(cancelButton);
    // 移除右侧弹性空间，使按钮完全靠右
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置窗口标题
    setWindowTitle("连接设置");
    
    // 设置窗口大小
    setMinimumSize(500, 400);
    
    // 设置标签字体
    QFont labelFont("Microsoft YaHei", 12);
    ipLabel->setFont(labelFont);
    portLabel->setFont(labelFont);
    dbLabel->setFont(labelFont);
    
    // 设置输入框字体
    QFont editFont("Microsoft YaHei", 11);
    ipLineEdit->setFont(editFont);
    portLineEdit->setFont(editFont);
    dbLineEdit->setFont(editFont);
    
    // 连接信号和槽
    connect(connectButton, &QPushButton::clicked, this, &ConnectDialog::connBtnClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ConnectDialog::cancelBtnClicked);
    
    // 在setupUI末尾添加验证器设置
    setupValidators();
}

void ConnectDialog::setupValidators()
{
    // IP地址验证器 - 只允许输入数字和点
    QRegularExpression ipRegex("^[0-9.]*$");
    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ipLineEdit->setValidator(ipValidator);
    
    // 端口验证器 - 只允许输入1-65535
    QRegularExpression portRegex("^[0-9]{0,5}$");  // 最多5位数字
    QRegularExpressionValidator *portValidator = new QRegularExpressionValidator(portRegex, this);
    portLineEdit->setValidator(portValidator);
    
    // 端口输入限制
    connect(portLineEdit, &QLineEdit::textChanged, this, [this](const QString &text)->void {
        if (!text.isEmpty()) {
            int value = text.toInt();
            if (value > 65535) {
                portLineEdit->setText(QString::number(65535));
            }
        }
    });
}

ConnectDialog::~ConnectDialog()
{
}

void ConnectDialog::connBtnClicked()
{
    QString ip = ipLineEdit->text();
    int port = portLineEdit->text().toInt();
    QString db = dbLineEdit->text();
    
    if(ip.isEmpty() || portLineEdit->text().isEmpty() || db.isEmpty()) {
        QMessageBox::warning(this, "警告", "请填写所有连接信息！");
        return;
    }
    
    connectButton->setEnabled(false);
    connectButton->setText("连接中...");
    
    // 创建新的socket
    QTcpSocket* socket = new QTcpSocket();
    
    // 连接到服务器
    socket->connectToHost(ip, port);
    
    // 等待连接成功
    if(socket->waitForConnected(3000)) {  // 3秒超时
        // TCP连接成功，发送数据库连接请求
        QJsonObject msgObj;
        msgObj["dbpath"] = db;
        
        QJsonObject root;
        root["funcid"] = "100000";
        root["appid"] = 10086;
        root["appkey"] = APPKEY;
        root["msg"] = msgObj;
        
        QByteArray data = QJsonDocument(root).toJson();
        socket->write(data);
        
        // 等待响应
        if(socket->waitForReadyRead(3000)) {
            QByteArray response = socket->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if(doc.isObject()) {
                QJsonObject respObj = doc.object();
                int status = respObj["status"].toInt();
                QString msg = respObj["msg"].toString();
                
                if(status == 0) {
                    // 数据库连接成功
                    socket->setParent(nullptr);
                    emit connectionEstablished(socket);
                    accept();
                    return;
                } else {
                    // 数据库连接失败
                    QMessageBox::critical(this, "错误", "连接数据库失败：" + msg);
                }
            } else {
                QMessageBox::critical(this, "错误", "服务器响应格式错误");
            }
        } else {
            QMessageBox::critical(this, "错误", "等待服务器响应超时");
        }
        
        // 如果到这里，说明数据库连接失败，关闭socket
        socket->disconnectFromHost();
        delete socket;
    } else {
        QMessageBox::critical(this, "错误", "连接服务器失败！");
        delete socket;
    }
    
    connectButton->setEnabled(true);
    connectButton->setText("连接");
}

void ConnectDialog::handleSocketError(QAbstractSocket::SocketError error)
{
    QString errorMsg;
    switch(error) {
        case QAbstractSocket::ConnectionRefusedError:
            errorMsg = "连接被服务器拒绝";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMsg = "找不到服务器";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorMsg = "连接超时";
            break;
        default:
            errorMsg = "连接错误：" + tcpSocket->errorString();
    }
    
    QMessageBox::critical(this, "连接错误", errorMsg);
    connectButton->setEnabled(true);
    connectButton->setText("连接");
}

void ConnectDialog::cancelBtnClicked()
{
    reject();  // 关闭对话框并返回QDialog::Rejected
}
