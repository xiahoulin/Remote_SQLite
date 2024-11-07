#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialog>
#include <QJsonObject>
#include <QIcon>
#include <QAction>
#include "connectdialog.h"
#include "scriptwidget.h"
#include "findtablewidget.h"
#include "socketmanager.h"
#include <QTcpSocket>
#include <QFile>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showConnectDialog();
    void onSelfQueryAction();
    void onDisconnectAction();
    void onOpenScriptDialog();
    void onQueryTableAction();

private:
    Ui::MainWindow *ui;
    QMenuBar *menu;
    QMenu *funcMenu;
    QMenu *settingMenu;
    QMenu *helpMenu;
    QAction *execSript;
    QAction *saveSript;
    QAction *queryTable;
    QAction *selfQuery;
    QAction *linkAct;
    QAction *disconnectAct;
    QAction *docsAct;
    QAction *vedioAct;
    ScriptWidget *scriptWidget;
    FindTableWidget *findTableWidget;
    void showAllWidget();
    void clearWidgets();
};
#endif // MAINWINDOW_H
