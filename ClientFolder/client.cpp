#include "client.h"
#include "./ui_client.h"
#include <ctype.h>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm>
#include <QFileDialog>


Client::Client(QWidget *parent): QMainWindow(parent), connection(2908, "127.0.0.1"), ui(new Ui::Client)
{
    ui->setupUi(this);
    ui->Logo->resize(600, 100);
    QPixmap picture("CastDoc.png");
    int width = ui->Logo->width();
    int height = ui->Logo->height();
    ui->Logo->setPixmap(picture.scaled(width, height, Qt::KeepAspectRatio));
    //ui->centralwidget->setStyleSheet("background-image: url(bg.jpg); background-color: green; background-position: center;");
    ui->centralwidget->setStyleSheet("background: lightgray;");
    this->setWindowTitle("CastDoc");
}

Client::~Client()
{
    delete ui;
}



void Client::on_Convert_clicked()
{
    ui->Convert->setDisabled(true);

    if (ui->Type1->text().isEmpty() || ui->Type2->text().isEmpty() || ui->FileName->text().isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please make sure all fields are filled!!");
        ui->Convert->setDisabled(false);
        return;
    }

    int sd = connection.GetSd();

    int w =  0;
    write(sd, &w, 4);

    std::string type1 = ui->Type1->text().toStdString();
    std::transform(type1.begin(), type1.end(), type1.begin(), ::tolower);

    std::string type2 = ui->Type2->text().toStdString();
    std::transform(type2.begin(), type2.end(), type2.begin(), ::tolower);

    int size = type1.length();
    ASSERT(write(sd, &size, 4) != -1, "Error at writing in socket!!");
    ASSERT(write(sd, type1.c_str(), type1.length()) != -1, "Error at writing in socket!!");

    size = type2.length();
    ASSERT(write(sd, &size, 4) != -1, "Error at writing in socket!!");
    ASSERT(write(sd, type2.c_str(), type2.length()) != -1, "Error at writing in socket!!");

    int r = 0;
    ASSERT(read(sd, &r, 4) != -1, "Error at reading from socket!!");

    char message[1000];
    sprintf(message, "Cant convert from %s to %s", type1.c_str(), type2.c_str());

    if(r == -1){
        QMessageBox::warning(this, "Type Error", message);
        ui->Convert->setDisabled(false);
        return;
    }

    std::string file_path = ui->FileName->text().toStdString();

    sprintf(message, "The file %s is not of type %s", file_path.c_str(), type1.c_str());
    if (file_path.rfind(type1) != file_path.length() - type1.length()){
        printf("%lu\n", file_path.rfind(type1));
        printf("%lu\n", file_path.length() - type1.length());
        r = -1;
        ASSERT(write(sd, &r, 4) != -1, "Error at writing in socket!!");
        QMessageBox::warning(this, "Type Error", message);
        ui->Convert->setDisabled(false);
        return;
    }
    else{
        r = 0;
        ASSERT(write(sd, &r, 4) != -1, "Error at writing in socket!!");
    }

    r = access(file_path.c_str(), F_OK);
    ASSERT(write(sd, &r, 4) != -1, "Error at writing in socket!!");

    sprintf(message,  "Cant open the file %s", file_path.c_str());

    if(r == -1){
        QMessageBox::warning(this, "File Error", message);
        ui->Convert->setDisabled(false);
        return;
    }

    char file_name[255];
    memset(file_name, 0, 255);
    std::filesystem::path path(file_path);
    strcpy(file_name, path.filename().c_str());

    std::string command = "cp '" + file_path + "' '" + file_name + "'";
    system(command.c_str());

    char file_hash[1000];
    memset(file_hash, 0, 1000);
    get_hash(file_name, file_hash);

    write(sd, file_hash, strlen(file_hash) + 1);

    int rez = 0;
    read(sd, &rez, 4);

    if(rez == -1)
        send_file(sd, file_name, file_hash);

    char file[255];
    receive_file(sd, file);

    path.replace_filename(file);

    command = "mv '" + std::string(file) + "' '" + std::string(path.c_str()) + "'";
    system(command.c_str());

    command = file_name;
    command = "rm '" + command + "'";
    system(command.c_str());

    sprintf(message, "The file %s has been successfully converted", file_path.c_str());
    QMessageBox::information(this, "Success", "The file has been successfully converted");

    ui->Convert->setDisabled(false);
}


void Client::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "CastDoc",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {
        int w = -1;
        write(connection.GetSd(), &w, 4);
        sleep(1);
        event->accept();
    } else {
        event->ignore();
    }
}


void Client::on_SelectFile_clicked()
{
    QString file_name;
    if(ui->Type1->text().toStdString().empty()){
        file_name = QFileDialog::getOpenFileName(this, tr("Open File"), "/home/fiodorov/", "*");
    }
    else{
        file_name = QFileDialog::getOpenFileName(this, tr("Open File"), "/home/fiodorov/", tr(("*." + ui->Type1->text().toStdString()).c_str()));
    }

    if(!file_name.toStdString().empty()){
        ui->FileName->setText(file_name);
        std::string file_extension = file_name.toStdString().substr(file_name.toStdString().find_last_of(".") + 1);
        ui->Type1->setText(QString::fromStdString(file_extension));
    }
}

