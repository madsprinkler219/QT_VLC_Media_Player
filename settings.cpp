#include "settings.h"
#include "ui_settings.h"
#include <QSettings>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    QSettings settings("KevinPC");
    this->ui->lineEdit->setText(settings.value("vlcLocation").toString());
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_pushButton_clicked()
{
    QSettings settings("KevinPC");
    settings.setValue("vlcLocation",this->ui->lineEdit->text());
    this->close();
}

void Settings::on_pushButton_2_clicked()
{
    this->close();
}
