#include "client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Client client;
    client.show();

    client.setFixedSize(client.size());
    return a.exec();
}
