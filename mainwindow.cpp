/*
 * 郑翔 202010 possword
 */

#include <QDesktopWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "build_passwd.c"
#include "db.c"
#include <stdlib.h>
#include <time.h>

int lastid = 0;
int myrand(void)
{
    srand((unsigned)time(NULL)); 
    return (rand() % poems_num + 1);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width()-this->width())/2,(desktop->height()-this->height())/2);

    if (access(POEMDBPATH, F_OK) == 0) {
        poem_db = db_init((char *)POEMDBPATH);
    } else {
        poem_db = db_init((char *)POEMDB);
    }
    if (poem_db) {
        get_poems_num(poem_db);
    }

    if (poems_num == 0) {
        ui->pushButton_recommend->setText(QString::fromUtf8("推荐一个"));
        ui->lineEdit_poeminput->setText(QString::fromUtf8("查询古诗词库失败"));
    } else {
        char poem[64] = {0};
        int id = myrand();

        if (get_poem(poem_db, id, poem) < 0) {
            ui->pushButton_recommend->setText(QString::fromUtf8("推荐一个"));
            ui->lineEdit_poeminput->setText(QString::fromUtf8("查询古诗词库失败"));
        } else {
            lastid = id;
            ui->pushButton_recommend->setText(QString::fromUtf8("再推荐一个"));
            ui->lineEdit_poeminput->setText(QString::fromUtf8(poem));
            poem2passwd(poem);
            ui->plainTextEdit->setPlainText(QString::fromUtf8(info));
            ui->lineEdit_passwdoutput->setText(passwd);
            ui->lineEdit_remarkoutput->setText(QString::fromUtf8(remark));
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_recommend_clicked()
{
    ui->pushButton_recommend->setText(QString::fromUtf8("再推荐一个"));
    ui->lineEdit_poeminput->setText("");
    ui->lineEdit_passwdoutput->setText("");
    ui->lineEdit_remarkoutput->setText("");
    ui->plainTextEdit->setPlainText("");

    char poem[64] = {0};
    int i = 0, id = myrand();

    for (i = 0; i < 10; i++) {
        if (id == lastid) {
            id = myrand();
        }
    }
    lastid = id;
    if (get_poem(poem_db, id, poem) < 0) {
        ui->lineEdit_poeminput->setText(QString::fromUtf8("查询古诗词库失败"));
    } else {
        ui->lineEdit_poeminput->setText(QString::fromUtf8(poem));
        poem2passwd(poem);
        ui->plainTextEdit->setPlainText(QString::fromUtf8(info));
        ui->lineEdit_passwdoutput->setText(passwd);
        ui->lineEdit_remarkoutput->setText(QString::fromUtf8(remark));
    }
}

void MainWindow::on_pushButton_favorite_clicked()
{
    ui->pushButton_recommend->setText(QString::fromUtf8("推荐一个"));
    ui->lineEdit_poeminput->setText("");
    ui->lineEdit_passwdoutput->setText("");
    ui->lineEdit_remarkoutput->setText("");
    ui->plainTextEdit->setPlainText("");

    ui->lineEdit_poeminput->setFocus();
}

void MainWindow::on_lineEdit_poeminput_returnPressed()
{
    char *poem = ui->lineEdit_poeminput->text().toUtf8().data();

    ui->plainTextEdit->setPlainText("");
    ui->lineEdit_passwdoutput->setText("");
    ui->lineEdit_remarkoutput->setText("");

    if (strlen(poem) < 15) {
        ui->lineEdit_poeminput->setText(QString::fromUtf8("重新输入至少5个汉字"));
    } else {
        poem2passwd(poem);
        ui->plainTextEdit->setPlainText(QString::fromUtf8(info));
        ui->lineEdit_passwdoutput->setText(passwd);
        ui->lineEdit_remarkoutput->setText(QString::fromUtf8(remark));
    }
}
