#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include "Connection.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();
    void closeEvent(QCloseEvent *event);

public:
    Connection connection;
    Ui::Client *ui;
private slots:
    void on_Convert_clicked();
    void on_SelectFile_clicked();
};
#endif // CLIENT_H
