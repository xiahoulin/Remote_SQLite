#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/pics/icons/sqlite.png"));
    setWindowTitle("SQLite远程连接器");
    setMinimumSize(1600, 1200);

    scriptWidget = nullptr;
    findTableWidget = nullptr;

    //展示控件
    showAllWidget();

    //绑定Action事件
    connect(linkAct, &QAction::triggered, this, &MainWindow::showConnectDialog);
    connect(selfQuery, &QAction::triggered, this, &MainWindow::onSelfQueryAction);
    connect(disconnectAct, &QAction::triggered, this, &MainWindow::onDisconnectAction);
    connect(execSript, &QAction::triggered, this, &MainWindow::onOpenScriptDialog);
    connect(queryTable, &QAction::triggered, this, &MainWindow::onQueryTableAction);
}

void MainWindow::onSelfQueryAction()
{
    QTcpSocket* socket = SocketManager::getInstance()->getSocket();
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "警告", "请先连接到数据库服务器！");
        return;
    }

    clearWidgets();
    scriptWidget = new ScriptWidget(socket, this);
    setCentralWidget(scriptWidget);
    scriptWidget->setFocus();
}

void MainWindow::showConnectDialog()
{
    ConnectDialog *dialog = new ConnectDialog(this);
    connect(dialog, &ConnectDialog::connectionEstablished, 
            this, [this](QTcpSocket* socket) {
        SocketManager::getInstance()->setSocket(socket);
    });
    
    int res = dialog->exec();
    if (res == QDialog::Accepted){
        QMessageBox::information(this, "连接成功", "连接成功");
        linkAct->setEnabled(false);
        disconnectAct->setEnabled(true);
    } else if (res == QDialog::Rejected){
        QMessageBox::information(this, "连接失败", "连接失败");
    }
}

void MainWindow::onDisconnectAction()
{
    SocketManager::getInstance()->closeSocket();
    clearWidgets();
    linkAct->setEnabled(true);
    disconnectAct->setEnabled(false);
    QMessageBox::information(this, "断开连接", "已成功断开连接");
}

void MainWindow::onOpenScriptDialog()
{
    QTcpSocket* socket = SocketManager::getInstance()->getSocket();
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "警告", "请先连接到数据库服务器！");
        return;
    }

    // 打开文件对话框
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("打开SQL脚本"),
        "D:/",  // 设置根目录为D盘
        tr("SQL Files (*.sql)"));  // 设置文件过滤器

    if (fileName.isEmpty()) {
        return;  // 用户取消了选择
    }

    // 读取文件内容
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件：" + fileName);
        return;
    }

    // 使用UTF-8编码读取文件内容
    QTextStream in(&file);
    in.setCodec("UTF-8");  // 设置UTF-8编码
    QString scriptContent = in.readAll();
    file.close();

    // 创建或显示scriptWidget
    if (!scriptWidget) {
        scriptWidget = new ScriptWidget(socket, this);
        setCentralWidget(scriptWidget);
    }

    // 设置脚本内容
    scriptWidget->setScriptContent(scriptContent);
    scriptWidget->setFocus();
}

void MainWindow::onQueryTableAction()
{
    QTcpSocket* socket = SocketManager::getInstance()->getSocket();
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "警告", "请先连接到数据库服务器！");
        return;
    }

    clearWidgets();  // 切换窗口前清理
    findTableWidget = new FindTableWidget(socket, this);
    setCentralWidget(findTableWidget);
    findTableWidget->setFocus();
}

MainWindow::~MainWindow()
{
    clearWidgets();
    delete scriptWidget;  // 最后清理 scriptWidget
    delete ui;
}

void MainWindow::showAllWidget(){
    //菜单
    menu = new QMenuBar(this);
    menu->setMinimumSize(1500, 25);
    QFont menuFont;
    menuFont.setPointSize(12);
    menu->setFont(menuFont);

    QFont actionFont;
    menuFont.setPointSize(20);

    //功能区
    funcMenu = new QMenu(this);
    funcMenu->setTitle("功能");
    //funcMenu->setIcon(QIcon(":/pics/icons/functions.png"));
    menu->addMenu(funcMenu);

    //执行脚本
    execSript = new QAction(this);
    execSript->setIcon(QIcon(":/pics/icons/exec.png"));
    execSript->setFont(actionFont);
    execSript->setText("执行脚本");
    execSript->setStatusTip("选择一个脚本执行");
    funcMenu->addAction(execSript);

    //保存脚本
    saveSript = new QAction(this);
    saveSript->setIcon(QIcon(":/pics/icons/save.png"));
    saveSript->setFont(actionFont);
    saveSript->setText("保存");
    saveSript->setStatusTip("保存当前编辑的脚本");
    //funcMenu->addAction(saveSript);
    


    //查找表
    queryTable = new QAction(this);
    queryTable->setIcon(QIcon(":/pics/icons/query.png"));
    queryTable->setFont(actionFont);
    queryTable->setText("查找表");
    queryTable->setStatusTip("查找数据库表数据");
    funcMenu->addAction(queryTable);

    //自定义查询
    selfQuery = new QAction(this);
    selfQuery->setIcon(QIcon(":/pics/icons/selfdef.png"));
    selfQuery->setFont(actionFont);
    selfQuery->setText("自定义查询");
    selfQuery->setStatusTip("自定义SQL语句查询");
    funcMenu->addAction(selfQuery);

    //设置
    settingMenu = new QMenu(this);
    settingMenu->setTitle("设置");
    //settingMenu->setIcon(QIcon(":/pics/icons/setting.png"));
    menu->addMenu(settingMenu);
    //连接
    linkAct = new QAction(this);
    linkAct->setIcon(QIcon(":/pics/icons/link.png"));
    linkAct->setText("连接到SQLite");
    linkAct->setStatusTip("连接到SQLite数据库");
    settingMenu->addAction(linkAct);

    //断开连接
    disconnectAct = new QAction(this);
    disconnectAct->setIcon(QIcon(":/pics/icons/disconnect.png"));
    disconnectAct->setText("断开连接");
    disconnectAct->setStatusTip("断开当前的Sqlite连接");
    settingMenu->addAction(disconnectAct);
    //初始的时候不可点击
    disconnectAct->setEnabled(false);

    //帮助
    helpMenu = new QMenu(this);
    helpMenu->setTitle("帮助");
    //helpMenu->setIcon(QIcon(":/pics/icons/help.png"));
    menu->addMenu(helpMenu);

    //使用文档
    docsAct = new QAction(this);
    docsAct->setIcon(QIcon(":/pics/icons/docs.png"));
    docsAct->setText("使用文档");
    docsAct->setStatusTip("查看使用说明");
    helpMenu->addAction(docsAct);

    //使用视频
    vedioAct = new QAction(this);
    vedioAct->setIcon(QIcon(":/pics/icons/vedio.png"));
    vedioAct->setText("使用视频");
    vedioAct->setStatusTip("查看使用案例视频");
    helpMenu->addAction(vedioAct);
    menu->addMenu(helpMenu);

    setMenuBar(menu);
    //
}

void MainWindow::clearWidgets()
{
    // 清理所有窗口
    if (scriptWidget) {
        delete scriptWidget;
        scriptWidget = nullptr;
    }
    if (findTableWidget) {
        delete findTableWidget;
        findTableWidget = nullptr;
    }
    // 清除中心部件
    setCentralWidget(nullptr);
}

